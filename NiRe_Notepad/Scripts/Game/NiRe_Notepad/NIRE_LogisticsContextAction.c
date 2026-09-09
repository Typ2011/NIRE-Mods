class NIRE_LogisticsRoleContextAction : SCR_BaseContextAction
{
	protected static const ResourceName REQUESTER_ICON = "{6BBC30A02FBC9A29}UI/Textures/Editor/Attributes/Attribute_Unlocked.edds";
	protected static const ResourceName LOGISTICIAN_ICON = "{564794579B2DB679}UI/Textures/Editor/Attributes/Attribute_Locked.edds";
	protected static int s_iNIRE_SnapshotTargetPlayerId;
	protected ref SCR_UIInfo m_NIRE_Info;
	protected int m_iTargetPlayerId;

	protected NIRE_ELogisticsRole GetRole()
	{
		return NIRE_ELogisticsRole.NONE;
	}

	override bool IsEnabled()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return controller && !controller.NIRE_IsPlayerLogisticsPermanent(m_iTargetPlayerId);
	}

	override bool IsServer()
	{
		return false;
	}

	override SCR_UIInfo GetInfo()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		NIRE_ELogisticsRole currentRole;
		LocalizedString description;
		ResourceName icon;
		if (controller)
			currentRole = controller.NIRE_GetPlayerLogisticsRole(m_iTargetPlayerId);
		NIRE_ELogisticsRole role = GetRole();
		if (role == NIRE_ELogisticsRole.REQUESTER)
		{
			description = "#NIRE-Logistics_RequesterAccessDescription";
			icon = REQUESTER_ICON;
			if (currentRole == role)
				m_NIRE_Info = SCR_UIInfo.CreateInfo("#NIRE-Logistics_RequesterAccessRevoke", description, icon);
			else
				m_NIRE_Info = SCR_UIInfo.CreateInfo("#NIRE-Logistics_RequesterAccessGrant", description, icon);
		}
		else
		{
			description = "#NIRE-Logistics_LogisticianAccessDescription";
			icon = LOGISTICIAN_ICON;
			if (currentRole == role)
				m_NIRE_Info = SCR_UIInfo.CreateInfo("#NIRE-Logistics_LogisticianAccessRevoke", description, icon);
			else
				m_NIRE_Info = SCR_UIInfo.CreateInfo("#NIRE-Logistics_LogisticianAccessGrant", description, icon);
		}

		return m_NIRE_Info;
	}

	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (SCR_EditorManagerEntity.IsLimitedInstance() && !SCR_Global.IsAdmin())
			return false;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return false;
		m_iTargetPlayerId = FindPlayerId(hoveredEntity, selectedEntities);
		if (m_iTargetPlayerId > 0 && s_iNIRE_SnapshotTargetPlayerId != m_iTargetPlayerId)
		{
			s_iNIRE_SnapshotTargetPlayerId = m_iTargetPlayerId;
			controller.NIRE_RequestLogisticsAccessSnapshot();
		}
		return m_iTargetPlayerId > 0;
	}

	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		int playerId = FindPlayerId(hoveredEntity, selectedEntities);
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return playerId > 0 && controller && !controller.NIRE_IsPlayerLogisticsPermanent(playerId);
	}

	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		int playerId = FindPlayerId(hoveredEntity, selectedEntities);
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && playerId > 0)
		{
			NIRE_ELogisticsRole role = GetRole();
			if (controller.NIRE_GetPlayerLogisticsRole(playerId) == role)
				role = NIRE_ELogisticsRole.NONE;
			controller.NIRE_SetLogisticsRole(playerId, role);
		}
	}

	protected int FindPlayerId(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities)
	{
		int playerId;
		if (hoveredEntity)
		{
			IEntity owner = hoveredEntity.GetOwner();
			if (ChimeraCharacter.Cast(owner))
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(owner);
			if (playerId > 0)
				return playerId;
		}

		foreach (SCR_EditableEntityComponent selectedEntity : selectedEntities)
		{
			IEntity owner = selectedEntity.GetOwner();
			if (!ChimeraCharacter.Cast(owner))
				continue;

			playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(owner);
			if (playerId > 0)
				return playerId;
		}

		return 0;
	}
}

class NIRE_RequesterAccessContextAction : NIRE_LogisticsRoleContextAction
{
	protected override NIRE_ELogisticsRole GetRole()
	{
		return NIRE_ELogisticsRole.REQUESTER;
	}
}

class NIRE_LogisticianAccessContextAction : NIRE_LogisticsRoleContextAction
{
	protected override NIRE_ELogisticsRole GetRole()
	{
		return NIRE_ELogisticsRole.LOGISTICIAN;
	}
}

modded class SCR_ContextActionsEditorComponent
{
	protected ref NIRE_RequesterAccessContextAction m_NIRE_RequesterAccessAction;
	protected ref NIRE_LogisticianAccessContextAction m_NIRE_LogisticianAccessAction;

	override void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		if (!m_NIRE_RequesterAccessAction)
			m_NIRE_RequesterAccessAction = new NIRE_RequesterAccessContextAction();
		if (!m_NIRE_LogisticianAccessAction)
			m_NIRE_LogisticianAccessAction = new NIRE_LogisticianAccessContextAction();

		int actionIndex = actions.Count();
		actions.Insert(m_NIRE_RequesterAccessAction);
		actions.Insert(m_NIRE_LogisticianAccessAction);
		super.EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
		actions.Remove(actionIndex + 1);
		actions.Remove(actionIndex);
	}
}
