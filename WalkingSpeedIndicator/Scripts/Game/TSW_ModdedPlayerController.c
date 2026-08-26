modded class SCR_PlayerController
{
	protected bool m_bTSWConfigRequested;

	protected bool TSW_IsLocalController()
	{
		return SCR_PlayerController.s_pLocalPlayerController == this;
	}

	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (!TSW_IsLocalController() || !to)
			return;

		TSW_RequestHudConfig();
	}

	void TSW_RequestHudConfig()
	{
		if (m_bTSWConfigRequested)
			return;

		m_bTSWConfigRequested = true;
		Rpc(TSW_RpcRequestHudConfig);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void TSW_RpcRequestHudConfig()
	{
		if (!Replication.IsServer())
			return;

		TSW_ConfigService.GetInstance().Bootstrap();
		Rpc(TSW_RpcReceiveHudConfig, TSW_ConfigService.GetInstance().GetConfigJson());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void TSW_RpcReceiveHudConfig(string configJson)
	{
		if (!TSW_IsLocalController())
			return;

		TSW_HudConfig config = TSW_HudConfig.LoadFromString(configJson);
		if (!config)
			return;

		TSW_ConfigService.SetClientConfig(config);
	}
}
