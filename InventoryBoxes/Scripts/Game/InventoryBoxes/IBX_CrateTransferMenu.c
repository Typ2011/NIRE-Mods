modded class SCR_InputButtonComponent
{
	override void OnButtonHold(float value)
	{
		if (m_sActionName == "IBX_InventoryCrateTransfer" && value > 0 && value < m_fDefaultClickTime)
			value = m_fDefaultClickTime;

		super.OnButtonHold(value);
	}
}

modded class SCR_InventoryMenuUI
{
	protected static const string IBX_CRATE_TRANSFER_ACTION = "IBX_InventoryCrateTransfer";
	protected static const string IBX_CRATE_TRANSFER_BUTTON = "ButtonIBXCrateTransfer";

	protected SCR_InventorySlotUI m_IBXPendingSlot;
	protected SCR_InventoryStorageBaseUI m_IBXPendingVehicleUI;
	protected IEntity m_IBXPendingItem;
	protected BaseInventoryStorageComponent m_IBXPendingSource;
	protected BaseInventoryStorageComponent m_IBXPendingTarget;
	protected bool m_IBXPendingUnload;
	protected bool m_IBXExecutingTransfer;

	override void NavigationBarUpdate()
	{
		super.NavigationBarUpdate();
		IBX_UpdateCrateTransferAction();
	}

	override void OnAction(SCR_InputButtonComponent comp, string action, SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1)
	{
		if (action == IBX_CRATE_TRANSFER_ACTION)
		{
			IBX_StartCrateTransfer();
			IBX_FinishCrateTransfer();
			return;
		}

		super.OnAction(comp, action, pParentStorage, traverseStorageIndex);
	}

	override void MoveItem(SCR_InventoryStorageBaseUI pStorageBaseUI = null)
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.MoveItem(pStorageBaseUI);
	}

	override protected void Action_MoveItemToStorage(SCR_InventoryStorageBaseUI toStorage = null)
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.Action_MoveItemToStorage(toStorage);
	}

	override protected void Action_MoveBetween()
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.Action_MoveBetween();
	}

	override protected void Action_Drop()
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.Action_Drop();
	}

	override protected void MoveBetweenFromVicinity()
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.MoveBetweenFromVicinity();
	}

	override protected void MoveBetweenToVicinity()
	{
		if (!m_IBXExecutingTransfer && IBX_IsSelectedCrate())
			return;

		super.MoveBetweenToVicinity();
	}

	protected void IBX_StartCrateTransfer()
	{
		if (m_IBXPendingItem || !GetCanInteract())
			return;

		SCR_InventorySlotUI slot = m_pFocusedSlotUI;
		InventoryItemComponent itemComponent;
		IEntity item;
		if (!IBX_GetCrateFromSlot(slot, itemComponent, item))
			return;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();
		BaseInventoryStorageComponent sourceStorage;
		if (parentSlot)
			sourceStorage = parentSlot.GetStorage();

		bool unload = IBX_IsVehicleStorage(sourceStorage);
		SCR_InventoryStorageBaseUI vehicleUI = IBX_FindVehicleStorageUI();
		if (unload)
			vehicleUI = slot.GetStorageUI();
		BaseInventoryStorageComponent targetStorage;
		if (vehicleUI)
			targetStorage = vehicleUI.GetCurrentNavigationStorage();

		if ((!unload && (parentSlot || !targetStorage)) || (unload && !sourceStorage))
			return;

		m_IBXPendingSlot = slot;
		m_IBXPendingVehicleUI = vehicleUI;
		m_IBXPendingItem = item;
		m_IBXPendingSource = sourceStorage;
		m_IBXPendingTarget = targetStorage;
		m_IBXPendingUnload = unload;
	}

	protected void IBX_FinishCrateTransfer()
	{
		if (IBX_IsPendingTransferValid())
		{
			m_IBXExecutingTransfer = true;
			if (m_IBXPendingUnload)
			{
				m_pSelectedSlotUI = m_IBXPendingSlot;
				MoveToVicinity(m_IBXPendingItem);
			}
			else
				m_InventoryManager.InsertItem(m_IBXPendingItem, m_IBXPendingTarget);
			m_IBXExecutingTransfer = false;
		}

		IBX_CancelCrateTransfer();
	}

	protected bool IBX_IsPendingTransferValid()
	{
		InventoryItemComponent itemComponent;
		IEntity liveItem;
		if (!IBX_GetCrateFromSlot(m_IBXPendingSlot, itemComponent, liveItem) || liveItem != m_IBXPendingItem || !GetCanInteract())
			return false;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();
		BaseInventoryStorageComponent currentSource;
		if (parentSlot)
			currentSource = parentSlot.GetStorage();

		if (m_IBXPendingUnload)
			return currentSource == m_IBXPendingSource && IBX_IsVehicleUIOpen(m_IBXPendingVehicleUI) && m_IBXPendingVehicleUI.GetCurrentNavigationStorage() == m_IBXPendingSource;

		return !parentSlot && IBX_IsVehicleUIOpen(m_IBXPendingVehicleUI) && m_IBXPendingVehicleUI.GetCurrentNavigationStorage() == m_IBXPendingTarget;
	}

	protected void IBX_CancelCrateTransfer()
	{
		m_IBXPendingSlot = null;
		m_IBXPendingVehicleUI = null;
		m_IBXPendingItem = null;
		m_IBXPendingSource = null;
		m_IBXPendingTarget = null;
		m_IBXPendingUnload = false;
		m_IBXExecutingTransfer = false;
	}

	protected void IBX_UpdateCrateTransferAction()
	{
		if (!m_pNavigationBar)
			return;

		m_pNavigationBar.SetButtonEnabled(IBX_CRATE_TRANSFER_BUTTON, false);
		InventoryItemComponent itemComponent;
		IEntity item;
		if (!IBX_GetCrateFromSlot(m_pFocusedSlotUI, itemComponent, item))
			return;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();
		BaseInventoryStorageComponent sourceStorage;
		if (parentSlot)
			sourceStorage = parentSlot.GetStorage();

		bool unload = IBX_IsVehicleStorage(sourceStorage);
		if (unload || (!parentSlot && IBX_FindVehicleStorageUI()))
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_USE, false);
			m_pNavigationBar.SetButtonEnabled(IBX_CRATE_TRANSFER_BUTTON, true);
			if (unload)
				m_pNavigationBar.SetButtonActionName(IBX_CRATE_TRANSFER_BUTTON, "UNLOAD CRATE");
			else
				m_pNavigationBar.SetButtonActionName(IBX_CRATE_TRANSFER_BUTTON, "LOAD CRATE");
		}
	}

	protected bool IBX_IsSelectedCrate()
	{
		InventoryItemComponent itemComponent;
		IEntity item;
		return IBX_GetCrateFromSlot(GetSelectedSlotUI(), itemComponent, item);
	}

	protected bool IBX_GetCrateFromSlot(SCR_InventorySlotUI slot, out InventoryItemComponent itemComponent, out IEntity item)
	{
		if (!slot)
			return false;

		itemComponent = slot.GetInventoryItemComponent();
		if (!itemComponent)
			return false;

		item = itemComponent.GetOwner();
		return item && item.FindComponent(IBX_GMInventoryEditorComponent);
	}

	protected SCR_InventoryStorageBaseUI IBX_FindVehicleStorageUI()
	{
		SCR_InventoryStorageBaseUI lootUI = GetLootStorage();
		if (lootUI && IBX_IsVehicleStorage(lootUI.GetCurrentNavigationStorage()))
			return lootUI;

		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (openedStorage && IBX_IsVehicleStorage(openedStorage.GetCurrentNavigationStorage()))
				return openedStorage;
		}

		return null;
	}

	protected bool IBX_IsVehicleUIOpen(SCR_InventoryStorageBaseUI vehicleUI)
	{
		if (!vehicleUI)
			return false;

		if (vehicleUI == GetLootStorage())
			return true;

		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (openedStorage == vehicleUI)
				return true;
		}

		return false;
	}

	protected bool IBX_IsVehicleStorage(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return false;

		IEntity owner = storage.GetOwner();
		return owner && owner.FindComponent(SCR_VehicleInventoryStorageManagerComponent);
	}
}
