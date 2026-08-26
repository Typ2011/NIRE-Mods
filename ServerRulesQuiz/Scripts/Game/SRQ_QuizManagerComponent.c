// SRQ_QuizManagerComponent.c - Global quiz service with automatic bootstrap

class SRQ_QuizManagerComponent
{
	protected const string PROFILE_CONFIG_DIR = "$profile:ServerRulesQuiz";
	protected const string PROFILE_CONFIG_PATH = "$profile:ServerRulesQuiz/ServerRulesConfig.json";
	protected const int IDENTITY_RETRY_DELAY_MS = 500;
	
	protected static ref SRQ_QuizManagerComponent s_Instance;
	
	protected string m_sConfigPath;
	protected ref SRQ_QuizConfig m_Config;
	protected ref map<int, int> m_PlayerAttemptCounts;
	protected ref set<int> m_AuditedPlayers;
	protected ref set<int> m_PendingIdentityPlayers;
	protected bool m_bLoaded;
	
	void SRQ_QuizManagerComponent()
	{
		m_sConfigPath = "Config/ServerRulesConfig.json";
		m_PlayerAttemptCounts = new map<int, int>();
		m_AuditedPlayers = new set<int>();
		m_PendingIdentityPlayers = new set<int>();
	}
	
	static SRQ_QuizManagerComponent GetInstance()
	{
		if (!s_Instance)
			s_Instance = new SRQ_QuizManagerComponent();
		
		return s_Instance;
	}
	
	void Bootstrap()
	{
		EnsureLoaded();
	}
	
	void MarkPlayerAudited(int playerId)
	{
		EnsureLoaded();
		if (!m_AuditedPlayers.Contains(playerId))
			m_AuditedPlayers.Insert(playerId);
		
		if (IsPlayerIdentityReady(playerId))
		{
			NotifyPlayerAuditReady(playerId);
			return;
		}
		
		QueueIdentityRetry(playerId);
	}
	
	bool IsPlayerAudited(int playerId)
	{
		return m_AuditedPlayers.Contains(playerId);
	}
	
	bool IsPlayerIdentityReady(int playerId)
	{
		return SRQ_PlayerStateManager.GetInstance().HasResolvedPlayerIdentity(playerId);
	}
	
	void QueueIdentityRetry(int playerId)
	{
		if (m_PendingIdentityPlayers.Contains(playerId))
			return;
		
		m_PendingIdentityPlayers.Insert(playerId);
		ArmaReforgerScripted game = ArmaReforgerScripted.Cast(GetGame());
		if (!game)
			return;
		
		game.GetCallqueue().CallLaterByName(this, "RetryPlayerIdentityReady", IDENTITY_RETRY_DELAY_MS, false, playerId);
	}
	
	void RetryPlayerIdentityReady(int playerId)
	{
		m_PendingIdentityPlayers.RemoveItem(playerId);
		if (!IsPlayerAudited(playerId))
			return;
		
		if (!IsPlayerIdentityReady(playerId))
		{
			QueueIdentityRetry(playerId);
			return;
		}
		
		Print("[SRQ] Player " + playerId + " identityId ready", LogLevel.NORMAL);
		NotifyPlayerAuditReady(playerId);
	}
	
	void OnPlayerDisconnected(int playerId)
	{
		m_PlayerAttemptCounts.Remove(playerId);
		m_AuditedPlayers.RemoveItem(playerId);
		m_PendingIdentityPlayers.RemoveItem(playerId);
	}
	
	SRQ_QuizConfig GetConfig()
	{
		EnsureLoaded();
		return m_Config;
	}
	
	bool DoesPlayerNeedQuiz(int playerId)
	{
		EnsureLoaded();
		if (!m_Config)
			return false;
		
		if (!IsPlayerAudited(playerId))
			return false;
		
		if (!IsPlayerIdentityReady(playerId))
			return false;
		
		if (m_Config.showQuizOnEveryConnect)
			return true;
		
		return !SRQ_PlayerStateManager.GetInstance().HasPlayerPassed(playerId);
	}
	
	void ResetPlayerAttemptCount(int playerId)
	{
		m_PlayerAttemptCounts.Remove(playerId);
	}
	
	void OnPlayerCompletedQuiz(int playerId, bool passed, int correctAnswers, int totalQuestions)
	{
		EnsureLoaded();
		
		int attemptNumber = GetAttemptCount(playerId) + 1;
		m_PlayerAttemptCounts.Set(playerId, attemptNumber);
		SRQ_QuizLogger.GetInstance().LogAttempt(playerId, passed, correctAnswers, totalQuestions, attemptNumber);
		
		if (passed)
		{
			SRQ_PlayerStateManager.GetInstance().SetPlayerPassed(playerId);
			m_PlayerAttemptCounts.Remove(playerId);
			Print("[SRQ] Player " + playerId + " passed the quiz!", LogLevel.NORMAL);
			return;
		}
		
		if (!m_Config || m_Config.maxAttemptsBeforeKick <= 0)
			return;
		
		if (attemptNumber < m_Config.maxAttemptsBeforeKick)
			return;
		
		SRQ_QuizLogger.GetInstance().LogRetryLimitDisconnect(playerId, m_Config.maxAttemptsBeforeKick);
		SCR_PlayerController controller = GetQuizController(playerId);
		if (controller)
			controller.SRQ_SendRetryLimitReached(m_Config.maxAttemptsBeforeKick);
	}
	
	string HandleAdminCommand(int playerId, string command, string args)
	{
		EnsureLoaded();
		
		if (!IsPlayerAdmin(playerId))
			return "SRQ admin command denied: you are not on the server admin list.";
		
		if (command == "srq" || command == "srqhelp")
			return "SRQ commands: /srqstats, /srqreset <playerId|exact name>";
		
		if (command == "srqstats")
			return BuildStatsSummary();
		
		if (command == "srqreset")
			return HandleResetCommand(playerId, args);
		
		return "Unknown SRQ command. Use /srqhelp.";
	}
	
	string GetConfigJson()
	{
		EnsureLoaded();
		if (!m_Config)
			return "";
		
		return m_Config.ExportToJson();
	}
	
	protected void EnsureLoaded()
	{
		if (m_bLoaded)
			return;
		
		m_bLoaded = true;
		string configPath = EnsureRuntimeConfig();
		m_Config = SRQ_QuizConfig.LoadFromFile(configPath);
		
		if (!m_Config)
			Print("[SRQ] WARNING: Could not load quiz config, quiz system disabled!", LogLevel.ERROR);
		else
			Print("[SRQ] Quiz service initialized with " + m_Config.GetQuestionCount() + " questions from " + configPath, LogLevel.NORMAL);
	}
	
	protected string EnsureRuntimeConfig()
	{
		FileIO.MakeDirectory(PROFILE_CONFIG_DIR);
		
		if (FileIO.FileExists(PROFILE_CONFIG_PATH))
			return PROFILE_CONFIG_PATH;
		
		bool copiedDefault = false;
		if (!m_sConfigPath.IsEmpty())
			copiedDefault = FileIO.CopyFile(m_sConfigPath, PROFILE_CONFIG_PATH);
		
		if (copiedDefault)
		{
			Print("[SRQ] Created runtime config from packaged default: " + PROFILE_CONFIG_PATH, LogLevel.NORMAL);
			return PROFILE_CONFIG_PATH;
		}
		
		SRQ_QuizConfig defaultConfig = SRQ_QuizConfig.CreateDefault();
		if (defaultConfig.SaveToFile(PROFILE_CONFIG_PATH))
			Print("[SRQ] Created runtime config from built-in defaults: " + PROFILE_CONFIG_PATH, LogLevel.NORMAL);
		else
			Print("[SRQ] Failed to create runtime config at: " + PROFILE_CONFIG_PATH, LogLevel.ERROR);
		
		return PROFILE_CONFIG_PATH;
	}
	
	protected int GetAttemptCount(int playerId)
	{
		if (!m_PlayerAttemptCounts.Contains(playerId))
			return 0;
		
		return m_PlayerAttemptCounts.Get(playerId);
	}
	
	protected bool IsPlayerAdmin(int playerId)
	{
		BackendApi api = GetGame().GetBackendApi();
		if (!api)
			return false;
		
		if (api.IsServerOwner(playerId))
			return true;
		
		return api.IsListedServerAdmin(playerId);
	}
	
	protected SCR_PlayerController GetQuizController(int playerId)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return null;
		
		return SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
	}
	
	protected void NotifyPlayerAuditReady(int playerId)
	{
		SCR_PlayerController controller = GetQuizController(playerId);
		if (controller)
			controller.SRQ_SendAuditReady();
	}
	
	protected string HandleResetCommand(int adminPlayerId, string args)
	{
		if (args.IsEmpty())
			return "Usage: /srqreset <playerId|exact name>";
		
		int targetPlayerId = ResolvePlayerId(args);
		if (targetPlayerId <= 0)
			return "SRQ reset failed: player not found. Use a connected player ID or exact name.";
		
		SRQ_PlayerStateManager.GetInstance().ResetPlayer(targetPlayerId);
		ResetPlayerAttemptCount(targetPlayerId);
		SRQ_QuizLogger.GetInstance().LogAdminReset(adminPlayerId, targetPlayerId);
		
		SCR_PlayerController targetController = GetQuizController(targetPlayerId);
		if (targetController)
			targetController.SRQ_SendQuizResetByAdmin(GetConfigJson());
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		string targetName = playerManager.GetPlayerName(targetPlayerId);
		return string.Format("SRQ reset complete for %1 (player %2).", targetName, targetPlayerId);
	}
	
	protected int ResolvePlayerId(string token)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return 0;
		
		int playerId = token.ToInt();
		if (playerId > 0 && playerManager.IsPlayerConnected(playerId))
			return playerId;
		
		array<int> players = new array<int>();
		playerManager.GetPlayers(players);
		foreach (int connectedPlayerId : players)
		{
			if (playerManager.GetPlayerName(connectedPlayerId) == token)
				return connectedPlayerId;
		}
		
		return 0;
	}
	
	protected string BuildStatsSummary()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int connectedPlayers = 0;
		int pendingPlayers = 0;
		if (playerManager)
		{
			connectedPlayers = playerManager.GetPlayerCount();
			array<int> players = new array<int>();
			playerManager.GetPlayers(players);
			foreach (int playerId : players)
			{
				if (DoesPlayerNeedQuiz(playerId))
					pendingPlayers++;
			}
		}
		
		return SRQ_QuizLogger.GetInstance().BuildStatsSummary(
			SRQ_PlayerStateManager.GetInstance().GetPassedPlayerCount(),
			pendingPlayers,
			connectedPlayers,
			m_Config);
	}
}
