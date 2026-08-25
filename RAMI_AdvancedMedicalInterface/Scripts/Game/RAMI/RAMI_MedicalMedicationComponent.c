modded class ACE_Medical_MedicationComponent
{
	protected static const int RAMI_ACTIVITY_LIMIT = 50;
	protected RAMI_ETriageLevel m_RAMI_TriageLevel;
	protected ref array<int> m_RAMI_ActivityTimes = {};
	protected ref array<string> m_RAMI_ActivityMessages = {};

	RAMI_ETriageLevel RAMI_GetTriageLevel()
	{
		return m_RAMI_TriageLevel;
	}

	void RAMI_SetTriageLevel(RAMI_ETriageLevel triageLevel)
	{
		if (!Replication.IsServer() || m_RAMI_TriageLevel == triageLevel)
			return;

		m_RAMI_TriageLevel = triageLevel;
		RAMI_AddActivity("TRIAGE SET // " + typename.EnumToString(RAMI_ETriageLevel, triageLevel));
	}

	void RAMI_AddActivity(string message)
	{
		if (!Replication.IsServer()
			|| (!message.StartsWith("TRIAGE SET //")
				&& !message.StartsWith("MEDICATION //")
				&& !message.StartsWith("TREATMENT //")))
			return;

		if (m_RAMI_ActivityMessages.Count() >= RAMI_ACTIVITY_LIMIT)
		{
			m_RAMI_ActivityTimes.RemoveOrdered(0);
			m_RAMI_ActivityMessages.RemoveOrdered(0);
		}

		m_RAMI_ActivityTimes.Insert(System.GetUnixTime());
		m_RAMI_ActivityMessages.Insert(message);
	}

	void RAMI_GetActivity(out array<int> times, out array<string> messages)
	{
		times = m_RAMI_ActivityTimes;
		messages = m_RAMI_ActivityMessages;
	}

	override void AddLogEntry(string message, int authorID = -1)
	{
		super.AddLogEntry(message, authorID);
		if (!Replication.IsServer())
			return;

		string authorName = GetGame().GetPlayerManager().GetPlayerName(authorID);
		if (authorName.IsEmpty())
			authorName = "N/A";
		RAMI_AddActivity(string.Format("MEDICATION // %1 // %2", message, authorName));
	}
}
