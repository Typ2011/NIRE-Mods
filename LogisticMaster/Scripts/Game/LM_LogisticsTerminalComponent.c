[ComponentEditorProps(category: "LogisticMaster", description: "Central logistics inventory and request terminal.")]
class LM_LogisticsTerminalComponentClass : ScriptComponentClass
{
}

class LM_LogisticsTerminalComponent : ScriptComponent
{
	protected static LM_LogisticsTerminalComponent s_Instance;

	[Attribute("", UIWidgets.EditBox, "Optional faction key. Empty accepts the interacting player's faction.")]
	protected string m_sFactionKey;

	[Attribute("50", UIWidgets.EditBox, "Radius used to discover nearby supply containers.", "1 500 1")]
	protected float m_fSupplyScanRadius;

	protected ref array<ref LM_LogisticsRequest> m_aRequests = {};
	protected ref set<int> m_AuthorizedPlayerIds = new set<int>();
	protected ref set<SCR_ResourceConsumer> m_SupplyConsumers = new set<SCR_ResourceConsumer>();
	protected ref set<InventoryStorageManagerComponent> m_StockInventories = new set<InventoryStorageManagerComponent>();
	protected int m_iNextRequestId = 1;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
	}

	static LM_LogisticsTerminalComponent GetInstance()
	{
		return s_Instance;
	}

	RplId GetTerminalRplId()
	{
		return Replication.FindItemId(GetOwner());
	}

	float GetSupplies()
	{
		m_SupplyConsumers.Clear();
		GetOwner().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fSupplyScanRadius, CollectSupplyEntity);

		float supplies;
		foreach (SCR_ResourceConsumer consumer : m_SupplyConsumers)
			supplies += consumer.GetAggregatedResourceValue();

		return supplies;
	}

	protected bool CollectSupplyEntity(IEntity entity)
	{
		SCR_ResourceComponent resources = SCR_ResourceComponent.FindResourceComponent(entity, true);
		if (!resources)
			return true;

		foreach (SCR_ResourceConsumer consumer : resources.GetConsumers())
			m_SupplyConsumers.Insert(consumer);

		return true;
	}

	void GetStockEntries(notnull array<ref LM_StockEntry> entries)
	{
		entries.Clear();
		RefreshStockInventories();

		// ponytail: O(n²) grouping is fine for depot-sized inventories; replace with a map above several hundred roots.
		foreach (InventoryStorageManagerComponent inventory : m_StockInventories)
		{
			array<IEntity> items = {};
			inventory.GetItems(items);
			foreach (IEntity item : items)
			{
				ResourceName prefab = SCR_ResourceNameUtils.GetPrefabName(item);
				if (prefab.IsEmpty())
					continue;

				LM_StockEntry existing;
				foreach (LM_StockEntry entry : entries)
				{
					if (entry.m_sItemPrefab == prefab)
					{
						existing = entry;
						break;
					}
				}

				if (existing)
					existing.m_iQuantity++;
				else
					entries.Insert(new LM_StockEntry(prefab, 1));
			}
		}
	}

	void GetStockCatalogEntries(notnull array<ref LM_StockEntry> entries)
	{
		entries.Clear();
		SCR_EntityCatalogManagerComponent catalog = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalog)
			return;

		array<ref LM_StockEntry> stock = {};
		GetStockEntries(stock);
		set<ResourceName> uniquePrefabs = new set<ResourceName>();
		array<SCR_ArsenalItem> items = {};
		catalog.GetArsenalItems(items);
		AddStockCatalogItems(items, uniquePrefabs, stock, entries);

		array<Faction> factions = {};
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		factionManager.GetFactionsList(factions);
		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;

			array<SCR_ArsenalItem> factionItems = {};
			if (catalog.GetFactionArsenalItems(factionItems, scrFaction))
				AddStockCatalogItems(factionItems, uniquePrefabs, stock, entries);
		}
	}

	protected void AddStockCatalogItems(notnull array<SCR_ArsenalItem> items, notnull set<ResourceName> uniquePrefabs, notnull array<ref LM_StockEntry> stock, notnull array<ref LM_StockEntry> entries)
	{
		foreach (SCR_ArsenalItem item : items)
		{
			ResourceName prefab = item.GetItemResourceName();
			if (prefab.IsEmpty() || uniquePrefabs.Contains(prefab))
				continue;

			uniquePrefabs.Insert(prefab);
			int quantity;
			foreach (LM_StockEntry stockEntry : stock)
			{
				if (stockEntry.m_sItemPrefab == prefab)
				{
					quantity = stockEntry.m_iQuantity;
					break;
				}
			}

			LM_StockEntry entry = new LM_StockEntry(prefab, quantity);
			entry.m_eArsenalItemType = item.GetItemType();
			entry.m_eArsenalItemMode = item.GetItemMode();
			UIInfo info = GetStockItemInfo(item);
			if (info && !info.GetName().IsEmpty())
				entry.m_sDisplayName = info.GetName();
			entries.Insert(entry);
		}
	}

	protected UIInfo GetStockItemInfo(notnull SCR_ArsenalItem item)
	{
		Resource resource = item.GetItemResource();
		if (!resource || !resource.IsValid())
			return null;

		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(resource, InventoryItemComponent);
		if (!componentSource)
			return null;

		SCR_ItemAttributeCollection attributes = SCR_ComponentHelper.GetInventoryItemInfo(componentSource);
		if (!attributes)
			return null;

		return attributes.GetUIInfo();
	}

	protected bool IsStockCatalogItem(ResourceName prefab)
	{
		array<ref LM_StockEntry> entries = {};
		GetStockCatalogEntries(entries);
		foreach (LM_StockEntry entry : entries)
		{
			if (entry.m_sItemPrefab == prefab)
				return true;
		}

		return false;
	}

	int CountItem(ResourceName prefab)
	{
		RefreshStockInventories();
		int count;
		foreach (InventoryStorageManagerComponent inventory : m_StockInventories)
		{
			array<IEntity> items = {};
			inventory.GetItems(items);
			foreach (IEntity item : items)
			{
				if (SCR_ResourceNameUtils.GetPrefabName(item) == prefab)
					count++;
			}
		}

		return count;
	}

	void EditStock(int playerId, ResourceName prefab, int quantity, bool add)
	{
		if (!Replication.IsServer() || !CanEditStock(playerId) || !IsStockCatalogItem(prefab) || quantity < 1 || quantity > 100)
			return;

		InventoryStorageManagerComponent inventory = GetInventory();
		if (!inventory)
			return;

		bool changed;
		if (add)
		{
			Resource itemResource = Resource.Load(prefab);
			if (!itemResource || !itemResource.IsValid())
				return;

			changed = inventory.TrySpawnPrefabToStorage(prefab, null, -1, EStoragePurpose.PURPOSE_ANY, null, quantity);
		}
		else
		{
			changed = RemoveStock(prefab, quantity);
		}

		if (changed)
			BroadcastStockChanged();
	}

	void SubmitRequest(int playerId, ResourceName prefab, int quantity)
	{
		if (quantity < 1 || quantity > 100 || !CanSubmit(playerId) || !IsStockCatalogItem(prefab) || CountItem(prefab) < quantity)
			return;

		string factionKey = GetPlayerFactionKey(playerId);
		LM_LogisticsRequest request = new LM_LogisticsRequest(m_iNextRequestId++, playerId, factionKey, prefab, quantity);
		m_aRequests.Insert(request);
		BroadcastRequest(request);
	}

	void ManageRequest(int playerId, int requestId, bool approve)
	{
		if (!CanManage(playerId))
			return;

		LM_LogisticsRequest request = FindRequest(requestId);
		if (!request || request.m_eStatus != LM_ELogisticsRequestStatus.OPEN || (!IsGameMaster(playerId) && request.m_sFactionKey != GetPlayerFactionKey(playerId)))
			return;

		request.m_iHandlerPlayerId = playerId;
		if (!approve)
		{
			request.m_eStatus = LM_ELogisticsRequestStatus.REJECTED;
			BroadcastRequest(request);
			return;
		}

		if (!RemoveStock(request.m_sItemPrefab, request.m_iQuantity))
			return;

		request.m_eStatus = LM_ELogisticsRequestStatus.READY;
		BroadcastRequest(request);
	}

	void CompleteRequest(int playerId, int requestId)
	{
		if (!CanManage(playerId))
			return;

		LM_LogisticsRequest request = FindRequest(requestId);
		if (!request || request.m_eStatus != LM_ELogisticsRequestStatus.READY || (!IsGameMaster(playerId) && request.m_sFactionKey != GetPlayerFactionKey(playerId)))
			return;

		request.m_iHandlerPlayerId = playerId;
		request.m_eStatus = LM_ELogisticsRequestStatus.COMPLETED;
		BroadcastRequest(request);
	}

	void SendSnapshot(SCR_PlayerController target, int playerId)
	{
		if (!target || !IsAuthorizedPlayer(playerId))
			return;

		string factionKey = GetPlayerFactionKey(playerId);
		foreach (LM_LogisticsRequest request : m_aRequests)
		{
			if (IsGameMaster(playerId) || request.m_sFactionKey == factionKey)
				target.LM_SendRequest(request);
		}

		target.LM_SendSnapshotDone();
	}

	void SendAuthorizationSnapshot(SCR_PlayerController target, int playerId)
	{
		if (!target || !CanManagePermissions(playerId))
			return;

		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int targetPlayerId : playerIds)
			target.LM_SendAuthorizationEntry(targetPlayerId, IsAuthorizedPlayer(targetPlayerId));

		target.LM_SendAuthorizationSnapshotDone();
	}

	void SetPlayerAuthorization(int playerId, int targetPlayerId, bool authorized)
	{
		if (!Replication.IsServer() || !CanManagePermissions(playerId))
			return;

		PlayerManager players = GetGame().GetPlayerManager();
		SCR_PlayerController target = SCR_PlayerController.Cast(players.GetPlayerController(targetPlayerId));
		if (!target)
			return;

		if (IsGameMaster(targetPlayerId))
			authorized = true;

		if (authorized)
			m_AuthorizedPlayerIds.Insert(targetPlayerId);
		else
			m_AuthorizedPlayerIds.Remove(targetPlayerId);

		target.LM_SendAuthorizationChanged(authorized);
		BroadcastAuthorizationEntry(targetPlayerId, authorized);
	}

	void RemovePlayerAuthorization(int playerId)
	{
		if (!Replication.IsServer() || !m_AuthorizedPlayerIds.Contains(playerId))
			return;

		m_AuthorizedPlayerIds.Remove(playerId);
		BroadcastAuthorizationEntry(playerId, false);
	}

	protected void RefreshStockInventories()
	{
		m_StockInventories.Clear();
		InventoryStorageManagerComponent inventory = GetInventory();
		if (inventory)
			m_StockInventories.Insert(inventory);

		GetOwner().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fSupplyScanRadius, CollectInventoryBoxEntity);
	}

	protected bool CollectInventoryBoxEntity(IEntity entity)
	{
		if (!entity.FindComponent(IBX_GMInventoryEditorComponent))
			return true;

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (inventory)
			m_StockInventories.Insert(inventory);

		return true;
	}

	protected InventoryStorageManagerComponent GetInventory()
	{
		return InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
	}

	protected LM_LogisticsRequest FindRequest(int requestId)
	{
		foreach (LM_LogisticsRequest request : m_aRequests)
		{
			if (request.m_iId == requestId)
				return request;
		}

		return null;
	}

	protected bool RemoveStock(ResourceName prefab, int quantity)
	{
		RefreshStockInventories();
		array<IEntity> matches = {};
		array<InventoryStorageManagerComponent> inventories = {};
		foreach (InventoryStorageManagerComponent inventory : m_StockInventories)
		{
			array<IEntity> items = {};
			inventory.GetItems(items);
			foreach (IEntity item : items)
			{
				if (SCR_ResourceNameUtils.GetPrefabName(item) != prefab)
					continue;

				matches.Insert(item);
				inventories.Insert(inventory);
			}
		}

		if (matches.Count() < quantity)
			return false;

		// Server executes requests sequentially, preventing double withdrawal.
		for (int i; i < quantity; i++)
		{
			if (!inventories[i].TryDeleteItem(matches[i]))
				return false;
		}

		return true;
	}

	bool CanAccess(int playerId)
	{
		return CanManagePermissions(playerId) || IsAuthorizedPlayer(playerId);
	}

	bool CanManage(int playerId)
	{
		return IsAuthorizedPlayer(playerId);
	}

	bool CanEditStock(int playerId)
	{
		return IsAuthorizedPlayer(playerId);
	}

	bool CanManagePermissions(int playerId)
	{
		return IsGameMaster(playerId);
	}

	protected bool CanSubmit(int playerId)
	{
		return IsAuthorizedPlayer(playerId);
	}

	protected bool IsAuthorizedPlayer(int playerId)
	{
		if (IsGameMaster(playerId))
			return true;

		if (Replication.IsServer())
			return m_AuthorizedPlayerIds.Contains(playerId);

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return controller && controller.LM_IsAuthorized();
	}

	protected bool IsGameMaster(int playerId)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		return core && core.GetEditorManager(playerId);
	}

	protected bool IsSameFaction(int playerId)
	{
		string playerFaction = GetPlayerFactionKey(playerId);
		return !playerFaction.IsEmpty() && (m_sFactionKey.IsEmpty() || m_sFactionKey == playerFaction);
	}

	protected string GetPlayerFactionKey(int playerId)
	{
		IEntity player = GetPlayerEntity(playerId);
		if (!player)
			return "";

		FactionAffiliationComponent affiliation = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		if (!affiliation)
			return "";

		return affiliation.GetAffiliatedFactionKey();
	}

	protected IEntity GetPlayerEntity(int playerId)
	{
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return null;

		return controller.GetControlledEntity();
	}

	protected void BroadcastStockChanged()
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			if (!IsAuthorizedPlayer(playerId))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.LM_SendStockChanged();
		}
	}

	protected void BroadcastAuthorizationEntry(int targetPlayerId, bool authorized)
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			if (!CanManagePermissions(playerId))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.LM_SendAuthorizationEntry(targetPlayerId, authorized);
		}
	}

	protected void BroadcastRequest(LM_LogisticsRequest request)
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			if (!IsAuthorizedPlayer(playerId) || (!IsGameMaster(playerId) && GetPlayerFactionKey(playerId) != request.m_sFactionKey))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.LM_SendRequest(request);
		}
	}
}
