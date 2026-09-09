class NIRE_NotepadMission
{
	string m_sName;
	string m_sText;
	string m_sTemplateText;
	ref array<string> m_aTemplatePrompts = {};
	ref array<string> m_aTemplateAnswers = {};
	int m_iTemplateLines;

	void NIRE_NotepadMission(string name = "")
	{
		m_sName = name;
	}
}

class NIRE_TemplateAnswerRowHandler : ScriptedWidgetEventHandler
{
	protected NIRE_NotepadController m_Controller;
	protected int m_iIndex;
	protected bool m_bPrompt;
	protected PanelWidget m_Focus;

	void NIRE_TemplateAnswerRowHandler(NIRE_NotepadController controller, int index, bool prompt, PanelWidget focus)
	{
		m_Controller = controller;
		m_iIndex = index;
		m_bPrompt = prompt;
		m_Focus = focus;
	}

	override bool OnChange(Widget w, bool finished)
	{
		MultilineEditBoxWidget editor = MultilineEditBoxWidget.Cast(w);
		if (editor)
			m_Controller.SetTemplateField(m_iIndex, m_bPrompt, editor.GetText());
		return false;
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		m_Focus.SetColor(Color.FromSRGBA(255, 198, 45, 255));
		return false;
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_Focus.SetColor(Color.FromSRGBA(120, 120, 120, 210));
		return false;
	}

	override bool OnWriteModeLeave(Widget w)
	{
		m_Focus.SetColor(Color.FromSRGBA(120, 120, 120, 210));
		return false;
	}
}

class NIRE_NoteLineRowHandler : ScriptedWidgetEventHandler
{
	protected NIRE_NotepadController m_Controller;
	protected PanelWidget m_Focus;

	void NIRE_NoteLineRowHandler(NIRE_NotepadController controller, PanelWidget focus)
	{
		m_Controller = controller;
		m_Focus = focus;
	}

	override bool OnChange(Widget w, bool finished)
	{
		m_Controller.OnLineEditorChanged(EditBoxWidget.Cast(w));
		return false;
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		m_Focus.SetColor(Color.FromSRGBA(255, 198, 45, 255));
		return false;
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_Focus.SetColor(Color.FromSRGBA(120, 120, 120, 210));
		return false;
	}

	override bool OnWriteModeLeave(Widget w)
	{
		m_Focus.SetColor(Color.FromSRGBA(120, 120, 120, 210));
		return false;
	}
}

class NIRE_NotepadMissionBook
{
	ref array<ref NIRE_NotepadMission> m_aMissions = {};
	int m_iActiveMission;
}

class NIRE_NotepadMenu : MenuBase
{
	protected ref NIRE_NotepadController m_Controller;
	protected static bool s_bSuppressNextPause;

	static void Toggle()
	{
		// The full-screen logistics workspace answers the toggle first, so the same key closes it and
		// puts the notepad it replaced back on screen.
		if (NIRE_LogisticsScreen.CloseAndRestoreIfOpen())
			return;

		if (NIRE_NotepadController.CloseMapOverlay())
			return;

		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;

		if (menuManager.FindMenuByPreset(ChimeraMenuPreset.NIRE_Notepad))
		{
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.NIRE_Notepad);
			return;
		}

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager && inputManager.IsContextActive("MapContext") && NIRE_NotepadController.OpenMapOverlay())
			return;

		menuManager.OpenMenu(ChimeraMenuPreset.NIRE_Notepad, 0, true, false);
	}

	//! Reopens the notepad the logistics workspace closed on its way in.
	static void OpenNotepad()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager || menuManager.FindMenuByPreset(ChimeraMenuPreset.NIRE_Notepad))
			return;

		menuManager.OpenMenu(ChimeraMenuPreset.NIRE_Notepad, 0, true, false);
	}

	static bool CloseIfOpen()
	{
		if (NIRE_LogisticsScreen.CloseIfOpen())
			return true;

		if (NIRE_NotepadController.CloseMapOverlay())
			return true;

		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager || !menuManager.FindMenuByPreset(ChimeraMenuPreset.NIRE_Notepad))
			return false;

		menuManager.CloseMenuByPreset(ChimeraMenuPreset.NIRE_Notepad);
		return true;
	}

	//! Runs on the Escape key going back up. ArmaReforgerScripted.OnMenuOpen answers the same key on
	//! the way down and only skips the pause menu while a menu is still open, so closing the notepad
	//! on the down edge hands the pause menu an empty screen to take over. Closing on the up edge
	//! leaves the notepad standing for exactly as long as that check needs it.
	static void CloseFromBack()
	{
		s_bSuppressNextPause = true;
		GetGame().GetCallqueue().Remove(ClearPauseSuppression);
		GetGame().GetCallqueue().CallLater(ClearPauseSuppression, 250, false);
		// One layer per back press: the logistics workspace answers for itself and leaves the notepad
		// standing behind it.
		if (NIRE_LogisticsScreen.ConsumeBack())
			return;

		CloseIfOpen();
	}

	static bool ConsumePauseSuppression()
	{
		bool suppress = s_bSuppressNextPause;
		s_bSuppressNextPause = false;
		return suppress;
	}

	protected static void ClearPauseSuppression()
	{
		s_bSuppressNextPause = false;
	}

	protected override void OnMenuOpen()
	{
		m_Controller = new NIRE_NotepadController();
		if (!m_Controller.Initialize(GetRootWidget(), false))
			Close();
	}

	protected override void OnMenuOpened()
	{
		if (m_Controller)
			m_Controller.FocusActiveTab();
	}

	protected override void OnMenuClose()
	{
		if (m_Controller)
			m_Controller.Shutdown();

		m_Controller = null;
	}
}

class NIRE_NotepadController
{
	protected static const ResourceName NOTEPAD_LAYOUT = "{1491238DE559429D}UI/layouts/NiRe_Notepad/NIRE_Notepad.layout";
	protected static const ResourceName MISSION_ROW_LAYOUT = "{2A7C19E4B6D830F1}UI/layouts/NiRe_Notepad/NIRE_MissionRow.layout";
	protected static const ResourceName SELECTOR_ROW_LAYOUT = "{C1E02C785E02616D}UI/layouts/InventoryBoxes/GMInventoryEditorRow.layout";
	protected static const ResourceName TEMPLATE_ANSWER_ROW_LAYOUT = "{8D02B4399E3F2A90}UI/layouts/NiRe_Notepad/NIRE_TemplateAnswerRow.layout";
	protected static const ResourceName NOTE_LINE_ROW_LAYOUT = "{8D02B4399E3F2AB0}UI/layouts/NiRe_Notepad/NIRE_NoteLineRow.layout";
	protected static const string PROFILE_FILE = "$profile:NiRe_Notepad.json";
	protected static const string PROFILE_BACKUP = "$profile:NiRe_Notepad.backup.json";
	protected static const string PROFILE_TEMP = "$profile:NiRe_Notepad.tmp.json";
	protected static const int PROFILE_VERSION = 6;
	protected static const int TAB_COUNT = 6;
	protected static const int LINE_EDITOR_COUNT = 10;
	protected static const int DRAWING_COLOR_COUNT = 5;
	//! Reserved: index 3 is the Logistics tab button, which opens NIRE_LogisticsScreen instead of
	//! becoming the active tab. The index stays taken so stored mission books keep their positions.
	protected static const int LOGISTICS_TAB = 3;
	protected static const int LOGISTICS_MAX_QUANTITY = 999;
	protected static const int TERRAIN_TAB = 5;

	protected static string s_sGeneralNote;
	protected static ref array<ref NIRE_NotepadMissionBook> s_aMissionBooks = {};
	protected static int s_iActiveTab;
	protected static bool s_bDataLoaded;
	protected static bool s_bPrimaryProfileValid;
	protected static ref NIRE_NotepadController s_MapOverlay;
	protected static ref array<float> s_aDrawingVertices = {};
	protected static ref array<int> s_aDrawingEnds = {};
	protected static ref array<int> s_aDrawingModes = {};
	protected static NIRE_NotepadController s_ActiveController;

	protected ref NIRE_NotepadMenuHandler m_Handler;
	protected ref array<ButtonWidget> m_aTabButtons = {};
	protected ref array<ButtonWidget> m_aMissionRows = {};
	protected ref array<TextWidget> m_aMissionRowTexts = {};
	protected ref array<ButtonWidget> m_aColorButtons = {};
	protected ref array<ref CanvasWidgetCommand> m_aBackgroundCommands = {};
	protected ref array<ref CanvasWidgetCommand> m_aTerrainCommands = {};
	protected Widget m_Root;
	protected Widget m_NotepadPanel;
	protected CanvasWidget m_Background;
	protected Widget m_MissionPanel;
	protected Widget m_MissionScroll;
	protected VerticalLayoutWidget m_MissionList;
	protected EditBoxWidget m_MissionName;
	protected Widget m_TemplateMenu;
	protected Widget m_Template5;
	protected Widget m_Template9;
	protected Widget m_TemplateFree;
	protected PanelWidget m_EditorFocus;
	protected Widget m_EditorBackground;
	protected ScrollLayoutWidget m_EditorScroll;
	protected MultilineEditBoxWidget m_Editor;
	protected ScrollLayoutWidget m_TemplateRowsScroll;
	protected VerticalLayoutWidget m_TemplateRowsList;
	protected ref array<MultilineEditBoxWidget> m_aTemplateEditors = {};
	protected VerticalLayoutWidget m_LineEditors;
	protected ref array<Widget> m_aLineEditorRows = {};
	protected ref array<EditBoxWidget> m_aLineEditors = {};
	protected Widget m_TerrainPanel;
	protected CanvasWidget m_TerrainCanvas;
	protected Widget m_TerrainInput;
	protected ButtonWidget m_PenButton;
	protected ButtonWidget m_EraserButton;
	protected ButtonWidget m_ClearDrawingButton;
	protected ButtonWidget m_NewMissionButton;
	protected TextWidget m_NewMissionText;
	protected ButtonWidget m_DeleteMissionButton;
	protected ButtonWidget m_CloseButton;
	protected TextWidget m_TabTitle;
	protected TextWidget m_MissionPanelTitle;
	protected TextWidget m_TemplateMenuLabel;
	protected TextWidget m_Status;
	protected bool m_bDeletePending;
	protected bool m_bClearDrawingPending;
	protected bool m_bDrawing;
	protected bool m_bEraser;
	protected int m_iDrawingColor;
	protected int m_iPendingTemplateLines;
	protected float m_fLastDrawingX;
	protected float m_fLastDrawingY;
	protected bool m_bMapOverlay;
	protected bool m_bInitialized;
	protected bool m_bUpdatingWidgets;

	static bool OpenMapOverlay()
	{
		if (s_MapOverlay)
			return true;

		MenuManager menuManager = GetGame().GetMenuManager();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!menuManager || !workspace)
			return false;

		MenuBase mapMenu = menuManager.GetTopMenu();
		if (!mapMenu || !mapMenu.GetRootWidget())
			return false;

		Widget root = workspace.CreateWidgets(NOTEPAD_LAYOUT, mapMenu.GetRootWidget());
		if (!root)
			return false;

		s_MapOverlay = new NIRE_NotepadController();
		if (!s_MapOverlay.Initialize(root, true))
		{
			root.RemoveFromHierarchy();
			s_MapOverlay = null;
			return false;
		}

		GetGame().GetCallqueue().CallLater(CheckMapContext, 250, true);
		return true;
	}

	static bool IsMapOverlayOpen()
	{
		return s_MapOverlay != null;
	}

	static bool CloseMapOverlay()
	{
		if (!s_MapOverlay)
			return false;

		GetGame().GetCallqueue().Remove(CheckMapContext);
		s_MapOverlay.Shutdown();
		s_MapOverlay = null;
		return true;
	}

	protected static void CheckMapContext()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager || !inputManager.IsContextActive("MapContext"))
			CloseMapOverlay();
	}

	bool Initialize(Widget root, bool mapOverlay)
	{
		if (!root)
			return false;

		EnsureDataLoaded();
		m_Root = root;
		m_bMapOverlay = mapOverlay;
		m_NotepadPanel = root.FindAnyWidget("NotepadPanel");
		m_Background = CanvasWidget.Cast(root.FindAnyWidget("NotepadBackground"));
		m_MissionPanel = root.FindAnyWidget("MissionPanel");
		m_MissionScroll = root.FindAnyWidget("MissionScroll");
		m_MissionList = VerticalLayoutWidget.Cast(root.FindAnyWidget("MissionList"));
		m_MissionName = EditBoxWidget.Cast(root.FindAnyWidget("MissionName"));
		m_EditorScroll = ScrollLayoutWidget.Cast(root.FindAnyWidget("EditorScroll"));
		m_Editor = MultilineEditBoxWidget.Cast(root.FindAnyWidget("NoteEditor"));
		m_EditorFocus = PanelWidget.Cast(root.FindAnyWidget("EditorFocus"));
		m_EditorBackground = root.FindAnyWidget("EditorBackground");
		m_TemplateRowsScroll = ScrollLayoutWidget.Cast(root.FindAnyWidget("TemplateRowsScroll"));
		m_TemplateRowsList = VerticalLayoutWidget.Cast(root.FindAnyWidget("TemplateRowsList"));
		m_LineEditors = VerticalLayoutWidget.Cast(root.FindAnyWidget("LineEditors"));
		m_TerrainPanel = root.FindAnyWidget("TerrainPanel");
		m_TerrainCanvas = CanvasWidget.Cast(root.FindAnyWidget("TerrainCanvas"));
		m_TerrainInput = root.FindAnyWidget("TerrainInput");
		m_PenButton = ButtonWidget.Cast(root.FindAnyWidget("TerrainPen"));
		m_EraserButton = ButtonWidget.Cast(root.FindAnyWidget("TerrainEraser"));
		m_ClearDrawingButton = ButtonWidget.Cast(root.FindAnyWidget("TerrainClear"));
		m_NewMissionButton = ButtonWidget.Cast(root.FindAnyWidget("NewMission"));
		m_NewMissionText = TextWidget.Cast(root.FindAnyWidget("NewMissionText"));
		m_DeleteMissionButton = ButtonWidget.Cast(root.FindAnyWidget("DeleteMission"));
		m_CloseButton = ButtonWidget.Cast(root.FindAnyWidget("CloseButton"));
		m_TemplateMenu = root.FindAnyWidget("TemplateMenu");
		m_Template5 = root.FindAnyWidget("Template5");
		m_Template9 = root.FindAnyWidget("Template9");
		m_TemplateFree = root.FindAnyWidget("TemplateFree");
		m_TabTitle = TextWidget.Cast(root.FindAnyWidget("ActiveTabTitle"));
		m_MissionPanelTitle = TextWidget.Cast(root.FindAnyWidget("MissionPanelTitle"));
		m_TemplateMenuLabel = TextWidget.Cast(root.FindAnyWidget("TemplateMenuLabel"));
		m_Status = TextWidget.Cast(root.FindAnyWidget("Status"));
		if (!m_NotepadPanel || !m_Background || !m_MissionPanel || !m_MissionScroll || !m_MissionList || !m_MissionName)
		{
			Print("NIRE: Notepad layout is incomplete", LogLevel.ERROR);
			return false;
		}
		if (!m_EditorScroll || !m_Editor || !m_EditorFocus || !m_EditorBackground || !m_TemplateRowsScroll || !m_TemplateRowsList || !m_LineEditors || !m_TerrainPanel)
		{
			Print("NIRE: Notepad layout is incomplete", LogLevel.ERROR);
			return false;
		}
		if (!m_TerrainCanvas || !m_TerrainInput || !m_PenButton || !m_EraserButton || !m_ClearDrawingButton || !m_CloseButton || !m_TemplateMenu)
		{
			Print("NIRE: Notepad layout is incomplete", LogLevel.ERROR);
			return false;
		}
		if (!m_Template5 || !m_Template9 || !m_TemplateFree || !m_TabTitle || !m_MissionPanelTitle || !m_TemplateMenuLabel || !m_Status)
		{
			Print("NIRE: Notepad layout is incomplete", LogLevel.ERROR);
			return false;
		}
		if (!m_NewMissionButton || !m_NewMissionText || !m_DeleteMissionButton)
		{
			Print("NIRE: Notepad layout is incomplete", LogLevel.ERROR);
			return false;
		}

		m_Handler = new NIRE_NotepadMenuHandler(this);
		m_NotepadPanel.AddHandler(m_Handler);
		AddButton(root, "TabGeneral", true);
		AddButton(root, "TabCAS", true);
		AddButton(root, "TabArtillery", true);
		AddButton(root, "TabLogistics", true);
		AddButton(root, "TabMedevac", true);
		AddButton(root, "TabEvacTransport", true);
		AddButton(root, "NewMission", false);
		AddButton(root, "DeleteMission", false);
		AddButton(root, "Template5", false);
		AddButton(root, "Template9", false);
		AddButton(root, "TemplateFree", false);
		AddButton(root, "TerrainPen", false);
		AddButton(root, "TerrainEraser", false);
		AddButton(root, "TerrainClear", false);
		AddButton(root, "TerrainColorWhite", false);
		AddButton(root, "TerrainColorRed", false);
		AddButton(root, "TerrainColorBlue", false);
		AddButton(root, "TerrainColorGreen", false);
		AddButton(root, "TerrainColorYellow", false);
		AddButton(root, "TerrainInput", false);
		AddButton(root, "CloseButton", false);
		if (m_aTabButtons.Count() != TAB_COUNT)
		{
			Print("NIRE: Notepad tab buttons are incomplete", LogLevel.ERROR);
			return false;
		}
		m_aColorButtons = {
			ButtonWidget.Cast(root.FindAnyWidget("TerrainColorWhite")),
			ButtonWidget.Cast(root.FindAnyWidget("TerrainColorRed")),
			ButtonWidget.Cast(root.FindAnyWidget("TerrainColorBlue")),
			ButtonWidget.Cast(root.FindAnyWidget("TerrainColorGreen")),
			ButtonWidget.Cast(root.FindAnyWidget("TerrainColorYellow"))
		};
		if (m_aColorButtons.Count() != DRAWING_COLOR_COUNT || m_aColorButtons.Contains(null))
		{
			Print("NIRE: Terrain color buttons are incomplete", LogLevel.ERROR);
			return false;
		}

		m_Editor.SetVirtualKeyboardTitle("#NIRE-Title");
		m_Editor.SetVirtualKeyboardDesc("#NIRE-Keyboard_EditNote");
		if (!InitializeLineEditors())
			return false;
		m_MissionName.SetVirtualKeyboardDesc("#NIRE-Keyboard_NameEntry");
		m_bInitialized = true;
		s_ActiveController = this;
		ApplyTab();
		SetEditorFocused(false);
		GetGame().GetCallqueue().CallLater(DrawBackground, 1, false);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("NIRE_CloseNotepad", EActionTrigger.UP, CloseFromBack);
			inputManager.AddActionListener("NIRE_EditorPointer", EActionTrigger.DOWN, HandleGlobalPointerDown);
		}

		return true;
	}

	void Shutdown()
	{
		StoreCurrentNote();
		SavePersistentData();
		GetGame().GetCallqueue().Remove(DrawBackground);
		GetGame().GetCallqueue().Remove(DrawTerrain);
		GetGame().GetCallqueue().Remove(SampleDrawing);
		GetGame().GetCallqueue().Remove(DeactivateEditor);
		m_bDrawing = false;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("NIRE_CloseNotepad", EActionTrigger.UP, CloseFromBack);
			inputManager.RemoveActionListener("NIRE_EditorPointer", EActionTrigger.DOWN, HandleGlobalPointerDown);
		}

		if (m_bMapOverlay && m_Root)
			m_Root.RemoveFromHierarchy();

		m_Root = null;
		m_NotepadPanel = null;
		m_Background = null;
		m_MissionPanel = null;
		m_MissionScroll = null;
		m_MissionList = null;
		m_MissionName = null;
		m_Editor = null;
		m_EditorFocus = null;
		m_EditorBackground = null;
		m_EditorScroll = null;
		m_TemplateRowsScroll = null;
		m_TemplateRowsList = null;
		m_aTemplateEditors.Clear();
		m_LineEditors = null;
		m_aLineEditorRows.Clear();
		m_aLineEditors.Clear();
		m_TerrainPanel = null;
		m_TerrainCanvas = null;
		m_TerrainInput = null;
		m_PenButton = null;
		m_EraserButton = null;
		m_ClearDrawingButton = null;
		m_NewMissionButton = null;
		m_NewMissionText = null;
		m_DeleteMissionButton = null;
		m_CloseButton = null;
		m_TemplateMenu = null;
		m_Template5 = null;
		m_Template9 = null;
		m_TemplateFree = null;
		m_TabTitle = null;
		m_MissionPanelTitle = null;
		m_TemplateMenuLabel = null;
		m_Status = null;
		m_Handler = null;
		m_aTabButtons.Clear();
		m_aMissionRows.Clear();
		m_aMissionRowTexts.Clear();
		m_aColorButtons.Clear();
		m_aBackgroundCommands.Clear();
		m_aTerrainCommands.Clear();
		if (s_ActiveController == this)
			s_ActiveController = null;
		m_bInitialized = false;
	}

	protected void AddButton(notnull Widget root, string name, bool isTab)
	{
		ButtonWidget button = ButtonWidget.Cast(root.FindAnyWidget(name));
		if (!button)
		{
			Print(string.Format("NIRE: Widget %1 is missing", name), LogLevel.ERROR);
			return;
		}

		button.AddHandler(m_Handler);
		if (isTab)
			m_aTabButtons.Insert(button);
	}

	void FocusActiveTab()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace && !m_aTabButtons.IsEmpty())
			workspace.SetFocusedWidget(m_aTabButtons[s_iActiveTab]);
	}

	//! Logistics is not a notepad tab any more: its button opens the full-screen supply request
	//! workspace, which needs the room the corner notepad cannot give long item names.
	void OpenLogisticsScreen()
	{
		NIRE_LogisticsScreen.Open();
	}

	void SelectTab(int tabIndex)
	{
		if (tabIndex < 0 || tabIndex >= TAB_COUNT || tabIndex == LOGISTICS_TAB || !m_Editor)
			return;

		StoreCurrentNote();
		s_iActiveTab = tabIndex;
		if (HasMissionBook(s_iActiveTab))
			EnsureMission(s_iActiveTab);
		ApplyTab();
		SavePersistentData();
	}

	protected void ApplyTab(bool showTemplateSelection = false)
	{
		m_bDeletePending = false;
		m_bClearDrawingPending = false;
		m_bUpdatingWidgets = true;
		m_TabTitle.SetText(GetTabTitle(s_iActiveTab));
		if (HasMissionBook(s_iActiveTab))
			m_MissionPanelTitle.SetText(GetMissionBookTitle());

		bool operationalTab = HasMissionBook(s_iActiveTab);
		bool terrainTab = s_iActiveTab == TERRAIN_TAB;
		if (operationalTab)
			EnsureMission(s_iActiveTab);
		showTemplateSelection = showTemplateSelection && HasTemplates(s_iActiveTab);
		SetContentVisible(!showTemplateSelection);
		SetTemplateMenuVisible(showTemplateSelection);
		m_TerrainPanel.SetVisible(terrainTab && !showTemplateSelection);
		m_TerrainPanel.SetEnabled(terrainTab && !showTemplateSelection);

		SetEditorLayout(operationalTab, false);
		if (operationalTab)
		{
			RefreshMissionList();
			NIRE_NotepadMission mission = GetActiveMission();
			m_MissionName.SetText(mission.m_sName);
			m_MissionName.SetVirtualKeyboardTitle(GetMissionBookTitle());
			if (mission.m_iTemplateLines < 1)
				SetLineEditorsText(mission.m_sText);
			else
				m_Editor.SetText(mission.m_sText);
			m_MissionPanelTitle.SetText(GetMissionBookTitle());
			SetEditorLayout(true, !showTemplateSelection && mission.m_iTemplateLines > 0);
			if (mission.m_iTemplateLines < 1)
				m_Status.SetText("#NIRE-Status_FreeText");
			else
				m_Status.SetText("#NIRE-Status_TemplateInserted");
		}
		else if (!terrainTab)
		{
			ClearMissionRows();
			m_MissionName.SetText(string.Empty);
			SetLineEditorsText(s_sGeneralNote);
			m_Status.SetText("#NIRE-Status_FreeText");
		}
		else
		{
			ClearMissionRows();
			m_MissionName.SetText(string.Empty);
			m_Editor.SetText(string.Empty);
			m_Status.SetText("#NIRE-Status_ChooseTemplate");
			DrawTerrain();
			UpdateDrawingToolColors();
		}

		m_bUpdatingWidgets = false;
		UpdateTabColors();
	}

	protected void SetContentVisible(bool visible)
	{
		bool terrainTab = s_iActiveTab == TERRAIN_TAB;
		bool showMissions = visible && HasMissionBook(s_iActiveTab);
		bool showLines = visible && UsesLineEditors();
		bool showEditor = visible && !terrainTab && !showLines;
		m_TabTitle.SetVisible(false);
		m_MissionPanel.SetVisible(showMissions);
		m_MissionPanel.SetEnabled(showMissions);
		FrameSlot.SetAnchorMin(m_MissionScroll, 0.05, 0.39);
		FrameSlot.SetAnchorMax(m_MissionScroll, 0.95, 0.84);
		m_MissionName.SetVisible(showMissions);
		m_MissionName.SetEnabled(showMissions);
		m_NewMissionButton.SetVisible(showMissions);
		m_NewMissionButton.SetEnabled(showMissions);
		m_DeleteMissionButton.SetVisible(showMissions);
		m_EditorFocus.SetVisible(showEditor);
		m_EditorBackground.SetVisible(showEditor);
		m_EditorScroll.SetVisible(showEditor);
		m_EditorScroll.SetEnabled(showEditor);
		m_Editor.SetVisible(showEditor);
		m_Editor.SetEnabled(showEditor);
		UpdateLineEditorCount();
		m_LineEditors.SetVisible(showLines);
		m_LineEditors.SetEnabled(showLines);
		m_TemplateRowsScroll.SetVisible(false);
		m_TemplateRowsScroll.SetEnabled(false);
		m_Status.SetVisible(visible);
	}

	protected void SetTemplateMenuVisible(bool visible)
	{
		bool artillery = s_iActiveTab == 2;
		m_TemplateMenu.SetVisible(visible);
		m_TemplateMenu.SetEnabled(visible);
		m_Template5.SetVisible(visible);
		m_Template5.SetEnabled(visible);
		m_Template9.SetVisible(visible);
		m_Template9.SetEnabled(visible);
		m_TemplateFree.SetVisible(visible);
		m_TemplateFree.SetEnabled(visible);
		TextWidget secondTemplateText = TextWidget.Cast(m_Template9.FindAnyWidget("Template9Text"));
		if (secondTemplateText)
		{
			if (artillery)
				secondTemplateText.SetText("#NIRE-Template_7Liner");
			else
				secondTemplateText.SetText("#NIRE-Template_9Liner");
		}
		m_TemplateMenuLabel.SetText("#NIRE-Template_Select");
	}

	protected void SetEditorLayout(bool withMissionList, bool withTemplate)
	{
		float leftFocus = 0.035;
		float leftBackground = 0.04;
		float leftEditor = 0.06;
		if (withMissionList)
		{
			leftFocus = 0.35;
			leftBackground = 0.355;
			leftEditor = 0.375;
		}
		FrameSlot.SetAnchorMin(m_EditorFocus, leftFocus, 0.245);
		FrameSlot.SetAnchorMax(m_EditorFocus, 0.965, 0.895);
		FrameSlot.SetAnchorMin(m_EditorBackground, leftBackground, 0.25);
		FrameSlot.SetAnchorMax(m_EditorBackground, 0.96, 0.89);
		FrameSlot.SetAnchorMin(m_EditorScroll, leftEditor, 0.27);
		FrameSlot.SetAnchorMax(m_EditorScroll, 0.94, 0.87);
		if (withMissionList)
			FrameSlot.SetAnchorMin(m_LineEditors, 0.355, 0.245);
		else
			FrameSlot.SetAnchorMin(m_LineEditors, 0.04, 0.245);
		FrameSlot.SetAnchorMax(m_LineEditors, 0.96, 0.81);
		m_Editor.SetFlags(WidgetFlags.WRAP_TEXT);
		m_Editor.SetTextWrapping(true);
		m_Editor.Update();
		m_EditorScroll.Update();
		if (withTemplate)
		{
			m_EditorFocus.SetVisible(false);
			m_EditorBackground.SetVisible(false);
			m_EditorScroll.SetVisible(false);
			m_EditorScroll.SetEnabled(false);
			m_Editor.SetVisible(false);
			m_Editor.SetEnabled(false);
			m_TemplateRowsScroll.SetVisible(true);
			m_TemplateRowsScroll.SetEnabled(true);
			RefreshTemplateRows();
		}
		else
		{
			m_TemplateRowsScroll.SetVisible(false);
			m_TemplateRowsScroll.SetEnabled(false);
			ClearTemplateRows();
		}
	}

	void FocusEditor(int button)
	{
		if (button != 0 || !m_Editor)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
			workspace.SetFocusedWidget(m_Editor);
		m_Editor.ActivateWriteMode();
	}

	protected bool InitializeLineEditors()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return false;

		for (int index = 0; index < LINE_EDITOR_COUNT; index++)
		{
			Widget row = workspace.CreateWidgets(NOTE_LINE_ROW_LAYOUT, m_LineEditors);
			EditBoxWidget editor;
			PanelWidget focus;
			if (row)
			{
				editor = EditBoxWidget.Cast(row.FindAnyWidget("NoteLine"));
				focus = PanelWidget.Cast(row.FindAnyWidget("NoteLineFocus"));
			}
			if (!editor || !focus)
				return false;

			editor.SetVirtualKeyboardTitle("#NIRE-Title");
			editor.SetVirtualKeyboardDesc("#NIRE-Keyboard_EditNote");
			editor.AddHandler(new NIRE_NoteLineRowHandler(this, focus));
			m_aLineEditorRows.Insert(row);
			m_aLineEditors.Insert(editor);
		}
		UpdateLineEditorCount();
		return true;
	}

	protected bool UsesLineEditors()
	{
		if (s_iActiveTab == 0)
			return true;

		NIRE_NotepadMission mission = GetActiveMission();
		return HasMissionBook(s_iActiveTab) && mission && mission.m_iTemplateLines < 1;
	}

	protected void UpdateLineEditorCount()
	{
		int lineCount = LINE_EDITOR_COUNT;
		foreach (int index, Widget row : m_aLineEditorRows)
		{
			row.SetVisible(index < lineCount);
			row.SetEnabled(index < lineCount);
			m_aLineEditors[index].SetEnabled(index < lineCount);
		}
	}

	protected void SetLineEditorsText(string text)
	{
		ref array<string> lines = {};
		text.Split("\n", lines, true);
		int lineCount = LINE_EDITOR_COUNT;
		for (int index = 0; index < m_aLineEditors.Count(); index++)
		{
			string line;
			if (index < lineCount - 1 && index < lines.Count())
				line = lines[index];
			else if (index == lineCount - 1)
			{
				for (int sourceIndex = lineCount - 1; sourceIndex < lines.Count(); sourceIndex++)
				{
					if (!line.IsEmpty())
						line += " / ";
					line += lines[sourceIndex];
				}
			}
			m_aLineEditors[index].SetText(line);
		}
	}

	protected string GetLineEditorsText()
	{
		int lastLine = LINE_EDITOR_COUNT - 1;
		while (lastLine >= 0 && m_aLineEditors[lastLine].GetText().IsEmpty())
			lastLine--;

		string text;
		for (int index = 0; index <= lastLine; index++)
		{
			if (index > 0)
				text += "\n";
			text += m_aLineEditors[index].GetText();
		}
		return text;
	}

	protected void SetLineEditorsEnabled(bool enabled)
	{
		int lineCount = LINE_EDITOR_COUNT;
		foreach (int index, EditBoxWidget editor : m_aLineEditors)
			editor.SetEnabled(enabled && index < lineCount);
	}

	void OnLineEditorChanged(EditBoxWidget editor)
	{
		if (m_bUpdatingWidgets)
			return;
		m_bDeletePending = false;
		if (s_iActiveTab == 0)
		{
			s_sGeneralNote = GetLineEditorsText();
			SavePersistentData();
		}
		else if (UsesLineEditors())
		{
			NIRE_NotepadMission mission = GetActiveMission();
			mission.m_sText = GetLineEditorsText();
			SavePersistentData();
		}
	}

	protected void RefreshTemplateRows()
	{
		ClearTemplateRows();
		NIRE_NotepadMission mission = GetActiveMission();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!mission || !workspace)
			return;

		NormalizeTemplateFields(mission);
		foreach (int index, string promptText : mission.m_aTemplatePrompts)
		{
			Widget row = workspace.CreateWidgets(TEMPLATE_ANSWER_ROW_LAYOUT, m_TemplateRowsList);
			if (!row)
				continue;

			MultilineEditBoxWidget prompt = MultilineEditBoxWidget.Cast(row.FindAnyWidget("TemplatePrompt"));
			MultilineEditBoxWidget answer = MultilineEditBoxWidget.Cast(row.FindAnyWidget("TemplateAnswer"));
			PanelWidget promptFocus = PanelWidget.Cast(row.FindAnyWidget("TemplatePromptFocus"));
			PanelWidget answerFocus = PanelWidget.Cast(row.FindAnyWidget("TemplateAnswerFocus"));
			if (!prompt || !answer || !promptFocus || !answerFocus)
				continue;

			prompt.SetText(promptText);
			answer.SetText(mission.m_aTemplateAnswers[index]);
			prompt.SetVirtualKeyboardTitle("#NIRE-Title");
			prompt.SetVirtualKeyboardDesc("#NIRE-Keyboard_EditNote");
			answer.SetVirtualKeyboardTitle("#NIRE-Title");
			answer.SetVirtualKeyboardDesc("#NIRE-Keyboard_EditNote");
			prompt.AddHandler(new NIRE_TemplateAnswerRowHandler(this, index, true, promptFocus));
			answer.AddHandler(new NIRE_TemplateAnswerRowHandler(this, index, false, answerFocus));
			m_aTemplateEditors.Insert(prompt);
			m_aTemplateEditors.Insert(answer);
		}
	}

	protected void ClearTemplateRows()
	{
		m_aTemplateEditors.Clear();
		if (!m_TemplateRowsList)
			return;

		Widget child = m_TemplateRowsList.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			child.RemoveFromHierarchy();
			child = next;
		}
	}

	void SetTemplateField(int index, bool prompt, string value)
	{
		NIRE_NotepadMission mission = GetActiveMission();
		if (!mission || index < 0 || index >= mission.m_aTemplatePrompts.Count())
			return;

		if (prompt)
			mission.m_aTemplatePrompts[index] = value;
		else
			mission.m_aTemplateAnswers[index] = value;
		SyncTemplateStrings(mission);
		m_iPendingTemplateLines = 0;
		SavePersistentData();
	}

	void OnEditorChanged()
	{
		if (m_bUpdatingWidgets)
			return;

		StoreCurrentNote();
		m_bDeletePending = false;
		SavePersistentData();
	}

	void OnMissionNameChanged(bool finished)
	{
		if (m_bUpdatingWidgets || !HasMissionBook(s_iActiveTab))
			return;

		NIRE_NotepadMission mission = GetActiveMission();
		if (!mission)
			return;

		mission.m_sName = m_MissionName.GetText();
		m_bDeletePending = false;
		if (finished && mission.m_sName.IsEmpty())
		{
			mission.m_sName = GetDefaultMissionName(GetActiveBook().m_iActiveMission);
			m_bUpdatingWidgets = true;
			m_MissionName.SetText(mission.m_sName);
			m_bUpdatingWidgets = false;
		}

		int activeMission = GetActiveBook().m_iActiveMission;
		if (activeMission >= 0 && activeMission < m_aMissionRowTexts.Count())
			m_aMissionRowTexts[activeMission].SetText(GetMissionListName(mission));
		SavePersistentData();
	}

	void SelectMission(int row)
	{
		if (m_bUpdatingWidgets || !HasMissionBook(s_iActiveTab))
			return;
		NIRE_NotepadMissionBook book = GetActiveBook();
		if (!book || row < 0 || row >= book.m_aMissions.Count())
			return;

		if (row != book.m_iActiveMission)
		{
			StoreCurrentNote();
			book.m_iActiveMission = row;
		}
		ApplyTab(HasTemplates(s_iActiveTab));
		SavePersistentData();
	}

	void NewMission()
	{
		if (!HasMissionBook(s_iActiveTab))
			return;
		StoreCurrentNote();
		NIRE_NotepadMissionBook book = GetActiveBook();
		int newIndex = book.m_aMissions.Count();
		book.m_aMissions.Insert(new NIRE_NotepadMission(GetDefaultMissionName(newIndex)));
		book.m_iActiveMission = newIndex;
		ApplyTab();
		SavePersistentData();

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
			workspace.SetFocusedWidget(m_MissionName);
	}

	void DeleteMission()
	{
		NIRE_NotepadMissionBook book = GetActiveBook();
		NIRE_NotepadMission mission = GetActiveMission();
		if (!book || !mission)
			return;

		if (!m_bDeletePending)
		{
			m_bDeletePending = true;
			m_Status.SetText(WidgetManager.Translate("#NIRE-Status_ConfirmDelete", GetMissionListName(mission)));
			return;
		}

		int deletedIndex = book.m_iActiveMission;
		book.m_aMissions.Remove(deletedIndex);
		if (deletedIndex >= book.m_aMissions.Count())
			book.m_iActiveMission = book.m_aMissions.Count() - 1;
		EnsureMission(s_iActiveTab);
		ApplyTab();
		m_Status.SetText("#NIRE-Status_EntryDeleted");
		SavePersistentData();
	}

	void SetEditorFocused(bool focused)
	{
		if (!m_EditorFocus)
			return;

		if (focused)
			m_EditorFocus.SetColor(Color.FromSRGBA(255, 198, 45, 255));
		else
			m_EditorFocus.SetColor(Color.FromSRGBA(120, 120, 120, 210));
	}

	void HoverMission(int row, bool hovered)
	{
		if (row < 0 || row >= m_aMissionRows.Count())
			return;

		if (row == GetActiveBook().m_iActiveMission)
			return;

		if (hovered)
			m_aMissionRows[row].SetColor(Color.FromSRGBA(112, 83, 28, 255));
		else
			m_aMissionRows[row].SetColor(Color.FromSRGBA(36, 36, 36, 240));
	}

	void ReleaseMapFocus()
	{
		ScheduleEditorDeactivation();
	}

	void HandlePointerDown(int x, int y)
	{
		if (IsPointerInsideWidget(m_EditorScroll, x, y))
			return;

		foreach (MultilineEditBoxWidget editor : m_aTemplateEditors)
		{
			if (IsPointerInsideWidget(editor, x, y))
				return;
		}
		foreach (EditBoxWidget lineEditor : m_aLineEditors)
		{
			if (IsPointerInsideWidget(lineEditor, x, y))
				return;
		}

		ScheduleEditorDeactivation();
	}

	protected void HandleGlobalPointerDown()
	{
		bool writing = m_Editor && m_Editor.IsInWriteMode();
		foreach (MultilineEditBoxWidget editor : m_aTemplateEditors)
			writing = writing || editor.IsInWriteMode();
		foreach (EditBoxWidget lineEditor : m_aLineEditors)
			writing = writing || lineEditor.IsInWriteMode();
		if (!writing)
			return;

		int x, y;
		WidgetManager.GetMousePos(x, y);
		HandlePointerDown(x, y);
	}

	protected bool IsPointerInsideWidget(Widget widget, int x, int y)
	{
		if (!widget || !widget.IsVisible())
			return false;

		float widgetX, widgetY, widgetWidth, widgetHeight;
		widget.GetScreenPos(widgetX, widgetY);
		widget.GetScreenSize(widgetWidth, widgetHeight);
		return x >= widgetX && x <= widgetX + widgetWidth && y >= widgetY && y <= widgetY + widgetHeight;
	}

	void ScheduleEditorDeactivation()
	{
		GetGame().GetCallqueue().Remove(DeactivateEditor);
		GetGame().GetCallqueue().CallLater(DeactivateEditor, 1, false);
	}

	void DeactivateEditor()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace && workspace.GetFocusedWidget() != m_MissionName && !m_aTabButtons.IsEmpty())
			workspace.SetFocusedWidget(m_aTabButtons[s_iActiveTab]);

		SetEditorFocused(false);
	}

	bool HandleBackControl()
	{
		CloseFromBack();
		return true;
	}

	void HoverDone(bool hovered)
	{
		if (!m_CloseButton)
			return;

		if (hovered)
			m_CloseButton.SetColor(Color.FromSRGBA(170, 126, 30, 255));
		else
			m_CloseButton.SetColor(Color.FromSRGBA(36, 36, 36, 240));
	}

	void SetDrawingTool(bool eraser)
	{
		m_bEraser = eraser;
		m_bClearDrawingPending = false;
		UpdateDrawingToolColors();
	}

	void SetDrawingColor(int colorIndex)
	{
		if (colorIndex < 0 || colorIndex >= DRAWING_COLOR_COUNT)
			return;

		m_iDrawingColor = colorIndex;
		m_bEraser = false;
		m_bClearDrawingPending = false;
		UpdateDrawingToolColors();
	}

	void ClearDrawing()
	{
		if (s_aDrawingEnds.IsEmpty())
		{
			m_Status.SetText("#NIRE-Status_DrawingCleared");
			return;
		}

		if (!m_bClearDrawingPending)
		{
			m_bClearDrawingPending = true;
			m_Status.SetText("#NIRE-Status_ConfirmClearDrawing");
			return;
		}

		m_bClearDrawingPending = false;
		s_aDrawingVertices.Clear();
		s_aDrawingEnds.Clear();
		s_aDrawingModes.Clear();
		DrawTerrain();
		SavePersistentData();
		m_Status.SetText("#NIRE-Status_DrawingCleared");
	}

	void StartDrawing(int x, int y, int button)
	{
		if (button != 0 || s_iActiveTab != TERRAIN_TAB || m_bDrawing)
			return;

		float canvasX, canvasY, width, height;
		m_TerrainCanvas.GetScreenPos(canvasX, canvasY);
		m_TerrainCanvas.GetScreenSize(width, height);
		if (width <= 0 || height <= 0 || x < canvasX || x > canvasX + width || y < canvasY || y > canvasY + height)
			return;

		m_bDrawing = true;
		if (m_bEraser)
		{
			EraseDrawingAt(x, y);
			GetGame().GetCallqueue().CallLater(SampleDrawing, 16, true);
			return;
		}

		s_aDrawingModes.Insert(m_iDrawingColor);
		s_aDrawingEnds.Insert(s_aDrawingVertices.Count());
		m_fLastDrawingX = (x - canvasX) / width;
		m_fLastDrawingY = (y - canvasY) / height;
		s_aDrawingVertices.Insert(m_fLastDrawingX);
		s_aDrawingVertices.Insert(m_fLastDrawingY);
		s_aDrawingVertices.Insert(Math.Min(m_fLastDrawingX + 1.0 / width, 1.0));
		s_aDrawingVertices.Insert(m_fLastDrawingY);
		s_aDrawingEnds[s_aDrawingEnds.Count() - 1] = s_aDrawingVertices.Count();
		DrawTerrain();
		GetGame().GetCallqueue().CallLater(SampleDrawing, 16, true);
	}

	void StopDrawing(int x, int y, int button)
	{
		if (button != 0 || !m_bDrawing)
			return;

		AddDrawingPoint(x, y);
		m_bDrawing = false;
		GetGame().GetCallqueue().Remove(SampleDrawing);
		SavePersistentData();
	}

	protected void SampleDrawing()
	{
		int x, y;
		WidgetManager.GetMousePos(x, y);
		AddDrawingPoint(x, y);
	}

	protected void AddDrawingPoint(int x, int y)
	{
		if (!m_bDrawing || !m_TerrainCanvas)
			return;
		if (m_bEraser)
		{
			EraseDrawingAt(x, y);
			return;
		}

		float canvasX, canvasY, width, height;
		m_TerrainCanvas.GetScreenPos(canvasX, canvasY);
		m_TerrainCanvas.GetScreenSize(width, height);
		if (width <= 0 || height <= 0 || x < canvasX || x > canvasX + width || y < canvasY || y > canvasY + height)
			return;

		float drawingX = (x - canvasX) / width;
		float drawingY = (y - canvasY) / height;
		float deltaX = drawingX - m_fLastDrawingX;
		float deltaY = drawingY - m_fLastDrawingY;
		if (deltaX * deltaX + deltaY * deltaY < 0.000004)
			return;

		s_aDrawingVertices.Insert(drawingX);
		s_aDrawingVertices.Insert(drawingY);
		s_aDrawingEnds[s_aDrawingEnds.Count() - 1] = s_aDrawingVertices.Count();
		m_fLastDrawingX = drawingX;
		m_fLastDrawingY = drawingY;
		DrawTerrain();
	}

	protected void EraseDrawingAt(int x, int y)
	{
		float canvasX, canvasY, width, height;
		m_TerrainCanvas.GetScreenPos(canvasX, canvasY);
		m_TerrainCanvas.GetScreenSize(width, height);
		if (width <= 0 || height <= 0 || x < canvasX || x > canvasX + width || y < canvasY || y > canvasY + height)
			return;

		float localX = x - canvasX;
		float localY = y - canvasY;
		bool changed;
		for (int stroke = s_aDrawingEnds.Count() - 1; stroke >= 0; stroke--)
		{
			int start;
			if (stroke > 0)
				start = s_aDrawingEnds[stroke - 1];
			int end = s_aDrawingEnds[stroke];
			bool hit;
			for (int vertex = start + 2; vertex < end; vertex += 2)
			{
				float startX = s_aDrawingVertices[vertex - 2] * width;
				float startY = s_aDrawingVertices[vertex - 1] * height;
				float segmentX = s_aDrawingVertices[vertex] * width - startX;
				float segmentY = s_aDrawingVertices[vertex + 1] * height - startY;
				float segmentLength = segmentX * segmentX + segmentY * segmentY;
				float position;
				if (segmentLength > 0)
					position = Math.Clamp(((localX - startX) * segmentX + (localY - startY) * segmentY) / segmentLength, 0, 1);
				float deltaX = startX + position * segmentX - localX;
				float deltaY = startY + position * segmentY - localY;
				if (deltaX * deltaX + deltaY * deltaY <= 225)
				{
					hit = true;
					break;
				}
			}

			if (!hit)
				continue;

			int removed = end - start;
			for (int index = 0; index < removed; index++)
				s_aDrawingVertices.Remove(start);
			s_aDrawingEnds.Remove(stroke);
			s_aDrawingModes.Remove(stroke);
			for (int following = stroke; following < s_aDrawingEnds.Count(); following++)
				s_aDrawingEnds[following] = s_aDrawingEnds[following] - removed;
			changed = true;
		}

		if (changed)
			DrawTerrain();
	}

	protected void DrawTerrain()
	{
		if (!m_TerrainCanvas)
			return;

		float width, height;
		m_TerrainCanvas.GetScreenSize(width, height);
		if (width <= 0 || height <= 0)
		{
			GetGame().GetCallqueue().CallLater(DrawTerrain, 16, false);
			return;
		}

		m_aTerrainCommands.Clear();
		int start;
		foreach (int stroke, int end : s_aDrawingEnds)
		{
			ref LineDrawCommand line = new LineDrawCommand();
			switch (s_aDrawingModes[stroke])
			{
				case 1: line.m_iColor = 0xFFE02020; break;
				case 2: line.m_iColor = 0xFF2078F0; break;
				case 3: line.m_iColor = 0xFF20CC50; break;
				case 4: line.m_iColor = 0xFFFFD020; break;
				default: line.m_iColor = 0xFFFFFFFF;
			}
			line.m_fWidth = 3;
			line.m_Vertices = {};
			for (int vertex = start; vertex < end; vertex += 2)
			{
				line.m_Vertices.Insert(s_aDrawingVertices[vertex] * width);
				line.m_Vertices.Insert(s_aDrawingVertices[vertex + 1] * height);
			}

			m_aTerrainCommands.Insert(line);
			start = end;
		}

		m_TerrainCanvas.SetDrawCommands(m_aTerrainCommands);
	}

	protected void UpdateDrawingToolColors()
	{
		if (!m_PenButton || !m_EraserButton)
			return;

		if (m_bEraser)
		{
			m_PenButton.SetColor(Color.FromSRGBA(36, 36, 36, 240));
			m_EraserButton.SetColor(Color.FromSRGBA(170, 126, 30, 255));
		}
		else
		{
			m_PenButton.SetColor(Color.FromSRGBA(170, 126, 30, 255));
			m_EraserButton.SetColor(Color.FromSRGBA(36, 36, 36, 240));
		}

		foreach (int colorIndex, ButtonWidget button : m_aColorButtons)
		{
			if (!m_bEraser && colorIndex == m_iDrawingColor)
				button.SetOpacity(1);
			else
				button.SetOpacity(0.45);
		}
	}

	void RequestSecondaryTemplate()
	{
		if (s_iActiveTab == 2)
			RequestTemplate(7);
		else
			RequestTemplate(9);
	}

	void RequestTemplate(int lineCount)
	{
		NIRE_NotepadMission mission = GetActiveMission();
		if (!m_Editor || !mission || !IsTemplateAvailable(s_iActiveTab, lineCount))
			return;
		m_bDeletePending = false;

		string templateText = GetTemplate(s_iActiveTab, lineCount);
		if (templateText.IsEmpty())
			return;
		if ((!mission.m_sTemplateText.IsEmpty() || !mission.m_sText.IsEmpty()) && m_iPendingTemplateLines != lineCount)
		{
			m_iPendingTemplateLines = lineCount;
			m_Status.SetText(WidgetManager.Translate("#NIRE-Status_ConfirmTemplate", string.Format("%1", lineCount)));
			return;
		}

		m_iPendingTemplateLines = 0;
		mission.m_iTemplateLines = lineCount;
		mission.m_aTemplatePrompts.Clear();
		mission.m_aTemplateAnswers.Clear();
		templateText.Split("\n", mission.m_aTemplatePrompts, false);
		foreach (string prompt : mission.m_aTemplatePrompts)
			mission.m_aTemplateAnswers.Insert(string.Empty);
		SyncTemplateStrings(mission);
		SavePersistentData();
		SetTemplateMenuVisible(false);
		SetContentVisible(true);
		SetEditorLayout(true, true);
		m_Status.SetText("#NIRE-Status_TemplateInserted");
	}

	void SelectFreeText()
	{
		NIRE_NotepadMission mission = GetActiveMission();
		if (!mission)
			return;

		m_bDeletePending = false;
		m_iPendingTemplateLines = 0;
		mission.m_iTemplateLines = 0;
		mission.m_sTemplateText = string.Empty;
		mission.m_aTemplatePrompts.Clear();
		mission.m_aTemplateAnswers.Clear();
		m_bUpdatingWidgets = true;
		SetLineEditorsText(string.Empty);
		m_bUpdatingWidgets = false;
		StoreCurrentNote();
		SavePersistentData();
		SetTemplateMenuVisible(false);
		SetContentVisible(true);
		SetEditorLayout(true, false);
		m_Status.SetText("#NIRE-Status_FreeText");
	}

	void CloseFromBack()
	{
		NIRE_NotepadMenu.CloseFromBack();
	}

	void Close()
	{
		if (m_bMapOverlay)
		{
			CloseMapOverlay();
			return;
		}

		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager)
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.NIRE_Notepad);
	}

	protected void DrawBackground()
	{
		if (!m_Background)
			return;

		float width, height;
		m_Background.GetScreenSize(width, height);
		if (width <= 0 || height <= 0)
		{
			GetGame().GetCallqueue().CallLater(DrawBackground, 16, false);
			return;
		}

		ref array<float> fillVertices = {};
		ref array<float> outlineVertices = {};
		m_Background.TessellateRoundedRectangle(Vector(0, 0, 0), Vector(width, height, 0), 20, 6, RectangleCorner.All, fillVertices);
		m_Background.TessellateRoundedRectangle(Vector(2.5, 2.5, 0), Vector(width - 5, height - 5, 0), 17.5, 6, RectangleCorner.All, outlineVertices);
		ref PolygonDrawCommand fill = new PolygonDrawCommand();
		fill.m_iColor = 0xCC000000;
		fill.m_Vertices = fillVertices;
		ref LineDrawCommand outline = new LineDrawCommand();
		outline.m_iColor = 0xFFAA7E1E;
		outline.m_fWidth = 5;
		outline.m_bShouldEnclose = true;
		outline.m_Vertices = outlineVertices;
		m_aBackgroundCommands.Clear();
		m_aBackgroundCommands.Insert(fill);
		m_aBackgroundCommands.Insert(outline);
		m_Background.SetDrawCommands(m_aBackgroundCommands);
	}

	protected void StoreCurrentNote()
	{
		if (!m_bInitialized || !m_Editor)
			return;

		if (s_iActiveTab == 0)
		{
			s_sGeneralNote = GetLineEditorsText();
			return;
		}

		NIRE_NotepadMission mission = GetActiveMission();
		if (mission && mission.m_iTemplateLines > 0)
			SyncTemplateStrings(mission);
		else if (mission)
			mission.m_sText = GetLineEditorsText();
	}

	protected static void NormalizeTemplateFields(notnull NIRE_NotepadMission mission)
	{
		if (mission.m_aTemplatePrompts.IsEmpty() && !mission.m_sTemplateText.IsEmpty())
			mission.m_sTemplateText.Split("\n", mission.m_aTemplatePrompts, false);
		if (mission.m_aTemplateAnswers.IsEmpty() && !mission.m_sText.IsEmpty())
			mission.m_sText.Split("\n", mission.m_aTemplateAnswers, false);

		while (mission.m_aTemplateAnswers.Count() < mission.m_aTemplatePrompts.Count())
			mission.m_aTemplateAnswers.Insert(string.Empty);
		while (mission.m_aTemplateAnswers.Count() > mission.m_aTemplatePrompts.Count())
			mission.m_aTemplateAnswers.RemoveOrdered(mission.m_aTemplateAnswers.Count() - 1);
		SyncTemplateStrings(mission);
	}

	protected static void SyncTemplateStrings(notnull NIRE_NotepadMission mission)
	{
		mission.m_sTemplateText = JoinTemplateFields(mission.m_aTemplatePrompts);
		mission.m_sText = JoinTemplateFields(mission.m_aTemplateAnswers);
	}

	protected static string JoinTemplateFields(notnull array<string> fields)
	{
		string value;
		foreach (int index, string field : fields)
		{
			if (index > 0)
				value += "\n";
			value += field;
		}
		return value;
	}

	protected static string EncodeTemplateFields(notnull array<string> fields)
	{
		string value;
		foreach (string field : fields)
			value += string.Format("%1:", field.Length()) + field;
		return value;
	}

	protected static bool DecodeTemplateFields(string value, notnull array<string> fields)
	{
		fields.Clear();
		int offset;
		while (offset < value.Length())
		{
			string remaining = value.Substring(offset, value.Length() - offset);
			int separator = remaining.IndexOf(":");
			if (separator < 1)
				return false;

			string lengthText = remaining.Substring(0, separator);
			int fieldLength = lengthText.ToInt();
			if (fieldLength < 0 || lengthText != string.Format("%1", fieldLength))
				return false;

			offset += separator + 1;
			if (offset + fieldLength > value.Length())
				return false;
			fields.Insert(value.Substring(offset, fieldLength));
			offset += fieldLength;
		}
		return true;
	}

	protected void RefreshMissionList()
	{
		NIRE_NotepadMissionBook book = GetActiveBook();
		ClearMissionRows();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		foreach (int index, NIRE_NotepadMission mission : book.m_aMissions)
		{
			Widget widget = workspace.CreateWidgets(MISSION_ROW_LAYOUT, m_MissionList);
			if (!widget)
				continue;

			ButtonWidget row = ButtonWidget.Cast(widget);
			TextWidget text = TextWidget.Cast(widget.FindAnyWidget("MissionRowText"));
			if (!row || !text)
				continue;

			row.SetUserID(index);
			row.AddHandler(m_Handler);
			text.SetText(GetMissionListName(mission));
			m_aMissionRows.Insert(row);
			m_aMissionRowTexts.Insert(text);
		}

		UpdateMissionRowColors();
	}

	protected void ClearMissionRows()
	{
		Widget child = m_MissionList.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			child.RemoveFromHierarchy();
			child = next;
		}

		m_aMissionRows.Clear();
		m_aMissionRowTexts.Clear();
	}

	protected void UpdateMissionRowColors()
	{
		NIRE_NotepadMissionBook book = GetActiveBook();
		foreach (int index, ButtonWidget row : m_aMissionRows)
		{
			if (index == book.m_iActiveMission)
				row.SetColor(Color.FromSRGBA(170, 126, 30, 255));
			else
				row.SetColor(Color.FromSRGBA(36, 36, 36, 240));
		}
	}

	protected string GetMissionListName(notnull NIRE_NotepadMission mission)
	{
		if (mission.m_sName.IsEmpty())
			return WidgetManager.Translate("#NIRE-Name_Unnamed");

		return mission.m_sName;
	}

	protected string GetDefaultMissionName(int index)
	{
		return GetDefaultMissionNameForTab(s_iActiveTab, index);
	}

	protected static void EnsureDataLoaded()
	{
		if (s_bDataLoaded)
			return;

		s_bDataLoaded = true;
		CreateEmptyBooks();
		if (LoadPersistentData(PROFILE_FILE))
		{
			s_bPrimaryProfileValid = true;
			return;
		}

		if (LoadPersistentData(PROFILE_TEMP))
		{
			FileIO.CopyFile(PROFILE_TEMP, PROFILE_BACKUP);
			return;
		}

		if (!LoadPersistentData(PROFILE_BACKUP))
			Print("NIRE: No valid saved notepad found; starting empty", LogLevel.NORMAL);
	}

	protected static void CreateEmptyBooks()
	{
		s_aMissionBooks.Clear();
		s_aDrawingVertices.Clear();
		s_aDrawingEnds.Clear();
		s_aDrawingModes.Clear();
		for (int tab = 0; tab < TAB_COUNT; tab++)
			s_aMissionBooks.Insert(new NIRE_NotepadMissionBook());
	}

	protected static void EnsureMission(int tabIndex)
	{
		if (!HasMissionBook(tabIndex) || tabIndex >= s_aMissionBooks.Count())
			return;

		NIRE_NotepadMissionBook book = s_aMissionBooks[tabIndex];
		if (book.m_aMissions.IsEmpty())
			book.m_aMissions.Insert(new NIRE_NotepadMission(GetDefaultMissionNameForTab(tabIndex, 0)));

		if (book.m_iActiveMission < 0 || book.m_iActiveMission >= book.m_aMissions.Count())
			book.m_iActiveMission = 0;
	}

	protected NIRE_NotepadMissionBook GetActiveBook()
	{
		if (!HasMissionBook(s_iActiveTab) || s_iActiveTab >= s_aMissionBooks.Count())
			return null;

		return s_aMissionBooks[s_iActiveTab];
	}

	protected NIRE_NotepadMission GetActiveMission()
	{
		NIRE_NotepadMissionBook book = GetActiveBook();
		if (!book || book.m_iActiveMission < 0 || book.m_iActiveMission >= book.m_aMissions.Count())
			return null;

		return book.m_aMissions[book.m_iActiveMission];
	}

	protected static void SavePersistentData()
	{
		if (!s_bDataLoaded)
			return;

		JsonSaveContext context = new JsonSaveContext();
		if (!context.WriteValue("version", PROFILE_VERSION) || !context.WriteValue("activeTab", s_iActiveTab) || !context.WriteValue("generalNote", s_sGeneralNote) || !context.WriteValue("drawingVertices", s_aDrawingVertices) || !context.WriteValue("drawingEnds", s_aDrawingEnds) || !context.WriteValue("drawingModes", s_aDrawingModes))
			return;

		for (int tab = 1; tab < TAB_COUNT; tab++)
		{
			ref array<string> names = {};
			ref array<string> texts = {};
			ref array<string> templateTexts = {};
			ref array<string> templatePrompts = {};
			ref array<string> templateAnswers = {};
			ref array<int> templates = {};
			NIRE_NotepadMissionBook book = s_aMissionBooks[tab];
			foreach (NIRE_NotepadMission mission : book.m_aMissions)
			{
				names.Insert(mission.m_sName);
				texts.Insert(mission.m_sText);
				templateTexts.Insert(mission.m_sTemplateText);
				templatePrompts.Insert(EncodeTemplateFields(mission.m_aTemplatePrompts));
				templateAnswers.Insert(EncodeTemplateFields(mission.m_aTemplateAnswers));
				templates.Insert(mission.m_iTemplateLines);
			}

			if (!context.WriteValue(string.Format("names%1", tab), names) || !context.WriteValue(string.Format("texts%1", tab), texts) || !context.WriteValue(string.Format("templateTexts%1", tab), templateTexts) || !context.WriteValue(string.Format("templatePrompts%1", tab), templatePrompts) || !context.WriteValue(string.Format("templateAnswers%1", tab), templateAnswers) || !context.WriteValue(string.Format("templates%1", tab), templates) || !context.WriteValue(string.Format("active%1", tab), book.m_iActiveMission))
				return;
		}

		if (!context.SaveToFile(PROFILE_TEMP))
		{
			Print("NIRE: Failed to write notepad profile", LogLevel.ERROR);
			return;
		}

		if (s_bPrimaryProfileValid && FileIO.FileExists(PROFILE_FILE))
			FileIO.CopyFile(PROFILE_FILE, PROFILE_BACKUP);

		if (FileIO.CopyFile(PROFILE_TEMP, PROFILE_FILE))
			s_bPrimaryProfileValid = true;
		else
		{
			s_bPrimaryProfileValid = false;
			Print("NIRE: Failed to install notepad profile; backup retained", LogLevel.ERROR);
		}

		FileIO.DeleteFile(PROFILE_TEMP);
	}

	protected static bool LoadPersistentData(string fileName)
	{
		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromFile(fileName))
			return false;

		int version;
		int activeTab;
		string generalNote;
		if (!context.ReadValue("version", version) || version < 1 || version > PROFILE_VERSION)
			return false;

		int storedTabCount = TAB_COUNT;
		if (version == 1)
			storedTabCount = 5;

		if (!context.ReadValue("activeTab", activeTab) || activeTab < 0 || activeTab >= storedTabCount || !context.ReadValue("generalNote", generalNote))
			return false;

		ref array<float> drawingVertices = {};
		ref array<int> drawingEnds = {};
		ref array<int> drawingModes = {};
		if (version >= 3)
		{
			if (!context.ReadValue("drawingVertices", drawingVertices) || !context.ReadValue("drawingEnds", drawingEnds) || !context.ReadValue("drawingModes", drawingModes) || drawingVertices.Count() % 2 != 0 || drawingEnds.Count() != drawingModes.Count())
				return false;

			int previousEnd;
			foreach (int drawingIndex, int drawingEnd : drawingEnds)
			{
				int maximumMode = DRAWING_COLOR_COUNT - 1;
				if (version == 3)
					maximumMode = 1;
				if (drawingEnd < previousEnd + 4 || drawingEnd > drawingVertices.Count() || drawingEnd % 2 != 0 || drawingModes[drawingIndex] < 0 || drawingModes[drawingIndex] > maximumMode)
					return false;
				previousEnd = drawingEnd;
			}
			if (previousEnd != drawingVertices.Count())
				return false;

			foreach (float drawingVertex : drawingVertices)
			{
				if (drawingVertex < 0 || drawingVertex > 1)
					return false;
			}

			if (version == 3)
			{
				ref array<float> migratedVertices = {};
				ref array<int> migratedEnds = {};
				ref array<int> migratedModes = {};
				int legacyStart;
				foreach (int legacyStroke, int legacyEnd : drawingEnds)
				{
					if (drawingModes[legacyStroke] == 0)
					{
						for (int legacyVertex = legacyStart; legacyVertex < legacyEnd; legacyVertex++)
							migratedVertices.Insert(drawingVertices[legacyVertex]);
						migratedEnds.Insert(migratedVertices.Count());
						migratedModes.Insert(0);
					}
					legacyStart = legacyEnd;
				}
				drawingVertices = migratedVertices;
				drawingEnds = migratedEnds;
				drawingModes = migratedModes;
			}
		}

		ref array<ref NIRE_NotepadMissionBook> loadedBooks = {};
		loadedBooks.Insert(new NIRE_NotepadMissionBook());
		for (int tab = 1; tab < storedTabCount; tab++)
		{
			ref array<string> names = {};
			ref array<string> texts = {};
			ref array<string> templateTexts = {};
			ref array<string> templatePrompts = {};
			ref array<string> templateAnswers = {};
			ref array<int> templates = {};
			int activeMission;
			if (!context.ReadValue(string.Format("names%1", tab), names) || !context.ReadValue(string.Format("texts%1", tab), texts) || !context.ReadValue(string.Format("active%1", tab), activeMission) || names.Count() != texts.Count())
				return false;
			if (version >= 3 && (!context.ReadValue(string.Format("templates%1", tab), templates) || templates.Count() != names.Count()))
				return false;
			if (version >= 5 && (!context.ReadValue(string.Format("templateTexts%1", tab), templateTexts) || templateTexts.Count() != names.Count()))
				return false;
			if (version >= 6 && (!context.ReadValue(string.Format("templatePrompts%1", tab), templatePrompts) || !context.ReadValue(string.Format("templateAnswers%1", tab), templateAnswers) || templatePrompts.Count() != names.Count() || templateAnswers.Count() != names.Count()))
				return false;

			ref NIRE_NotepadMissionBook book = new NIRE_NotepadMissionBook();
			for (int missionIndex = 0; missionIndex < names.Count(); missionIndex++)
			{
				string name = names[missionIndex];
				string legacyIndex = string.Format("%1", missionIndex + 1);
				if (tab == 3 && (name == "Fire Mission " + legacyIndex || name == "Supply Mission " + legacyIndex))
					name = GetDefaultMissionNameForTab(tab, missionIndex);
				else if (tab == 4 && name == "Fire Mission " + legacyIndex)
					name = GetDefaultMissionNameForTab(tab, missionIndex);
				else if (tab == 5 && IsLegacyTransportMissionName(name, legacyIndex))
					name = GetDefaultMissionNameForTab(tab, missionIndex);

				ref NIRE_NotepadMission mission = new NIRE_NotepadMission(name);
				mission.m_sText = texts[missionIndex];
				if (version >= 3)
				{
					mission.m_iTemplateLines = templates[missionIndex];
					if (mission.m_iTemplateLines != 0 && !IsTemplateAvailable(tab, mission.m_iTemplateLines))
						return false;
				}
				if (version >= 5)
					mission.m_sTemplateText = templateTexts[missionIndex];
				else if (mission.m_iTemplateLines > 0)
					mission.m_sTemplateText = GetTemplate(tab, mission.m_iTemplateLines);
				if (tab == 4)
					MigrateLegacyMedevacTemplate(mission);
				if (version >= 6)
				{
					if (!DecodeTemplateFields(templatePrompts[missionIndex], mission.m_aTemplatePrompts) || !DecodeTemplateFields(templateAnswers[missionIndex], mission.m_aTemplateAnswers))
						return false;
				}
				if (mission.m_iTemplateLines > 0)
					NormalizeTemplateFields(mission);
				book.m_aMissions.Insert(mission);
			}

			if (!book.m_aMissions.IsEmpty() && (activeMission < 0 || activeMission >= book.m_aMissions.Count()))
				return false;

			book.m_iActiveMission = activeMission;
			loadedBooks.Insert(book);
		}

		while (loadedBooks.Count() < TAB_COUNT)
			loadedBooks.Insert(new NIRE_NotepadMissionBook());

		s_sGeneralNote = generalNote;
		// A profile written before the logistics screen can still name the retired Logistics tab.
		if (activeTab == LOGISTICS_TAB)
			activeTab = 0;

		s_iActiveTab = activeTab;
		s_aMissionBooks = loadedBooks;
		s_aDrawingVertices = drawingVertices;
		s_aDrawingEnds = drawingEnds;
		s_aDrawingModes = drawingModes;
		return true;
	}

	protected static void MigrateLegacyMedevacTemplate(notnull NIRE_NotepadMission mission)
	{
		if (mission.m_iTemplateLines != 0)
			return;

		if (mission.m_sText == "1. Pickup location: \n2. Number / type of casualties: \n3. Urgency: \n4. Security / marking: \n5. Radio contact: ")
			mission.m_iTemplateLines = 5;
		else if (mission.m_sText == "1. Pickup location: \n2. Radio frequency / call sign: \n3. Patients by urgency: \n4. Special equipment required: \n5. Patients by transport type: \n6. Security at pickup site: \n7. Pickup-site marking: \n8. Patient nationality / status: \n9. Contamination / terrain: ")
			mission.m_iTemplateLines = 9;
		else
			return;

		mission.m_sTemplateText = mission.m_sText;
		mission.m_sText = string.Empty;
	}

	protected static string GetDefaultMissionNameForTab(int tabIndex, int index)
	{
		string number = string.Format("%1", index + 1);
		switch (tabIndex)
		{
			case 3: return WidgetManager.Translate("#NIRE-Name_SupplyRequest", number);
			case 4: return WidgetManager.Translate("#NIRE-Name_Evac", number);
			case 5: return WidgetManager.Translate("#NIRE-Name_EvacTransport", number);
		}

		return WidgetManager.Translate("#NIRE-Name_FireMission", number);
	}

	protected static bool IsLegacyTransportMissionName(string name, string number)
	{
		ref array<string> legacyPrefixes = {
			"Evac / Transport ", "Evakuace / přeprava ", "Evacuación / transporte ", "Évacuation / transport ",
			"Evacuazione / trasporto ", "後送 / 輸送 ", "후송 / 수송 ", "Ewakuacja / transport ",
			"Evacuação / transporte ", "Эвакуация / транспорт ", "Евакуація / транспорт ", "撤离 / 运输 "
		};
		foreach (string prefix : legacyPrefixes)
		{
			if (name == prefix + number)
				return true;
		}

		return false;
	}

	protected void UpdateTabColors()
	{
		foreach (int index, ButtonWidget button : m_aTabButtons)
		{
			if (index == s_iActiveTab)
				button.SetColor(Color.FromSRGBA(170, 126, 30, 255));
			else
				button.SetColor(Color.FromSRGBA(36, 36, 36, 240));
		}
	}

	protected LocalizedString GetTabTitle(int tabIndex)
	{
		switch (tabIndex)
		{
			case 1: return "#NIRE-Title_CAS";
			case 2: return "#NIRE-Title_Artillery";
			case 3: return "#NIRE-Title_Logistics";
			case 4: return "#NIRE-Title_MEDEVAC";
			case 5: return "#NIRE-Title_EvacTransport";
		}

		return "#NIRE-Title_General";
	}

	protected LocalizedString GetMissionBookTitle()
	{
		switch (s_iActiveTab)
		{
			case 4: return "#NIRE-List_Evacs";
			case 5: return "#NIRE-List_EvacTransport";
		}

		return "#NIRE-List_FireMissions";
	}

	protected static bool HasMissionBook(int tabIndex)
	{
		return tabIndex > 0 && tabIndex != LOGISTICS_TAB && tabIndex < TERRAIN_TAB;
	}

	protected static bool HasTemplates(int tabIndex)
	{
		return tabIndex == 1 || tabIndex == 2 || tabIndex == 4;
	}

	protected static bool IsTemplateAvailable(int tabIndex, int lineCount)
	{
		if (tabIndex == 2)
			return lineCount == 5 || lineCount == 7;

		return (tabIndex == 1 || tabIndex == 4) && (lineCount == 5 || lineCount == 9);
	}

	protected static string GetTemplate(int tabIndex, int lineCount)
	{
		if (!IsTemplateAvailable(tabIndex, lineCount))
			return string.Empty;

		ref array<string> templateIds = {
			"#NIRE-Template_CAS5", "#NIRE-Template_CAS9",
			"#NIRE-Template_Artillery5", "#NIRE-Template_Artillery7",
			"#NIRE-Template_MEDEVAC5", "#NIRE-Template_MEDEVAC9"
		};
		int templateIndex = 0;
		if (tabIndex == 2)
			templateIndex = 2;
		else if (tabIndex == 4)
			templateIndex = 4;
		if (lineCount != 5)
			templateIndex++;

		string templateText = WidgetManager.Translate(templateIds[templateIndex]);
		templateText.Replace("\\n", "\n");
		return templateText;
	}
}
