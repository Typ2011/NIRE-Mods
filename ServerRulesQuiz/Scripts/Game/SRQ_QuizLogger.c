// SRQ_QuizLogger.c - Persistent quiz stats and server logging

class SRQ_QuizLogger
{
	protected static ref SRQ_QuizLogger s_Instance;
	
	protected string m_sStatsFilePath;
	protected bool m_bInitialized;
	protected int m_iTotalAttempts;
	protected int m_iTotalPasses;
	protected int m_iTotalFailures;
	protected int m_iTotalAdminResets;
	protected int m_iTotalRetryLimitDisconnects;
	
	void SRQ_QuizLogger()
	{
		m_sStatsFilePath = "$profile:ServerRulesQuiz/QuizStats.json";
	}
	
	static SRQ_QuizLogger GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new SRQ_QuizLogger();
			s_Instance.LoadState();
		}
		
		return s_Instance;
	}
	
	void LogAttempt(int playerId, bool passed, int correctAnswers, int totalQuestions, int attemptNumber)
	{
		LoadState();
		
		m_iTotalAttempts++;
		if (passed)
			m_iTotalPasses++;
		else
			m_iTotalFailures++;
		
		SaveState();
		
		string result = "FAILED";
		if (passed)
			result = "PASSED";
		
		Print(string.Format("[SRQ] Quiz attempt | player=%1 name=%2 result=%3 score=%4/%5 attempt=%6",
			playerId,
			GetPlayerName(playerId),
			result,
			correctAnswers,
			totalQuestions,
			attemptNumber), LogLevel.NORMAL);
	}
	
	void LogAdminReset(int adminPlayerId, int targetPlayerId)
	{
		LoadState();
		m_iTotalAdminResets++;
		SaveState();
		
		Print(string.Format("[SRQ] Admin reset | admin=%1 (%2) target=%3 (%4)",
			adminPlayerId,
			GetPlayerName(adminPlayerId),
			targetPlayerId,
			GetPlayerName(targetPlayerId)), LogLevel.NORMAL);
	}
	
	void LogRetryLimitDisconnect(int playerId, int maxAttempts)
	{
		LoadState();
		m_iTotalRetryLimitDisconnects++;
		SaveState();
		
		Print(string.Format("[SRQ] Retry limit reached | player=%1 name=%2 maxAttempts=%3",
			playerId,
			GetPlayerName(playerId),
			maxAttempts), LogLevel.WARNING);
	}
	
	string BuildStatsSummary(int passedPlayers, int pendingPlayers, int connectedPlayers, SRQ_QuizConfig config)
	{
		LoadState();
		
		string repeatMode = "first-time only";
		if (config && config.showQuizOnEveryConnect)
			repeatMode = "every connect";
		
		int maxAttempts = 0;
		if (config)
			maxAttempts = config.maxAttemptsBeforeKick;
		
		string summary = "SRQ stats";
		summary += " | attempts=" + m_iTotalAttempts.ToString();
		summary += " passes=" + m_iTotalPasses.ToString();
		summary += " fails=" + m_iTotalFailures.ToString();
		summary += " resets=" + m_iTotalAdminResets.ToString();
		summary += " retry-disconnects=" + m_iTotalRetryLimitDisconnects.ToString();
		summary += " passed-players=" + passedPlayers.ToString();
		summary += " pending=" + pendingPlayers.ToString();
		summary += " connected=" + connectedPlayers.ToString();
		summary += " max-attempts=" + maxAttempts.ToString();
		summary += " mode=" + repeatMode;
		return summary;
	}
	
	protected string GetPlayerName(int playerId)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return "Unknown";
		
		string playerName = playerManager.GetPlayerName(playerId);
		if (playerName.IsEmpty())
			return "Unknown";
		
		return playerName;
	}
	
	protected void LoadState()
	{
		if (m_bInitialized)
			return;
		
		m_bInitialized = true;
		FileIO.MakeDirectory("$profile:ServerRulesQuiz");
		
		if (!FileIO.FileExists(m_sStatsFilePath))
			return;
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.LoadFromFile(m_sStatsFilePath))
		{
			Print("[SRQ] Failed to load quiz stats file", LogLevel.WARNING);
			return;
		}
		
		loadContext.ReadValue("totalAttempts", m_iTotalAttempts);
		loadContext.ReadValue("totalPasses", m_iTotalPasses);
		loadContext.ReadValue("totalFailures", m_iTotalFailures);
		loadContext.ReadValue("totalAdminResets", m_iTotalAdminResets);
		loadContext.ReadValue("totalRetryLimitDisconnects", m_iTotalRetryLimitDisconnects);
	}
	
	protected void SaveState()
	{
		FileIO.MakeDirectory("$profile:ServerRulesQuiz");
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("totalAttempts", m_iTotalAttempts);
		saveContext.WriteValue("totalPasses", m_iTotalPasses);
		saveContext.WriteValue("totalFailures", m_iTotalFailures);
		saveContext.WriteValue("totalAdminResets", m_iTotalAdminResets);
		saveContext.WriteValue("totalRetryLimitDisconnects", m_iTotalRetryLimitDisconnects);
		
		if (!saveContext.SaveToFile(m_sStatsFilePath))
		{
			Print("[SRQ] Failed to save quiz stats file", LogLevel.ERROR);
		}
	}
}
