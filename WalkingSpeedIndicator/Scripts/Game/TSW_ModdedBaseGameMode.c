modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();
		if (!Replication.IsServer())
			return;

		TSW_ConfigService.GetInstance().Bootstrap();
	}

	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		if (!Replication.IsServer())
			return;

		TSW_ConfigService.GetInstance().Bootstrap();
	}
}
