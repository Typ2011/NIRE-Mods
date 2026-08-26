// SRQ_ModdedPlayerController.c - Modded player controller for quiz system

modded class SCR_PlayerController
{
	protected bool m_bSRQAuditReady;
	protected bool m_bSRQQuizPending;
	protected bool m_bSRQQuizShown;
	protected bool m_bSRQSessionCleared;
	protected bool m_bSRQStatusRequested;
	protected bool m_bSRQRestoreQuizAfterPause;
	protected ref SRQ_QuizDialog m_SRQDialog;
	protected ref SRQ_QuizConfig m_SRQConfig;
	protected int m_iSRQSavedCurrentQuestion;
	protected int m_iSRQSavedCorrectAnswers;
	protected int m_iSRQSavedTotalAnswered;
	protected bool m_bSRQSavedShowingRules;
	protected bool m_bSRQSavedQuizPassed;
	protected bool m_bSRQSavedResultSubmitted;
	
	// Helper: returns true when this instance is the local machine's player controller.
	// SCR_PlayerController.s_pLocalPlayerController is the authoritative static reference;
	// GetGame().GetPlayerController() is unreliable across dedicated-server / peer-client
	// boundaries and must not be used for this check.
	protected bool SRQ_IsLocalController()
	{
		return SCR_PlayerController.s_pLocalPlayerController == this;
	}
	
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
		
		if (!SRQ_IsLocalController())
			return;
		
		// Ignore de-spawn events; wait for a real spawn.
		if (!to)
			return;
		
		SRQ_CheckQuizRequired();
	}
	
	void SRQ_CheckQuizRequired()
	{
		if (!SRQ_IsLocalController())
			return;
		
		if (m_bSRQSessionCleared || m_bSRQQuizShown || m_bSRQStatusRequested || !m_bSRQAuditReady)
			return;
		
		if (!GetControlledEntity())
			return;
		
		// Only ask the server once per spawn/audit-ready cycle.
		m_bSRQStatusRequested = true;
		Rpc(SRQ_RpcRequestQuizStatus);
	}
	
	void SRQ_ShowQuiz()
	{
		if (!SRQ_IsLocalController())
			return;
		
		if (m_bSRQQuizShown || m_SRQDialog || !m_SRQConfig)
			return;
		
		m_bSRQQuizShown = true;
		m_SRQDialog = new SRQ_QuizDialog();
		m_SRQDialog.ShowDialog(this, m_SRQConfig);
		
		if (m_bSRQRestoreQuizAfterPause && m_SRQDialog)
		{
			m_SRQDialog.SRQ_RestoreState(
				m_iSRQSavedCurrentQuestion,
				m_iSRQSavedCorrectAnswers,
				m_iSRQSavedTotalAnswered,
				m_bSRQSavedShowingRules,
				m_bSRQSavedQuizPassed,
				m_bSRQSavedResultSubmitted
			);
			m_bSRQRestoreQuizAfterPause = false;
		}
	}
	
	void SRQ_OnQuizPassed(int correctAnswers = 0, int totalQuestions = 0)
	{
		m_bSRQQuizPending = false;
		m_bSRQSessionCleared = true;
		m_bSRQStatusRequested = false;
		Rpc(SRQ_RpcSubmitQuizResult, true, correctAnswers, totalQuestions);
		Print("[SRQ] Quiz passed, player can now spawn", LogLevel.NORMAL);
	}
	
	void SRQ_OnQuizFailed(int correctAnswers, int totalQuestions)
	{
		Rpc(SRQ_RpcSubmitQuizResult, false, correctAnswers, totalQuestions);
	}
	
	void SRQ_OnQuizClosed()
	{
		m_bSRQQuizShown = false;
		m_SRQDialog = null;
	}
	
	void SRQ_RequestAdminCommand(string command, string args)
	{
		Rpc(SRQ_RpcRequestAdminCommand, command, args);
	}
	
	void SRQ_CloseQuiz()
	{
		if (m_SRQDialog)
			m_SRQDialog.Close();
		
		SRQ_OnQuizClosed();
	}
	
	bool SRQ_IsQuizPending()
	{
		return m_bSRQQuizPending;
	}
	
	void SRQ_OpenPauseMenuFromQuiz()
	{
		if (!SRQ_IsLocalController())
			return;
		
		if (!m_SRQDialog)
		{
			ArmaReforgerScripted.OpenPauseMenu(true, false);
			return;
		}
		
		m_iSRQSavedCurrentQuestion = m_SRQDialog.SRQ_GetCurrentQuestionIndex();
		m_iSRQSavedCorrectAnswers = m_SRQDialog.SRQ_GetCorrectAnswers();
		m_iSRQSavedTotalAnswered = m_SRQDialog.SRQ_GetTotalAnswered();
		m_bSRQSavedShowingRules = m_SRQDialog.SRQ_IsShowingRules();
		m_bSRQSavedQuizPassed = m_SRQDialog.SRQ_HasPassedQuiz();
		m_bSRQSavedResultSubmitted = m_SRQDialog.SRQ_HasSubmittedResult();
		m_bSRQRestoreQuizAfterPause = true;
		
		if (PauseMenuUI.m_OnPauseMenuClosed)
			PauseMenuUI.m_OnPauseMenuClosed.Insert(SRQ_OnPauseMenuClosed);
		
		m_SRQDialog.SRQ_PrepareForPause();
		m_SRQDialog.Close();
		ArmaReforgerScripted.OpenPauseMenu(false, false);
	}
	
	void SRQ_OnPauseMenuClosed()
	{
		if (PauseMenuUI.m_OnPauseMenuClosed)
			PauseMenuUI.m_OnPauseMenuClosed.Remove(SRQ_OnPauseMenuClosed);
		
		if (!SRQ_IsLocalController() || !m_bSRQRestoreQuizAfterPause || !m_bSRQQuizPending)
			return;
		
		SRQ_ShowQuiz();
	}
	
	// ---- Public dispatch wrappers (called from outside this class, e.g. SRQ_QuizManagerComponent) ----
	// These exist so external code doesn't need to call Rpc() with a foreign function reference.
	
	void SRQ_SendAuditReady()
	{
		Rpc(SRQ_RpcAuditReady);
	}
	
	void SRQ_SendRetryLimitReached(int maxAttempts)
	{
		Rpc(SRQ_RpcRetryLimitReached, maxAttempts);
	}
	
	void SRQ_SendQuizResetByAdmin(string configJson)
	{
		Rpc(SRQ_RpcQuizResetByAdmin, configJson);
	}
	
	// ---- Server RPCs (dispatched from client via Rpc(), body runs on server only) ----
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void SRQ_RpcRequestQuizStatus()
	{
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent manager = SRQ_QuizManagerComponent.GetInstance();
		if (!manager)
			return;
		
		int playerId = GetPlayerId();
		if (!manager.IsPlayerAudited(playerId) || !manager.IsPlayerIdentityReady(playerId))
		{
			Rpc(SRQ_RpcQuizStatusPending);
			return;
		}
		
		manager.ResetPlayerAttemptCount(playerId);
		bool needsQuiz = manager.DoesPlayerNeedQuiz(playerId);
		string configJson = "";
		if (needsQuiz)
			configJson = manager.GetConfigJson();
		
		Rpc(SRQ_RpcReceiveQuizStatus, needsQuiz, configJson);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void SRQ_RpcSubmitQuizResult(bool passed, int correctAnswers, int totalQuestions)
	{
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent manager = SRQ_QuizManagerComponent.GetInstance();
		if (!manager)
			return;
		
		manager.OnPlayerCompletedQuiz(GetPlayerId(), passed, correctAnswers, totalQuestions);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void SRQ_RpcRequestAdminCommand(string command, string args)
	{
		if (!Replication.IsServer())
			return;
		
		SRQ_QuizManagerComponent manager = SRQ_QuizManagerComponent.GetInstance();
		if (!manager)
			return;
		
		string response = manager.HandleAdminCommand(GetPlayerId(), command, args);
		Rpc(SRQ_RpcAdminCommandResult, response);
	}
	
	// ---- Owner RPCs (dispatched from server via Rpc(), body runs on owning client only) ----
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcReceiveQuizStatus(bool needsQuiz, string configJson)
	{
		if (!SRQ_IsLocalController())
			return;
		
		m_bSRQStatusRequested = false;
		m_bSRQQuizPending = needsQuiz;
		if (!needsQuiz)
		{
			m_bSRQSessionCleared = true;
			return;
		}
		
		m_bSRQSessionCleared = false;
		m_SRQConfig = SRQ_QuizConfig.LoadFromString(configJson);
		if (!m_SRQConfig)
		{
			Print("[SRQ] Failed to deserialize quiz config from server", LogLevel.ERROR);
			return;
		}
		
		SRQ_ShowQuiz();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcQuizStatusPending()
	{
		if (!SRQ_IsLocalController())
			return;
		
		// The server now retries identity readiness and sends AuditReady only when the
		// player can meaningfully request quiz status, so there is no client poll loop.
		m_bSRQStatusRequested = false;
		m_bSRQSessionCleared = false;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcAuditReady()
	{
		if (!SRQ_IsLocalController())
			return;
		
		m_bSRQAuditReady = true;
		m_bSRQSessionCleared = false;
		m_bSRQStatusRequested = false;
		SRQ_CheckQuizRequired();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcAdminCommandResult(string message)
	{
		if (!SRQ_IsLocalController())
			return;
		
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		if (chatPanelManager)
			chatPanelManager.ShowHelpMessage(message);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcRetryLimitReached(int maxAttempts)
	{
		if (!SRQ_IsLocalController())
			return;
		
		m_bSRQQuizPending = false;
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		if (chatPanelManager)
			chatPanelManager.ShowHelpMessage(string.Format("You reached the SRQ retry limit (%1).", maxAttempts));
		
		SRQ_CloseQuiz();
		DisconnectFromGame();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void SRQ_RpcQuizResetByAdmin(string configJson)
	{
		if (!SRQ_IsLocalController())
			return;
		
		m_bSRQAuditReady = true;
		m_bSRQQuizPending = true;
		m_bSRQQuizShown = false;
		m_bSRQSessionCleared = false;
		m_bSRQStatusRequested = false;
		m_SRQConfig = SRQ_QuizConfig.LoadFromString(configJson);
		
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		if (chatPanelManager)
			chatPanelManager.ShowHelpMessage("Your Server Rules Quiz clearance was reset by an admin.");
	}
}
