modded class SCR_InterfaceSettingsSubMenu
{
	protected static const ResourceName RAMI_SETTINGS_LAYOUT = "{6A15B001C0100001}UI/layouts/RAMI_InterfaceSettings.layout";
	protected Widget m_RAMI_SettingsRoot;

	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		Widget content;
		if (m_wScroll)
			content = m_wScroll.GetChildren();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!content || !workspace)
		{
			Print("RAMI: Interface settings content was not found", LogLevel.ERROR);
			return;
		}

		Widget settingsRoot = m_wRoot.FindAnyWidget("RAMISettings");
		if (!settingsRoot)
			settingsRoot = workspace.CreateWidgets(RAMI_SETTINGS_LAYOUT, content);
		m_RAMI_SettingsRoot = settingsRoot;
	}

	override void OnTabShow()
	{
		super.OnTabShow();
		RAMI_BindSettings();
	}

	protected void RAMI_BindSettings()
	{
		BaseContainer settings = RAMI_UISettingsModule.GetInstance();
		if (!m_RAMI_SettingsRoot || !settings)
			return;

		SCR_SelectionWidgetComponent textColor = SCR_SelectionWidgetComponent.GetSelectionComponent("RAMIButtonTextColor", m_RAMI_SettingsRoot);
		if (textColor)
		{
			int value;
			settings.Get(RAMI_UISettingsModule.BUTTON_TEXT_COLOR, value);
			textColor.SetCurrentItem(value, false, false);
			textColor.m_OnChanged.Remove(RAMI_SetButtonTextColor);
			textColor.m_OnChanged.Insert(RAMI_SetButtonTextColor);
		}

		SCR_SelectionWidgetComponent colorblindMode = SCR_SelectionWidgetComponent.GetSelectionComponent("RAMIColorblindMode", m_RAMI_SettingsRoot);
		if (colorblindMode)
		{
			bool value;
			settings.Get(RAMI_UISettingsModule.COLORBLIND_MODE, value);
			colorblindMode.SetCurrentItem(value, false, false);
			colorblindMode.m_OnChanged.Remove(RAMI_SetColorblindMode);
			colorblindMode.m_OnChanged.Insert(RAMI_SetColorblindMode);
		}
	}

	protected void RAMI_SetButtonTextColor(SCR_SelectionWidgetComponent component, int value)
	{
		BaseContainer settings = RAMI_UISettingsModule.GetInstance();
		if (!settings)
			return;

		settings.Set(RAMI_UISettingsModule.BUTTON_TEXT_COLOR, value);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}

	protected void RAMI_SetColorblindMode(SCR_SelectionWidgetComponent component, int value)
	{
		BaseContainer settings = RAMI_UISettingsModule.GetInstance();
		if (!settings)
			return;

		settings.Set(RAMI_UISettingsModule.COLORBLIND_MODE, value != 0);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
}
