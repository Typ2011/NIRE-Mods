class RAMI_UISettingsModule : ModuleGameSettings
{
	const string BUTTON_TEXT_COLOR = "m_iButtonTextColor";
	const string COLORBLIND_MODE = "m_bColorblindMode";
	const int BUTTON_TEXT_COLOR_COUNT = 10;

	[Attribute("0")]
	int m_iButtonTextColor;

	[Attribute("0")]
	bool m_bColorblindMode;

	static BaseContainer GetInstance()
	{
		return GetGame().GetGameUserSettings().GetModule("RAMI_UISettingsModule");
	}
}
