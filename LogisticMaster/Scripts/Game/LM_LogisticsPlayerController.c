modded class SCR_PlayerController
{
	protected ref array<ref LM_LogisticsRequest> m_aLM_Requests = {};
	protected bool m_bLM_Authorized;
	protected ref array<int> m_aLM_AuthorizationPlayerIds = {};
	protected ref array<bool> m_aLM_AuthorizationStates = {};

	array<ref LM_LogisticsRequest> LM_GetRequests()
	{
		return m_aLM_Requests;
	}

	bool LM_IsAuthorized()
	{
		return m_bLM_Authorized;
	}

	array<int> LM_GetAuthorizationPlayerIds()
	{
		return m_aLM_AuthorizationPlayerIds;
	}

	bool LM_IsPlayerAuthorized(int playerId)
	{
		int index = m_aLM_AuthorizationPlayerIds.Find(playerId);
		return index >= 0 && m_aLM_AuthorizationStates[index];
	}

	void LM_RequestSnapshot(RplId terminalId)
	{
		m_aLM_Requests.Clear();
		Rpc(RpcAsk_LM_RequestSnapshot, terminalId);
	}

	void LM_SubmitRequest(RplId terminalId, ResourceName prefab, int quantity)
	{
		Rpc(RpcAsk_LM_SubmitRequest, terminalId, prefab, quantity);
	}

	void LM_ManageRequest(RplId terminalId, int requestId, bool approve)
	{
		Rpc(RpcAsk_LM_ManageRequest, terminalId, requestId, approve);
	}

	void LM_CompleteRequest(RplId terminalId, int requestId)
	{
		Rpc(RpcAsk_LM_CompleteRequest, terminalId, requestId);
	}

	void LM_EditStock(RplId terminalId, ResourceName prefab, int quantity, bool add)
	{
		Rpc(RpcAsk_LM_EditStock, terminalId, prefab, quantity, add);
	}

	void LM_RequestAuthorizationSnapshot(RplId terminalId)
	{
		m_aLM_AuthorizationPlayerIds.Clear();
		m_aLM_AuthorizationStates.Clear();
		Rpc(RpcAsk_LM_RequestAuthorizationSnapshot, terminalId);
	}

	void LM_SetPlayerAuthorization(RplId terminalId, int playerId, bool authorized)
	{
		Rpc(RpcAsk_LM_SetPlayerAuthorization, terminalId, playerId, authorized);
	}

	void LM_SendRequest(LM_LogisticsRequest request)
	{
		Rpc(RpcDo_LM_UpsertRequest, request.m_iId, request.m_iRequesterPlayerId, request.m_iHandlerPlayerId, request.m_sFactionKey, request.m_sItemPrefab, request.m_iQuantity, request.m_eStatus);
	}

	void LM_SendSnapshotDone()
	{
		Rpc(RpcDo_LM_SnapshotDone);
	}

	void LM_SendStockChanged()
	{
		Rpc(RpcDo_LM_StockChanged);
	}

	void LM_SendAuthorizationEntry(int playerId, bool authorized)
	{
		Rpc(RpcDo_LM_UpsertAuthorizationEntry, playerId, authorized);
	}

	void LM_SendAuthorizationSnapshotDone()
	{
		Rpc(RpcDo_LM_AuthorizationSnapshotDone);
	}

	void LM_SendAuthorizationChanged(bool authorized)
	{
		Rpc(RpcDo_LM_AuthorizationChanged, authorized);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_RequestSnapshot(RplId terminalId)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.SendSnapshot(this, LM_GetPlayerId());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_SubmitRequest(RplId terminalId, ResourceName prefab, int quantity)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.SubmitRequest(LM_GetPlayerId(), prefab, quantity);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_ManageRequest(RplId terminalId, int requestId, bool approve)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.ManageRequest(LM_GetPlayerId(), requestId, approve);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_CompleteRequest(RplId terminalId, int requestId)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.CompleteRequest(LM_GetPlayerId(), requestId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_EditStock(RplId terminalId, ResourceName prefab, int quantity, bool add)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.EditStock(LM_GetPlayerId(), prefab, quantity, add);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_RequestAuthorizationSnapshot(RplId terminalId)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.SendAuthorizationSnapshot(this, LM_GetPlayerId());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LM_SetPlayerAuthorization(RplId terminalId, int playerId, bool authorized)
	{
		LM_LogisticsTerminalComponent terminal = LM_ResolveTerminal(terminalId);
		if (terminal)
			terminal.SetPlayerAuthorization(LM_GetPlayerId(), playerId, authorized);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_UpsertRequest(int id, int requesterId, int handlerId, string factionKey, ResourceName prefab, int quantity, LM_ELogisticsRequestStatus status)
	{
		LM_LogisticsRequest request;
		foreach (LM_LogisticsRequest existing : m_aLM_Requests)
		{
			if (existing.m_iId == id)
			{
				request = existing;
				break;
			}
		}

		if (!request)
		{
			request = new LM_LogisticsRequest(id, requesterId, factionKey, prefab, quantity);
			m_aLM_Requests.Insert(request);
		}

		request.m_iHandlerPlayerId = handlerId;
		request.m_eStatus = status;
		LM_LogisticsTerminalUI.RefreshOpen();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_SnapshotDone()
	{
		LM_LogisticsTerminalUI.RefreshOpen();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_StockChanged()
	{
		LM_LogisticsTerminalUI.RefreshOpen();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_UpsertAuthorizationEntry(int playerId, bool authorized)
	{
		int index = m_aLM_AuthorizationPlayerIds.Find(playerId);
		if (index < 0)
		{
			m_aLM_AuthorizationPlayerIds.Insert(playerId);
			m_aLM_AuthorizationStates.Insert(authorized);
		}
		else
		{
			m_aLM_AuthorizationStates[index] = authorized;
		}

		LM_LogisticsTerminalUI.RefreshOpen();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_AuthorizationSnapshotDone()
	{
		LM_LogisticsTerminalUI.RefreshOpen();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_LM_AuthorizationChanged(bool authorized)
	{
		m_bLM_Authorized = authorized;
		LM_LogisticsTerminalUI.CloseIfOpen();
	}

	protected int LM_GetPlayerId()
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			if (players.GetPlayerController(playerId) == this)
				return playerId;
		}

		return 0;
	}

	protected LM_LogisticsTerminalComponent LM_ResolveTerminal(RplId terminalId)
	{
		IEntity owner = IEntity.Cast(Replication.FindItem(terminalId));
		if (!owner)
			return null;

		return LM_LogisticsTerminalComponent.Cast(owner.FindComponent(LM_LogisticsTerminalComponent));
	}
}
