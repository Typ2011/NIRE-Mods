modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();

		if (Replication.IsServer() && !LM_LogisticsTerminalComponent.GetInstance())
		{
			Resource terminalResource = Resource.Load("{45064F588129BF48}Prefabs/Props/LM_LogisticsTerminal.et");
			if (terminalResource && terminalResource.IsValid())
				GetGame().SpawnEntityPrefab(terminalResource, GetGame().GetWorld());
		}

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.AddActionListener("LM_ToggleLogisticsMaster", EActionTrigger.UP, LM_OpenMenu);
	}

	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		LM_LogisticsTerminalComponent terminal = LM_LogisticsTerminalComponent.GetInstance();
		if (terminal)
			terminal.RemovePlayerAuthorization(playerId);
	}

	override void OnGameEnd()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("LM_ToggleLogisticsMaster", EActionTrigger.UP, LM_OpenMenu);

		LM_LogisticsTerminalUI.CloseIfOpen();
		super.OnGameEnd();
	}

	protected void LM_OpenMenu()
	{
		if (LM_LogisticsTerminalUI.CloseIfOpen())
			return;

		LM_LogisticsTerminalUI.Open(LM_LogisticsTerminalComponent.GetInstance());
	}
}
