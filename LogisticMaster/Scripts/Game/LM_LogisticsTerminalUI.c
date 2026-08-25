class LM_MUIStockButton : MUI_Button
{
	protected LM_LogisticsTerminalUI m_UI;
	protected ref LM_StockEntry m_Entry;

	void Init(LM_LogisticsTerminalUI ui, LM_StockEntry entry)
	{
		m_UI = ui;
		m_Entry = entry;
		SetText(string.Format("%1  x%2", entry.m_sDisplayName, entry.m_iQuantity));
		SetFillWidth();
		SetHeight(34);
		SetMinHeight(34);
	}

	override void OnClicked()
	{
		if (m_UI)
			m_UI.SelectStock(m_Entry);
	}
}

class LM_MUIOrderButton : MUI_Button
{
	protected LM_LogisticsTerminalUI m_UI;
	protected ref LM_LogisticsRequest m_Request;

	void Init(LM_LogisticsTerminalUI ui, LM_LogisticsRequest request, string text)
	{
		m_UI = ui;
		m_Request = request;
		SetText(text);
		SetFillWidth();
		SetHeight(42);
	}

	override void OnClicked()
	{
		if (m_UI)
			m_UI.SelectOrder(m_Request);
	}
}

class LM_MUIAuthorizationButton : MUI_Button
{
	protected LM_LogisticsTerminalUI m_UI;
	protected int m_iPlayerId;
	protected bool m_bAuthorized;

	void Init(LM_LogisticsTerminalUI ui, int playerId, string playerName, bool authorized, bool gameMaster)
	{
		m_UI = ui;
		m_iPlayerId = playerId;
		m_bAuthorized = authorized;
		string state = "NICHT BERECHTIGT";
		if (authorized)
			state = "BERECHTIGT";
		if (gameMaster)
			state = "BERECHTIGT (GAME MASTER)";
		SetText(string.Format("%1  /  %2", playerName, state));
		SetFillWidth();
		SetHeight(42);
		if (gameMaster)
		{
			MakeAccent();
			SetEnabled(false);
		}
		else if (authorized)
			MakeDanger();
	}

	override void OnClicked()
	{
		if (m_UI)
			m_UI.SetPlayerAuthorization(m_iPlayerId, !m_bAuthorized);
	}
}

class LM_MUIStockCategoryButton : MUI_Button
{
	protected LM_LogisticsTerminalUI m_UI;
	protected LM_EStockCategory m_eCategory;

	void Init(LM_LogisticsTerminalUI ui, LM_EStockCategory category, string label)
	{
		m_UI = ui;
		m_eCategory = category;
		SetText(label);
	}

	override void OnClicked()
	{
		if (m_UI)
			m_UI.SelectStockCategory(m_eCategory);
	}
}

class LM_MUIPageTabButton : MUI_Button
{
	protected LM_LogisticsTerminalUI m_UI;
	protected int m_iPage;

	void Init(LM_LogisticsTerminalUI ui, int page, string label)
	{
		m_UI = ui;
		m_iPage = page;
		SetText(label);
	}

	override void OnClicked()
	{
		if (m_UI)
			m_UI.SelectPage(m_iPage);
	}
}

class LM_LogisticsTerminalUI : MenuBase
{
	protected static ref LM_LogisticsTerminalUI s_Instance;
	protected static LM_LogisticsTerminalComponent s_PendingTerminal;

	protected LM_LogisticsTerminalComponent m_Terminal;
	protected Widget m_wRoot;
	protected ref MUI_Runtime m_Runtime;
	protected MUI_Panel m_InventoryPage;
	protected MUI_Panel m_RequestPage;
	protected MUI_Panel m_OrdersPage;
	protected MUI_Panel m_AuthorizationPage;
	protected MUI_ScrollView m_InventoryList;
	protected MUI_ScrollView m_OrdersList;
	protected MUI_ScrollView m_AuthorizationList;
	protected MUI_Label m_SuppliesLabel;
	protected MUI_Label m_PageLabel;
	protected MUI_Label m_SelectedItemLabel;
	protected MUI_Label m_SelectedOrderLabel;
	protected MUI_TextField m_QuantityField;
	protected MUI_TextField m_StockSearchField;
	protected MUI_TextField m_StockQuantityField;
	protected MUI_Label m_StockSelectionLabel;
	protected ref array<ref MUI_Button> m_PageButtons = {};
	protected ref array<ref LM_MUIStockCategoryButton> m_StockCategoryButtons = {};
	protected MUI_Button m_SubmitButton;
	protected MUI_Button m_ConfirmButton;
	protected MUI_Button m_RejectButton;
	protected MUI_Row m_ManageRow;
	protected ref LM_StockEntry m_SelectedStock;
	protected ref LM_LogisticsRequest m_SelectedOrder;
	protected bool m_bCanEditStock;
	protected bool m_bCanManagePermissions;
	protected int m_iPage;
	protected LM_EStockCategory m_eStockCategory;
	protected string m_sLastStockFilter;

	static void Open(LM_LogisticsTerminalComponent terminal)
	{
		if (!terminal || !terminal.CanAccess(SCR_PlayerController.GetLocalPlayerId()))
			return;

		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager.FindMenuByPreset(ChimeraMenuPreset.LM_LogisticsTerminal))
			return;

		s_PendingTerminal = terminal;
		menuManager.OpenMenu(ChimeraMenuPreset.LM_LogisticsTerminal, 0, true);
	}

	static void RefreshOpen()
	{
		if (s_Instance)
			s_Instance.Refresh();
	}

	protected override void OnMenuOpen()
	{
		s_Instance = this;
		m_Terminal = s_PendingTerminal;
		m_bCanEditStock = m_Terminal.CanEditStock(SCR_PlayerController.GetLocalPlayerId());
		m_bCanManagePermissions = m_Terminal.CanManagePermissions(SCR_PlayerController.GetLocalPlayerId());
		m_wRoot = GetRootWidget();
		if (!m_wRoot)
		{
			CloseMenu();
			return;
		}

		m_Runtime = new MUI_Runtime();
		if (!m_Runtime.Mount(m_wRoot))
		{
			CloseMenu();
			return;
		}

		BuildUI();
		m_Runtime.GetOnBack().Insert(CloseMenu);

		SelectPage(0);
		Refresh();

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		if (m_bCanEditStock)
			controller.LM_RequestSnapshot(m_Terminal.GetTerminalRplId());
		if (m_bCanManagePermissions)
			controller.LM_RequestAuthorizationSnapshot(m_Terminal.GetTerminalRplId());
	}

	protected void BuildUI()
	{
		MUI_Panel root = m_Runtime.CreatePanel("Root");
		root.MakeOverlay();
		root.AddChild(m_Runtime.CreateFxBackdrop("Backdrop"));

		MUI_Card card = m_Runtime.CreateCard("Terminal");
		card.SetWidth(960);
		card.SetHeight(720);
		card.SetAlign(0.5, 0.5);
		card.SetPadding(28);
		card.SetGap(14);
		root.AddChild(card);

		MUI_LiveHeader header = m_Runtime.CreateLiveHeader("LOGISTIC MASTER", "Header");
		header.SetKicker("LOGISTICS NETWORK");
		card.AddChild(header);

		MUI_Row status = m_Runtime.CreateRow("Status");
		status.SetHeight(28);
		m_SuppliesLabel = m_Runtime.CreateLabel("SUPPLIES  0", "Supplies");
		m_SuppliesLabel.SetBold(true);
		m_SuppliesLabel.SetColor(MUI_Theme.Accent);
		m_SuppliesLabel.SetVisible(m_bCanEditStock);
		m_PageLabel = m_Runtime.CreateLabel("SEITE 1/3 / BESTAND", "Page");
		m_PageLabel.SetMuted(true);
		status.AddChild(m_SuppliesLabel);
		status.AddChild(m_PageLabel);
		card.AddChild(status);
		BuildPageTabs(card);

		if (m_bCanEditStock)
		{
			BuildInventoryPage();
			BuildRequestPage();
			BuildOrdersPage();
			card.AddChild(m_InventoryPage);
			card.AddChild(m_RequestPage);
			card.AddChild(m_OrdersPage);
		}

		if (m_bCanManagePermissions)
		{
			BuildAuthorizationPage();
			card.AddChild(m_AuthorizationPage);
		}

		m_Runtime.SetRoot(root);
	}

	protected void BuildPageTabs(notnull MUI_Card card)
	{
		MUI_Row tabs = m_Runtime.CreateRow("PageTabs");
		LM_MUIPageTabButton inventoryTab = new LM_MUIPageTabButton();
		inventoryTab.Init(this, 0, "BESTAND");
		tabs.AddChild(inventoryTab);
		m_PageButtons.Insert(inventoryTab);

		LM_MUIPageTabButton requestTab = new LM_MUIPageTabButton();
		requestTab.Init(this, 1, "ANFORDERN");
		tabs.AddChild(requestTab);
		m_PageButtons.Insert(requestTab);

		LM_MUIPageTabButton ordersTab = new LM_MUIPageTabButton();
		ordersTab.Init(this, 2, "AUFTRÄGE");
		tabs.AddChild(ordersTab);
		m_PageButtons.Insert(ordersTab);

		if (m_bCanManagePermissions)
		{
			LM_MUIPageTabButton authorizationTab = new LM_MUIPageTabButton();
			authorizationTab.Init(this, 3, "BERECHTIGUNGEN");
			tabs.AddChild(authorizationTab);
			m_PageButtons.Insert(authorizationTab);
		}

		MUI_Button closeButton = m_Runtime.CreateButton("SCHLIESSEN", "Close");
		closeButton.GetOnClicked().Insert(CloseMenu);
		tabs.AddChild(closeButton);
		card.AddChild(tabs);
	}

	protected void BuildInventoryPage()
	{
		m_InventoryPage = m_Runtime.CreatePanel("InventoryPage");
		m_InventoryPage.SetHeight(466);
		m_InventoryPage.SetPadding(18);
		m_InventoryPage.SetGap(10);
		if (m_bCanEditStock)
		{
			m_InventoryPage.SetPadding(14);
			m_InventoryPage.SetGap(8);
		}
		MUI_Label title = m_Runtime.CreateLabel("01  BESTAND", "InventoryTitle");
		title.SetFontSize(24);
		title.SetBold(true);
		m_InventoryPage.AddChild(title);
		string hintText = "Material wählen. Auswahl öffnet Anforderung.";
		if (m_bCanEditStock)
			hintText = "Arsenal-Gegenstand suchen und auswählen.";
		MUI_Label hint = m_Runtime.CreateLabel(hintText, "InventoryHint");
		hint.SetMuted(true);
		m_InventoryPage.AddChild(hint);

		MUI_Row categories = m_Runtime.CreateRow("StockCategories");
		AddStockCategoryTab(categories, LM_EStockCategory.WEAPONS, "WAFFEN");
		AddStockCategoryTab(categories, LM_EStockCategory.AMMUNITION, "MUNITION");
		AddStockCategoryTab(categories, LM_EStockCategory.CLOTHING, "KLEIDUNG");
		AddStockCategoryTab(categories, LM_EStockCategory.MEDICAL, "MEDIZIN");
		AddStockCategoryTab(categories, LM_EStockCategory.EXPLOSIVES, "SPRENGSTOFF");
		AddStockCategoryTab(categories, LM_EStockCategory.EQUIPMENT, "AUSRÜSTUNG");
		m_StockCategoryButtons[0].MakeAccent();
		m_InventoryPage.AddChild(categories);

		m_InventoryList = m_Runtime.CreateScrollView("InventoryList");
		m_InventoryList.SetViewportHeight(350);
		if (m_bCanEditStock)
			m_InventoryList.SetViewportHeight(125);
		m_InventoryList.SetPadding(8);
		m_InventoryPage.AddChild(m_InventoryList);

		if (!m_bCanEditStock)
			return;

		MUI_Row editFields = m_Runtime.CreateRow("StockEditFields");
		m_StockSearchField = m_Runtime.CreateTextField("GEGENSTAND SUCHEN", "StockSearch");
		m_StockQuantityField = m_Runtime.CreateTextField("MENGE", "StockQuantity");
		m_StockQuantityField.SetWidth(160);
		m_StockQuantityField.SetText("1");
		editFields.AddChild(m_StockSearchField);
		editFields.AddChild(m_StockQuantityField);
		m_InventoryPage.AddChild(editFields);

		m_StockSelectionLabel = m_Runtime.CreateLabel("Kein Gegenstand ausgewählt", "StockSelection");
		m_StockSelectionLabel.SetMuted(true);
		m_InventoryPage.AddChild(m_StockSelectionLabel);

		MUI_Row editActions = m_Runtime.CreateRow("StockEditActions");
		MUI_Button addButton = m_Runtime.CreateButton("HINZUFÜGEN", "AddStock");
		addButton.MakeAccent();
		addButton.GetOnClicked().Insert(AddStock);
		MUI_Button removeButton = m_Runtime.CreateButton("ENTFERNEN", "RemoveStock");
		removeButton.MakeDanger();
		removeButton.GetOnClicked().Insert(RemoveStock);
		editActions.AddChild(addButton);
		editActions.AddChild(removeButton);
		m_InventoryPage.AddChild(editActions);
	}

	protected void AddStockCategoryTab(notnull MUI_Row tabs, LM_EStockCategory category, string label)
	{
		LM_MUIStockCategoryButton button = new LM_MUIStockCategoryButton();
		button.Init(this, category, label);
		tabs.AddChild(button);
		m_StockCategoryButtons.Insert(button);
	}

	protected void BuildAuthorizationPage()
	{
		m_AuthorizationPage = m_Runtime.CreatePanel("AuthorizationPage");
		m_AuthorizationPage.SetHeight(466);
		m_AuthorizationPage.SetPadding(18);
		m_AuthorizationPage.SetGap(10);
		MUI_Label title = m_Runtime.CreateLabel("BERECHTIGUNGEN", "AuthorizationTitle");
		title.SetFontSize(24);
		title.SetBold(true);
		m_AuthorizationPage.AddChild(title);
		MUI_Label hint = m_Runtime.CreateLabel("Spieler auswählen. Klick schaltet vollständigen LogisticMaster-Zugriff.", "AuthorizationHint");
		hint.SetMuted(true);
		m_AuthorizationPage.AddChild(hint);
		m_AuthorizationList = m_Runtime.CreateScrollView("AuthorizationList");
		m_AuthorizationList.SetViewportHeight(350);
		m_AuthorizationList.SetPadding(8);
		m_AuthorizationPage.AddChild(m_AuthorizationList);
	}

	protected void BuildRequestPage()
	{
		m_RequestPage = m_Runtime.CreatePanel("RequestPage");
		m_RequestPage.SetHeight(466);
		m_RequestPage.SetPadding(18);
		m_RequestPage.SetGap(14);
		MUI_Label title = m_Runtime.CreateLabel("02  ANFORDERN", "RequestTitle");
		title.SetFontSize(24);
		title.SetBold(true);
		m_RequestPage.AddChild(title);
		m_SelectedItemLabel = m_Runtime.CreateLabel("Kein Gegenstand gewählt", "SelectedItem");
		m_SelectedItemLabel.SetMuted(true);
		m_RequestPage.AddChild(m_SelectedItemLabel);
		m_QuantityField = m_Runtime.CreateTextField("MENGE", "Quantity");
		m_QuantityField.SetText("1");
		m_RequestPage.AddChild(m_QuantityField);
		m_SubmitButton = m_Runtime.CreateButton("ANFORDERUNG SENDEN", "Submit");
		m_SubmitButton.MakeAccent();
		m_SubmitButton.SetEnabled(false);
		m_SubmitButton.GetOnClicked().Insert(SubmitRequest);
		m_RequestPage.AddChild(m_SubmitButton);
		MUI_Label notice = m_Runtime.CreateLabel("Bestand wird bei Bestätigung erneut geprüft.", "RequestNotice");
		notice.SetMuted(true);
		m_RequestPage.AddChild(notice);
	}

	protected void BuildOrdersPage()
	{
		m_OrdersPage = m_Runtime.CreatePanel("OrdersPage");
		m_OrdersPage.SetHeight(466);
		m_OrdersPage.SetPadding(18);
		m_OrdersPage.SetGap(10);
		MUI_Label title = m_Runtime.CreateLabel("03  AUFTRÄGE", "OrdersTitle");
		title.SetFontSize(24);
		title.SetBold(true);
		m_OrdersPage.AddChild(title);
		m_SelectedOrderLabel = m_Runtime.CreateLabel("Fraktionsgebundene Warteschlange", "SelectedOrder");
		m_SelectedOrderLabel.SetMuted(true);
		m_OrdersPage.AddChild(m_SelectedOrderLabel);
		m_OrdersList = m_Runtime.CreateScrollView("OrdersList");
		m_OrdersList.SetViewportHeight(290);
		m_OrdersList.SetPadding(8);
		m_OrdersPage.AddChild(m_OrdersList);

		m_ManageRow = m_Runtime.CreateRow("ManageActions");
		m_ConfirmButton = m_Runtime.CreateButton("BESTÄTIGEN / ABSCHLIESSEN", "Confirm");
		m_ConfirmButton.MakeAccent();
		m_ConfirmButton.SetEnabled(false);
		m_ConfirmButton.GetOnClicked().Insert(ConfirmOrder);
		m_RejectButton = m_Runtime.CreateButton("ABLEHNEN", "Reject");
		m_RejectButton.MakeDanger();
		m_RejectButton.SetEnabled(false);
		m_RejectButton.GetOnClicked().Insert(RejectOrder);
		m_ManageRow.AddChild(m_ConfirmButton);
		m_ManageRow.AddChild(m_RejectButton);
		m_ManageRow.SetVisible(m_Terminal.CanManage(SCR_PlayerController.GetLocalPlayerId()));
		m_OrdersPage.AddChild(m_ManageRow);
	}

	protected override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (m_Runtime)
			m_Runtime.Tick(tDelta);

		if (m_StockSearchField && m_sLastStockFilter != m_StockSearchField.GetText())
			RefreshStockList();
	}

	void SelectPage(int page)
	{
		if (page < 0 || page > 3 || (page == 3 && !m_bCanManagePermissions))
			return;

		m_iPage = page;
		m_InventoryPage.SetVisible(page == 0);
		m_RequestPage.SetVisible(page == 1);
		m_OrdersPage.SetVisible(page == 2);
		if (m_AuthorizationPage)
			m_AuthorizationPage.SetVisible(page == 3);

		foreach (int index, MUI_Button button : m_PageButtons)
		{
			if (index == page)
				button.MakeAccent();
			else
				button.MakeDefault();
		}

		if (page == 0)
			m_PageLabel.SetText("BESTAND");
		else if (page == 1)
			m_PageLabel.SetText("ANFORDERN");
		else if (page == 2)
			m_PageLabel.SetText("AUFTRÄGE");
		else
			m_PageLabel.SetText("BERECHTIGUNGEN");
	}

	protected void Refresh()
	{
		if (!m_Terminal)
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;

		if (m_bCanEditStock && m_InventoryList && m_OrdersList)
		{
			m_SuppliesLabel.SetText(string.Format("SUPPLIES  %1", Math.Round(m_Terminal.GetSupplies())));
			RefreshStockList();

			m_SelectedOrder = null;
			m_SelectedOrderLabel.SetText("Fraktionsgebundene Warteschlange");
			m_ConfirmButton.SetEnabled(false);
			m_RejectButton.SetEnabled(false);
			m_OrdersList.ClearChildren();
			foreach (LM_LogisticsRequest request : controller.LM_GetRequests())
			{
				string requester = GetGame().GetPlayerManager().GetPlayerName(request.m_iRequesterPlayerId);
				string line = string.Format("#%1  %2 x%3  %4  (%5)", request.m_iId, request.m_sDisplayName, request.m_iQuantity, request.GetStatusLabel(), requester);
				LM_MUIOrderButton orderButton = new LM_MUIOrderButton();
				orderButton.Init(this, request, line);
				m_OrdersList.AddChild(orderButton);
			}
		}

		if (!m_bCanManagePermissions || !m_AuthorizationList)
			return;

		m_AuthorizationList.ClearChildren();
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			string playerName = players.GetPlayerName(playerId);
			if (playerName.IsEmpty())
				playerName = string.Format("Spieler #%1", playerId);
			bool gameMaster = m_Terminal.CanManagePermissions(playerId);
			LM_MUIAuthorizationButton button = new LM_MUIAuthorizationButton();
			button.Init(this, playerId, playerName, gameMaster || controller.LM_IsPlayerAuthorized(playerId), gameMaster);
			m_AuthorizationList.AddChild(button);
		}
	}

	protected void RefreshStockList()
	{
		if (!m_Terminal || !m_InventoryList)
			return;

		m_InventoryList.ClearChildren();
		array<ref LM_StockEntry> stock = {};
		m_Terminal.GetStockCatalogEntries(stock);
		string filter;
		if (m_StockSearchField)
			filter = m_StockSearchField.GetText();
		m_sLastStockFilter = filter;
		filter.ToLower();

		foreach (LM_StockEntry entry : stock)
		{
			if (!MatchesStockCategory(entry))
				continue;

			string searchable = entry.m_sDisplayName;
			searchable.ToLower();
			if (!filter.IsEmpty() && !searchable.Contains(filter))
				continue;

			LM_MUIStockButton stockButton = new LM_MUIStockButton();
			stockButton.Init(this, entry);
			m_InventoryList.AddChild(stockButton);
		}
	}

	void SelectStockCategory(LM_EStockCategory category)
	{
		m_eStockCategory = category;
		foreach (int index, LM_MUIStockCategoryButton button : m_StockCategoryButtons)
		{
			if (index == category)
				button.MakeAccent();
			else
				button.MakeDefault();
		}
		RefreshStockList();
	}

	protected bool MatchesStockCategory(notnull LM_StockEntry entry)
	{
		if (entry.m_eArsenalItemMode & SCR_EArsenalItemMode.AMMUNITION)
			return m_eStockCategory == LM_EStockCategory.AMMUNITION;

		SCR_EArsenalItemType type = entry.m_eArsenalItemType;
		if (type & (SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.LEGS | SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR | SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK))
			return m_eStockCategory == LM_EStockCategory.CLOTHING;

		if (type & SCR_EArsenalItemType.HEAL)
			return m_eStockCategory == LM_EStockCategory.MEDICAL;

		if (type & (SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.EXPLOSIVES))
			return m_eStockCategory == LM_EStockCategory.EXPLOSIVES;

		if (type & (SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MORTARS))
			return m_eStockCategory == LM_EStockCategory.WEAPONS;

		return m_eStockCategory == LM_EStockCategory.EQUIPMENT;
	}

	void SetPlayerAuthorization(int playerId, bool authorized)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && m_Terminal && m_bCanManagePermissions)
			controller.LM_SetPlayerAuthorization(m_Terminal.GetTerminalRplId(), playerId, authorized);
	}

	void SelectStock(LM_StockEntry entry)
	{
		m_SelectedStock = entry;
		m_SelectedItemLabel.SetText(string.Format("%1  /  verfügbar: %2", entry.m_sDisplayName, entry.m_iQuantity));
		m_SelectedItemLabel.SetMuted(false);
		m_SubmitButton.SetEnabled(true);
		if (m_bCanEditStock && m_StockSelectionLabel)
		{
			m_StockSelectionLabel.SetText("Ausgewählt: " + entry.m_sDisplayName);
			m_StockSelectionLabel.SetMuted(false);
		}
		else
			SelectPage(1);
	}

	protected void AddStock()
	{
		EditStock(true);
	}

	protected void RemoveStock()
	{
		EditStock(false);
	}

	protected void EditStock(bool add)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !m_Terminal || !m_SelectedStock || !m_StockQuantityField)
			return;

		int quantity = m_StockQuantityField.GetText().ToInt();
		if (quantity < 1 || quantity > 100)
			return;

		controller.LM_EditStock(m_Terminal.GetTerminalRplId(), m_SelectedStock.m_sItemPrefab, quantity, add);
	}

	void SelectOrder(LM_LogisticsRequest request)
	{
		m_SelectedOrder = request;
		m_SelectedOrderLabel.SetText(string.Format("Ausgewählt: #%1  %2", request.m_iId, request.GetStatusLabel()));
		m_SelectedOrderLabel.SetMuted(false);
		m_ConfirmButton.SetEnabled(true);
		m_RejectButton.SetEnabled(request.m_eStatus == LM_ELogisticsRequestStatus.OPEN);
	}

	protected void SubmitRequest()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		int quantity = m_QuantityField.GetText().ToInt();
		if (controller && m_Terminal && m_SelectedStock && quantity > 0)
			controller.LM_SubmitRequest(m_Terminal.GetTerminalRplId(), m_SelectedStock.m_sItemPrefab, quantity);
	}

	protected void ConfirmOrder()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller || !m_Terminal || !m_SelectedOrder)
			return;

		if (m_SelectedOrder.m_eStatus == LM_ELogisticsRequestStatus.READY)
			controller.LM_CompleteRequest(m_Terminal.GetTerminalRplId(), m_SelectedOrder.m_iId);
		else
			controller.LM_ManageRequest(m_Terminal.GetTerminalRplId(), m_SelectedOrder.m_iId, true);
	}

	protected void RejectOrder()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller && m_Terminal && m_SelectedOrder)
			controller.LM_ManageRequest(m_Terminal.GetTerminalRplId(), m_SelectedOrder.m_iId, false);
	}

	void CloseMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.LM_LogisticsTerminal);
	}

	protected override void OnMenuClose()
	{
		if (m_Runtime)
			m_Runtime.Unmount();
		m_Runtime = null;
		m_wRoot = null;
		m_Terminal = null;
		s_PendingTerminal = null;
		s_Instance = null;
	}

	static bool CloseIfOpen()
	{
		if (!s_Instance)
			return false;

		s_Instance.CloseMenu();
		return true;
	}
}
