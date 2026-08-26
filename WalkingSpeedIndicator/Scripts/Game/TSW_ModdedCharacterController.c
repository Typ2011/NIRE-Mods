modded class SCR_CharacterControllerComponent
{
	protected const ResourceName TSW_HUD_LAYOUT = "{C44F2642A5D101BC}UI/layouts/TSW_SpeedIndicator.layout";
	protected const float TSW_HUD_HOLD_TIME = 2.0;
	protected const float TSW_HUD_FADE_DURATION = 0.35;

	protected Widget m_wTSWHudRoot;
	protected ImageWidget m_wTSWBackground;
	protected TextWidget m_wTSWSpeedValue;
	protected int m_iTSWLastDisplayedPercent = -1;
	protected float m_fTSWHudIdleTime;
	protected float m_fTSWHudAlpha = -1.0;

	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		super.OnPrepareControls(owner, am, dt, player);

		if (!player || !IsPlayerControlled())
			return;

		if (TSW_ShouldHideHud())
		{
			TSW_DestroyHud();
			return;
		}

		TSW_EnsureHud();
		TSW_UpdateHud(GetDynamicSpeed());
		TSW_UpdateFade(dt);
	}

	void ~SCR_CharacterControllerComponent()
	{
		TSW_DestroyHud();
	}

	protected void TSW_EnsureHud()
	{
		if (m_wTSWHudRoot && m_wTSWSpeedValue)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		m_wTSWHudRoot = workspace.CreateWidgets(TSW_HUD_LAYOUT);
		if (!m_wTSWHudRoot)
		{
			Print(string.Format("[TSW] HUD create failed for layout %1", TSW_HUD_LAYOUT));
			TSW_DestroyHud();
			return;
		}

		m_wTSWBackground = ImageWidget.Cast(m_wTSWHudRoot.FindAnyWidget("Background"));
		m_wTSWSpeedValue = TextWidget.Cast(m_wTSWHudRoot.FindAnyWidget("SpeedValue"));
		if (!m_wTSWSpeedValue)
		{
			Print("[TSW] HUD create failed: SpeedValue widget not found");
			TSW_DestroyHud();
			return;
		}

		TSW_ResetFade();
		Print("[TSW] HUD created successfully");
	}

	protected void TSW_DestroyHud()
	{
		if (m_wTSWHudRoot)
			m_wTSWHudRoot.RemoveFromHierarchy();

		m_wTSWHudRoot = null;
		m_wTSWBackground = null;
		m_wTSWSpeedValue = null;
		m_fTSWHudIdleTime = 0;
		m_fTSWHudAlpha = -1.0;
	}

	protected void TSW_UpdateHud(float dynamicSpeed)
	{
		if (!m_wTSWSpeedValue)
			return;

		if (dynamicSpeed < 0)
			dynamicSpeed = 0;
		else if (dynamicSpeed > 1)
			dynamicSpeed = 1;

		int speedPercent = Math.Round(dynamicSpeed * 100);
		if (speedPercent == m_iTSWLastDisplayedPercent)
			return;

		m_iTSWLastDisplayedPercent = speedPercent;
		m_wTSWSpeedValue.SetText(string.Format("Walk Speed: %1%% - %2", speedPercent, TSW_GetSpeedLabel(dynamicSpeed)));
		TSW_ResetFade();
	}

	protected bool TSW_ShouldHideHud()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return false;

		return menuManager.IsAnyMenuOpen() || menuManager.IsAnyDialogOpen();
	}

	protected string TSW_GetSpeedLabel(float dynamicSpeed)
	{
		TSW_HudConfig config = TSW_ConfigService.GetClientConfig();

		if (dynamicSpeed < 0.33)
			return config.slowLabel;

		if (dynamicSpeed < 0.66)
			return config.normalLabel;

		return config.fastLabel;
	}

	protected void TSW_ResetFade()
	{
		m_fTSWHudIdleTime = 0;
		TSW_ApplyHudAlpha(1.0);
	}

	protected void TSW_UpdateFade(float dt)
	{
		if (!m_wTSWSpeedValue)
			return;

		m_fTSWHudIdleTime += dt;

		float targetAlpha = 1.0;
		if (m_fTSWHudIdleTime > TSW_HUD_HOLD_TIME)
		{
			float fadeProgress = (m_fTSWHudIdleTime - TSW_HUD_HOLD_TIME) / TSW_HUD_FADE_DURATION;
			targetAlpha = 1.0 - Math.Clamp(fadeProgress, 0.0, 1.0);
		}

		TSW_ApplyHudAlpha(targetAlpha);
	}

	protected void TSW_ApplyHudAlpha(float alpha)
	{
		alpha = Math.Clamp(alpha, 0.0, 1.0);
		if (Math.AbsFloat(alpha - m_fTSWHudAlpha) < 0.01)
			return;

		m_fTSWHudAlpha = alpha;

		int textAlpha = Math.Round(alpha * 255.0);
		int backgroundAlpha = Math.Round(alpha * 150.0);

		if (m_wTSWSpeedValue)
			m_wTSWSpeedValue.SetColorInt((textAlpha << 24) | 0x00FFFFFF);

		if (m_wTSWBackground)
			m_wTSWBackground.SetColorInt((backgroundAlpha << 24) | 0x00000000);
	}
}
