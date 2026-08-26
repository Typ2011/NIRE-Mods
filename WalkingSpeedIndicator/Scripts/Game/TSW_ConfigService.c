class TSW_ConfigService
{
	protected const string PROFILE_CONFIG_DIR = "$profile:WalkingSpeedIndicator";
	protected const string PROFILE_CONFIG_PATH = "$profile:WalkingSpeedIndicator/WalkingSpeedIndicatorConfig.json";

	protected static ref TSW_ConfigService s_Instance;
	protected static ref TSW_HudConfig s_ClientConfig;

	protected ref TSW_HudConfig m_Config;
	protected bool m_bLoaded;

	static TSW_ConfigService GetInstance()
	{
		if (!s_Instance)
			s_Instance = new TSW_ConfigService();

		return s_Instance;
	}

	void Bootstrap()
	{
		EnsureLoaded();
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
		FileIO.MakeDirectory(PROFILE_CONFIG_DIR);

		if (!FileIO.FileExists(PROFILE_CONFIG_PATH))
		{
			m_Config = TSW_HudConfig.CreateDefault();
			m_Config.SaveToFile(PROFILE_CONFIG_PATH);
			Print("[TSW] Created config: " + PROFILE_CONFIG_PATH, LogLevel.NORMAL);
		}

		if (!m_Config)
			m_Config = TSW_HudConfig.LoadFromFile(PROFILE_CONFIG_PATH);

		if (!m_Config)
		{
			m_Config = TSW_HudConfig.CreateDefault();
			Print("[TSW] Failed to load config, using defaults", LogLevel.WARNING);
		}
	}

	static TSW_HudConfig GetClientConfig()
	{
		if (!s_ClientConfig)
			s_ClientConfig = TSW_HudConfig.CreateDefault();

		return s_ClientConfig;
	}

	static void SetClientConfig(TSW_HudConfig config)
	{
		if (!config)
			return;

		s_ClientConfig = config;
	}
}
