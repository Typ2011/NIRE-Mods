modded class SCR_EditorManagerEntity
{
	void IBX_RequestInventoryMutation(RplId editableId, IBX_EInventoryMutation mutation, ResourceName prefab, int count)
	{
		Rpc(IBX_RpcAsk_InventoryMutation, editableId, mutation, prefab, count);
	}

	void IBX_RequestInventoryPreset(RplId editableId, int presetIndex)
	{
		Rpc(IBX_RpcAsk_InventoryPreset, editableId, presetIndex);
	}

	void IBX_RequestInventoryExport(RplId editableId)
	{
		Rpc(IBX_RpcAsk_InventoryExport, editableId);
	}

	void IBX_RequestInventoryCopy(RplId editableId)
	{
		Rpc(IBX_RpcAsk_InventoryCopy, editableId);
	}

	void IBX_RequestInventoryRefresh(RplId editableId)
	{
		Rpc(IBX_RpcAsk_InventoryRefresh, editableId);
	}

	void IBX_RequestInventoryPaste(RplId editableId, string items)
	{
		Rpc(IBX_RpcAsk_InventoryPaste, editableId, items);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryMutation(RplId editableId, IBX_EInventoryMutation mutation, ResourceName prefab, int count)
	{
		if (!IsOpened())
		{
			IBX_SendInventoryMutationResult("Inventory edit rejected: Game Master editor is not open.");
			return;
		}

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (!inventoryEditor)
		{
			IBX_SendInventoryMutationResult("Inventory edit failed: crate is unavailable.");
			return;
		}

		count = Math.ClampInt(count, 1, IBX_GMInventoryEditorComponent.MAX_MUTATION_QUANTITY);
		string result = "Inventory edit failed: unsupported operation.";
		switch (mutation)
		{
			case IBX_EInventoryMutation.ADD:
				if (!prefab.IsEmpty())
					result = inventoryEditor.AddServer(prefab, count);
				break;

			case IBX_EInventoryMutation.REMOVE:
				if (!prefab.IsEmpty())
					result = inventoryEditor.RemoveServer(prefab, count);
				break;

			case IBX_EInventoryMutation.CLEAR:
				result = inventoryEditor.ClearServer();
				break;
		}

		IBX_SendInventoryMutationResult(result, inventoryEditor);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryPreset(RplId editableId, int presetIndex)
	{
		if (!IsOpened())
		{
			IBX_SendInventoryMutationResult("Preset rejected: Game Master editor is not open.");
			return;
		}

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (!inventoryEditor)
		{
			IBX_SendInventoryMutationResult("Preset failed: crate is unavailable.");
			return;
		}

		IBX_SendInventoryMutationResult(inventoryEditor.ApplyPresetServer(presetIndex), inventoryEditor);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryExport(RplId editableId)
	{
		if (!IsOpened())
			return;

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (!inventoryEditor)
			return;

		array<ResourceName> prefabs = {};
		array<int> counts = {};
		inventoryEditor.GetInventorySnapshot(prefabs, counts);
		Rpc(IBX_RpcDo_InventoryExport, prefabs, counts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryCopy(RplId editableId)
	{
		if (!IsOpened())
			return;

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (!inventoryEditor)
			return;

		array<ResourceName> prefabs = {};
		array<int> counts = {};
		inventoryEditor.GetInventorySnapshot(prefabs, counts);
		Rpc(IBX_RpcDo_InventoryCopy, prefabs, counts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryRefresh(RplId editableId)
	{
		if (!IsOpened())
			return;

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (inventoryEditor)
			IBX_SendInventoryMutationResult("Inventory loaded.", inventoryEditor);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_InventoryPaste(RplId editableId, string items)
	{
		if (!IsOpened())
		{
			IBX_SendInventoryMutationResult("Paste rejected: Game Master editor is not open.");
			return;
		}

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_ResolveInventoryEditor(editableId);
		if (!inventoryEditor)
		{
			IBX_SendInventoryMutationResult("Paste failed: crate is unavailable.");
			return;
		}

		IBX_SendInventoryMutationResult(inventoryEditor.PasteInventoryServer(items), inventoryEditor);
	}

	protected IBX_GMInventoryEditorComponent IBX_ResolveInventoryEditor(RplId editableId)
	{
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(Replication.FindItem(editableId));
		if (!editable)
			return null;

		return IBX_GMInventoryEditorComponent.Cast(editable.GetOwner().FindComponent(IBX_GMInventoryEditorComponent));
	}

	protected void IBX_SendInventoryMutationResult(string message, IBX_GMInventoryEditorComponent inventoryEditor = null)
	{
		array<ResourceName> prefabs = {};
		array<int> counts = {};
		if (inventoryEditor)
			inventoryEditor.GetInventorySnapshot(prefabs, counts);
		Rpc(IBX_RpcDo_InventoryMutationResult, message, inventoryEditor != null, prefabs, counts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void IBX_RpcDo_InventoryMutationResult(string message, bool hasSnapshot, array<ResourceName> prefabs, array<int> counts)
	{
		IBX_GMInventoryEditorUI.ReportMutationResult(message, hasSnapshot, prefabs, counts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void IBX_RpcDo_InventoryExport(array<ResourceName> prefabs, array<int> counts)
	{
		IBX_GMInventoryEditorUI.ReportInventoryExport(prefabs, counts);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void IBX_RpcDo_InventoryCopy(array<ResourceName> prefabs, array<int> counts)
	{
		IBX_GMInventoryEditorUI.ReportInventoryCopy(prefabs, counts);
	}
}
