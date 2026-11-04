modded class ArmaReforgerScripted
{
	protected bool m_bRAMI_MissionHeaderApplied;
	protected bool m_bRAMI_AllowMenuWhileUnconscious;

	override protected void OnMissionSet(MissionHeader mission)
	{
		super.OnMissionSet(mission);
		if (!Replication.IsServer() || m_bRAMI_MissionHeaderApplied)
			return;

		SCR_MissionHeader header = SCR_MissionHeader.Cast(mission);
		if (!header)
			return;

		// As in ACE, retain the first header: later headers can omit server JSON overrides (T172515).
		m_bRAMI_AllowMenuWhileUnconscious = header.m_bAllowToOpenAMIMenuWhileUnconscious;
		m_bRAMI_MissionHeaderApplied = true;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGameMode());
		if (gameMode)
			gameMode.RAMI_SetMenuAllowedWhileUnconscious(m_bRAMI_AllowMenuWhileUnconscious);
	}

	bool RAMI_IsMenuAllowedWhileUnconscious()
	{
		// World initialization can precede the first OnMissionSet callback.
		if (!m_bRAMI_MissionHeaderApplied)
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(GetMissionHeader());
			if (header)
				return header.m_bAllowToOpenAMIMenuWhileUnconscious;
		}
		return m_bRAMI_AllowMenuWhileUnconscious;
	}
}
