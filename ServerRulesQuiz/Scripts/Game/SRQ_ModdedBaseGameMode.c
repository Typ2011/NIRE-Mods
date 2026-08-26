// SRQ_ModdedBaseGameMode.c - Auto-bootstrap SRQ service on game mode startup

modded class SCR_BaseGameMode
{
	override void OnGameStart()
	{
		super.OnGameStart();
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent.GetInstance().Bootstrap();
		Print("[SRQ] Base game mode startup hook initialized", LogLevel.NORMAL);
	}
	
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent.GetInstance().Bootstrap();
		Print("[SRQ] Player " + playerId + " connected - awaiting audit", LogLevel.NORMAL);
	}
	
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent manager = SRQ_QuizManagerComponent.GetInstance();
		manager.MarkPlayerAudited(iPlayerID);
		Print("[SRQ] Player " + iPlayerID + " audit complete - identity sync pending", LogLevel.NORMAL);
	}
	
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent.GetInstance().OnPlayerDisconnected(playerId);
	}
}
