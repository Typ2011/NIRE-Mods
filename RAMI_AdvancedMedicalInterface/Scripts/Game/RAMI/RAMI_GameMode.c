modded class SCR_BaseGameMode
{
	[RplProp()]
	protected bool m_bRAMI_AllowMenuWhileUnconscious;

	override void OnGameStart()
	{
		super.OnGameStart();

		if (Replication.IsServer())
		{
			ArmaReforgerScripted game = ArmaReforgerScripted.Cast(GetGame());
			if (game)
				RAMI_SetMenuAllowedWhileUnconscious(game.RAMI_IsMenuAllowedWhileUnconscious());
			if (!GetGame().GetMissionHeader())
				Print("AMI: No active mission header. Unconscious menu access stays disabled; launch the scenario through its mission header to apply its AMI setting.", LogLevel.WARNING);
		}

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

	bool RAMI_IsMenuAllowedWhileUnconscious()
	{
		return m_bRAMI_AllowMenuWhileUnconscious;
	}

	void RAMI_SetMenuAllowedWhileUnconscious(bool allow)
	{
		if (!Replication.IsServer())
			return;

		m_bRAMI_AllowMenuWhileUnconscious = allow;
		Replication.BumpMe();
	}
}
