class RAMI_AvailableActionContext : SCR_AvailableActionContext
{
	void RAMI_AvailableActionContext()
	{
		m_sAction = "RAMI_ToggleMedicalMenu";
		m_sName = "#RAMI-OPEN_AMI";
		m_iTimeToShow = 0;
		m_iTimeForHide = 0;
	}

	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data, float timeSlice)
	{
		if (!RAMI_MedicalMenuUI.CanOpenMedicalMenu())
			return false;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager || inputManager.GetActionValue("RAMI_MedicalMenuModifier") < 0.5)
			return false;

		MenuManager menuManager = GetGame().GetMenuManager();
		return !menuManager || !menuManager.FindMenuByPreset(ChimeraMenuPreset.RAMI_MedicalMenu);
	}
}
