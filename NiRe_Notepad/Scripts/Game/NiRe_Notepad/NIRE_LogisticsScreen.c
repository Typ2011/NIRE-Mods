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
		if (w.GetName() == "FavoriteButton")
			m_Screen.ToggleFavoriteItem(m_iIndex);
		else
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

//! Mikes UI label drawn vertically centred in its own box, optionally also horizontally. The stock
//! label draws from the top left of a box measured before the row hands out its width, so beside
//! centred buttons its text sat high and a second wrapped line was cut off.
class NIRE_CenteredLabel : MUI_Label
{
	protected bool m_bCenterX;

	void SetCenterX(bool center)
	{
		m_bCenterX = center;
		InvalidatePaint();
	}

	override void PaintForeground(MUI_RenderSurface surface)
	{
		surface.DrawText(DrawX(), DrawY(), m_World.m_fW, m_World.m_fH, m_sText, m_Style.m_iFontSize, MUI_ColorUtil.Fade(m_Style.m_Text, GetDrawOpacity()), m_Style.m_bBold, m_bCenterX, true, true);
	}
}

//! Count buttons of one crate prefab offered while a logistician fulfils an accepted request.
class NIRE_LogisticsCrateHandler
{
	protected NIRE_LogisticsScreen m_Screen;
	protected int m_iIndex;

	void NIRE_LogisticsCrateHandler(NIRE_LogisticsScreen screen, int index)
	{
		m_Screen = screen;
		m_iIndex = index;
	}

	void Increase()
	{
		m_Screen.ChangeCrateCount(m_iIndex, 1);
	}

	void Decrease()
	{
		m_Screen.ChangeCrateCount(m_iIndex, -1);
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
	protected static const ResourceName ARSENAL_ROW_LAYOUT = "{8D02B4399E3F2AE0}UI/layouts/NiRe_Notepad/NIRE_FavoriteArsenalRow.layout";
	protected static const ResourceName REQUEST_ROW_LAYOUT = "{2A7C19E4B6D830F1}UI/layouts/NiRe_Notepad/NIRE_MissionRow.layout";
	protected static const ResourceName CONTENTS_ROW_LAYOUT = "{8D02B4399E3F2A10}UI/layouts/NiRe_Notepad/NIRE_SelectedMaterialRow.layout";
	protected static const ResourceName INVENTORY_BOXES_REGISTRY = "{5500189E8072CD5B}Configs/Editor/InventoryBoxes.conf";
	protected static const ResourceName FAVORITE_ICON_SET = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset";
	protected static const int MAX_QUANTITY = 999;
	protected static const int MAX_CRATES = 20;
	protected static const int NOTE_LINE_COUNT = 4;
	protected static const int NOTE_LINE_LENGTH = 45;
	protected static const int COORDINATE_DIGITS = 6;
	protected static const int REQUEST_ROW_FONT_SIZE = 20;
	protected static const int REQUEST_ROW_STATUS_FONT_SIZE = 16;
	protected static const float HOVER_PREVIEW_SIZE = 240;

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
	protected EditBoxWidget m_ModalFocus;
	protected ref MUI_Button m_FavoritesButton;
	protected ImageWidget m_FavoritesIcon;
	protected Widget m_HoverPreviewPanel;
	protected ItemPreviewWidget m_HoverPreview;
	protected ref SCR_ItemAttributeCollection m_HoverAttributeCollection;
	protected PreviewRenderAttributes m_HoverRenderAttributes;

	protected MUI_Panel m_ScreenFrame;
	protected MUI_Panel m_RequestViewport;
	protected MUI_Panel m_ArsenalViewport;
	protected MUI_Panel m_ContentsViewport;
	protected MUI_Panel m_FactionMenu;
	protected MUI_Panel m_CrateMenu;
	protected MUI_Panel m_NewRequestConfirm;
	protected MUI_ScrollView m_FactionItems;
	protected MUI_ScrollView m_CrateItems;
	protected MUI_Label m_ContentsTitle;
	protected MUI_Label m_Status;
	protected MUI_TextField m_Search;
	protected MUI_TextField m_Quantity;
	protected MUI_TextField m_Coordinate;
	protected MUI_Button m_FactionFilter;
	protected MUI_Button m_NewRequestButton;
	protected MUI_Button m_CopyRequestButton;
	protected MUI_Button m_DeleteRequestButton;
	protected MUI_TextField m_RequestName;
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
	protected ref array<Widget> m_aArsenalPreviews = {};
	protected ref array<int> m_aArsenalPreviewIndices = {};
	protected ref array<SCR_EArsenalItemType> m_aArsenalTypes = {};
	protected ref array<SCR_EArsenalItemMode> m_aArsenalModes = {};
	protected ref set<ResourceName> m_GeneralPrefabs = new set<ResourceName>();
	protected ref array<ref set<ResourceName>> m_aFactionPrefabs = {};
	protected ref array<string> m_aFactionLabels = {};
	protected ref array<ref NIRE_LogisticsFactionHandler> m_aFactionHandlers = {};
	protected ref array<ResourceName> m_aCratePrefabs = {};
	protected ref array<MUI_Row> m_aCrateRows = {};
	protected ref array<MUI_Image> m_aCrateImages = {};
	protected ref array<MUI_Label> m_aCrateCountLabels = {};
	protected ref array<MUI_Button> m_aCrateDecreaseButtons = {};
	protected ref array<int> m_aCrateCounts = {};
	protected ref array<float> m_aCrateVolumes = {};
	protected ref array<float> m_aCrateWeights = {};
	protected ref array<ref NIRE_LogisticsCrateHandler> m_aCrateHandlers = {};
	protected MUI_TextField m_CrateName;
	protected MUI_Label m_CrateLoad;
	protected MUI_Label m_CrateWarning;
	protected MUI_Button m_CreateCratesButton;

	protected ref array<ResourceName> m_aMaterialPrefabs = {};
	protected ref array<string> m_aMaterialNames = {};
	protected ref array<int> m_aMaterialQuantities = {};
	protected ref array<ButtonWidget> m_aRequestRows = {};

	protected IBX_EArsenalTab m_eCategory;
	protected int m_iFaction;
	protected int m_iSelectedRequestId = -1;
	protected int m_iPendingDeleteRequestId = -1;
	protected int m_iSelectedArsenalIndex = -1;
	protected int m_iHoverPreviewIndex = -1;
	protected NIRE_ELogisticsDeliveryMode m_eDeliveryMode;
	protected ResourceName m_SelectedWeaponPrefab;
	protected ref set<ResourceName> m_SelectedWeaponAmmunition = new set<ResourceName>();
	protected bool m_bDraft = true;
	protected bool m_bUpdatingWidgets;
	protected string m_sLastSearch;
	protected bool m_bFavoritesOnly;

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
		m_FavoritesIcon = ImageWidget.Cast(m_Root.FindAnyWidget("FavoritesFilterIcon"));
		m_HoverPreviewPanel = m_Root.FindAnyWidget("HoverPreviewPanel");
		m_HoverPreview = ItemPreviewWidget.Cast(m_Root.FindAnyWidget("HoverPreview"));
		m_ArsenalList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("ArsenalList"));
		m_ContentsScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("ContentsScroll"));
		m_ContentsList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("ContentsList"));
		m_ModalFocus = EditBoxWidget.Cast(m_Root.FindAnyWidget("ModalFocus"));
		if (!m_RequestScroll || !m_RequestList || !m_ArsenalScroll || !m_ArsenalList || !m_ContentsScroll || !m_ContentsList || !m_ModalFocus)
		{
			Print("NIRE: Logistics screen layout is incomplete", LogLevel.ERROR);
			return false;
		}
		if (!m_FavoritesIcon || !m_HoverPreviewPanel || !m_HoverPreview)
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

		MUI_Button closeFactions;
		m_FactionMenu = BuildPickerOverlay(root, "Faction", m_FactionItems, closeFactions);
		closeFactions.GetOnClicked().Insert(CloseFactionMenu);
		MUI_Panel crateHeader = m_MikesUI.CreatePanel("CratePickerHeader");
		crateHeader.SetFill(Color.FromInt(0));
		crateHeader.SetGap(4);
		m_CrateName = m_MikesUI.CreateTextField(Translate("#NIRE-Logistics_CrateName"), "CrateName");
		m_CrateName.SetFillWidth();
		crateHeader.AddChild(m_CrateName);
		m_CrateLoad =m_MikesUI.CreateLabel(string.Empty, "CrateLoad");
		m_CrateLoad.SetBold(true);
		crateHeader.AddChild(m_CrateLoad);
		m_CrateWarning = m_MikesUI.CreateLabel(Translate("#NIRE-Logistics_CratesDoNotFit"), "CrateWarning");
		m_CrateWarning.SetColor(MUI_Theme.DangerHover);
		crateHeader.AddChild(m_CrateWarning);
		m_CreateCratesButton = m_MikesUI.CreateButton(Translate("#NIRE-Logistics_CreateCrate"), "CreateCrates");
		m_CreateCratesButton.MakeAccent();
		m_CreateCratesButton.GetOnClicked().Insert(CreateSelectedCrates);
		MUI_Button closeCrates;
		m_CrateMenu = BuildPickerOverlay(root, "Crate", m_CrateItems, closeCrates, crateHeader, m_CreateCratesButton);
		closeCrates.GetOnClicked().Insert(CloseCrateMenu);
		BuildNewRequestConfirm(root);

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
		// Stacked, not a row: a horizontal Mikes UI stack clamps its leftover width at zero and never
		// shrinks a child below its own text width, so in German the two labels grew past this narrow
		// column and painted over the arsenal column beside it.
		MUI_Panel requestActions = m_MikesUI.CreatePanel("RequestActions");
		requestActions.SetFill(Color.FromInt(0));
		requestActions.SetFillWidth();
		requestActions.SetHeight(128);
		requestActions.SetGrow(0);
		requestActions.SetGap(10);
		requests.AddChild(requestActions);
		MUI_Row newRequestActions = m_MikesUI.CreateRow("NewRequestActions");
		newRequestActions.SetFillWidth();
		newRequestActions.SetHeight(44);
		newRequestActions.SetGap(6);
		requestActions.AddChild(newRequestActions);
		m_NewRequestButton = m_MikesUI.CreateButton(Translate("#NIRE-Button_New"), "NewRequest");
		m_NewRequestButton.MakeAccent();
		m_NewRequestButton.SetFillWidth();
		m_NewRequestButton.SetGrow(1);
		m_NewRequestButton.GetOnClicked().Insert(RequestNewDraft);
		newRequestActions.AddChild(m_NewRequestButton);
		m_CopyRequestButton = m_MikesUI.CreateButton(Translate("#NIRE-Button_CopyRequest"), "CopyRequest");
		m_CopyRequestButton.SetWidth(70);
		m_CopyRequestButton.SetGrow(0);
		m_CopyRequestButton.GetOnClicked().Insert(CopySelectedRequest);
		newRequestActions.AddChild(m_CopyRequestButton);
		m_DeleteRequestButton = m_MikesUI.CreateButton(Translate("#NIRE-Button_Delete"), "DeleteRequest");
		m_DeleteRequestButton.MakeDanger();
		m_DeleteRequestButton.SetWidth(85);
		m_DeleteRequestButton.SetGrow(0);
		m_DeleteRequestButton.GetOnClicked().Insert(DeleteSelectedRequest);
		newRequestActions.AddChild(m_DeleteRequestButton);
		m_RequestName = m_MikesUI.CreateTextField(Translate("#NIRE-Logistics_RequestName"), "RequestName");
		m_RequestName.SetFillWidth();
		requestActions.AddChild(m_RequestName);

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
		m_FavoritesButton = m_MikesUI.CreateButton(string.Empty, "FavoritesFilter");
		m_FavoritesButton.SetWidth(58);
		m_FavoritesButton.SetGrow(0);
		m_FavoritesButton.SetAlign(0, 1);
		m_FavoritesButton.GetOnClicked().Insert(ToggleFavoritesFilter);
		filters.AddChild(m_FavoritesButton);
		m_FactionFilter = m_MikesUI.CreateButton(Translate("#NIRE-Selector_AllFactions"), "FactionFilter");
		m_FactionFilter.SetWidth(220);
		m_FactionFilter.SetGrow(0);
		m_FactionFilter.SetAlign(0, 1);
		m_FactionFilter.GetOnClicked().Insert(ToggleFactionMenu);
		filters.AddChild(m_FactionFilter);

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
		// The caption is the field's own and is set again on every mode switch. A separate label sat
		// on the row baseline while the field paints its box 22px lower, so the two never lined up,
		// and its fixed 260px pushed the row past its width once the labels were German.
		m_Coordinate = m_MikesUI.CreateTextField(Translate("#NIRE-Logistics_PickupCoordinate"), "Coordinate");
		m_Coordinate.SetWidth(360);
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
	//! The faction and crate pickers are overlay cards rather than panels in their column. A panel in
	//! the flow is painted before every row under it, so a long list ended up behind the request
	//! fields, and it never scrolls, so anything past the column height could not be reached at all.
	//! The native item lists paint above the whole Mikes UI canvas, so SyncNativeViewports hides them
	//! while a picker is open. header sits above the list and action left of Done.
	//------------------------------------------------------------------------------------------------
	protected MUI_Panel BuildPickerOverlay(notnull MUI_Panel root, string name, out MUI_ScrollView list, out MUI_Button close, MUI_Node header = null, MUI_Button action = null)
	{
		MUI_Panel overlay = m_MikesUI.CreatePanel(name + "Overlay");
		overlay.MakeOverlay();
		overlay.SetVisible(false);
		root.AddChild(overlay);

		MUI_Panel card = m_MikesUI.CreatePanel(name + "PickerCard");
		card.SetFill(MUI_Theme.DeepFrost);
		card.SetWidth(680);
		card.SetGrow(0);
		card.SetAlign(0.5, 0.5);
		card.SetRadius(16);
		card.SetPadding(24);
		card.SetGap(12);
		overlay.AddChild(card);
		if (header)
			card.AddChild(header);

		list = m_MikesUI.CreateScrollView(name + "PickerList");
		list.SetMaxViewportHeight(560);
		list.SetPadding(12);
		card.AddChild(list);

		MUI_Row actions = m_MikesUI.CreateRow(name + "PickerActions");
		actions.SetFillWidth();
		actions.SetHeight(54);
		actions.SetGap(10);
		card.AddChild(actions);
		if (action)
		{
			action.SetGrow(1);
			actions.AddChild(action);
		}

		close = m_MikesUI.CreateButton(Translate("#NIRE-Button_Done"), name + "PickerClose");
		close.SetGrow(1);
		actions.AddChild(close);
		return overlay;
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildNewRequestConfirm(notnull MUI_Panel root)
	{
		m_NewRequestConfirm = m_MikesUI.CreatePanel("NewRequestConfirm");
		m_NewRequestConfirm.MakeOverlay();
		m_NewRequestConfirm.SetVisible(false);
		root.AddChild(m_NewRequestConfirm);

		MUI_Panel dialog = m_MikesUI.CreatePanel("NewRequestConfirmDialog");
		dialog.SetFill(MUI_Theme.DeepFrost);
		dialog.SetWidth(600);
		dialog.SetHeight(210);
		dialog.SetAlign(0.5, 0.5);
		dialog.SetRadius(16);
		dialog.SetPadding(24);
		dialog.SetGap(16);
		m_NewRequestConfirm.AddChild(dialog);

		MUI_Label prompt = m_MikesUI.CreateLabel(Translate("#NIRE-Status_ConfirmNewRequest"), "NewRequestConfirmPrompt");
		prompt.SetFillWidth();
		prompt.SetHeight(70);
		dialog.AddChild(prompt);

		MUI_Row actions = m_MikesUI.CreateRow("NewRequestConfirmActions");
		actions.SetFillWidth();
		actions.SetHeight(54);
		actions.SetGap(10);
		dialog.AddChild(actions);

		MUI_Button cancel = m_MikesUI.CreateButton(Translate("#NIRE-Button_Cancel"), "CancelNewRequest");
		cancel.SetGrow(1);
		cancel.GetOnClicked().Insert(CancelNewRequest);
		actions.AddChild(cancel);

		MUI_Button confirm = m_MikesUI.CreateButton(Translate("#NIRE-Button_NewRequest"), "ConfirmNewRequest");
		confirm.MakeAccent();
		confirm.SetGrow(1);
		confirm.GetOnClicked().Insert(ConfirmNewRequest);
		actions.AddChild(confirm);
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

		// Mikes UI paints all text in one layer above all shapes, so an overlay card cannot cover the
		// labels under it. The screen behind any overlay is hidden instead, before the frame is painted.
		bool overlay = m_FactionMenu.IsVisible() || m_CrateMenu.IsVisible() || m_NewRequestConfirm.IsVisible();
		m_ScreenFrame.SetVisible(!overlay);
		// A crate image is a native widget that only checks its own flag. Once the picker or its row
		// is hidden it would be put back on the screen at its last position, so it follows both.
		foreach (int index, MUI_Image image : m_aCrateImages)
			image.SetVisible(m_CrateMenu.IsVisible() && m_aCrateRows[index].IsVisible());
		m_MikesUI.Tick(0.016);
		SyncNativeViewports();
		string search = StripLeadingSpaces(m_Search.GetText());
		if (search != m_Search.GetText())
			m_Search.SetText(search);

		string coordinate = FormatCoordinate(m_Coordinate.GetText());
		if (coordinate != m_Coordinate.GetText())
			m_Coordinate.SetText(coordinate);

		if (search != m_sLastSearch)
		{
			m_sLastSearch = search;
			RefreshArsenalList();
			ScrollArsenalToTop();
		}
		UpdateHoverPreview();
	}

	//! Grid references are "XXX-XXX". Only digits survive, so a pasted "123 456" and a hand-typed
	//! "123-456" both land in one format, and the dash is put back as soon as the field is left.
	//------------------------------------------------------------------------------------------------
	protected static string FormatCoordinate(string text)
	{
		string source = "0123456789";
		string digits;
		for (int index = 0; index < text.Length() && digits.Length() < COORDINATE_DIGITS; index++)
		{
			string character = text.Substring(index, 1);
			if (source.Contains(character))
				digits += character;
		}

		if (digits.Length() <= 3)
			return digits;

		return digits.Substring(0, 3) + "-" + digits.Substring(3, digits.Length() - 3);
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
		bool picker = (m_FactionMenu && m_FactionMenu.IsVisible()) || (m_CrateMenu && m_CrateMenu.IsVisible()) || (m_NewRequestConfirm && m_NewRequestConfirm.IsVisible());
		if (picker)
			HideHoverPreview(m_iHoverPreviewIndex);
		SyncNativeViewport(m_RequestScroll, m_RequestViewport, !picker);
		SyncNativeViewport(m_ArsenalScroll, m_ArsenalViewport, !picker);
		SyncNativeViewport(m_FavoritesIcon, m_FavoritesButton, !picker);
		if (!picker)
		{
			MUI_Rect favoriteRect = m_FavoritesButton.GetWorldRect();
			FrameSlot.SetPos(m_FavoritesIcon, favoriteRect.m_fX + (favoriteRect.m_fW - 32) * 0.5, favoriteRect.m_fY + (favoriteRect.m_fH - 32) * 0.5);
			FrameSlot.SetSize(m_FavoritesIcon, 32, 32);
		}
		SyncNativeViewport(m_ContentsScroll, m_ContentsViewport, !picker);
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
		m_CopyRequestButton.SetEnabled(hasAccess && !m_bDraft && GetSelectedRequest());
		m_DeleteRequestButton.SetEnabled(hasAccess && !m_bDraft && GetSelectedRequest());
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

			// The row layout is shared with the corner notepad, which is a fraction of this screen's
			// width, so the larger sizes are set here rather than in the resource.
			// SetExactFontSize also raises the minimum, which would clip a long localized status
			// instead of shrinking it, so the minimums are put back afterwards.
			text.SetExactFontSize(REQUEST_ROW_FONT_SIZE);
			text.SetMinFontSize(REQUEST_ROW_STATUS_FONT_SIZE);
			divider.SetExactFontSize(REQUEST_ROW_FONT_SIZE);
			statusText.SetExactFontSize(REQUEST_ROW_STATUS_FONT_SIZE);
			statusText.SetMinFontSize(12);
			if (request.m_sName.IsEmpty())
				text.SetText(WidgetManager.Translate("#NIRE-Name_SupplyRequest", string.Format("%1", request.m_iId)));
			else
				text.SetText(request.m_sName);
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
		m_iPendingDeleteRequestId = -1;
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
		m_RequestName.SetText(request.m_sName);
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
	protected void RequestNewDraft()
	{
		m_iPendingDeleteRequestId = -1;
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_HasLogisticsAccess())
			return;

		if (!m_RequestName.GetText().Trim().IsEmpty() || !m_aMaterialPrefabs.IsEmpty() || !m_Coordinate.GetText().Trim().IsEmpty() || !GetNoteText().Trim().IsEmpty() || m_eDeliveryMode != NIRE_ELogisticsDeliveryMode.PICKUP)
		{
			m_NewRequestConfirm.SetVisible(true);
			return;
		}

		StartDraft();
	}

	//------------------------------------------------------------------------------------------------
	protected void CancelNewRequest()
	{
		m_NewRequestConfirm.SetVisible(false);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void ConfirmNewRequest()
	{
		m_NewRequestConfirm.SetVisible(false);
		StartDraft();
	}

	//------------------------------------------------------------------------------------------------
	void StartDraft()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_HasLogisticsAccess())
			return;

		m_bDraft = true;
		m_iSelectedRequestId = -1;
		m_iPendingDeleteRequestId = -1;
		m_iSelectedArsenalIndex = -1;
		m_SelectedWeaponPrefab = string.Empty;
		m_SelectedWeaponAmmunition.Clear();
		m_aMaterialPrefabs.Clear();
		m_aMaterialNames.Clear();
		m_aMaterialQuantities.Clear();
		m_eDeliveryMode = NIRE_ELogisticsDeliveryMode.PICKUP;
		m_bUpdatingWidgets = true;
		m_RequestName.SetText(string.Empty);
		m_Coordinate.SetText(string.Empty);
		SetNoteText(string.Empty);
		m_bUpdatingWidgets = false;
		m_CrateMenu.SetVisible(false);
		RefreshArsenalList();
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void CopySelectedRequest()
	{
		if (m_bDraft || !GetSelectedRequest())
			return;

		m_bDraft = true;
		m_iSelectedRequestId = -1;
		m_iPendingDeleteRequestId = -1;
		m_RequestName.SetText(string.Empty);
		m_iSelectedArsenalIndex = -1;
		m_SelectedWeaponPrefab = string.Empty;
		m_SelectedWeaponAmmunition.Clear();
		m_CrateMenu.SetVisible(false);
		RefreshArsenalList();
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteSelectedRequest()
	{
		NIRE_LogisticsRequest request = GetSelectedRequest();
		if (m_bDraft || !request)
			return;

		if (m_iPendingDeleteRequestId != request.m_iId)
		{
			m_iPendingDeleteRequestId = request.m_iId;
			Refresh();
			return;
		}

		m_iPendingDeleteRequestId = -1;
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.NIRE_DeleteLogisticsRequest(request.m_iId);
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
		bool editable = controller && controller.NIRE_IsLogistician() && request.m_eStatus != NIRE_ELogisticsRequestStatus.COMPLETED;
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
		if (m_iPendingDeleteRequestId == request.m_iId)
		{
			string requestName = request.m_sName;
			if (requestName.IsEmpty())
				requestName = WidgetManager.Translate("#NIRE-Name_SupplyRequest", string.Format("%1", request.m_iId));
			m_Status.SetText(WidgetManager.Translate("#NIRE-Status_ConfirmDelete", requestName));
		}
		else
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
		m_RequestName.SetEnabled(enabled);
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
			m_Coordinate.SetLabel(Translate("#NIRE-Logistics_Coordinate"));
		else
			m_Coordinate.SetLabel(Translate("#NIRE-Logistics_PickupCoordinate"));
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
		HideHoverPreview(m_iHoverPreviewIndex);
		m_aArsenalPreviews.Clear();
		m_aArsenalPreviewIndices.Clear();
		ClearChildren(m_ArsenalList);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		array<string> searchTerms = {};
		BuildSearchTerms(m_Search.GetText(), searchTerms);
		foreach (int index, string label : m_aArsenalLabels)
		{
			if (!MatchesCategory(index) || !MatchesFaction(index) || (m_bFavoritesOnly && !NIRE_NotepadController.IsFavoriteItem(m_aArsenalPrefabs[index])))
				continue;

			if (!MatchesSearchTerms(label, searchTerms))
				continue;

			CreateArsenalRow(index, label, workspace, false);
			if (m_eCategory != IBX_EArsenalTab.WEAPONS || m_aArsenalPrefabs[index] != m_SelectedWeaponPrefab)
				continue;

			foreach (ResourceName ammunition : m_SelectedWeaponAmmunition)
			{
				int ammunitionIndex = m_aArsenalPrefabs.Find(ammunition);
				if (ammunitionIndex >= 0 && MatchesFaction(ammunitionIndex) && (!m_bFavoritesOnly || NIRE_NotepadController.IsFavoriteItem(ammunition)))
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
		ButtonWidget favorite = ButtonWidget.Cast(widget.FindAnyWidget("FavoriteButton"));
		ImageWidget icon = ImageWidget.Cast(widget.FindAnyWidget("FavoriteIcon"));
		if (favorite && icon)
		{
			if (NIRE_NotepadController.IsFavoriteItem(m_aArsenalPrefabs[index]))
				icon.LoadImageFromSet(0, FAVORITE_ICON_SET, "favourite");
			favorite.AddHandler(new NIRE_LogisticsArsenalRowHandler(this, index));
		}
		if (index == m_iSelectedArsenalIndex)
			row.SetColor(Color.FromSRGBA(170, 126, 30, 255));
		else if (m_aMaterialPrefabs.Contains(m_aArsenalPrefabs[index]))
			row.SetColor(Color.FromSRGBA(112, 83, 28, 255));
		else if (compatible)
			row.SetColor(Color.FromSRGBA(82, 107, 56, 255));

		SetRowPreview(widget, m_aArsenalPrefabs[index]);
		SizeLayoutWidget previewSize = SizeLayoutWidget.Cast(widget.FindAnyWidget("PreviewSize"));
		if (previewSize)
		{
			previewSize.SetWidthOverride(40);
			previewSize.SetHeightOverride(40);
		}
		Widget preview = previewSize;
		if (preview)
		{
			m_aArsenalPreviews.Insert(preview);
			m_aArsenalPreviewIndices.Insert(index);
		}
		row.AddHandler(new NIRE_LogisticsArsenalRowHandler(this, index));
	}

	protected void UpdateHoverPreview()
	{
		if (!m_ArsenalScroll.IsVisible())
		{
			HideHoverPreview(m_iHoverPreviewIndex);
			return;
		}

		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		float scrollX, scrollY, scrollWidth, scrollHeight;
		m_ArsenalScroll.GetScreenPos(scrollX, scrollY);
		m_ArsenalScroll.GetScreenSize(scrollWidth, scrollHeight);
		if (mouseX < scrollX || mouseX >= scrollX + scrollWidth || mouseY < scrollY || mouseY >= scrollY + scrollHeight)
		{
			HideHoverPreview(m_iHoverPreviewIndex);
			return;
		}

		foreach (int rowIndex, Widget preview : m_aArsenalPreviews)
		{
			float previewX, previewY, previewWidth, previewHeight;
			preview.GetScreenPos(previewX, previewY);
			preview.GetScreenSize(previewWidth, previewHeight);
			if (mouseX < previewX || mouseX >= previewX + previewWidth || mouseY < previewY || mouseY >= previewY + previewHeight)
				continue;

			int index = m_aArsenalPreviewIndices[rowIndex];
			if (index != m_iHoverPreviewIndex)
				ShowHoverPreview(index, preview);
			return;
		}
		HideHoverPreview(m_iHoverPreviewIndex);
	}

	void ShowHoverPreview(int index, Widget preview)
	{
		if (index < 0 || index >= m_aArsenalPrefabs.Count() || !preview || !m_HoverPreviewPanel || !m_HoverPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager;
		if (world)
			previewManager = world.GetItemPreviewManager();
		if (!workspace || !previewManager)
			return;

		Widget parent = m_HoverPreviewPanel.GetParent();
		if (!parent)
			return;
		float previewX, previewY, previewWidth, previewHeight;
		float parentX, parentY, parentWidth, parentHeight;
		preview.GetScreenPos(previewX, previewY);
		preview.GetScreenSize(previewWidth, previewHeight);
		parent.GetScreenPos(parentX, parentY);
		parent.GetScreenSize(parentWidth, parentHeight);
		float popupX = Math.Clamp(workspace.DPIUnscale(previewX + previewWidth - parentX) + 12, 0, workspace.DPIUnscale(parentWidth) - HOVER_PREVIEW_SIZE);
		float popupY = Math.Clamp(workspace.DPIUnscale(previewY - parentY), 0, workspace.DPIUnscale(parentHeight) - HOVER_PREVIEW_SIZE);
		FrameSlot.SetPos(m_HoverPreviewPanel, popupX, popupY);
		FrameSlot.SetSize(m_HoverPreviewPanel, HOVER_PREVIEW_SIZE, HOVER_PREVIEW_SIZE);
		m_HoverPreviewPanel.SetVisible(true);
		m_HoverAttributeCollection = null;
		m_HoverRenderAttributes = null;
		if (m_eCategory == IBX_EArsenalTab.WEAPONS)
		{
			Resource resource = Resource.Load(m_aArsenalPrefabs[index]);
			if (resource && resource.IsValid())
			{
				IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
				IEntityComponentSource componentSource = SCR_ComponentHelper.GetInventoryItemComponentSource(entitySource);
				if (componentSource)
					m_HoverAttributeCollection = SCR_ComponentHelper.GetInventoryItemInfo(componentSource);
				if (m_HoverAttributeCollection)
					m_HoverRenderAttributes = PreviewRenderAttributes.Cast(m_HoverAttributeCollection.FindAttribute(PreviewRenderAttributes));
				if (m_HoverRenderAttributes)
					m_HoverRenderAttributes.ZoomCamera(20);
			}
		}
		previewManager.SetPreviewItemFromPrefab(m_HoverPreview, m_aArsenalPrefabs[index], m_HoverRenderAttributes);
		m_iHoverPreviewIndex = index;
	}

	void HideHoverPreview(int index)
	{
		if (index != m_iHoverPreviewIndex || !m_HoverPreviewPanel)
			return;
		m_HoverPreviewPanel.SetVisible(false);
		m_iHoverPreviewIndex = -1;
	}

	void ToggleFavoriteItem(int index)
	{
		if (index < 0 || index >= m_aArsenalPrefabs.Count())
			return;
		NIRE_NotepadController.ToggleFavoriteItem(m_aArsenalPrefabs[index]);
		if (m_bFavoritesOnly && !NIRE_NotepadController.IsFavoriteItem(m_aArsenalPrefabs[index]) && m_iSelectedArsenalIndex == index)
			m_iSelectedArsenalIndex = -1;
		RefreshArsenalList();
	}

	protected void ToggleFavoritesFilter()
	{
		m_bFavoritesOnly = !m_bFavoritesOnly;
		m_iSelectedArsenalIndex = -1;
		if (m_bFavoritesOnly)
			m_FavoritesIcon.LoadImageFromSet(0, FAVORITE_ICON_SET, "favourite");
		else
			m_FavoritesIcon.LoadImageFromSet(0, FAVORITE_ICON_SET, "favouriteOff");
		RefreshArsenalList();
		ScrollArsenalToTop();
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
	protected void CloseFactionMenu()
	{
		m_FactionMenu.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseCrateMenu()
	{
		m_CrateMenu.SetVisible(false);
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
			button.SetGrow(0);
			button.GetOnClicked().Insert(handler.Select);
			m_FactionItems.AddChild(button);
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
		// Formatted here as well as in the tick: the tick cannot touch the field while the native edit
		// box still owns its text, so a request sent straight from the keyboard would carry the raw
		// digits.
		string coordinate = FormatCoordinate(m_Coordinate.GetText());

		string materialData = BuildMaterialData();
		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		bool allowedContents = !serverConfig || serverConfig.AllowsAnyCrateContents(m_aMaterialPrefabs);
		string name = StripLeadingSpaces(m_RequestName.GetText()).Trim();
		if (name.Length() > 32)
		{
			m_Status.SetText(Translate("#NIRE-Logistics_RequestNameTooLong"));
			return;
		}
		if (!controller || !CanEditRequest() || !allowedContents || materialData.IsEmpty() || name.IsEmpty() || (m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY && coordinate.IsEmpty()))
		{
			m_Status.SetText(Translate("#NIRE-Logistics_InvalidRequest"));
			return;
		}

		controller.NIRE_SubmitLogisticsRequest(m_iSelectedRequestId, materialData, m_eDeliveryMode, coordinate, GetNoteText(), name);
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

	//! The crate list is an overlay card rather than a second full screen: an accepted request
	//! already names the contents, so only which crates and how many of each are still open.
	//! Every candidate row is built once and the server rules only decide which of them are shown,
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
		foreach (int index, MUI_Row row : m_aCrateRows)
		{
			bool allowed = !request || !serverConfig || serverConfig.AllowsCrate(m_aCratePrefabs[index]) && CarriesAnyItem(serverConfig, m_aCratePrefabs[index], request);
			row.SetVisible(allowed);
			ChangeCrateCount(index, -m_aCrateCounts[index]);
			anyAllowed = anyAllowed || allowed;
		}

		m_CrateName.SetText(string.Empty);
		UpdateCrateLoad();
		m_CrateMenu.SetVisible(anyAllowed);
		if (!anyAllowed)
			m_Status.SetText(Translate("#NIRE-Logistics_InvalidRequest"));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool CarriesAnyItem(notnull NIRE_LogisticsServerConfig serverConfig, ResourceName cratePrefab, notnull NIRE_LogisticsRequest request)
	{
		foreach (ResourceName itemPrefab : request.m_aMaterialPrefabs)
		{
			if (serverConfig.GetMaximumCount(cratePrefab, itemPrefab) != 0)
				return true;
		}

		return false;
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

	//! One picker row: the crate's editor preview, its name and a count. Fixed widths everywhere but
	//! the name, because a Mikes UI row never shrinks its children.
	//------------------------------------------------------------------------------------------------
	protected void AddCrateOption(ResourceName prefab)
	{
		if (m_aCratePrefabs.Contains(prefab))
			return;

		string label = FilePath.StripExtension(FilePath.StripPath(prefab));
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.ExtractEditableUIInfoFromPrefab(prefab);
		if (info && !info.GetName().IsEmpty())
			label = Translate(info.GetName());

		float volume;
		float weight;
		GetCrateCapacity(prefab, volume, weight);
		int index = m_aCratePrefabs.Count();
		m_aCratePrefabs.Insert(prefab);
		m_aCrateCounts.Insert(0);
		m_aCrateVolumes.Insert(volume);
		m_aCrateWeights.Insert(weight);
		NIRE_LogisticsCrateHandler handler = new NIRE_LogisticsCrateHandler(this, index);
		m_aCrateHandlers.Insert(handler);

		MUI_Row row = m_MikesUI.CreateRow();
		row.SetFillWidth();
		row.SetHeight(72);
		row.SetGap(10);
		m_CrateItems.AddChild(row);
		m_aCrateRows.Insert(row);

		MUI_Image image = m_MikesUI.CreateImage();
		image.SetWidth(96);
		image.SetHeight(64);
		image.SetAlign(0, 0.5);
		if (info)
			image.SetImage(info.GetImage());
		row.AddChild(image);
		m_aCrateImages.Insert(image);

		MUI_Label name = CreateCenteredLabel(label, false);
		name.SetWidth(1);
		name.SetGrow(1);
		row.AddChild(name);

		MUI_Button decrease = m_MikesUI.CreateButton("-");
		decrease.SetWidth(56);
		decrease.SetGrow(0);
		decrease.SetAlign(0, 0.5);
		decrease.SetEnabled(false);
		decrease.GetOnClicked().Insert(handler.Decrease);
		row.AddChild(decrease);
		m_aCrateDecreaseButtons.Insert(decrease);

		MUI_Label count = CreateCenteredLabel("0", true);
		count.SetWidth(48);
		count.SetGrow(0);
		count.SetBold(true);
		row.AddChild(count);
		m_aCrateCountLabels.Insert(count);

		MUI_Button increase = m_MikesUI.CreateButton("+");
		increase.SetWidth(56);
		increase.SetGrow(0);
		increase.SetAlign(0, 0.5);
		increase.GetOnClicked().Insert(handler.Increase);
		row.AddChild(increase);
	}

	//! Fills the whole row height, so two wrapped lines of a long crate name still fit.
	//------------------------------------------------------------------------------------------------
	protected MUI_Label CreateCenteredLabel(string text, bool centerX)
	{
		ref NIRE_CenteredLabel label = new NIRE_CenteredLabel();
		m_MikesUI.Adopt(label);
		label.SetCenterX(centerX);
		label.SetText(text);
		label.SetHeight(72);
		return label;
	}

	//! The server refuses more than MAX_CRATES crates per order, so the total stops there.
	//------------------------------------------------------------------------------------------------
	void ChangeCrateCount(int index, int delta)
	{
		if (index < 0 || index >= m_aCrateCounts.Count())
			return;

		int total;
		foreach (int selected : m_aCrateCounts)
			total += selected;
		if (delta > 0 && total >= MAX_CRATES)
			return;

		int count = Math.ClampInt(m_aCrateCounts[index] + delta, 0, MAX_CRATES);
		m_aCrateCounts[index] = count;
		m_aCrateCountLabels[index].SetText(count.ToString());
		m_aCrateDecreaseButtons[index].SetEnabled(count > 0);
		UpdateCrateLoad();
	}

	//! An estimate from the prefabs alone: pooled volume and weight against the request, plus the
	//! per-crate limits of the server rules. Pooling can only overrate the selection, so "does not
	//! fit" here is certain and blocks creation; otherwise the server's real fill has the final word.
	//------------------------------------------------------------------------------------------------
	protected void UpdateCrateLoad()
	{
		NIRE_LogisticsRequest request = GetSelectedRequest();
		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		int crates;
		float maxVolume;
		float maxWeight;
		bool unlimitedVolume;
		bool unlimitedWeight;
		foreach (int index, int count : m_aCrateCounts)
		{
			if (count < 1)
				continue;

			crates += count;
			maxVolume += m_aCrateVolumes[index] * count;
			maxWeight += m_aCrateWeights[index] * count;
			unlimitedVolume = unlimitedVolume || m_aCrateVolumes[index] <= 0;
			unlimitedWeight = unlimitedWeight || m_aCrateWeights[index] <= 0;
		}

		float volume;
		float weight;
		bool fits = request && crates > 0;
		if (request)
		{
			foreach (int materialIndex, ResourceName itemPrefab : request.m_aMaterialPrefabs)
			{
				int quantity = request.m_aMaterialQuantities[materialIndex];
				float itemVolume;
				float itemWeight;
				GetItemSize(itemPrefab, itemVolume, itemWeight);
				volume += itemVolume * quantity;
				weight += itemWeight * quantity;
				if (serverConfig && GetSelectedCrateLimit(serverConfig, itemPrefab) < quantity)
					fits = false;
			}
		}

		if (!unlimitedVolume && volume > maxVolume)
			fits = false;
		if (!unlimitedWeight && weight > maxWeight)
			fits = false;

		m_CrateLoad.SetText(WidgetManager.Translate("#NIRE-Logistics_CrateLoad", crates.ToString(), FormatAmount(volume / 1000), FormatAmount(maxVolume / 1000), FormatAmount(weight), FormatAmount(maxWeight)));
		m_CrateWarning.SetVisible(crates > 0 && !fits);
		m_CreateCratesButton.SetEnabled(fits);
	}

	//! How many of one item the selected crates may carry together under the server rules.
	//------------------------------------------------------------------------------------------------
	protected int GetSelectedCrateLimit(notnull NIRE_LogisticsServerConfig serverConfig, ResourceName itemPrefab)
	{
		int limit;
		foreach (int index, int count : m_aCrateCounts)
		{
			if (count < 1)
				continue;

			int maximum = serverConfig.GetMaximumCount(m_aCratePrefabs[index], itemPrefab);
			if (maximum < 0)
				return int.MAX;

			limit += maximum * count;
		}

		return limit;
	}

	//------------------------------------------------------------------------------------------------
	protected static string FormatAmount(float value)
	{
		int rounded = Math.Ceil(value);
		return rounded.ToString();
	}

	//! Volume and weight limit of a crate prefab's own storage. These crates carry a disabled vanilla
	//! storage next to the addon's one, so the first enabled universal storage is the one read.
	//------------------------------------------------------------------------------------------------
	protected static void GetCrateCapacity(ResourceName prefab, out float volume, out float weight)
	{
		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid())
			return;

		IEntitySource source = resource.GetResource().ToEntitySource();
		if (!source)
			return;

		for (int index = 0; index < source.GetComponentCount(); index++)
		{
			IEntityComponentSource component = source.GetComponent(index);
			typename type = component.GetClassName().ToType();
			bool enabled = true;
			component.Get("Enabled", enabled);
			if (!enabled || !type || !type.IsInherited(SCR_UniversalInventoryStorageComponent))
				continue;

			component.Get("MaxCumulativeVolume", volume);
			component.Get("m_fMaxWeight", weight);
			return;
		}
	}

	//! Volume and weight of one item, read from its prefab so nothing has to be spawned.
	//------------------------------------------------------------------------------------------------
	protected static void GetItemSize(ResourceName prefab, out float volume, out float weight)
	{
		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid())
			return;

		IEntityComponentSource item = SCR_BaseContainerTools.FindComponentSource(resource, InventoryItemComponent);
		BaseContainer attributes;
		if (item)
			attributes = item.GetObject("Attributes");
		BaseContainer physical;
		if (attributes)
			physical = attributes.GetObject("ItemPhysAttributes");
		if (!physical)
			return;

		physical.Get("ItemVolume", volume);
		physical.Get("Weight", weight);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateSelectedCrates()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !controller.NIRE_IsLogistician())
			return;

		string crateData;
		foreach (int index, int count : m_aCrateCounts)
		{
			if (count < 1)
				continue;

			if (!crateData.IsEmpty())
				crateData += ";";
			crateData += string.Format("%1=%2", count, m_aCratePrefabs[index]);
		}

		if (!crateData.IsEmpty())
			controller.NIRE_CreateLogisticsCrates(m_iSelectedRequestId, crateData, StripLeadingSpaces(m_CrateName.GetText()));
	}

	//! The server's answer when the selected crates could not take the whole request. The picker
	//! stays open, so another crate can be added straight away.
	//------------------------------------------------------------------------------------------------
	static void ShowCratesDoNotFit()
	{
		if (s_Instance && s_Instance.m_CrateWarning)
			s_Instance.m_CrateWarning.SetVisible(true);
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
		if (m_NewRequestConfirm && m_NewRequestConfirm.IsVisible())
		{
			CancelNewRequest();
			return;
		}
		if (m_FactionMenu && m_FactionMenu.IsVisible())
		{
			CloseFactionMenu();
			return;
		}
		if (m_CrateMenu && m_CrateMenu.IsVisible())
		{
			CloseCrateMenu();
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
