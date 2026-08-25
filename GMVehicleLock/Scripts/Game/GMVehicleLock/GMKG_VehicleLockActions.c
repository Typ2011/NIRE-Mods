//! Shared lookup and UI constants for the Game Master vehicle lock.
class GMKG_VehicleLock
{
	static const ResourceName ICON_LOCKED = "{564794579B2DB679}UI/Textures/Editor/Attributes/Attribute_Locked.edds";
	static const ResourceName ICON_UNLOCKED = "{6BBC30A02FBC9A29}UI/Textures/Editor/Attributes/Attribute_Unlocked.edds";

	static const ResourceName ATTRIBUTE_CATEGORY = "{7FA3F309CBFCDACF}Configs/Editor/AttributeCategories/Vehicle.conf";
	static const ResourceName ATTRIBUTE_LAYOUT = "{D0F3CE0C63A5AEBB}UI/layouts/Editor/Attributes/AttributePrefabs/AttributePrefab_Checkbox.layout";

	//------------------------------------------------------------------------------------------------
	//! Get the lock component of a vehicle. Every vanilla vehicle inherits one from Vehicle_Base.et.
	//! \param[in] item Editable entity component or plain entity
	//! \return Lock component, null when the item is not a vehicle
	static SCR_BaseLockComponent GetLockComponent(Managed item)
	{
		IEntity entity;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (editableEntity)
		{
			if (editableEntity.GetEntityType() != EEditableEntityType.VEHICLE)
				return null;

			entity = editableEntity.GetOwner();
		}
		else
		{
			entity = IEntity.Cast(item);
		}

		if (!entity)
			return null;

		return SCR_BaseLockComponent.Cast(entity.FindComponent(SCR_BaseLockComponent));
	}
}

//------------------------------------------------------------------------------------------------
//! Right click action on selected vehicles. Created in script, see GMKG_RegisterLockActions().
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class GMKG_VehicleLockContextAction : SCR_SelectedEntitiesContextAction
{
	protected bool m_bGMKG_TargetLocked;

	//------------------------------------------------------------------------------------------------
	// constructor
	void GMKG_VehicleLockContextAction(bool targetLocked)
	{
		m_bGMKG_TargetLocked = targetLocked;
		m_bEnabled = true;
		m_Effects = {};

		if (targetLocked)
			m_Info = SCR_UIInfo.CreateInfo("Lock Vehicle", "Players can no longer enter the vehicle.", GMKG_VehicleLock.ICON_LOCKED);
		else
			m_Info = SCR_UIInfo.CreateInfo("Unlock Vehicle", "Players can enter the vehicle again.", GMKG_VehicleLock.ICON_UNLOCKED);
	}

	//------------------------------------------------------------------------------------------------
	//! Attribute defaults are not applied to script created actions, so state it explicitly.
	override bool IsServer()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_BaseLockComponent lockComponent = GMKG_VehicleLock.GetLockComponent(selectedEntity);
		if (!lockComponent)
			return false;

		return lockComponent.GMKG_IsLocked() != m_bGMKG_TargetLocked;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		SCR_BaseLockComponent lockComponent = GMKG_VehicleLock.GetLockComponent(selectedEntity);
		if (lockComponent)
			lockComponent.GMKG_SetLocked(m_bGMKG_TargetLocked);
	}
}

//------------------------------------------------------------------------------------------------
//! SCR_EditorAttributeUIInfo created in script needs its description colour, the config default is
//! never applied and GetDescriptionIconColor() would dereference null.
class GMKG_VehicleLockAttributeUIInfo : SCR_EditorAttributeUIInfo
{
	// constructor
	void GMKG_VehicleLockAttributeUIInfo()
	{
		m_cDescriptionIconColor = new Color(1, 1, 1, 1);
	}
}

//------------------------------------------------------------------------------------------------
//! Checkbox in the Game Master "Edit" attribute window of a vehicle.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class GMKG_VehicleLockEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void GMKG_VehicleLockEditorAttribute()
	{
		m_aAttributeDynamicDescriptions = {};

		GMKG_VehicleLockAttributeUIInfo info = new GMKG_VehicleLockAttributeUIInfo();
		info.SetName("Vehicle Locked");
		info.SetDescription("Locked vehicles cannot be entered by players.");
		m_UIInfo = info;

		m_CategoryConfig = GMKG_VehicleLock.ATTRIBUTE_CATEGORY;
		m_Layout = GMKG_VehicleLock.ATTRIBUTE_LAYOUT;
	}

	//------------------------------------------------------------------------------------------------
	//! Read and write the value on the server, it owns the lock state.
	override bool IsServer()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_BaseLockComponent lockComponent = GMKG_VehicleLock.GetLockComponent(item);
		if (!lockComponent)
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(lockComponent.GMKG_IsLocked());
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_BaseLockComponent lockComponent = GMKG_VehicleLock.GetLockComponent(item);
		if (lockComponent)
			lockComponent.GMKG_SetLocked(var.GetBool());
	}
}

//------------------------------------------------------------------------------------------------
//! Registration. The vanilla action and attribute lists come from EditorModeEdit.et, which a mod can
//! only replace as a whole. Appending to the prefab data instead keeps every vanilla entry intact and
//! survives game updates. The RPC in SCR_BaseActionsEditorComponent addresses actions by index, so
//! both ends must append the same entries in the same order - which they do, the list is static.
modded class SCR_ContextActionsEditorComponentClass
{
	protected ref array<ref SCR_BaseEditorAction> m_aGMKG_LockActions;

	//------------------------------------------------------------------------------------------------
	//! The prefab data array does not own its entries, so keep a strong reference here.
	void GMKG_RegisterLockActions()
	{
		if (m_aGMKG_LockActions)
			return;

		m_aGMKG_LockActions = {};
		m_aGMKG_LockActions.Insert(new GMKG_VehicleLockContextAction(true));
		m_aGMKG_LockActions.Insert(new GMKG_VehicleLockContextAction(false));

		foreach (SCR_BaseEditorAction action : m_aGMKG_LockActions)
		{
			m_ActionsSorted.Insert(action);
		}
	}
}

modded class SCR_ContextActionsEditorComponent
{
	//------------------------------------------------------------------------------------------------
	override void EOnEditorInit()
	{
		GMKG_RegisterLockActions();
		super.EOnEditorInit();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorInitServer()
	{
		GMKG_RegisterLockActions();
		super.EOnEditorInitServer();
	}

	//------------------------------------------------------------------------------------------------
	protected void GMKG_RegisterLockActions()
	{
		SCR_ContextActionsEditorComponentClass prefabData = SCR_ContextActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			prefabData.GMKG_RegisterLockActions();
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_AttributesManagerEditorComponentClass
{
	protected ref SCR_BaseEditorAttribute m_GMKG_LockAttribute;

	//------------------------------------------------------------------------------------------------
	void GMKG_RegisterLockAttribute()
	{
		if (m_GMKG_LockAttribute)
			return;

		m_GMKG_LockAttribute = new GMKG_VehicleLockEditorAttribute();
		m_aAttributes.Insert(m_GMKG_LockAttribute);
	}
}

modded class SCR_AttributesManagerEditorComponent
{
	//------------------------------------------------------------------------------------------------
	//! Register before super, it calls Initialize() on every attribute it finds.
	override void EOnEditorInit()
	{
		GMKG_RegisterLockAttribute();
		super.EOnEditorInit();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorInitServer()
	{
		GMKG_RegisterLockAttribute();
		super.EOnEditorInitServer();
	}

	//------------------------------------------------------------------------------------------------
	protected void GMKG_RegisterLockAttribute()
	{
		SCR_AttributesManagerEditorComponentClass prefabData = SCR_AttributesManagerEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			prefabData.GMKG_RegisterLockAttribute();
	}
}
