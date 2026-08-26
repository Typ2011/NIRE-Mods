// SRQ_PlayerStateManager.c - Persistent player state tracking

class SRQ_PlayerStateManager
{
	protected static ref SRQ_PlayerStateManager s_Instance;
	
	protected ref set<string> m_PassedPlayers;
	protected string m_sStateFilePath;
	protected bool m_bInitialized;
	
	void SRQ_PlayerStateManager()
	{
		m_PassedPlayers = new set<string>();
		m_sStateFilePath = "$profile:ServerRulesQuiz/PlayerState.json";
		m_bInitialized = false;
	}
	
	static SRQ_PlayerStateManager GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new SRQ_PlayerStateManager();
			s_Instance.LoadState();
		}
		return s_Instance;
	}
	
	string GetPlayerIdentity(int playerId)
	{
		UUID identityId = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		Print("ID " + identityId);
		if (identityId.IsNull() || identityId.IsEmpty())
			return "";
		
		return identityId;
	}
	
	bool HasResolvedPlayerIdentity(int playerId)
	{
		return !GetPlayerIdentity(playerId).IsEmpty();
	}
	
	bool HasPlayerPassed(int playerId)
	{
		string identity = GetPlayerIdentity(playerId);
		if (identity.IsEmpty())
		{
			Print("[SRQ] Could not get identityId for player " + playerId, LogLevel.WARNING);
			return false;
		}
		
		return m_PassedPlayers.Contains(identity);
	}
	
	void SetPlayerPassed(int playerId)
	{
		string identity = GetPlayerIdentity(playerId);
		if (identity.IsEmpty())
		{
			Print("[SRQ] Could not get identityId for player " + playerId, LogLevel.WARNING);
			return;
		}
		
		if (!m_PassedPlayers.Contains(identity))
		{
			m_PassedPlayers.Insert(identity);
			SaveState();
			Print("[SRQ] Player " + playerId + " (" + identity + ") marked as passed", LogLevel.NORMAL);
		}
	}
	
	void ResetPlayer(int playerId)
	{
		string identity = GetPlayerIdentity(playerId);
		if (identity.IsEmpty())
			return;
		
		ResetPlayerByIdentity(identity);
		Print("[SRQ] Player " + playerId + " quiz state reset", LogLevel.NORMAL);
	}
	
	void ResetPlayerByIdentity(string identity)
	{
		if (identity.IsEmpty())
			return;
		
		if (!m_PassedPlayers.Contains(identity))
			return;
		
		m_PassedPlayers.RemoveItem(identity);
		SaveState();
	}
	
	int GetPassedPlayerCount()
	{
		return m_PassedPlayers.Count();
	}
	
	protected void LoadState()
	{
		if (m_bInitialized)
			return;
		
		m_bInitialized = true;
		FileIO.MakeDirectory("$profile:ServerRulesQuiz");
		
		if (!FileIO.FileExists(m_sStateFilePath))
		{
			Print("[SRQ] No existing player state file, starting fresh", LogLevel.NORMAL);
			return;
		}
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.LoadFromFile(m_sStateFilePath))
		{
			Print("[SRQ] Failed to load player state file", LogLevel.WARNING);
			return;
		}
		
		array<string> passedList = new array<string>();
		if (loadContext.ReadValue("passedPlayers", passedList))
		{
			foreach (string identity : passedList)
			{
				m_PassedPlayers.Insert(identity);
			}
		}
		
		Print("[SRQ] Loaded " + m_PassedPlayers.Count() + " passed players from state file", LogLevel.NORMAL);
	}
	
	protected void SaveState()
	{
		FileIO.MakeDirectory("$profile:ServerRulesQuiz");
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		array<string> passedList = new array<string>();
		for (int i = 0; i < m_PassedPlayers.Count(); i++)
		{
			passedList.Insert(m_PassedPlayers.Get(i));
		}
		
		saveContext.WriteValue("passedPlayers", passedList);
		
		if (saveContext.SaveToFile(m_sStateFilePath))
		{
			Print("[SRQ] Saved player state (" + passedList.Count() + " players)", LogLevel.NORMAL);
		}
		else
		{
			Print("[SRQ] Failed to save player state!", LogLevel.ERROR);
		}
	}
}
