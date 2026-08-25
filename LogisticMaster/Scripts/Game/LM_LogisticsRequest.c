// Shared logistics request and stock data.

enum LM_ELogisticsRequestStatus
{
	OPEN,
	READY,
	REJECTED,
	COMPLETED
}

class LM_StockEntry
{
	ResourceName m_sItemPrefab;
	string m_sDisplayName;
	int m_iQuantity;
	SCR_EArsenalItemType m_eArsenalItemType;
	SCR_EArsenalItemMode m_eArsenalItemMode;

	void LM_StockEntry(ResourceName itemPrefab, int quantity)
	{
		m_sItemPrefab = itemPrefab;
		m_sDisplayName = FilePath.StripExtension(FilePath.StripPath(itemPrefab));
		m_iQuantity = quantity;
	}
}

class LM_LogisticsRequest
{
	int m_iId;
	int m_iRequesterPlayerId;
	int m_iHandlerPlayerId = -1;
	string m_sFactionKey;
	ResourceName m_sItemPrefab;
	string m_sDisplayName;
	int m_iQuantity;
	LM_ELogisticsRequestStatus m_eStatus = LM_ELogisticsRequestStatus.OPEN;

	void LM_LogisticsRequest(int id = 0, int requesterPlayerId = 0, string factionKey = "", ResourceName itemPrefab = "", int quantity = 0)
	{
		m_iId = id;
		m_iRequesterPlayerId = requesterPlayerId;
		m_sFactionKey = factionKey;
		m_sItemPrefab = itemPrefab;
		m_sDisplayName = FilePath.StripExtension(FilePath.StripPath(itemPrefab));
		m_iQuantity = quantity;
	}

	string GetStatusLabel()
	{
		switch (m_eStatus)
		{
			case LM_ELogisticsRequestStatus.OPEN: return "Offen";
			case LM_ELogisticsRequestStatus.READY: return "Bereit";
			case LM_ELogisticsRequestStatus.REJECTED: return "Abgelehnt";
			case LM_ELogisticsRequestStatus.COMPLETED: return "Abgeschlossen";
		}

		return "Unbekannt";
	}
}
