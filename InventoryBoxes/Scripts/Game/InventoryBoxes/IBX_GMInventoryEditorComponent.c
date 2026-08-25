class IBX_GMInventoryEditorComponentClass : ScriptComponentClass
{
}

enum IBX_EInventoryMutation
{
	ADD,
	REMOVE,
	CLEAR
}

class IBX_GMInventoryEditorComponent : ScriptComponent
{
	static const int MAX_ITEM_TYPES = 100;
	static const int MAX_MUTATION_QUANTITY = 1000;
	static const int MAX_PRESET_ITEMS = 10000;
	static const int MAX_NAME_LENGTH = 32;

	// Custom crate name, authoritative on the server. Clients receive live changes through
	// RpcDo_SetName and catch up on join through RplLoad. Deliberately not [RplProp]: that has never
	// once fired on a proxy copy of a ScriptComponent in this addon (see PROJECT_CONTEXT.md).
	protected string m_sCrateName;

	// The prefab's own storage name, captured before anything overwrites it, so clearing a custom
	// name puts the original back.
	protected string m_sDefaultName;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		BaseInventoryStorageComponent storage = GetStorage();
		if (!storage)
			return;

		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(storage.GetAttributes());
		if (attributes)
			attributes.SetDraggable(false);

		UIInfo storageInfo = GetStorageUIInfo();
		if (storageInfo)
			m_sDefaultName = storageInfo.GetName();
	}

	static IBX_GMInventoryEditorComponent Get(IEntity owner)
	{
		if (!owner)
			return null;

		return IBX_GMInventoryEditorComponent.Cast(owner.FindComponent(IBX_GMInventoryEditorComponent));
	}

	void GetInventoryItems(out notnull array<IEntity> items)
	{
		BaseInventoryStorageComponent storage = GetStorage();
		if (storage)
			storage.GetAll(items, false);
	}

	void GetInventorySnapshot(out notnull array<ResourceName> prefabs, out notnull array<int> counts)
	{
		map<ResourceName, int> itemCounts = new map<ResourceName, int>();
		array<IEntity> items = {};
		GetInventoryItems(items);
		foreach (IEntity item : items)
		{
			EntityPrefabData prefabData = item.GetPrefabData();
			if (prefabData)
			{
				ResourceName prefab = prefabData.GetPrefabName();
				int count;
				itemCounts.Find(prefab, count);
				itemCounts.Set(prefab, count + 1);
			}
		}

		foreach (ResourceName prefab, int count : itemCounts)
		{
			prefabs.Insert(prefab);
			counts.Insert(count);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Crate naming. Its own client-to-server path rather than the SCR_EditorManagerEntity route the
	// inventory mutations use, because a plain player renaming a crate has no editor manager at all.
	//------------------------------------------------------------------------------------------------

	string GetCrateName()
	{
		return m_sCrateName;
	}

	//! Custom name if one is set, the prefab's own storage name otherwise.
	string GetDisplayName()
	{
		if (m_sCrateName.IsEmpty())
			return m_sDefaultName;

		return m_sCrateName;
	}

	//! Called from the Game Master editor and from the world rename action, both of which run on a
	//! client. An empty name clears the custom one.
	//!
	//! Deliberately not an Rpc() on this component. A client-to-server RPC declared here never
	//! arrives on a dedicated server - measured: applying a preset (server-side, no client leg)
	//! named the crate correctly while a Game Master rename through this path did nothing, and the
	//! server-to-client broadcast below demonstrably works. The player controller is the channel
	//! vanilla uses for every client request and is owned by the requesting client, Game Master or
	//! not.
	void RequestRename(string name)
	{
		if (IsAuthority())
		{
			RenameServer(name);
			return;
		}

		if (name.Length() > MAX_NAME_LENGTH)
			name = name.Substring(0, MAX_NAME_LENGTH);

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!controller || !rpl)
		{
			IBX_GMInventoryEditorUI.ReportMutationStatus("Rename failed: crate is not available for network editing.");
			return;
		}

		controller.IBX_RequestCrateRename(rpl.Id(), name);
	}

	void RenameServer(string name)
	{
		if (!IsAuthority())
			return;

		name = SanitizeName(name);
		if (name == m_sCrateName)
			return;

		// A broadcast RPC does not loop back to its own sender, so this machine applies it directly
		// as well - matters for a listen-server host.
		ApplyNameLocally(name);
		Rpc(RpcDo_SetName, name);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetName(string name)
	{
		ApplyNameLocally(name);
	}

	//! Server-side only. A leading '#' makes the engine treat the whole string as a localization key,
	//! so it is stripped rather than escaped.
	protected static string SanitizeName(string name)
	{
		name.Replace("#", "");
		name.Replace("\n", " ");
		name.Replace("\t", " ");
		name = name.Trim();
		if (name.Length() > MAX_NAME_LENGTH)
			name = name.Substring(0, MAX_NAME_LENGTH);

		return name;
	}

	//! Runs on every machine.
	//!
	//! Nothing here writes the storage's own ItemAttributeCollection UIInfo. Measured: that object
	//! is shared between every instance of a prefab, so naming one crate renamed every crate of that
	//! type and, worse, poisoned m_sDefaultName for the next one placed, which read its "default"
	//! back out of the already-renamed shared object. The inventory header is overridden per open
	//! storage in the modded SCR_InventoryStorageBaseUI below instead.
	//!
	//! SetInfoInstance is safe by contrast - it is the documented per-instance override, and the
	//! info object handed to it is a fresh one rather than the shared prefab info being mutated.
	protected void ApplyNameLocally(string name)
	{
		m_sCrateName = name;

		// Pushed rather than pulled after the mutation result: applying a preset renames the crate,
		// and the two travel as separate RPCs on separate replicated objects, so their arrival order
		// is not guaranteed. No-op when the editor is closed or open on another crate.
		IBX_GMInventoryEditorUI.ReportCrateName(this, name);

		string display = GetDisplayName();
		if (display.IsEmpty())
			return;

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editable)
			return;

		SCR_UIInfo info = editable.GetInfo();
		if (!info)
			return;

		editable.SetInfoInstance(SCR_UIInfo.CreateInfo(display, info.GetDescription(), info.GetIconPath(), info.GetIconSetName()));
	}

	//! Join-in-progress: a client that connects after the rename never sees the broadcast.
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_sCrateName);
		return true;
	}

	override bool RplLoad(ScriptBitReader reader)
	{
		// Always true: a false here fails the whole load, and an unreadable name is not worth
		// dropping the crate over.
		string name;
		if (reader.ReadString(name))
			ApplyNameLocally(name);

		return true;
	}

	//! Resolved fresh each call - a proxy's RplComponent may not be queryable at init, and a stale
	//! "is authority" default would make a client mutate its non-authoritative copy.
	protected bool IsAuthority()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return true;

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl)
			return true;

		return !rpl.IsProxy();
	}

	protected UIInfo GetStorageUIInfo()
	{
		BaseInventoryStorageComponent storage = GetStorage();
		if (!storage)
			return null;

		ItemAttributeCollection attributes = storage.GetAttributes();
		if (!attributes)
			return null;

		return attributes.GetUIInfo();
	}

	void RequestAdd(ResourceName prefab, int count)
	{
		if (prefab.IsEmpty())
			return;

		RequestMutation(IBX_EInventoryMutation.ADD, prefab, count);
	}

	void RequestRemove(ResourceName prefab, int count)
	{
		if (prefab.IsEmpty())
			return;

		RequestMutation(IBX_EInventoryMutation.REMOVE, prefab, count);
	}

	void RequestClear()
	{
		RequestMutation(IBX_EInventoryMutation.CLEAR, "", 1);
	}

	void RequestPreset(int presetIndex)
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryPreset(editableId, presetIndex);
	}

	void RequestExport()
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryExport(editableId);
	}

	void RequestCopy()
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryCopy(editableId);
	}

	void RequestRefresh()
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryRefresh(editableId);
	}

	void RequestPaste(string items)
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryPaste(editableId, items);
	}

	protected void RequestMutation(IBX_EInventoryMutation mutation, ResourceName prefab, int count)
	{
		RplId editableId;
		SCR_EditorManagerEntity editor;
		if (GetNetworkContext(editableId, editor))
			editor.IBX_RequestInventoryMutation(editableId, mutation, prefab, Math.ClampInt(count, 1, MAX_MUTATION_QUANTITY));
	}

	protected bool GetNetworkContext(out RplId editableId, out SCR_EditorManagerEntity editor)
	{
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editable || !editable.IsReplicated(editableId))
		{
			IBX_GMInventoryEditorUI.ReportMutationStatus("Crate is not available for network editing.");
			return false;
		}

		editor = SCR_EditorManagerEntity.GetInstance();
		if (!editor)
		{
			IBX_GMInventoryEditorUI.ReportMutationStatus("Game Master editor is unavailable.");
			return false;
		}

		return true;
	}

	string AddServer(ResourceName prefab, int count)
	{
		InventoryStorageManagerComponent manager = GetManager();
		BaseInventoryStorageComponent storage = GetStorage();
		Resource resource = Resource.Load(prefab);
		if (!manager || !storage || !resource || !resource.IsValid())
			return "Could not add items: crate storage or item is unavailable.";

		array<IEntity> items = {};
		storage.GetAll(items, false);
		set<ResourceName> itemTypes = new set<ResourceName>();
		foreach (IEntity item : items)
		{
			EntityPrefabData prefabData = item.GetPrefabData();
			if (prefabData)
				itemTypes.Insert(prefabData.GetPrefabName());
		}

		if (!itemTypes.Contains(prefab) && itemTypes.Count() >= MAX_ITEM_TYPES)
			return "Crate already contains 100 item types.";

		count = Math.ClampInt(count, 1, MAX_MUTATION_QUANTITY);
		int added;
		for (int i = 0; i < count; i++)
		{
			IEntity item = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld());
			if (!item)
				break;

			if (manager.TryInsertItemInStorage(item, storage))
			{
				added++;
				continue;
			}

			SCR_EntityHelper.DeleteEntityAndChildren(item);
			break;
		}

		return string.Format("Added %1 of %2 requested item(s).", added, count);
	}

	string RemoveServer(ResourceName prefab, int count)
	{
		InventoryStorageManagerComponent manager = GetManager();
		BaseInventoryStorageComponent storage = GetStorage();
		if (!manager || !storage)
			return "Could not remove items: crate storage is unavailable.";

		array<IEntity> items = {};
		storage.GetAll(items, false);
		int requested = count;
		int removed;
		foreach (IEntity item : items)
		{
			if (count <= 0)
				break;

			EntityPrefabData prefabData = item.GetPrefabData();
			if (!prefabData || prefabData.GetPrefabName() != prefab)
				continue;

			if (manager.TryDeleteItem(item))
			{
				count--;
				removed++;
			}
		}

		return string.Format("Removed %1 of %2 requested item(s).", removed, requested);
	}

	string ClearServer()
	{
		InventoryStorageManagerComponent manager = GetManager();
		BaseInventoryStorageComponent storage = GetStorage();
		if (!manager || !storage)
			return "Could not clear items: crate storage is unavailable.";

		array<IEntity> items = {};
		storage.GetAll(items, false);
		int removed;
		foreach (IEntity item : items)
		{
			if (manager.TryDeleteItem(item))
				removed++;
		}

		return string.Format("Cleared %1 of %2 item(s).", removed, items.Count());
	}

	string ApplyPresetServer(int presetIndex)
	{
		IBX_CratePresetConfig config = IBX_CratePresetConfig.Load();
		if (!config || !config.m_aPresets || presetIndex < 0 || presetIndex >= config.m_aPresets.Count())
			return "Preset is unavailable.";

		IBX_CratePreset preset = config.m_aPresets[presetIndex];
		if (!preset || !preset.Parse())
			return "Preset is invalid.";

		bool replaced;
		string result = ReplaceContentsServer(preset, "Applied preset " + preset.m_sName, replaced);

		// A preset names the crate it fills, so the label and the contents never drift apart. Only
		// once the contents actually landed - a failed apply leaves the existing name alone.
		if (replaced)
			RenameServer(preset.m_sName);

		return result;
	}

	string PasteInventoryServer(string items)
	{
		if (items.IsEmpty() || items.Length() > 32768)
			return "Clipboard inventory is empty or too large.";

		IBX_CratePreset preset = new IBX_CratePreset();
		preset.m_sName = "Clipboard";
		preset.m_sItems = items;
		if (!preset.Parse())
			return "Clipboard inventory is invalid.";

		// Deliberately does not rename: pasted inventory carries no name of its own.
		bool replaced;
		return ReplaceContentsServer(preset, "Pasted inventory", replaced);
	}

	protected string ReplaceContentsServer(notnull IBX_CratePreset preset, string successMessage, out bool replaced)
	{
		replaced = false;
		if (!GetManager() || !GetStorage())
			return "Could not replace inventory: crate storage is unavailable.";

		foreach (ResourceName prefab : preset.m_Prefabs)
		{
			Resource resource = Resource.Load(prefab);
			if (!resource || !resource.IsValid())
				return "Inventory data contains an unavailable item resource.";
		}

		ClearServer();
		array<IEntity> remaining = {};
		GetInventoryItems(remaining);
		if (!remaining.IsEmpty())
			return "Could not empty crate before applying preset.";

		for (int i = 0; i < preset.m_Prefabs.Count(); i++)
			AddServer(preset.m_Prefabs[i], preset.m_Counts[i]);

		array<ResourceName> prefabs = {};
		array<int> counts = {};
		GetInventorySnapshot(prefabs, counts);
		int added;
		foreach (int count : counts)
			added += count;

		replaced = true;
		return string.Format("%1: added %2 of %3 item(s).", successMessage, added, preset.m_TotalItems);
	}

	protected InventoryStorageManagerComponent GetManager()
	{
		return InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
	}

	protected BaseInventoryStorageComponent GetStorage()
	{
		return BaseInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_UniversalInventoryStorageComponent));
	}
}
