modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.AddActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.DOWN, RAMI_OpenMenu);

		GetGame().GetCallqueue().CallLater(RAMI_CaptureBodyZoneStates, 100, true);
	}

	override void OnGameEnd()
	{
		GetGame().GetCallqueue().Remove(RAMI_CaptureBodyZoneStates);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.DOWN, RAMI_OpenMenu);

		super.OnGameEnd();
	}

	protected void RAMI_CaptureBodyZoneStates()
	{
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (!characters)
			return;

		foreach (SCR_ChimeraCharacter character : characters)
			RAMI_MedicalMenuUI.CaptureBodyZoneState(character);
	}

	protected void RAMI_OpenMenu()
	{
		RAMI_MedicalMenuUI.OpenMedicalMenu();
	}
}
