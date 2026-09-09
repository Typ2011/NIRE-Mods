//! Click handler for one arsenal row. The row is a native button so it can carry an item preview,
//! which Mikes UI cannot draw itself.
class NIRE_LogisticsArsenalRowHandler : ScriptedWidgetEventHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;

	void NIRE_LogisticsArsenalRowHandler(NIRE_LogisticsScreen screen, int index)
	{
		m_Screen = screen;
		m_iIndex = index;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_Screen.SelectArsenalItem(m_iIndex);
		return true;
	}
}

//! Click handler for one entry in the request list on the left.
class NIRE_LogisticsRequestRowHandler : ScriptedWidgetEventHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;

	void NIRE_LogisticsRequestRowHandler(NIRE_LogisticsScreen screen, int index)
	{
		m_Screen = screen;
		m_iIndex = index;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_Screen.SelectRequestRow(m_iIndex);
		return true;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_Screen.HoverRequestRow(m_iIndex, true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_Screen.HoverRequestRow(m_iIndex, false);
		return false;
	}
}

//! Quantity editor and remove button of one requested-contents row.
class NIRE_LogisticsContentsRowHandler : ScriptedWidgetEventHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;
	protected PanelWidget m_Highlight;

	void NIRE_LogisticsContentsRowHandler(NIRE_LogisticsScreen screen, int index, PanelWidget highlight)
	{
		m_Screen = screen;
		m_iIndex = index;
		m_Highlight = highlight;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w.GetName() == "SelectedMaterialRemove")
			m_Screen.RemoveMaterial(m_iIndex);
		return true;
	}

	override bool OnChange(Widget w, bool finished)
	{
		if (w.GetName() != "SelectedMaterialQuantity")
			return false;

		EditBoxWidget quantity = EditBoxWidget.Cast(w);
		string value = quantity.GetText();
		if (value.Length() > 3)
		{
			value = value.Substring(0, 3);
			quantity.SetText(value);
		}
		if (finished)
			m_Screen.SetMaterialQuantity(m_iIndex, value.ToInt());
		return false;
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		if (w.GetName() == "SelectedMaterialQuantity")
			m_Highlight.SetColor(Color.FromSRGBA(255, 198, 45, 255));
		return false;
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (w.GetName() == "SelectedMaterialQuantity")
			m_Highlight.SetColor(Color.FromSRGBA(110, 110, 110, 180));
		return false;
	}

	override bool OnWriteModeLeave(Widget w)
	{
		if (w.GetName() == "SelectedMaterialQuantity")
			m_Highlight.SetColor(Color.FromSRGBA(110, 110, 110, 180));
		return false;
	}
}

//! Menu entry of the arsenal faction filter.
class NIRE_LogisticsFactionHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;

	void NIRE_LogisticsFactionHandler(NIRE_LogisticsScreen screen, int index)
	{
		m_Screen = screen;
		m_iIndex = index;
	}

	void Select()
	{
		m_Screen.SelectFaction(m_iIndex);
	}
}

//! One crate prefab offered while a logistician fulfils an accepted request.
class NIRE_LogisticsCrateHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;

	void NIRE_LogisticsCrateHandler(NIRE_LogisticsScreen screen, int index)
	{
		m_Screen = screen;
		m_iIndex = index;
	}

	void Select()
	{
		m_Screen.CreateCrate(m_iIndex);
	}
}

//! Menu shell for the supply request workspace. It is a real menu rather than a bare workspace modal
//! because only an open menu frees the cursor and takes movement off the player; a modal on its own
//! left the player walking around behind the screen.
class NIRE_LogisticsScreenMenu : MenuBase
{
	protected override void OnMenuOpen()
	{
		if (!NIRE_LogisticsScreen.AttachTo(GetRootWidget()))
			Close();
	}

	protected override void OnMenuClose()
	{
		NIRE_LogisticsScreen.Detach();
	}
}

//! Full-screen supply request workspace.
//! Built on the same Mikes UI card and native-viewport pattern as the InventoryBoxes Game Master
//! crate editor, because a request needs as much room for item names as a crate inventory does.
//! Everything is client-side; the shared state still travels through the SCR_PlayerController RPCs.
class NIRE_LogisticsScreen : ScriptedWidgetEventHandler
{
	protected static const ResourceName ARSENAL_ROW_LAYOUT = "{C1E02C785E02616D}UI/layouts/InventoryBoxes/GMInventoryEditorRow.layout";
	protected static const ResourceName REQUEST_ROW_LAYOUT = "{2A7C19E4B6D830F1}UI/layouts/NiRe_Notepad/NIRE_MissionRow.layout";
	protected static const ResourceName CONTENTS_ROW_LAYOUT = "{8D02B4399E3F2A10}UI/layouts/NiRe_Notepad/NIRE_SelectedMaterialRow.layout";
	protected static const ResourceName CRATE_CARD_LAYOUT = "{8D02B4399E3F2A5F}UI/layouts/NiRe_Notepad/NIRE_CrateContentsCard.layout";
	protected static const ResourceName INVENTORY_BOXES_REGISTRY = "{5500189E8072CD5B}Configs/Editor/InventoryBoxes.conf";
	protected static const int MAX_QUANTITY = 999;
	protected static const int NOTE_LINE_COUNT = 4;
	protected static const int NOTE_LINE_LENGTH = 45;
	protected static const int COORDINATE_LENGTH = 7;
	protected static const float CRATE_RANGE = 5.0;

	protected static ref NIRE_LogisticsScreen s_Instance;
	protected static bool s_bSuppressPauseMenu;
	protected static bool s_bConsumedBack;
	protected static bool s_bReopenNotepad;
	protected static bool s_bRestoreOnClose;
	protected static bool s_bReopenMapOverlay;

	protected Widget m_Root;
	protected ref MUI_Runtime m_MikesUI;
	protected ScrollLayoutWidget m_RequestScroll;
	protected VerticalLayoutWidget m_RequestList;
	protected ScrollLayoutWidget m_ArsenalScroll;
	protected VerticalLayoutWidget m_ArsenalList;
	protected ScrollLayoutWidget m_ContentsScroll;
	protected VerticalLayoutWidget m_ContentsList;
	protected ScrollLayoutWidget m_CrateScroll;
	protected VerticalLayoutWidget m_CrateList;
	protected EditBoxWidget m_ModalFocus;

	protected MUI_Panel m_ScreenFrame;
	protected MUI_Panel m_RequestViewport;
	protected MUI_Panel m_ArsenalViewport;
	protected MUI_Panel m_ContentsViewport;
	protected MUI_Panel m_CrateViewport;
	protected MUI_Panel m_CrateOverlay;
	protected MUI_Panel m_FactionMenu;
	protected MUI_Panel m_CrateMenu;
	protected MUI_Label m_ContentsTitle;
	protected MUI_Label m_CoordinateLabel;
	protected MUI_Label m_Status;
	protected MUI_TextField m_Search;
	protected MUI_TextField m_Quantity;
	protected MUI_TextField m_Coordinate;
	protected MUI_Button m_FactionFilter;
	protected MUI_Button m_NewRequestButton;
	protected MUI_Button m_CheckCrateButton;
	protected MUI_Button m_AddButton;
	protected MUI_Button m_PickupButton;
	protected MUI_Button m_DeliveryButton;
	protected MUI_Button m_SubmitButton;
	protected MUI_Button m_AcceptButton;
	protected MUI_Button m_HoldButton;
	protected MUI_Button m_RejectButton;
	protected MUI_Button m_CrateButton;
	protected ref array<MUI_Button> m_aCategoryButtons = {};
	protected ref array<MUI_TextField> m_aNoteFields = {};

	protected ref array<ResourceName> m_aArsenalPrefabs = {};
	protected ref array<string> m_aArsenalLabels = {};
	protected ref array<SCR_EArsenalItemType> m_aArsenalTypes = {};
	protected ref array<SCR_EArsenalItemMode> m_aArsenalModes = {};
	protected ref set<ResourceName> m_GeneralPrefabs = new set<ResourceName>();
	protected ref array<ref set<ResourceName>> m_aFactionPrefabs = {};
	protected ref array<string> m_aFactionLabels = {};
	protected ref array<ref NIRE_LogisticsFactionHandler> m_aFactionHandlers = {};
	protected ref array<ResourceName> m_aCratePrefabs = {};
	protected ref array<MUI_Button> m_aCrateButtons = {};
	protected ref array<ref NIRE_LogisticsCrateHandler> m_aCrateHandlers = {};
	protected ref array<IEntity> m_aNearbyCrates = {};

	protected ref array<ResourceName> m_aMaterialPrefabs = {};
	protected ref array<string> m_aMaterialNames = {};
	protected ref array<int> m_aMaterialQuantities = {};
	protected ref array<ButtonWidget> m_aRequestRows = {};

	protected IBX_EArsenalTab m_eCategory;
	protected int m_iFaction;
	protected int m_iSelectedRequestId = -1;
	protected int m_iSelectedArsenalIndex = -1;
	protected NIRE_ELogisticsDeliveryMode m_eDeliveryMode;
	protected ResourceName m_SelectedWeaponPrefab;
	protected ref set<ResourceName> m_SelectedWeaponAmmunition = new set<ResourceName>();
	protected IEntity m_CrateSearchPlayer;
	protected vector m_vCrateSearchOrigin;
	protected bool m_bDraft = true;
	protected bool m_bUpdatingWidgets;
	protected string m_sLastSearch;

	//------------------------------------------------------------------------------------------------
	static void Open()
	{
		if (s_Instance)
		{
			s_Instance.CloseAndRestore();
			return;
		}

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		// Exactly one layer open at a time. This workspace covers the screen, so the notepad behind it
		// was invisible anyway - but the engine hides a menu under a modal without offering any way to
		// show it again, and an open-but-hidden notepad still swallowed the next Escape. Closing it
		// here and reopening it on the way out keeps the visible state and the menu state agreeing.
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;

		s_bReopenMapOverlay = NIRE_NotepadController.IsMapOverlayOpen();
		s_bReopenNotepad = NIRE_NotepadMenu.CloseIfOpen();
		menuManager.OpenMenu(ChimeraMenuPreset.NIRE_LogisticsMenu, 0, true, false);
	}

	//! Called by the menu shell once the engine has built the layout.
	//------------------------------------------------------------------------------------------------
	static bool AttachTo(Widget root)
	{
		if (!root)
			return false;

		s_Instance = new NIRE_LogisticsScreen();
		if (s_Instance.Init(root))
			return true;

		s_Instance = null;
		RestoreNotepad();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void Detach()
	{
		if (s_Instance)
		{
			s_Instance.Teardown();
			s_Instance = null;
		}

		// MenuBase.Close only queues the close, so the notepad has to come back from here rather than
		// from the caller - otherwise it reopens while this menu is still on screen.
		if (!s_bRestoreOnClose)
		{
			s_bReopenNotepad = false;
			s_bReopenMapOverlay = false;
			return;
		}

		s_bRestoreOnClose = false;
		RestoreNotepad();
	}

	//! Closes the workspace and brings back the notepad it replaced. Used by every deliberate exit -
	//! the CLOSE button, Escape and the notepad toggle key - unlike the plain Close, which is what
	//! shutdown paths such as the end of a game want.
	//------------------------------------------------------------------------------------------------
	void CloseAndRestore()
	{
		s_bRestoreOnClose = true;
		Close();
	}

	//------------------------------------------------------------------------------------------------
	static bool CloseAndRestoreIfOpen()
	{
		if (!s_Instance)
			return false;

		s_Instance.CloseAndRestore();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void RestoreNotepad()
	{
		if (!s_bReopenNotepad)
			return;

		s_bReopenNotepad = false;
		if (s_bReopenMapOverlay)
		{
			s_bReopenMapOverlay = false;
			NIRE_NotepadController.OpenMapOverlay();
			return;
		}

		NIRE_NotepadMenu.OpenNotepad();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOpen()
	{
		return s_Instance != null;
	}

	//------------------------------------------------------------------------------------------------
	static bool CloseIfOpen()
	{
		if (!s_Instance)
			return false;

		s_Instance.Close();
		return true;
	}

	//! Called by the logistics RPCs whenever a request or the local role changes.
	//------------------------------------------------------------------------------------------------
	static void RefreshIfOpen()
	{
		if (s_Instance)
			s_Instance.Refresh();
	}

	//! Answers a back press on behalf of this screen and reports whether it did. Mikes UI raises its
	//! own back on the key going down while the notepad's Escape action arrives on the way up, so
	//! whichever of the two lands second must find the press already spent - otherwise one Escape
	//! closes this workspace and the notepad behind it.
	//------------------------------------------------------------------------------------------------
	static bool ConsumeBack()
	{
		if (s_Instance)
		{
			s_Instance.HandleBack();
			return true;
		}

		if (!s_bConsumedBack)
			return false;

		s_bConsumedBack = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void ClearConsumedBack()
	{
		s_bConsumedBack = false;
	}

	//------------------------------------------------------------------------------------------------
	static bool ConsumePauseSuppression()
	{
		if (!s_bSuppressPauseMenu)
			return false;

		s_bSuppressPauseMenu = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Init(notnull Widget root)
	{
		m_Root = root;
		m_Root.AddHandler(this);
		m_RequestScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("RequestScroll"));
		m_RequestList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("RequestList"));
		m_ArsenalScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("ArsenalScroll"));
		m_ArsenalList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("ArsenalList"));
		m_ContentsScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("ContentsScroll"));
		m_ContentsList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("ContentsList"));
		m_CrateScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("CrateScroll"));
		m_CrateList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("CrateList"));
		m_ModalFocus = EditBoxWidget.Cast(m_Root.FindAnyWidget("ModalFocus"));
		if (!m_RequestScroll || !m_RequestList || !m_ArsenalScroll || !m_ArsenalList || !m_ContentsScroll || !m_ContentsList || !m_CrateScroll || !m_CrateList || !m_ModalFocus)
		{
			Print("NIRE: Logistics screen layout is incomplete", LogLevel.ERROR);
			return false;
		}

		BuildMikesUI();
		if (!m_MikesUI)
		{
			Print("NIRE: Mikes UI could not be mounted", LogLevel.ERROR);
			return false;
		}

		m_Quantity.SetText("1");
		LoadArsenalCatalog();
		LoadCrateOptions();
		SelectCategory(IBX_EArsenalTab.WEAPONS);
		RequestSnapshot();
		StartDraft();
		// StartDraft stops early without logistics access, so the no-access state is drawn here.
		Refresh();
		// The menu already owns the cursor and the input context; the modal is only here to give the
		// Mikes UI text fields a keyboard focus target.
		GetGame().GetWorkspace().AddModal(m_Root, m_ModalFocus);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildMikesUI()
	{
		Widget host = m_Root.FindAnyWidget("MikesUIHost");
		if (!host)
			return;

		m_MikesUI = new MUI_Runtime();
		if (!m_MikesUI.Mount(host))
		{
			m_MikesUI = null;
			return;
		}

		MUI_Panel root = m_MikesUI.CreatePanel("LogisticsRoot");
		root.MakeOverlay();
		root.SetFill(Color.FromInt(0));
		m_ScreenFrame = m_MikesUI.CreatePanel("LogisticsFrame");
		m_ScreenFrame.SetFill(MUI_Theme.Border);
		m_ScreenFrame.SetFillWidth();
		m_ScreenFrame.SetFillHeight();
		m_ScreenFrame.SetRadius(18);
		m_ScreenFrame.SetPadding(2);
		root.AddChild(m_ScreenFrame);

		MUI_Panel card = m_MikesUI.CreatePanel("LogisticsCard");
		card.SetFill(MUI_Theme.DeepFrost);
		card.SetFillWidth();
		card.SetFillHeight();
		card.SetRadius(16);
		card.SetPadding(24);
		card.SetGap(10);
		m_ScreenFrame.AddChild(card);

		BuildHeader(card);
		BuildBody(card);
		BuildRequestFields(card);

		m_Status = m_MikesUI.CreateLabel(string.Empty, "Status");
		m_Status.SetMuted(true);
		m_Status.SetHeight(28);
		card.AddChild(m_Status);

		BuildCrateOverlay(root);

		m_MikesUI.SetRoot(root);
		m_MikesUI.GetOnBack().Insert(HandleBack);
		m_MikesUI.Tick(0);
		SyncNativeViewports();
		GetGame().GetCallqueue().CallLater(TickMikesUI, 16, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildHeader(notnull MUI_Panel card)
	{
		MUI_Row header = m_MikesUI.CreateRow("Header");
		header.SetFillWidth();
		header.SetHeight(48);
		header.SetGap(12);
		card.AddChild(header);

		MUI_Label title = m_MikesUI.CreateLabel(Translate("#NIRE-Title_Logistics"), "Title");
		title.SetFontSize(MUI_Theme.FONT_TITLE);
		title.SetBold(true);
		title.SetFillWidth();
		title.SetGrow(1);
		title.SetAlign(0, 1);
		header.AddChild(title);

		MUI_Button close = m_MikesUI.CreateButton(Translate("#NIRE-Button_Done"), "Close");
		close.SetWidth(140);
		close.SetGrow(0);
		close.SetAlign(0, 1);
		close.GetOnClicked().Insert(CloseAndRestore);
		header.AddChild(close);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildBody(notnull MUI_Panel card)
	{
		MUI_Row body = m_MikesUI.CreateRow("Body");
		body.SetFillWidth();
		body.SetHeight(1);
		body.SetGrow(1);
		body.SetGap(20);
		card.AddChild(body);

		// The request list only carries an id and a status. The arsenal column is the widest because
		// it holds the search field and the unabbreviated item names; the contents column repeats
		// those names next to a three-digit quantity, so it needs less.
		MUI_Panel requests = CreateColumn("RequestColumn", 1);
		MUI_Panel arsenal = CreateColumn("ArsenalColumn", 3);
		MUI_Panel contents = CreateColumn("ContentsColumn", 2);
		body.AddChild(requests);
		body.AddChild(arsenal);
		body.AddChild(contents);

		MUI_Label requestTitle = m_MikesUI.CreateLabel(Translate("#NIRE-List_SupplyRequestsStatus"), "RequestTitle");
		requestTitle.SetBold(true);
		requestTitle.SetHeight(28);
		requests.AddChild(requestTitle);
		m_RequestViewport = CreateViewport("RequestViewport");
		requests.AddChild(m_RequestViewport);
		MUI_Row requestActions = m_MikesUI.CreateRow("RequestActions");
		requestActions.SetFillWidth();
		requestActions.SetHeight(56);
		requestActions.SetGap(10);
		requests.AddChild(requestActions);
		m_NewRequestButton = m_MikesUI.CreateButton(Translate("#NIRE-Button_NewRequest"), "NewRequest");
		m_NewRequestButton.MakeAccent();
		m_NewRequestButton.SetGrow(1);
		m_NewRequestButton.GetOnClicked().Insert(StartDraft);
		requestActions.AddChild(m_NewRequestButton);
		m_CheckCrateButton = m_MikesUI.CreateButton(Translate("#NIRE-Button_CompareCrate"), "CheckCrate");
		m_CheckCrateButton.SetGrow(1);
		m_CheckCrateButton.GetOnClicked().Insert(ShowNearbyCrates);
		requestActions.AddChild(m_CheckCrateButton);

		MUI_Label arsenalTitle = m_MikesUI.CreateLabel(Translate("#NIRE-Logistics_Material"), "ArsenalTitle");
		arsenalTitle.SetBold(true);
		arsenalTitle.SetHeight(28);
		arsenal.AddChild(arsenalTitle);

		MUI_Row tabs = m_MikesUI.CreateRow("Categories");
		tabs.SetFillWidth();
		tabs.SetHeight(44);
		tabs.SetGap(6);
		arsenal.AddChild(tabs);
		MUI_Button tab = CreateCategoryButton(tabs, "#NIRE-Selector_Weapons");
		tab.GetOnClicked().Insert(SelectWeapons);
		tab = CreateCategoryButton(tabs, "#NIRE-Selector_Ammunition");
		tab.GetOnClicked().Insert(SelectAmmunition);
		tab = CreateCategoryButton(tabs, "#NIRE-Selector_Clothing");
		tab.GetOnClicked().Insert(SelectClothing);
		tab = CreateCategoryButton(tabs, "#NIRE-Selector_Medical");
		tab.GetOnClicked().Insert(SelectMedical);
		tab = CreateCategoryButton(tabs, "#NIRE-Selector_Explosives");
		tab.GetOnClicked().Insert(SelectExplosives);
		tab = CreateCategoryButton(tabs, "#NIRE-Selector_Equipment");
		tab.GetOnClicked().Insert(SelectEquipment);

		MUI_Row filters = m_MikesUI.CreateRow("Filters");
		filters.SetFillWidth();
		filters.SetHeight(74);
		filters.SetGap(10);
		arsenal.AddChild(filters);
		m_Search = m_MikesUI.CreateTextField(Translate("#NIRE-Logistics_Search"), "Search");
		m_Search.SetFillWidth();
		m_Search.SetGrow(1);
		filters.AddChild(m_Search);
		m_FactionFilter = m_MikesUI.CreateButton(Translate("#NIRE-Selector_AllFactions"), "FactionFilter");
		m_FactionFilter.SetWidth(220);
		m_FactionFilter.SetGrow(0);
		m_FactionFilter.SetAlign(0, 1);
		m_FactionFilter.GetOnClicked().Insert(ToggleFactionMenu);
		filters.AddChild(m_FactionFilter);

		m_FactionMenu = m_MikesUI.CreatePanel("FactionMenu");
		m_FactionMenu.SetVisible(false);
		arsenal.AddChild(m_FactionMenu);

		m_ArsenalViewport = CreateViewport("ArsenalViewport");
		arsenal.AddChild(m_ArsenalViewport);

		MUI_Row addRow = m_MikesUI.CreateRow("AddRow");
		addRow.SetFillWidth();
		addRow.SetHeight(74);
		addRow.SetGap(10);
		arsenal.AddChild(addRow);
		m_Quantity = m_MikesUI.CreateTextField(Translate("#NIRE-Logistics_Quantity"), "Quantity");
		m_Quantity.SetWidth(150);
		m_Quantity.SetGrow(0);
		addRow.AddChild(m_Quantity);
		m_AddButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Add"), "Add");
		m_AddButton.MakeAccent();
		m_AddButton.SetGrow(1);
		m_AddButton.SetAlign(0, 1);
		m_AddButton.GetOnClicked().Insert(AddSelectedMaterial);
		addRow.AddChild(m_AddButton);

		m_ContentsTitle = m_MikesUI.CreateLabel(string.Empty, "ContentsTitle");
		m_ContentsTitle.SetBold(true);
		m_ContentsTitle.SetHeight(28);
		contents.AddChild(m_ContentsTitle);
		m_CrateMenu = m_MikesUI.CreatePanel("CrateMenu");
		m_CrateMenu.SetVisible(false);
		contents.AddChild(m_CrateMenu);
		m_ContentsViewport = CreateViewport("ContentsViewport");
		contents.AddChild(m_ContentsViewport);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildRequestFields(notnull MUI_Panel card)
	{
		MUI_Row detail = m_MikesUI.CreateRow("Detail");
		detail.SetFillWidth();
		detail.SetHeight(74);
		detail.SetGap(10);
		card.AddChild(detail);
		m_PickupButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Pickup"), "Pickup");
		m_PickupButton.SetGrow(1);
		m_PickupButton.SetAlign(0, 1);
		m_PickupButton.GetOnClicked().Insert(SelectPickup);
		detail.AddChild(m_PickupButton);
		m_DeliveryButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Delivery"), "Delivery");
		m_DeliveryButton.SetGrow(1);
		m_DeliveryButton.SetAlign(0, 1);
		m_DeliveryButton.GetOnClicked().Insert(SelectDelivery);
		detail.AddChild(m_DeliveryButton);
		// The caption sits in its own label because pickup and delivery name the grid differently and
		// a field's own caption is fixed once it is created.
		m_CoordinateLabel = m_MikesUI.CreateLabel(Translate("#NIRE-Logistics_PickupCoordinate"), "CoordinateLabel");
		m_CoordinateLabel.SetWidth(260);
		m_CoordinateLabel.SetGrow(0);
		m_CoordinateLabel.SetAlign(0, 1);
		detail.AddChild(m_CoordinateLabel);
		m_Coordinate = m_MikesUI.CreateTextField(string.Empty, "Coordinate");
		m_Coordinate.SetWidth(280);
		m_Coordinate.SetGrow(0);
		detail.AddChild(m_Coordinate);

		// Four fields rather than one box: the wire format is still four lines of 45 characters, and
		// side by side each line now has far more room than the corner notepad could give it.
		MUI_Row notes = m_MikesUI.CreateRow("Notes");
		notes.SetFillWidth();
		notes.SetHeight(74);
		notes.SetGap(10);
		card.AddChild(notes);
		for (int index = 0; index < NOTE_LINE_COUNT; index++)
		{
			MUI_TextField note = m_MikesUI.CreateTextField(string.Format("%1 %2", Translate("#NIRE-Logistics_Note"), index + 1), string.Format("Note%1", index));
			note.SetFillWidth();
			note.SetGrow(1);
			notes.AddChild(note);
			m_aNoteFields.Insert(note);
		}

		MUI_Row actions = m_MikesUI.CreateRow("Actions");
		actions.SetFillWidth();
		actions.SetHeight(56);
		actions.SetGap(10);
		card.AddChild(actions);
		m_SubmitButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Submit"), "Submit");
		m_SubmitButton.MakeAccent();
		m_SubmitButton.SetGrow(1);
		m_SubmitButton.GetOnClicked().Insert(SubmitRequest);
		actions.AddChild(m_SubmitButton);
		m_AcceptButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Accept"), "Accept");
		m_AcceptButton.SetGrow(1);
		m_AcceptButton.GetOnClicked().Insert(AdvanceRequest);
		actions.AddChild(m_AcceptButton);
		m_HoldButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Hold"), "Hold");
		m_HoldButton.SetGrow(1);
		m_HoldButton.GetOnClicked().Insert(HoldRequest);
		actions.AddChild(m_HoldButton);
		m_RejectButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_Reject"), "Reject");
		m_RejectButton.MakeDanger();
		m_RejectButton.SetGrow(1);
		m_RejectButton.GetOnClicked().Insert(RejectRequest);
		actions.AddChild(m_RejectButton);
		m_CrateButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_CreateCrate"), "CreateCrate");
		m_CrateButton.SetGrow(1);
		m_CrateButton.GetOnClicked().Insert(ToggleCrateMenu);
		actions.AddChild(m_CrateButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCrateOverlay(notnull MUI_Panel root)
	{
		m_CrateOverlay = m_MikesUI.CreatePanel("CrateOverlay");
		m_CrateOverlay.MakeOverlay();
		m_CrateOverlay.SetVisible(false);
		root.AddChild(m_CrateOverlay);

		MUI_Panel crateCard = m_MikesUI.CreatePanel("CrateCard");
		crateCard.SetFill(MUI_Theme.DeepFrost);
		crateCard.SetWidth(1100);
		crateCard.SetHeight(760);
		crateCard.SetAlign(0.5, 0.5);
		crateCard.SetRadius(16);
		crateCard.SetPadding(24);
		crateCard.SetGap(12);
		m_CrateOverlay.AddChild(crateCard);

		MUI_Label title = m_MikesUI.CreateLabel(Translate("#NIRE-Crate_Contents"), "CrateTitle");
		title.SetFontSize(MUI_Theme.FONT_TITLE);
		title.SetBold(true);
		title.SetHeight(42);
		crateCard.AddChild(title);

		m_CrateViewport = CreateViewport("CrateViewport");
		crateCard.AddChild(m_CrateViewport);

		MUI_Button close = m_MikesUI.CreateButton(Translate("#NIRE-Button_Done"), "CloseCrates");
		close.SetHeight(54);
		close.SetGrow(0);
		close.SetFillWidth();
		close.GetOnClicked().Insert(HideNearbyCrates);
		crateCard.AddChild(close);
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Panel CreateColumn(string name, int grow)
	{
		MUI_Panel panel = m_MikesUI.CreatePanel(name);
		panel.SetFill(Color.FromInt(0));
		panel.SetWidth(1);
		panel.SetFillHeight();
		panel.SetGrow(grow);
		panel.SetGap(8);
		return panel;
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Panel CreateViewport(string name)
	{
		MUI_Panel panel = m_MikesUI.CreatePanel(name);
		panel.SetFill(MUI_Theme.Field);
		panel.SetFillWidth();
		panel.SetFillHeight();
		panel.SetGrow(1);
		panel.SetRadius(8);
		return panel;
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Button CreateCategoryButton(notnull MUI_Row parent, string label)
	{
		MUI_Button button = m_MikesUI.CreateButton(Translate(label));
		button.SetGrow(1);
		parent.AddChild(button);
		m_aCategoryButtons.Insert(button);
		return button;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Translate(string key)
	{
		return WidgetManager.Translate(key);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickMikesUI()
	{
		if (!m_MikesUI)
			return;

		m_MikesUI.Tick(0.016);
		SyncNativeViewports();
		string search = StripLeadingSpaces(m_Search.GetText());
		if (search != m_Search.GetText())
			m_Search.SetText(search);

		if (search != m_sLastSearch)
		{
			m_sLastSearch = search;
			RefreshArsenalList();
			ScrollArsenalToTop();
		}
	}

	//! The keystroke that activates a Mikes UI text field can also land inside it, so the first
	//! character typed ends up behind a stray space. The search then matches nothing and a quantity
	//! parses as zero. Only the leading spaces go - spaces inside a query such as "5 56" are kept.
	//------------------------------------------------------------------------------------------------
	protected static string StripLeadingSpaces(string text)
	{
		int index;
		while (index < text.Length() && text.Substring(index, 1) == " ")
			index++;

		if (index == 0)
			return text;

		return text.Substring(index, text.Length() - index);
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncNativeViewports()
	{
		bool crates = m_CrateOverlay && m_CrateOverlay.IsVisible();
		SyncNativeViewport(m_RequestScroll, m_RequestViewport, !crates);
		SyncNativeViewport(m_ArsenalScroll, m_ArsenalViewport, !crates);
		SyncNativeViewport(m_ContentsScroll, m_ContentsViewport, !crates);
		SyncNativeViewport(m_CrateScroll, m_CrateViewport, crates);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SyncNativeViewport(Widget widget, MUI_Node viewport, bool visible)
	{
		if (!widget || !viewport)
			return;

		widget.SetVisible(visible);
		widget.SetEnabled(visible);
		if (!visible)
			return;

		MUI_Rect rect = viewport.GetWorldRect();
		FrameSlot.SetAnchorMin(widget, 0, 0);
		FrameSlot.SetAnchorMax(widget, 0, 0);
		FrameSlot.SetPos(widget, rect.m_fX, rect.m_fY);
		FrameSlot.SetSize(widget, rect.m_fW, rect.m_fH);
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestSnapshot()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		if (controller.NIRE_HasLogisticsAccess())
			controller.NIRE_RequestLogisticsSnapshot();
		controller.NIRE_RequestLogisticsAccessSnapshot();
	}

	//------------------------------------------------------------------------------------------------
	void Refresh()
	{
		if (!m_MikesUI)
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		bool hasAccess = controller && controller.NIRE_HasLogisticsAccess();
		m_NewRequestButton.SetEnabled(hasAccess);
		m_CheckCrateButton.SetEnabled(controller && controller.NIRE_IsLogistician());
		if (!hasAccess)
		{
			ClearChildren(m_RequestList);
			ClearChildren(m_ContentsList);
			m_aRequestRows.Clear();
			SetRequestFieldsEnabled(false);
			SetManagementVisible(false);
			m_Status.SetText(Translate("#NIRE-Logistics_NoAccess"));
			return;
		}

		RefreshRequestList();
		NIRE_LogisticsRequest request = GetSelectedRequest();
		if (!m_bDraft && !request)
		{
			StartDraft();
			return;
		}

		if (m_bDraft)
			ShowDraft();
		else
			ShowRequest(request);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshRequestList()
	{
		ClearChildren(m_RequestList);
		m_aRequestRows.Clear();
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!controller || !workspace)
			return;

		foreach (int index, NIRE_LogisticsRequest request : controller.NIRE_GetLogisticsRequests())
		{
			Widget widget = workspace.CreateWidgets(REQUEST_ROW_LAYOUT, m_RequestList);
			ButtonWidget row = ButtonWidget.Cast(widget);
			TextWidget text;
			TextWidget divider;
			TextWidget statusText;
			if (widget)
			{
				text = TextWidget.Cast(widget.FindAnyWidget("MissionRowText"));
				divider = TextWidget.Cast(widget.FindAnyWidget("MissionRowDivider"));
				statusText = TextWidget.Cast(widget.FindAnyWidget("MissionRowStatus"));
			}
			if (!row || !text || !divider || !statusText)
				continue;

			text.SetText(WidgetManager.Translate("#NIRE-Name_SupplyRequest", string.Format("%1", request.m_iId)));
			divider.SetVisible(true);
			statusText.SetText(Translate(GetStatusLabel(request.m_eStatus)));
			statusText.SetVisible(true);
			row.AddHandler(new NIRE_LogisticsRequestRowHandler(this, index));
			m_aRequestRows.Insert(row);
		}

		UpdateRequestRowColors();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRequestRowColors()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		array<ref NIRE_LogisticsRequest> requests = controller.NIRE_GetLogisticsRequests();
		foreach (int index, ButtonWidget row : m_aRequestRows)
		{
			if (index < requests.Count() && requests[index].m_iId == m_iSelectedRequestId)
				row.SetColor(Color.FromSRGBA(170, 126, 30, 255));
			else
				row.SetColor(Color.FromSRGBA(36, 36, 36, 240));
		}
	}

	//------------------------------------------------------------------------------------------------
	void HoverRequestRow(int index, bool hovered)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || index < 0 || index >= m_aRequestRows.Count())
			return;

		array<ref NIRE_LogisticsRequest> requests = controller.NIRE_GetLogisticsRequests();
		if (index < requests.Count() && requests[index].m_iId == m_iSelectedRequestId)
			return;

		if (hovered)
			m_aRequestRows[index].SetColor(Color.FromSRGBA(112, 83, 28, 255));
		else
			m_aRequestRows[index].SetColor(Color.FromSRGBA(36, 36, 36, 240));
	}

	//------------------------------------------------------------------------------------------------
	void SelectRequestRow(int index)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_HasLogisticsAccess())
			return;

		array<ref NIRE_LogisticsRequest> requests = controller.NIRE_GetLogisticsRequests();
		if (index < 0 || index >= requests.Count())
			return;

		NIRE_LogisticsRequest request = requests[index];
		m_bDraft = false;
		m_iSelectedRequestId = request.m_iId;
		m_iSelectedArsenalIndex = -1;
		m_SelectedWeaponPrefab = string.Empty;
		m_SelectedWeaponAmmunition.Clear();
		m_aMaterialPrefabs.Clear();
		m_aMaterialNames.Clear();
		m_aMaterialQuantities.Clear();
		foreach (int materialIndex, ResourceName prefab : request.m_aMaterialPrefabs)
		{
			m_aMaterialPrefabs.Insert(prefab);
			m_aMaterialNames.Insert(request.m_aMaterialNames[materialIndex]);
			m_aMaterialQuantities.Insert(request.m_aMaterialQuantities[materialIndex]);
		}
		m_eDeliveryMode = request.m_eDeliveryMode;
		m_bUpdatingWidgets = true;
		m_Coordinate.SetText(request.m_sCoordinate);
		SetNoteText(request.m_sNote);
		m_bUpdatingWidgets = false;
		RefreshArsenalList();
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected NIRE_LogisticsRequest GetSelectedRequest()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return null;

		foreach (NIRE_LogisticsRequest request : controller.NIRE_GetLogisticsRequests())
		{
			if (request.m_iId == m_iSelectedRequestId)
				return request;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void StartDraft()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_HasLogisticsAccess())
			return;

		m_bDraft = true;
		m_iSelectedRequestId = -1;
		m_iSelectedArsenalIndex = -1;
		m_SelectedWeaponPrefab = string.Empty;
		m_SelectedWeaponAmmunition.Clear();
		m_aMaterialPrefabs.Clear();
		m_aMaterialNames.Clear();
		m_aMaterialQuantities.Clear();
		m_eDeliveryMode = NIRE_ELogisticsDeliveryMode.PICKUP;
		m_bUpdatingWidgets = true;
		m_Coordinate.SetText(string.Empty);
		SetNoteText(string.Empty);
		m_bUpdatingWidgets = false;
		HideNearbyCrates();
		m_CrateMenu.SetVisible(false);
		RefreshArsenalList();
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowDraft()
	{
		SetRequestFieldsEnabled(true);
		m_SubmitButton.SetVisible(true);
		m_SubmitButton.SetEnabled(true);
		SetManagementVisible(false);
		UpdateModeColors();
		UpdateCoordinateCaption();
		RefreshContentsList();
		UpdateRequestRowColors();
		m_Status.SetText(Translate("#NIRE-Logistics_NewRequest"));
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowRequest(notnull NIRE_LogisticsRequest request)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		bool editable = controller && controller.NIRE_IsLogistician();
		SetRequestFieldsEnabled(editable);
		m_SubmitButton.SetVisible(editable);
		m_SubmitButton.SetEnabled(editable);
		if (editable)
			UpdateManagement(request);
		else
			SetManagementVisible(false);
		UpdateModeColors();
		UpdateCoordinateCaption();
		RefreshContentsList();
		UpdateRequestRowColors();
		m_Status.SetText(Translate(GetStatusLabel(request.m_eStatus)));
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanEditRequest()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return controller && controller.NIRE_HasLogisticsAccess() && (m_bDraft || controller.NIRE_IsLogistician());
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRequestFieldsEnabled(bool enabled)
	{
		m_AddButton.SetEnabled(enabled);
		m_Quantity.SetEnabled(enabled);
		m_PickupButton.SetEnabled(enabled);
		m_DeliveryButton.SetEnabled(enabled);
		m_Coordinate.SetEnabled(enabled);
		foreach (MUI_TextField note : m_aNoteFields)
			note.SetEnabled(enabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetManagementVisible(bool visible)
	{
		m_AcceptButton.SetVisible(visible);
		m_AcceptButton.SetEnabled(visible);
		m_HoldButton.SetVisible(visible);
		m_HoldButton.SetEnabled(visible);
		m_RejectButton.SetVisible(visible);
		m_RejectButton.SetEnabled(visible);
		m_CrateButton.SetVisible(visible);
		m_CrateButton.SetEnabled(visible);
		if (!visible)
			m_CrateMenu.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateManagement(notnull NIRE_LogisticsRequest request)
	{
		SetManagementVisible(false);
		m_AcceptButton.SetText(Translate("#NIRE-Logistics_Accept"));
		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.OPEN || request.m_eStatus == NIRE_ELogisticsRequestStatus.HOLD || request.m_eStatus == NIRE_ELogisticsRequestStatus.REJECTED)
		{
			ShowManagementButton(m_AcceptButton);
			ShowManagementButton(m_HoldButton);
			ShowManagementButton(m_RejectButton);
			return;
		}
		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.ACCEPTED)
		{
			ShowManagementButton(m_HoldButton);
			ShowManagementButton(m_RejectButton);
			ShowManagementButton(m_CrateButton);
			return;
		}

		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.READY)
			m_AcceptButton.SetText(Translate("#NIRE-Logistics_StatusDeliveryInTransit"));
		else if (request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT)
			m_AcceptButton.SetText(Translate("#NIRE-Logistics_StatusDeliveryArrived"));
		else if (request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED || request.m_eStatus == NIRE_ELogisticsRequestStatus.PICKUP_READY)
			m_AcceptButton.SetText(Translate("#NIRE-Logistics_StatusCompleted"));
		else
			return;

		ShowManagementButton(m_AcceptButton);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ShowManagementButton(notnull MUI_Button button)
	{
		button.SetVisible(true);
		button.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectPickup()
	{
		SetDeliveryMode(NIRE_ELogisticsDeliveryMode.PICKUP);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectDelivery()
	{
		SetDeliveryMode(NIRE_ELogisticsDeliveryMode.DELIVERY);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetDeliveryMode(NIRE_ELogisticsDeliveryMode mode)
	{
		if (!CanEditRequest())
			return;

		m_eDeliveryMode = mode;
		UpdateModeColors();
		UpdateCoordinateCaption();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModeColors()
	{
		if (m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY)
		{
			m_PickupButton.MakeDefault();
			m_DeliveryButton.MakeAccent();
			return;
		}

		m_PickupButton.MakeAccent();
		m_DeliveryButton.MakeDefault();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCoordinateCaption()
	{
		if (m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY)
			m_CoordinateLabel.SetText(Translate("#NIRE-Logistics_Coordinate"));
		else
			m_CoordinateLabel.SetText(Translate("#NIRE-Logistics_PickupCoordinate"));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetNoteText(string text)
	{
		ref array<string> lines = {};
		text.Split("\n", lines, true);
		foreach (int index, MUI_TextField note : m_aNoteFields)
		{
			string line;
			if (index < NOTE_LINE_COUNT - 1 && index < lines.Count())
				line = lines[index];
			else if (index == NOTE_LINE_COUNT - 1)
			{
				for (int sourceIndex = NOTE_LINE_COUNT - 1; sourceIndex < lines.Count(); sourceIndex++)
				{
					if (!line.IsEmpty())
						line += " / ";
					line += lines[sourceIndex];
				}
			}
			if (line.Length() > NOTE_LINE_LENGTH)
				line = line.Substring(0, NOTE_LINE_LENGTH);
			note.SetText(line);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetNoteText()
	{
		int lastLine = m_aNoteFields.Count() - 1;
		while (lastLine >= 0 && GetNoteLine(lastLine).IsEmpty())
			lastLine--;

		string text;
		for (int index = 0; index <= lastLine; index++)
		{
			if (index > 0)
				text += "\n";
			text += GetNoteLine(index);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetNoteLine(int index)
	{
		string line = m_aNoteFields[index].GetText();
		if (line.Length() > NOTE_LINE_LENGTH)
			line = line.Substring(0, NOTE_LINE_LENGTH);

		return line;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshContentsList()
	{
		ClearChildren(m_ContentsList);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		bool editable = CanEditRequest();
		int totalItems;
		foreach (int index, string materialName : m_aMaterialNames)
		{
			totalItems += m_aMaterialQuantities[index];
			Widget widget = workspace.CreateWidgets(CONTENTS_ROW_LAYOUT, m_ContentsList);
			TextWidget text;
			EditBoxWidget quantity;
			ButtonWidget remove;
			PanelWidget highlight;
			if (widget)
			{
				text = TextWidget.Cast(widget.FindAnyWidget("SelectedMaterialName"));
				quantity = EditBoxWidget.Cast(widget.FindAnyWidget("SelectedMaterialQuantity"));
				remove = ButtonWidget.Cast(widget.FindAnyWidget("SelectedMaterialRemove"));
				highlight = PanelWidget.Cast(widget.FindAnyWidget("SelectedMaterialQuantityHighlight"));
			}
			if (!text || !quantity || !remove || !highlight)
				continue;

			text.SetText(materialName);
			quantity.SetText(string.Format("%1", m_aMaterialQuantities[index]));
			quantity.SetEnabled(editable);
			remove.SetVisible(editable);
			remove.SetEnabled(editable);
			if (editable)
			{
				NIRE_LogisticsContentsRowHandler handler = new NIRE_LogisticsContentsRowHandler(this, index, highlight);
				quantity.AddHandler(handler);
				remove.AddHandler(handler);
			}
		}

		m_ContentsTitle.SetText(WidgetManager.Translate("#NIRE-Logistics_RequestContents", string.Format("%1", m_aMaterialNames.Count()), string.Format("%1", totalItems)));
	}

	//------------------------------------------------------------------------------------------------
	void SetMaterialQuantity(int index, int quantity)
	{
		if (!CanEditRequest() || index < 0 || index >= m_aMaterialQuantities.Count())
			return;

		m_aMaterialQuantities[index] = Math.ClampInt(quantity, 1, MAX_QUANTITY);
		GetGame().GetCallqueue().Remove(RefreshContentsList);
		GetGame().GetCallqueue().CallLater(RefreshContentsList, 1, false);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveMaterial(int index)
	{
		if (!CanEditRequest() || index < 0 || index >= m_aMaterialPrefabs.Count())
			return;

		if (m_aMaterialPrefabs[index] == m_SelectedWeaponPrefab)
		{
			m_SelectedWeaponPrefab = string.Empty;
			m_SelectedWeaponAmmunition.Clear();
		}
		m_aMaterialPrefabs.Remove(index);
		m_aMaterialNames.Remove(index);
		m_aMaterialQuantities.Remove(index);
		RefreshContentsList();
		RefreshArsenalList();
	}

	//! Selecting a weapon also lists the ammunition it accepts, so a requester does not have to know
	//! the caliber by heart.
	//------------------------------------------------------------------------------------------------
	void SelectArsenalItem(int index)
	{
		if (index < 0 || index >= m_aArsenalPrefabs.Count())
			return;

		m_iSelectedArsenalIndex = index;
		ResourceName prefab = m_aArsenalPrefabs[index];
		if (m_eCategory == IBX_EArsenalTab.WEAPONS && IsWeapon(index))
		{
			if (m_SelectedWeaponPrefab == prefab)
			{
				m_SelectedWeaponPrefab = string.Empty;
				m_SelectedWeaponAmmunition.Clear();
			}
			else
			{
				m_SelectedWeaponPrefab = prefab;
				LoadWeaponAmmunition(prefab);
			}
		}

		RefreshArsenalList();
		m_Status.SetText(m_aArsenalLabels[index]);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddSelectedMaterial()
	{
		if (!CanEditRequest() || m_iSelectedArsenalIndex < 0 || m_iSelectedArsenalIndex >= m_aArsenalPrefabs.Count())
		{
			m_Status.SetText(Translate("#NIRE-Logistics_SelectMaterial"));
			return;
		}

		ResourceName prefab = m_aArsenalPrefabs[m_iSelectedArsenalIndex];
		int quantity = Math.ClampInt(StripLeadingSpaces(m_Quantity.GetText()).ToInt(), 1, MAX_QUANTITY);
		int existing = m_aMaterialPrefabs.Find(prefab);
		if (existing >= 0)
			m_aMaterialQuantities[existing] = Math.ClampInt(m_aMaterialQuantities[existing] + quantity, 1, MAX_QUANTITY);
		else
		{
			m_aMaterialPrefabs.Insert(prefab);
			m_aMaterialNames.Insert(m_aArsenalLabels[m_iSelectedArsenalIndex]);
			m_aMaterialQuantities.Insert(quantity);
		}

		RefreshContentsList();
		RefreshArsenalList();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshArsenalList()
	{
		ClearChildren(m_ArsenalList);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		array<string> searchTerms = {};
		BuildSearchTerms(m_Search.GetText(), searchTerms);
		foreach (int index, string label : m_aArsenalLabels)
		{
			if (!MatchesCategory(index) || !MatchesFaction(index))
				continue;

			if (!MatchesSearchTerms(label, searchTerms))
				continue;

			CreateArsenalRow(index, label, workspace, false);
			if (m_eCategory != IBX_EArsenalTab.WEAPONS || m_aArsenalPrefabs[index] != m_SelectedWeaponPrefab)
				continue;

			foreach (ResourceName ammunition : m_SelectedWeaponAmmunition)
			{
				int ammunitionIndex = m_aArsenalPrefabs.Find(ammunition);
				if (ammunitionIndex >= 0 && MatchesFaction(ammunitionIndex))
					CreateArsenalRow(ammunitionIndex, "    " + Translate("#NIRE-Selector_Ammunition") + ": " + m_aArsenalLabels[ammunitionIndex], workspace, true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateArsenalRow(int index, string label, notnull WorkspaceWidget workspace, bool compatible)
	{
		Widget widget = workspace.CreateWidgets(ARSENAL_ROW_LAYOUT, m_ArsenalList);
		ButtonWidget row = ButtonWidget.Cast(widget);
		TextWidget text;
		if (widget)
			text = TextWidget.Cast(widget.FindAnyWidget("Label"));
		if (!row || !text)
			return;

		text.SetText(label);
		if (index == m_iSelectedArsenalIndex)
			row.SetColor(Color.FromSRGBA(170, 126, 30, 255));
		else if (m_aMaterialPrefabs.Contains(m_aArsenalPrefabs[index]))
			row.SetColor(Color.FromSRGBA(112, 83, 28, 255));
		else if (compatible)
			row.SetColor(Color.FromSRGBA(82, 107, 56, 255));

		SetRowPreview(widget, m_aArsenalPrefabs[index]);
		row.AddHandler(new NIRE_LogisticsArsenalRowHandler(this, index));
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetRowPreview(notnull Widget row, ResourceName prefab)
	{
		SizeLayoutWidget previewSize = SizeLayoutWidget.Cast(row.FindAnyWidget("PreviewSize"));
		ItemPreviewWidget preview = ItemPreviewWidget.Cast(row.FindAnyWidget("Preview"));
		if (!previewSize || !preview)
			return;

		if (prefab.IsEmpty())
		{
			previewSize.SetVisible(false);
			return;
		}

		previewSize.EnableWidthOverride(true);
		previewSize.EnableHeightOverride(true);
		previewSize.SetWidthOverride(56);
		previewSize.SetHeightOverride(56);
		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager;
		if (world)
			previewManager = world.GetItemPreviewManager();
		if (previewManager)
			previewManager.SetPreviewItemFromPrefab(preview, prefab);
		else
			preview.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectWeapons()
	{
		SelectCategory(IBX_EArsenalTab.WEAPONS);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectAmmunition()
	{
		SelectCategory(IBX_EArsenalTab.AMMUNITION);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectClothing()
	{
		SelectCategory(IBX_EArsenalTab.CLOTHING);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectMedical()
	{
		SelectCategory(IBX_EArsenalTab.MEDICAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectExplosives()
	{
		SelectCategory(IBX_EArsenalTab.EXPLOSIVES);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectEquipment()
	{
		SelectCategory(IBX_EArsenalTab.EQUIPMENT);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectCategory(IBX_EArsenalTab category)
	{
		if (category != m_eCategory)
		{
			m_SelectedWeaponPrefab = string.Empty;
			m_SelectedWeaponAmmunition.Clear();
			m_iSelectedArsenalIndex = -1;
		}

		m_eCategory = category;
		foreach (MUI_Button button : m_aCategoryButtons)
			button.MakeDefault();

		m_aCategoryButtons[category].MakeAccent();
		RefreshArsenalList();
		ScrollArsenalToTop();
	}

	//! Every whitespace-separated term has to appear somewhere in the name, in any order, so
	//! "m16 olive" finds "M16 Carbine - Olive" without the words being adjacent or in that order.
	//------------------------------------------------------------------------------------------------
	protected static void BuildSearchTerms(string search, notnull array<string> terms)
	{
		terms.Clear();
		string lower = search;
		lower.ToLower();
		array<string> words = {};
		lower.Split(" ", words, true);
		foreach (string word : words)
		{
			if (!word.IsEmpty())
				terms.Insert(word);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool MatchesSearchTerms(string label, notnull array<string> terms)
	{
		if (terms.IsEmpty())
			return true;

		string lower = label;
		lower.ToLower();
		foreach (string term : terms)
		{
			if (!lower.Contains(term))
				return false;
		}

		return true;
	}

	//! Only the filters reset the scroll position. Selecting a row also rebuilds the list, and
	//! jumping to the top there would throw the player away from the item they just clicked.
	//------------------------------------------------------------------------------------------------
	protected void ScrollArsenalToTop()
	{
		if (m_ArsenalScroll)
			m_ArsenalScroll.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleFactionMenu()
	{
		m_FactionMenu.SetVisible(!m_FactionMenu.IsVisible());
	}

	//------------------------------------------------------------------------------------------------
	void SelectFaction(int index)
	{
		if (index < 0 || index >= m_aFactionLabels.Count())
			return;

		m_iFaction = index;
		m_FactionFilter.SetText(m_aFactionLabels[index]);
		m_FactionMenu.SetVisible(false);
		RefreshArsenalList();
		ScrollArsenalToTop();
	}

	//------------------------------------------------------------------------------------------------
	protected bool MatchesCategory(int index)
	{
		if (index >= m_aArsenalTypes.Count() || index >= m_aArsenalModes.Count())
			return false;

		SCR_EArsenalItemType type = m_aArsenalTypes[index];
		SCR_EArsenalItemMode mode = m_aArsenalModes[index];
		if (mode & SCR_EArsenalItemMode.AMMUNITION)
			return m_eCategory == IBX_EArsenalTab.AMMUNITION;
		if (type & (SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.LEGS | SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR | SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK))
			return m_eCategory == IBX_EArsenalTab.CLOTHING;
		if (type & SCR_EArsenalItemType.HEAL)
			return m_eCategory == IBX_EArsenalTab.MEDICAL;
		if (type & (SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.EXPLOSIVES))
			return m_eCategory == IBX_EArsenalTab.EXPLOSIVES;
		if (type & (SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MORTARS))
			return m_eCategory == IBX_EArsenalTab.WEAPONS;

		return m_eCategory == IBX_EArsenalTab.EQUIPMENT;
	}

	//------------------------------------------------------------------------------------------------
	protected bool MatchesFaction(int index)
	{
		if (index < 0 || index >= m_aArsenalPrefabs.Count())
			return false;

		ResourceName prefab = m_aArsenalPrefabs[index];
		if (m_iFaction <= 0 || m_GeneralPrefabs.Contains(prefab))
			return true;

		int factionIndex = m_iFaction - 1;
		return factionIndex < m_aFactionPrefabs.Count() && m_aFactionPrefabs[factionIndex].Contains(prefab);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsWeapon(int index)
	{
		if (index >= m_aArsenalTypes.Count() || index >= m_aArsenalModes.Count() || m_aArsenalModes[index] & SCR_EArsenalItemMode.AMMUNITION)
			return false;

		return m_aArsenalTypes[index] & (SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MORTARS);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadArsenalCatalog()
	{
		m_aFactionLabels.Insert(Translate("#NIRE-Selector_AllFactions"));
		SCR_EntityCatalogManagerComponent catalog = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalog)
			return;

		set<ResourceName> uniquePrefabs = new set<ResourceName>();
		array<SCR_ArsenalItem> items = {};
		catalog.GetArsenalItems(items);
		foreach (SCR_ArsenalItem generalItem : items)
			m_GeneralPrefabs.Insert(generalItem.GetItemResourceName());
		AddArsenalItems(items, uniquePrefabs);

		array<Faction> factions = {};
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			factionManager.GetFactionsList(factions);
		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;

			array<SCR_ArsenalItem> factionItems = {};
			if (!catalog.GetFactionArsenalItems(factionItems, scrFaction) || factionItems.IsEmpty())
				continue;

			set<ResourceName> factionPrefabs = new set<ResourceName>();
			foreach (SCR_ArsenalItem factionItem : factionItems)
				factionPrefabs.Insert(factionItem.GetItemResourceName());
			m_aFactionPrefabs.Insert(factionPrefabs);
			m_aFactionLabels.Insert(Translate(scrFaction.GetFactionName()));
			AddArsenalItems(factionItems, uniquePrefabs);
		}

		foreach (int index, string label : m_aFactionLabels)
		{
			NIRE_LogisticsFactionHandler handler = new NIRE_LogisticsFactionHandler(this, index);
			m_aFactionHandlers.Insert(handler);
			MUI_Button button = m_MikesUI.CreateButton(label);
			button.SetFillWidth();
			button.GetOnClicked().Insert(handler.Select);
			m_FactionMenu.AddChild(button);
		}
		m_FactionMenu.SetVisible(false);
		m_FactionFilter.SetText(m_aFactionLabels[0]);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddArsenalItems(notnull array<SCR_ArsenalItem> items, notnull set<ResourceName> uniquePrefabs)
	{
		foreach (SCR_ArsenalItem item : items)
		{
			ResourceName prefab = item.GetItemResourceName();
			if (prefab.IsEmpty() || uniquePrefabs.Contains(prefab))
				continue;

			uniquePrefabs.Insert(prefab);
			string label = FilePath.StripExtension(FilePath.StripPath(prefab));
			Resource resource = item.GetItemResource();
			IEntityComponentSource componentSource;
			if (resource && resource.IsValid())
				componentSource = SCR_BaseContainerTools.FindComponentSource(resource, InventoryItemComponent);
			SCR_ItemAttributeCollection attributes;
			if (componentSource)
				attributes = SCR_ComponentHelper.GetInventoryItemInfo(componentSource);
			UIInfo info;
			if (attributes)
				info = attributes.GetUIInfo();
			if (info && !info.GetName().IsEmpty())
				label = Translate(info.GetName());

			m_aArsenalPrefabs.Insert(prefab);
			m_aArsenalLabels.Insert(label);
			m_aArsenalTypes.Insert(item.GetItemType());
			m_aArsenalModes.Insert(item.GetItemMode());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadWeaponAmmunition(ResourceName prefab)
	{
		m_SelectedWeaponAmmunition.Clear();
		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid())
			return;

		IEntity weapon = GetGame().SpawnEntityPrefabLocal(resource, GetGame().GetWorld());
		if (!weapon)
			return;

		set<typename> weaponWells = new set<typename>();
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
		if (weaponComponent)
		{
			array<BaseMuzzleComponent> muzzles = {};
			weaponComponent.GetMuzzlesList(muzzles);
			foreach (BaseMuzzleComponent muzzle : muzzles)
			{
				BaseMagazineWell well = muzzle.GetMagazineWell();
				if (well)
					weaponWells.Insert(well.Type());
			}
		}
		SCR_EntityHelper.DeleteEntityAndChildren(weapon);
		if (weaponWells.IsEmpty())
			return;

		foreach (int index, ResourceName candidate : m_aArsenalPrefabs)
		{
			if (index >= m_aArsenalModes.Count() || !(m_aArsenalModes[index] & SCR_EArsenalItemMode.AMMUNITION))
				continue;

			Resource candidateResource = Resource.Load(candidate);
			if (!candidateResource || !candidateResource.IsValid())
				continue;

			IEntity preview = GetGame().SpawnEntityPrefabLocal(candidateResource, GetGame().GetWorld());
			if (!preview)
				continue;

			BaseMagazineComponent magazine = BaseMagazineComponent.Cast(preview.FindComponent(BaseMagazineComponent));
			if (magazine && IsCompatibleWell(magazine.GetMagazineWell(), weaponWells))
				m_SelectedWeaponAmmunition.Insert(candidate);

			SCR_EntityHelper.DeleteEntityAndChildren(preview);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsCompatibleWell(BaseMagazineWell magazineWell, notnull set<typename> weaponWells)
	{
		if (!magazineWell)
			return false;

		typename magazineWellType = magazineWell.Type();
		foreach (typename weaponWellType : weaponWells)
		{
			if (magazineWellType.IsInherited(weaponWellType) || weaponWellType.IsInherited(magazineWellType))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SubmitRequest()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		string coordinate = m_Coordinate.GetText().Trim();
		if (coordinate.Length() > COORDINATE_LENGTH)
			coordinate = coordinate.Substring(0, COORDINATE_LENGTH);

		string materialData = BuildMaterialData();
		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		bool allowedContents = !serverConfig || serverConfig.AllowsAnyCrateContents(m_aMaterialPrefabs, m_aMaterialQuantities);
		if (!controller || !CanEditRequest() || !allowedContents || materialData.IsEmpty() || (m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY && coordinate.IsEmpty()))
		{
			m_Status.SetText(Translate("#NIRE-Logistics_InvalidRequest"));
			return;
		}

		controller.NIRE_SubmitLogisticsRequest(m_iSelectedRequestId, materialData, m_eDeliveryMode, coordinate, GetNoteText());
		m_Status.SetText(Translate("#NIRE-Logistics_RequestSent"));
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildMaterialData()
	{
		string materialData;
		foreach (int index, ResourceName prefab : m_aMaterialPrefabs)
		{
			if (!materialData.IsEmpty())
				materialData += ";";
			materialData += string.Format("%1=%2", m_aMaterialQuantities[index], prefab);
		}

		return materialData;
	}

	//------------------------------------------------------------------------------------------------
	protected void AdvanceRequest()
	{
		NIRE_LogisticsRequest request = GetSelectedRequest();
		if (!request)
			return;

		NIRE_ELogisticsRequestStatus status = NIRE_ELogisticsRequestStatus.ACCEPTED;
		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.READY)
			status = NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT;
		else if (request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT)
			status = NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED;
		else if (request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED || request.m_eStatus == NIRE_ELogisticsRequestStatus.PICKUP_READY)
			status = NIRE_ELogisticsRequestStatus.COMPLETED;

		SetRequestStatus(status);
	}

	//------------------------------------------------------------------------------------------------
	protected void HoldRequest()
	{
		SetRequestStatus(NIRE_ELogisticsRequestStatus.HOLD);
	}

	//------------------------------------------------------------------------------------------------
	protected void RejectRequest()
	{
		SetRequestStatus(NIRE_ELogisticsRequestStatus.REJECTED);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetRequestStatus(NIRE_ELogisticsRequestStatus status)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && controller.NIRE_IsLogistician() && m_iSelectedRequestId > 0)
			controller.NIRE_ManageLogisticsRequest(m_iSelectedRequestId, status);
	}

	//! The crate list is a dropdown under the contents column rather than a second full screen: an
	//! accepted request already names the contents, so only the crate prefab is still open.
	//! Every candidate button is built once and the server rules only decide which of them are shown,
	//! so the menu never has to remove Mikes UI children while it is mounted.
	//------------------------------------------------------------------------------------------------
	protected void ToggleCrateMenu()
	{
		if (m_CrateMenu.IsVisible())
		{
			m_CrateMenu.SetVisible(false);
			return;
		}

		bool anyAllowed;
		NIRE_LogisticsRequest request = GetSelectedRequest();
		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		foreach (int index, MUI_Button button : m_aCrateButtons)
		{
			bool allowed = !request || !serverConfig || serverConfig.AllowsCrateContents(m_aCratePrefabs[index], request.m_aMaterialPrefabs, request.m_aMaterialQuantities);
			button.SetVisible(allowed);
			button.SetEnabled(allowed);
			anyAllowed = anyAllowed || allowed;
		}

		m_CrateMenu.SetVisible(anyAllowed);
		if (!anyAllowed)
			m_Status.SetText(Translate("#NIRE-Logistics_InvalidRequest"));
	}

	//! Union of the InventoryBoxes placeable registry and every crate the server configuration names,
	//! so a rule can still offer a crate that is not in the registry.
	//------------------------------------------------------------------------------------------------
	protected void LoadCrateOptions()
	{
		Resource holder = BaseContainerTools.LoadContainer(INVENTORY_BOXES_REGISTRY);
		SCR_PlaceableEntitiesRegistry registry;
		if (holder && holder.IsValid())
			registry = SCR_PlaceableEntitiesRegistry.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));

		if (registry)
		{
			foreach (ResourceName prefab : registry.GetPrefabs())
			{
				Resource resource = Resource.Load(prefab);
				if (resource && resource.IsValid() && SCR_BaseContainerTools.FindComponentSource(resource, IBX_GMInventoryEditorComponent))
					AddCrateOption(prefab);
			}
		}

		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		if (!serverConfig || !serverConfig.HasCrateRules())
			return;

		foreach (NIRE_LogisticsCrateRule rule : serverConfig.m_aCrates)
		{
			if (rule && rule.IsValid())
				AddCrateOption(rule.m_sCratePrefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddCrateOption(ResourceName prefab)
	{
		if (m_aCratePrefabs.Contains(prefab))
			return;

		string label = FilePath.StripExtension(FilePath.StripPath(prefab));
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.ExtractEditableUIInfoFromPrefab(prefab);
		if (info && !info.GetName().IsEmpty())
			label = Translate(info.GetName());

		int index = m_aCratePrefabs.Count();
		m_aCratePrefabs.Insert(prefab);
		NIRE_LogisticsCrateHandler handler = new NIRE_LogisticsCrateHandler(this, index);
		m_aCrateHandlers.Insert(handler);
		MUI_Button button = m_MikesUI.CreateButton(label);
		button.SetFillWidth();
		button.GetOnClicked().Insert(handler.Select);
		m_CrateMenu.AddChild(button);
		m_aCrateButtons.Insert(button);
	}

	//------------------------------------------------------------------------------------------------
	void CreateCrate(int index)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_IsLogistician() || index < 0 || index >= m_aCratePrefabs.Count())
			return;

		controller.NIRE_CreateLogisticsCrate(m_iSelectedRequestId, m_aCratePrefabs[index]);
		m_CrateMenu.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowNearbyCrates()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_IsLogistician())
			return;

		m_aNearbyCrates.Clear();
		IEntity player = controller.GetControlledEntity();
		if (player)
		{
			m_CrateSearchPlayer = player;
			m_vCrateSearchOrigin = player.GetOrigin();
			player.GetWorld().QueryEntitiesBySphere(m_vCrateSearchOrigin, CRATE_RANGE, FindNearbyCrate);
		}

		RefreshCrateCards();
		m_CrateOverlay.SetVisible(true);
		m_ScreenFrame.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideNearbyCrates()
	{
		if (!m_CrateOverlay)
			return;

		m_CrateOverlay.SetVisible(false);
		m_ScreenFrame.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindNearbyCrate(IEntity entity)
	{
		if (!entity)
			return true;

		IEntity root = entity.GetRootParent();
		if (entity == m_CrateSearchPlayer || root == m_CrateSearchPlayer || ChimeraCharacter.Cast(entity) || ChimeraCharacter.Cast(root))
			return true;

		IBX_GMInventoryEditorComponent inventoryBox = IBX_GMInventoryEditorComponent.Cast(entity.FindComponent(IBX_GMInventoryEditorComponent));
		if (!inventoryBox || m_aNearbyCrates.Contains(entity))
			return true;

		m_aNearbyCrates.Insert(entity);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshCrateCards()
	{
		ClearChildren(m_CrateList);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		if (m_aNearbyCrates.IsEmpty())
		{
			CreateCrateContentRow(m_CrateList, string.Empty, Translate("#NIRE-Status_NoNearbyCrate"), workspace);
			return;
		}

		foreach (IEntity crate : m_aNearbyCrates)
		{
			if (!crate)
				continue;

			Widget card = workspace.CreateWidgets(CRATE_CARD_LAYOUT, m_CrateList);
			TextWidget title;
			VerticalLayoutWidget itemList;
			if (card)
			{
				title = TextWidget.Cast(card.FindAnyWidget("CrateContentsCardTitle"));
				itemList = VerticalLayoutWidget.Cast(card.FindAnyWidget("CrateContentsCardList"));
			}
			if (!title || !itemList)
				continue;

			title.SetText(GetCrateDisplayName(crate));
			array<IEntity> items = {};
			BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(crate.FindComponent(SCR_UniversalInventoryStorageComponent));
			if (storage)
				storage.GetAll(items, false);

			map<ResourceName, int> counts = new map<ResourceName, int>();
			map<ResourceName, string> labels = new map<ResourceName, string>();
			foreach (IEntity item : items)
			{
				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;

				ResourceName prefab = prefabData.GetPrefabName();
				string label = Translate("#NIRE-Name_UnnamedItem");
				InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
				UIInfo info;
				if (inventoryItem)
					info = inventoryItem.GetUIInfo();
				if (info && !info.GetName().IsEmpty())
					label = Translate(info.GetName());

				int count;
				counts.Find(prefab, count);
				counts.Set(prefab, count + 1);
				labels.Set(prefab, label);
			}

			if (counts.Count() == 0)
			{
				CreateCrateContentRow(itemList, string.Empty, Translate("#NIRE-Crate_Empty"), workspace);
				continue;
			}

			foreach (ResourceName prefab, int count : counts)
				CreateCrateContentRow(itemList, prefab, string.Format("%1  x%2", labels[prefab], count), workspace);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetCrateDisplayName(notnull IEntity crate)
	{
		EntityPrefabData prefabData = crate.GetPrefabData();
		if (!prefabData)
			return WidgetManager.Translate("#NIRE-Name_UnnamedItem");

		ResourceName prefab = prefabData.GetPrefabName();
		string label = FilePath.StripExtension(FilePath.StripPath(prefab));
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.ExtractEditableUIInfoFromPrefab(prefab);
		if (info && !info.GetName().IsEmpty())
			label = WidgetManager.Translate(info.GetName());

		return label;
	}

	//------------------------------------------------------------------------------------------------
	protected static void CreateCrateContentRow(notnull VerticalLayoutWidget list, ResourceName prefab, string label, notnull WorkspaceWidget workspace)
	{
		Widget widget = workspace.CreateWidgets(ARSENAL_ROW_LAYOUT, list);
		TextWidget text;
		if (widget)
			text = TextWidget.Cast(widget.FindAnyWidget("Label"));
		if (!text)
			return;

		text.SetText(label);
		SetRowPreview(widget, prefab);
	}

	//------------------------------------------------------------------------------------------------
	protected static LocalizedString GetStatusLabel(NIRE_ELogisticsRequestStatus status)
	{
		switch (status)
		{
			case NIRE_ELogisticsRequestStatus.ACCEPTED: return "#NIRE-Logistics_StatusAccepted";
			case NIRE_ELogisticsRequestStatus.HOLD: return "#NIRE-Logistics_StatusHold";
			case NIRE_ELogisticsRequestStatus.REJECTED: return "#NIRE-Logistics_StatusRejected";
			case NIRE_ELogisticsRequestStatus.READY: return "#NIRE-Logistics_StatusReady";
			case NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT: return "#NIRE-Logistics_StatusDeliveryInTransit";
			case NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED: return "#NIRE-Logistics_StatusDeliveryArrived";
			case NIRE_ELogisticsRequestStatus.PICKUP_READY: return "#NIRE-Logistics_StatusPickupReady";
			case NIRE_ELogisticsRequestStatus.COMPLETED: return "#NIRE-Logistics_StatusCompleted";
		}

		return "#NIRE-Logistics_StatusOpen";
	}

	//------------------------------------------------------------------------------------------------
	protected static void ClearChildren(Widget parent)
	{
		if (!parent)
			return;

		Widget child = parent.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			child.RemoveFromHierarchy();
			child = next;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleBack()
	{
		s_bSuppressPauseMenu = true;
		s_bConsumedBack = true;
		GetGame().GetCallqueue().Remove(ClearPauseSuppression);
		GetGame().GetCallqueue().CallLater(ClearPauseSuppression, 250, false);
		GetGame().GetCallqueue().Remove(ClearConsumedBack);
		GetGame().GetCallqueue().CallLater(ClearConsumedBack, 250, false);
		if (m_CrateOverlay && m_CrateOverlay.IsVisible())
		{
			HideNearbyCrates();
			return;
		}

		CloseAndRestore();
	}

	//------------------------------------------------------------------------------------------------
	protected static void ClearPauseSuppression()
	{
		s_bSuppressPauseMenu = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnController(Widget w, ControlID control, int value)
	{
		if (control != ControlID.BACK || value <= 0)
			return false;

		HandleBack();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager)
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.NIRE_LogisticsMenu);
	}

	//! Runs from the menu shell's OnMenuClose. The engine owns the root widget here, so this only
	//! releases what this class added to it.
	//------------------------------------------------------------------------------------------------
	protected void Teardown()
	{
		GetGame().GetCallqueue().Remove(TickMikesUI);
		GetGame().GetCallqueue().Remove(RefreshContentsList);
		GetGame().GetCallqueue().Remove(ClearPauseSuppression);
		if (m_MikesUI)
			m_MikesUI.Unmount();

		m_MikesUI = null;
		if (m_Root)
			GetGame().GetWorkspace().RemoveModal(m_Root);

		m_Root = null;
	}
}
