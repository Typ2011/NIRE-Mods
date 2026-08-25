class IBX_GMInventoryEditorRowHandler : ScriptedWidgetEventHandler
{
	protected IBX_GMInventoryEditorUI m_UI;
	protected ResourceName m_Prefab;
	protected bool m_IsCurrentItem;

	void IBX_GMInventoryEditorRowHandler(IBX_GMInventoryEditorUI ui, ResourceName prefab, bool isCurrentItem)
	{
		m_UI = ui;
		m_Prefab = prefab;
		m_IsCurrentItem = isCurrentItem;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_UI.SelectItem(m_Prefab, m_IsCurrentItem);
		return true;
	}
}

class IBX_GMFactionHandler
{
	protected IBX_GMInventoryEditorUI m_UI;
	protected int m_Index;

	void IBX_GMFactionHandler(IBX_GMInventoryEditorUI ui, int index)
	{
		m_UI = ui;
		m_Index = index;
	}

	void Select()
	{
		m_UI.SelectFaction(m_Index);
	}
}

class IBX_GMPresetHandler
{
	protected IBX_GMInventoryEditorUI m_UI;
	protected int m_Index;

	void IBX_GMPresetHandler(IBX_GMInventoryEditorUI ui, int index)
	{
		m_UI = ui;
		m_Index = index;
	}

	void Select()
	{
		m_UI.SelectPreset(m_Index);
	}
}

enum IBX_EArsenalTab
{
	WEAPONS,
	AMMUNITION,
	CLOTHING,
	MEDICAL,
	EXPLOSIVES,
	EQUIPMENT
}

class IBX_GMInventoryEditorUI : ScriptedWidgetEventHandler
{
	protected const ResourceName LAYOUT = "{6F2FC06861E41600}UI/layouts/InventoryBoxes/GMInventoryEditor.layout";
	protected const ResourceName ROW_LAYOUT = "{C1E02C785E02616D}UI/layouts/InventoryBoxes/GMInventoryEditorRow.layout";

	protected static ref IBX_GMInventoryEditorUI s_Instance;
	protected static string s_CopiedInventory;
	protected static bool s_HasCopiedInventory;
	protected static bool s_SuppressPauseMenu;
	protected IBX_GMInventoryEditorComponent m_Component;
	protected Widget m_Root;
	protected ref MUI_Runtime m_MikesUI;
	protected ScrollLayoutWidget m_ArsenalScroll;
	protected VerticalLayoutWidget m_ArsenalList;
	protected ScrollLayoutWidget m_CurrentScroll;
	protected VerticalLayoutWidget m_CurrentList;
	protected EditBoxWidget m_ModalFocus;
	protected MUI_TextField m_Search;
	protected MUI_TextField m_Quantity;
	protected MUI_TextField m_CrateName;
	protected MUI_Button m_FactionFilter;
	protected MUI_Panel m_FactionMenu;
	protected MUI_Button m_AddButton;
	protected MUI_Button m_PresetFilter;
	protected MUI_Button m_ApplyPresetButton;
	protected MUI_Panel m_PresetMenu;
	protected MUI_Panel m_EditorFrame;
	protected MUI_Panel m_ExportOverlay;
	protected MUI_Label m_ExportText;
	protected MUI_Label m_CurrentTitle;
	protected MUI_Label m_Status;
	protected MUI_Panel m_ArsenalViewport;
	protected MUI_Panel m_CurrentViewport;
	protected ResourceName m_SelectedArsenalPrefab;
	protected ResourceName m_SelectedCurrentPrefab;
	protected ResourceName m_SelectedWeaponPrefab;
	protected ref array<ResourceName> m_ArsenalPrefabs = {};
	protected ref map<ResourceName, SCR_EArsenalItemType> m_ArsenalTypes = new map<ResourceName, SCR_EArsenalItemType>();
	protected ref map<ResourceName, SCR_EArsenalItemMode> m_ArsenalModes = new map<ResourceName, SCR_EArsenalItemMode>();
	protected ref map<ResourceName, string> m_ArsenalLabels = new map<ResourceName, string>();
	protected ref set<ResourceName> m_GeneralPrefabs = new set<ResourceName>();
	protected ref set<ResourceName> m_CompatibleAmmunition = new set<ResourceName>();
	protected ref set<ResourceName> m_SelectedWeaponAmmunition = new set<ResourceName>();
	protected ref array<ref set<ResourceName>> m_FactionPrefabs = {};
	protected ref array<string> m_FactionLabels = {"All Factions"};
	protected ref array<ref IBX_GMFactionHandler> m_FactionHandlers = {};
	protected ref array<ref IBX_GMPresetHandler> m_PresetHandlers = {};
	protected ref array<ref IBX_CratePreset> m_Presets = {};
	protected ref array<int> m_PresetConfigIndices = {};
	protected int m_SelectedFaction;
	protected int m_SelectedPreset = -1;
	protected ref array<MUI_Button> m_TabButtons = {};
	protected IBX_EArsenalTab m_SelectedTab;
	protected bool m_AddPending;
	protected string m_LastSearch;

	static void Open(notnull IBX_GMInventoryEditorComponent component)
	{
		if (s_Instance)
			s_Instance.Close();

		s_Instance = new IBX_GMInventoryEditorUI();
		s_Instance.Init(component);
	}

	static void OpenForExport(notnull IBX_GMInventoryEditorComponent component)
	{
		Open(component);
		component.RequestExport();
	}

	static void OpenForPaste(notnull IBX_GMInventoryEditorComponent component)
	{
		Open(component);
		if (s_Instance)
			s_Instance.PasteInventory();
	}

	protected void Init(notnull IBX_GMInventoryEditorComponent component)
	{
		m_Component = component;
		m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
		if (!m_Root)
		{
			Close();
			return;
		}
		m_Root.AddHandler(this);
		m_ArsenalScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("ArsenalScroll"));
		m_ArsenalList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("ArsenalList"));
		m_CurrentScroll = ScrollLayoutWidget.Cast(m_Root.FindAnyWidget("CurrentScroll"));
		m_CurrentList = VerticalLayoutWidget.Cast(m_Root.FindAnyWidget("CurrentList"));
		m_ModalFocus = EditBoxWidget.Cast(m_Root.FindAnyWidget("ModalFocus"));
		InitMikesUI();
		if (!m_MikesUI)
		{
			Close();
			return;
		}

		m_Quantity.SetText("1");
		m_CrateName.SetText(component.GetCrateName());
		LoadArsenalCatalog();
		LoadPresets();
		LoadCompatibleAmmunition();
		SelectTab(IBX_EArsenalTab.WEAPONS);
		RefreshCurrentList();
		component.RequestRefresh();
		GetGame().GetWorkspace().AddModal(m_Root, m_ModalFocus);
	}

	protected void InitMikesUI()
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

		MUI_Panel root = m_MikesUI.CreatePanel("InventoryEditorRoot");
		root.MakeOverlay();
		root.SetFill(Color.FromInt(0));
		m_EditorFrame = m_MikesUI.CreatePanel("InventoryEditorFrame");
		m_EditorFrame.SetFill(MUI_Theme.Border);
		m_EditorFrame.SetFillWidth();
		m_EditorFrame.SetFillHeight();
		m_EditorFrame.SetRadius(18);
		m_EditorFrame.SetPadding(2);
		root.AddChild(m_EditorFrame);
		MUI_Panel card = m_MikesUI.CreatePanel("InventoryEditorCard");
		card.SetFill(MUI_Theme.DeepFrost);
		card.SetFillWidth();
		card.SetFillHeight();
		card.SetRadius(16);
		card.SetPadding(26);
		card.SetGap(12);
		m_EditorFrame.AddChild(card);

		// 74 rather than 48: a MUI_TextField is a caption stacked on top of its input box and needs
		// the same room the filter and quantity rows give theirs. At 48 the box overflowed the
		// header and painted over the crate contents title.
		MUI_Row header = m_MikesUI.CreateRow("Header");
		header.SetFillWidth();
		header.SetHeight(74);
		header.SetGap(12);
		card.AddChild(header);

		MUI_Label title = m_MikesUI.CreateLabel("EDIT CRATE INVENTORY", "Title");
		title.SetFontSize(MUI_Theme.FONT_TITLE);
		title.SetBold(true);
		title.SetFillWidth();
		title.SetGrow(1);
		title.SetAlign(0, 1);
		header.AddChild(title);

		// Renamed inline rather than through a second window: the editor is already a workspace
		// modal. The same field is what the player-facing IBX_CrateRenameMenu offers.
		m_CrateName = m_MikesUI.CreateTextField("CRATE NAME", "CrateName");
		m_CrateName.SetWidth(320);
		m_CrateName.SetGrow(0);
		header.AddChild(m_CrateName);

		// Bottom-aligned so the buttons sit level with the text field's input box rather than with
		// its caption, matching the filter and quantity rows.
		MUI_Button rename = m_MikesUI.CreateButton("RENAME", "Rename");
		rename.SetWidth(140);
		rename.SetGrow(0);
		rename.SetAlign(0, 1);
		rename.GetOnClicked().Insert(RenameCrate);
		header.AddChild(rename);

		MUI_Button close = m_MikesUI.CreateButton("CLOSE", "Close");
		close.SetWidth(120);
		close.SetGrow(0);
		close.SetAlign(0, 1);
		close.GetOnClicked().Insert(Close);
		header.AddChild(close);

		MUI_Row body = m_MikesUI.CreateRow("Body");
		body.SetFillWidth();
		body.SetHeight(1);
		body.SetGrow(1);
		body.SetGap(24);
		card.AddChild(body);

		MUI_Panel arsenal = CreateColumn("ArsenalColumn");
		MUI_Panel current = CreateColumn("CurrentColumn");
		body.AddChild(arsenal);
		body.AddChild(current);

		MUI_Label arsenalTitle = m_MikesUI.CreateLabel("ARSENAL", "ArsenalTitle");
		arsenalTitle.SetBold(true);
		arsenalTitle.SetHeight(28);
		arsenal.AddChild(arsenalTitle);

		MUI_Row tabs = m_MikesUI.CreateRow("Tabs");
		tabs.SetFillWidth();
		tabs.SetHeight(44);
		tabs.SetGap(6);
		arsenal.AddChild(tabs);
		MUI_Button tab = CreateTab(tabs, "WEAPONS");
		tab.GetOnClicked().Insert(SelectWeapons);
		tab = CreateTab(tabs, "AMMO");
		tab.GetOnClicked().Insert(SelectAmmunition);
		tab = CreateTab(tabs, "CLOTHING");
		tab.GetOnClicked().Insert(SelectClothing);
		tab = CreateTab(tabs, "MEDICAL");
		tab.GetOnClicked().Insert(SelectMedical);
		tab = CreateTab(tabs, "EXPLOSIVES");
		tab.GetOnClicked().Insert(SelectExplosives);
		tab = CreateTab(tabs, "EQUIPMENT");
		tab.GetOnClicked().Insert(SelectEquipment);

		MUI_Row filters = m_MikesUI.CreateRow("Filters");
		filters.SetFillWidth();
		filters.SetHeight(74);
		filters.SetGap(10);
		arsenal.AddChild(filters);
		m_Search = m_MikesUI.CreateTextField("SEARCH", "Search");
		m_Search.SetFillWidth();
		m_Search.SetGrow(1);
		filters.AddChild(m_Search);
		m_FactionFilter = m_MikesUI.CreateButton("ALL FACTIONS", "FactionFilter");
		m_FactionFilter.SetWidth(190);
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
		m_Quantity = m_MikesUI.CreateTextField("QUANTITY", "Quantity");
		m_Quantity.SetWidth(150);
		addRow.AddChild(m_Quantity);
		m_AddButton = m_MikesUI.CreateButton("ADD", "Add");
		m_AddButton.MakeAccent();
		m_AddButton.SetGrow(1);
		m_AddButton.SetAlign(0, 1);
		m_AddButton.GetOnClicked().Insert(AddSelected);
		addRow.AddChild(m_AddButton);

		m_CurrentTitle = m_MikesUI.CreateLabel("CRATE CONTENTS (0/100 TYPES, 0 ITEMS)", "CurrentTitle");
		m_CurrentTitle.SetBold(true);
		m_CurrentTitle.SetHeight(28);
		current.AddChild(m_CurrentTitle);
		MUI_Panel currentTopSpacer = m_MikesUI.CreatePanel("CurrentTopSpacer");
		currentTopSpacer.SetFill(Color.FromInt(0));
		currentTopSpacer.SetHeight(52);
		current.AddChild(currentTopSpacer);
		MUI_Row presetRow = m_MikesUI.CreateRow("PresetRow");
		presetRow.SetFillWidth();
		presetRow.SetHeight(74);
		presetRow.SetGap(10);
		current.AddChild(presetRow);
		m_PresetFilter = m_MikesUI.CreateButton("NO PRESETS CONFIGURED", "PresetFilter");
		m_PresetFilter.SetGrow(1);
		m_PresetFilter.SetAlign(0, 1);
		m_PresetFilter.GetOnClicked().Insert(TogglePresetMenu);
		presetRow.AddChild(m_PresetFilter);
		m_ApplyPresetButton = m_MikesUI.CreateButton("APPLY PRESET", "ApplyPreset");
		m_ApplyPresetButton.MakeAccent();
		m_ApplyPresetButton.SetWidth(180);
		m_ApplyPresetButton.SetGrow(0);
		m_ApplyPresetButton.SetAlign(0, 1);
		m_ApplyPresetButton.SetEnabled(false);
		m_ApplyPresetButton.GetOnClicked().Insert(ApplySelectedPreset);
		presetRow.AddChild(m_ApplyPresetButton);
		m_PresetMenu = m_MikesUI.CreatePanel("PresetMenu");
		m_PresetMenu.SetVisible(false);
		current.AddChild(m_PresetMenu);
		m_CurrentViewport = CreateViewport("CurrentViewport");
		current.AddChild(m_CurrentViewport);

		MUI_Row removeRow = m_MikesUI.CreateRow("RemoveRow");
		removeRow.SetFillWidth();
		removeRow.SetHeight(74);
		removeRow.SetGap(10);
		current.AddChild(removeRow);
		MUI_Button remove = m_MikesUI.CreateButton("REMOVE", "Remove");
		remove.SetGrow(1);
		remove.SetAlign(0, 1);
		remove.GetOnClicked().Insert(RemoveSelected);
		removeRow.AddChild(remove);
		MUI_Button clear = m_MikesUI.CreateButton("EMPTY CRATE", "Clear");
		clear.MakeDanger();
		clear.SetGrow(1);
		clear.SetAlign(0, 1);
		clear.GetOnClicked().Insert(ClearCrate);
		removeRow.AddChild(clear);
		MUI_Button copyInventory = m_MikesUI.CreateButton("COPY", "CopyInventory");
		copyInventory.SetGrow(1);
		copyInventory.SetAlign(0, 1);
		copyInventory.GetOnClicked().Insert(CopyInventory);
		removeRow.AddChild(copyInventory);
		MUI_Button export = m_MikesUI.CreateButton("EXPORT PRESET", "ExportPreset");
		export.SetGrow(1);
		export.SetAlign(0, 1);
		export.GetOnClicked().Insert(ExportPreset);
		removeRow.AddChild(export);
		MUI_Button paste = m_MikesUI.CreateButton("PASTE", "PasteInventory");
		paste.SetGrow(1);
		paste.SetAlign(0, 1);
		paste.GetOnClicked().Insert(PasteInventory);
		removeRow.AddChild(paste);

		m_Status = m_MikesUI.CreateLabel("Select an arsenal item on the left and a crate item on the right.", "Status");
		m_Status.SetMuted(true);
		m_Status.SetHeight(28);
		card.AddChild(m_Status);

		BuildExportOverlay(root);

		m_MikesUI.SetRoot(root);
		m_MikesUI.GetOnBack().Insert(HandleBack);
		m_MikesUI.Tick(0);
		SyncNativeViewports();
		GetGame().GetCallqueue().CallLater(TickMikesUI, 16, true);
	}

	protected MUI_Panel CreateColumn(string name)
	{
		MUI_Panel panel = m_MikesUI.CreatePanel(name);
		panel.SetFill(Color.FromInt(0));
		panel.SetWidth(1);
		panel.SetFillHeight();
		panel.SetGrow(1);
		panel.SetGap(8);
		return panel;
	}

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

	protected MUI_Button CreateTab(notnull MUI_Row parent, string text)
	{
		MUI_Button button = m_MikesUI.CreateButton(text);
		button.SetGrow(1);
		parent.AddChild(button);
		m_TabButtons.Insert(button);
		return button;
	}

	protected void BuildExportOverlay(notnull MUI_Panel root)
	{
		m_ExportOverlay = m_MikesUI.CreatePanel("ExportOverlay");
		m_ExportOverlay.MakeOverlay();
		m_ExportOverlay.SetVisible(false);
		root.AddChild(m_ExportOverlay);

		MUI_Panel exportCard = m_MikesUI.CreatePanel("ExportCard");
		exportCard.SetWidth(1000);
		exportCard.SetHeight(650);
		exportCard.SetAlign(0.5, 0.5);
		exportCard.SetPadding(24);
		exportCard.SetGap(12);
		m_ExportOverlay.AddChild(exportCard);

		MUI_Label title = m_MikesUI.CreateLabel("EXPORT CRATE PRESET", "ExportTitle");
		title.SetFontSize(MUI_Theme.FONT_TITLE);
		title.SetBold(true);
		title.SetHeight(42);
		exportCard.AddChild(title);

		MUI_Label help = m_MikesUI.CreateLabel("Copy this value into the preset's m_sItems field in Configs/Inventory/CratePresets.conf.", "ExportHelp");
		help.SetMuted(true);
		help.SetHeight(54);
		exportCard.AddChild(help);

		MUI_Panel content = m_MikesUI.CreatePanel("ExportContent");
		content.SetFill(MUI_Theme.Field);
		content.SetFillWidth();
		content.SetHeight(430);
		content.SetPadding(12);
		exportCard.AddChild(content);
		m_ExportText = m_MikesUI.CreateLabel("", "ExportText");
		m_ExportText.SetFontSize(MUI_Theme.FONT_SMALL);
		m_ExportText.SetHeight(406);
		content.AddChild(m_ExportText);

		MUI_Row actions = m_MikesUI.CreateRow("ExportActions");
		actions.SetFillWidth();
		actions.SetHeight(54);
		actions.SetGap(10);
		exportCard.AddChild(actions);
		MUI_Button copy = m_MikesUI.CreateButton("COPY TO CLIPBOARD", "CopyPreset");
		copy.MakeAccent();
		copy.SetGrow(1);
		copy.GetOnClicked().Insert(CopyExport);
		actions.AddChild(copy);
		MUI_Button close = m_MikesUI.CreateButton("CLOSE", "CloseExport");
		close.SetWidth(160);
		close.SetGrow(0);
		close.GetOnClicked().Insert(CloseExport);
		actions.AddChild(close);
	}

	protected void TickMikesUI()
	{
		if (!m_MikesUI)
			return;

		m_MikesUI.Tick(0.016);
		SyncNativeViewports();
		string search = m_Search.GetText();
		if (search != m_LastSearch)
		{
			m_LastSearch = search;
			RefreshArsenalList();
		}
	}

	protected void SyncNativeViewports()
	{
		SyncNativeViewport(m_ArsenalScroll, m_ArsenalViewport);
		SyncNativeViewport(m_CurrentScroll, m_CurrentViewport);
	}

	protected static void SyncNativeViewport(Widget widget, MUI_Node viewport)
	{
		if (!widget || !viewport)
			return;

		MUI_Rect rect = viewport.GetWorldRect();
		FrameSlot.SetAnchorMin(widget, 0, 0);
		FrameSlot.SetAnchorMax(widget, 0, 0);
		FrameSlot.SetPos(widget, rect.m_fX, rect.m_fY);
		FrameSlot.SetSize(widget, rect.m_fW, rect.m_fH);
	}

	protected void LoadArsenalCatalog()
	{
		SCR_EntityCatalogManagerComponent catalog = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalog)
			return;

		set<ResourceName> uniquePrefabs = new set<ResourceName>();
		array<SCR_ArsenalItem> generalItems = {};
		catalog.GetArsenalItems(generalItems);
		foreach (SCR_ArsenalItem item : generalItems)
		{
			ResourceName prefab = item.GetItemResourceName();
			if (!prefab.IsEmpty())
				m_GeneralPrefabs.Insert(prefab);
			AddCatalogItem(item, uniquePrefabs);
		}

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
			foreach (SCR_ArsenalItem item : factionItems)
			{
				factionPrefabs.Insert(item.GetItemResourceName());
				AddCatalogItem(item, uniquePrefabs);
			}

			m_FactionLabels.Insert(scrFaction.GetFactionName());
			m_FactionPrefabs.Insert(factionPrefabs);
		}

		m_SelectedFaction = 0;
		m_FactionFilter.SetText(m_FactionLabels[0]);
		foreach (int index, string label : m_FactionLabels)
		{
			IBX_GMFactionHandler handler = new IBX_GMFactionHandler(this, index);
			m_FactionHandlers.Insert(handler);
			MUI_Button button = m_MikesUI.CreateButton(label);
			button.SetFillWidth();
			button.GetOnClicked().Insert(handler.Select);
			m_FactionMenu.AddChild(button);
		}
		m_FactionMenu.SetVisible(false);
	}

	protected void LoadPresets()
	{
		IBX_CratePresetConfig config = IBX_CratePresetConfig.Load();
		if (!config || !config.m_aPresets)
			return;

		foreach (int configIndex, IBX_CratePreset preset : config.m_aPresets)
		{
			if (!preset || !preset.Parse())
				continue;

			int presetIndex = m_Presets.Count();
			m_Presets.Insert(preset);
			m_PresetConfigIndices.Insert(configIndex);
			IBX_GMPresetHandler handler = new IBX_GMPresetHandler(this, presetIndex);
			m_PresetHandlers.Insert(handler);
			MUI_Button button = m_MikesUI.CreateButton(preset.m_sName);
			button.SetFillWidth();
			button.GetOnClicked().Insert(handler.Select);
			m_PresetMenu.AddChild(button);
		}

		if (!m_Presets.IsEmpty())
			m_PresetFilter.SetText("SELECT PRESET");
	}

	void SelectFaction(int index)
	{
		m_SelectedFaction = index;
		m_FactionFilter.SetText(m_FactionLabels[index]);
		m_FactionMenu.SetVisible(false);
		RefreshArsenalList();
	}

	void SelectPreset(int index)
	{
		if (index < 0 || index >= m_Presets.Count())
			return;

		m_SelectedPreset = index;
		m_PresetFilter.SetText(m_Presets[index].m_sName);
		m_PresetMenu.SetVisible(false);
		m_ApplyPresetButton.SetEnabled(true);
	}

	protected void AddCatalogItem(notnull SCR_ArsenalItem item, notnull set<ResourceName> uniquePrefabs)
	{
		ResourceName prefab = item.GetItemResourceName();
		if (prefab.IsEmpty() || uniquePrefabs.Contains(prefab))
			return;

		uniquePrefabs.Insert(prefab);
		m_ArsenalPrefabs.Insert(prefab);
		m_ArsenalTypes.Set(prefab, item.GetItemType());
		m_ArsenalModes.Set(prefab, item.GetItemMode());
		UIInfo info = GetItemInfo(item);
		if (info)
			m_ArsenalLabels.Set(prefab, info.GetName());
	}

	protected void RefreshArsenalList()
	{
		ClearChildren(m_ArsenalList);
		string filter = m_Search.GetText();
		filter.ToLower();

		foreach (ResourceName prefab : m_ArsenalPrefabs)
		{
			if (!MatchesSelectedTab(prefab) || !MatchesSelectedFaction(prefab))
				continue;

			string label = GetLabel(prefab);
			string searchable = label;
			searchable.ToLower();
			if (!filter.IsEmpty() && !searchable.Contains(filter))
				continue;

			bool compatible = m_SelectedTab == IBX_EArsenalTab.AMMUNITION && m_CompatibleAmmunition.Contains(prefab);
			CreateRow(m_ArsenalList, prefab, label, false, compatible);

			if (m_SelectedTab == IBX_EArsenalTab.WEAPONS && prefab == m_SelectedWeaponPrefab)
			{
				foreach (ResourceName ammunition : m_ArsenalPrefabs)
				{
					if (m_SelectedWeaponAmmunition.Contains(ammunition) && MatchesSelectedFaction(ammunition))
						CreateRow(m_ArsenalList, ammunition, "    Ammunition: " + GetLabel(ammunition), false, true);
				}
			}
		}
	}

	protected void RefreshCurrentList()
	{
		if (!m_Root || !m_Component)
			return;

		ClearChildren(m_CurrentList);
		map<ResourceName, int> counts = new map<ResourceName, int>();
		array<IEntity> items = {};
		m_Component.GetInventoryItems(items);

		foreach (IEntity item : items)
		{
			EntityPrefabData prefabData = item.GetPrefabData();
			if (!prefabData)
				continue;

			ResourceName prefab = prefabData.GetPrefabName();
			string knownLabel;
			if (!m_ArsenalLabels.Find(prefab, knownLabel))
			{
				InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
				UIInfo info;
				if (inventoryItem)
					info = inventoryItem.GetUIInfo();

				if (info && !info.GetName().IsEmpty())
					m_ArsenalLabels.Set(prefab, info.GetName());
			}

			int count = 0;
			counts.Find(prefab, count);
			counts.Set(prefab, count + 1);
		}

		foreach (ResourceName prefab, int count : counts)
			CreateRow(m_CurrentList, prefab, string.Format("%1  x%2", GetLabel(prefab), count), true);

		m_CurrentTitle.SetText(string.Format("CRATE CONTENTS (%1/100 TYPES, %2 ITEMS)", counts.Count(), items.Count()));
	}

	protected void RefreshCurrentListFromSnapshot(notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		if (!m_Root || !m_Component)
			return;

		ClearChildren(m_CurrentList);
		int totalItems;
		foreach (int index, ResourceName prefab : prefabs)
		{
			int count = counts[index];
			totalItems += count;
			CreateRow(m_CurrentList, prefab, string.Format("%1  x%2", GetLabel(prefab), count), true);
		}

		m_CurrentTitle.SetText(string.Format("CRATE CONTENTS (%1/100 TYPES, %2 ITEMS)", prefabs.Count(), totalItems));
	}

	protected void CreateRow(notnull VerticalLayoutWidget parent, ResourceName prefab, string label, bool isCurrentItem, bool highlight = false)
	{
		Widget row = GetGame().GetWorkspace().CreateWidgets(ROW_LAYOUT, parent);
		if (!row)
			return;

		TextWidget textWidget = TextWidget.Cast(row.FindAnyWidget("Label"));
		textWidget.SetText(label);

		SizeLayoutWidget previewSize = SizeLayoutWidget.Cast(row.FindAnyWidget("PreviewSize"));
		previewSize.EnableWidthOverride(true);
		previewSize.EnableHeightOverride(true);
		previewSize.SetWidthOverride(56);
		previewSize.SetHeightOverride(56);

		ItemPreviewWidget preview = ItemPreviewWidget.Cast(row.FindAnyWidget("Preview"));
		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager;
		if (world)
			previewManager = world.GetItemPreviewManager();

		if (previewManager)
			previewManager.SetPreviewItemFromPrefab(preview, prefab);
		else
			preview.SetVisible(false);

		if (highlight)
			row.SetColorInt(0xFF526B38);

		row.AddHandler(new IBX_GMInventoryEditorRowHandler(this, prefab, isCurrentItem));
	}

	void SelectItem(ResourceName prefab, bool isCurrentItem)
	{
		if (isCurrentItem)
		{
			m_SelectedCurrentPrefab = prefab;
			m_SelectedArsenalPrefab = prefab;
			m_Status.SetText("Crate item selected: " + GetLabel(prefab) + " - remove it or add copies.");
		}
		else
		{
			m_SelectedArsenalPrefab = prefab;
			if (m_SelectedTab == IBX_EArsenalTab.WEAPONS && IsWeapon(prefab))
			{
				if (m_SelectedWeaponPrefab == prefab)
				{
					m_SelectedWeaponPrefab = "";
					m_SelectedWeaponAmmunition = new set<ResourceName>();
					RefreshArsenalList();
					m_Status.SetText("Ammunition preview closed: " + GetLabel(prefab));
					return;
				}

				m_SelectedWeaponPrefab = prefab;
				LoadWeaponAmmunition(prefab);
				RefreshArsenalList();
				m_Status.SetText(string.Format("Weapon selected: %1 - %2 compatible ammunition types are shown below.", GetLabel(prefab), m_SelectedWeaponAmmunition.Count()));
			}
			else
			{
				m_Status.SetText("Selected from arsenal: " + GetLabel(prefab));
			}
		}
	}

	protected void ToggleFactionMenu()
	{
		m_FactionMenu.SetVisible(!m_FactionMenu.IsVisible());
	}

	protected void TogglePresetMenu()
	{
		if (!m_Presets.IsEmpty())
			m_PresetMenu.SetVisible(!m_PresetMenu.IsVisible());
	}

	protected void ApplySelectedPreset()
	{
		if (m_SelectedPreset < 0 || m_SelectedPreset >= m_PresetConfigIndices.Count())
			return;

		m_Component.RequestPreset(m_PresetConfigIndices[m_SelectedPreset]);
	}

	protected void RenameCrate()
	{
		string name = m_CrateName.GetText();
		m_Component.RequestRename(name);
		if (name.IsEmpty())
			m_Status.SetText("Crate name cleared.");
		else
			m_Status.SetText("Crate renamed to " + name + ".");
	}

	protected void ExportPreset()
	{
		m_Component.RequestExport();
	}

	protected void CopyInventory()
	{
		m_Component.RequestCopy();
	}

	protected void PasteInventory()
	{
		string items = System.ImportFromClipboard();
		if (items.IsEmpty() && s_HasCopiedInventory)
			items = s_CopiedInventory;
		if (items.IsEmpty())
		{
			ReportMutationStatus("Clipboard does not contain inventory data.");
			return;
		}

		m_Component.RequestPaste(items);
	}

	protected void AddSelected()
	{
		if (m_AddPending || m_SelectedArsenalPrefab.IsEmpty())
			return;

		m_AddPending = true;
		m_AddButton.SetEnabled(false);
		m_Component.RequestAdd(m_SelectedArsenalPrefab, GetQuantity());
		GetGame().GetCallqueue().CallLater(FinishAdd, 500, false);
	}

	protected void RemoveSelected()
	{
		if (m_SelectedCurrentPrefab.IsEmpty())
			return;

		m_Component.RequestRemove(m_SelectedCurrentPrefab, GetQuantity());
	}

	protected void ClearCrate()
	{
		m_Component.RequestClear();
	}

	//! Called by the crate whenever its name changes on this machine, so the field does not go stale
	//! after a preset renames the crate out from under it.
	static void ReportCrateName(IBX_GMInventoryEditorComponent component, string name)
	{
		if (!s_Instance || s_Instance.m_Component != component || !s_Instance.m_CrateName)
			return;

		s_Instance.m_CrateName.SetText(name);
	}

	static void ReportMutationStatus(string message)
	{
		if (s_Instance && s_Instance.m_Status)
			s_Instance.m_Status.SetText(message);
	}

	static void ReportMutationResult(string message, bool hasSnapshot, notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		ReportMutationStatus(message);
		if (hasSnapshot && s_Instance)
			s_Instance.RefreshCurrentListFromSnapshot(prefabs, counts);
	}

	static void ReportInventoryExport(notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		if (!s_Instance || prefabs.Count() != counts.Count())
			return;

		s_Instance.m_ExportText.SetText(BuildInventoryString(prefabs, counts));
		s_Instance.m_FactionMenu.SetVisible(false);
		s_Instance.m_PresetMenu.SetVisible(false);
		s_Instance.m_EditorFrame.SetVisible(false);
		s_Instance.m_ExportOverlay.SetVisible(true);
		s_Instance.m_ArsenalScroll.SetVisible(false);
		s_Instance.m_CurrentScroll.SetVisible(false);
	}

	static void ReportInventoryCopy(notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		if (!s_Instance || prefabs.Count() != counts.Count())
			return;

		StoreInventoryCopy(BuildInventoryString(prefabs, counts));
		ReportMutationStatus("Inventory copied.");
	}

	protected static string BuildInventoryString(notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		string items;
		foreach (int index, ResourceName prefab : prefabs)
		{
			if (index > 0)
				items += ";";
			items += counts[index].ToString() + "=" + prefab;
		}

		return items;
	}

	protected static void StoreInventoryCopy(string items)
	{
		s_CopiedInventory = items;
		s_HasCopiedInventory = true;
		System.ExportToClipboard(items);
	}

	protected void CopyExport()
	{
		StoreInventoryCopy(m_ExportText.GetText());
	}

	protected void CloseExport()
	{
		m_ExportOverlay.SetVisible(false);
		m_EditorFrame.SetVisible(true);
		m_ArsenalScroll.SetVisible(true);
		m_CurrentScroll.SetVisible(true);
	}

	protected void HandleBack()
	{
		s_SuppressPauseMenu = true;
		GetGame().GetCallqueue().CallLater(ClearPauseMenuSuppression, 1, false);
		if (m_ExportOverlay && m_ExportOverlay.IsVisible())
			CloseExport();
		else
			Close();
	}

	protected static void ClearPauseMenuSuppression()
	{
		s_SuppressPauseMenu = false;
	}

	static bool ConsumePauseMenuSuppression()
	{
		if (!s_SuppressPauseMenu)
			return false;

		s_SuppressPauseMenu = false;
		return true;
	}

	override bool OnController(Widget w, ControlID control, int value)
	{
		if (control != ControlID.BACK || value <= 0)
			return false;

		HandleBack();
		return true;
	}

	protected void SelectTab(IBX_EArsenalTab tab)
	{
		if (tab != m_SelectedTab)
		{
			m_SelectedWeaponPrefab = "";
			m_SelectedWeaponAmmunition = new set<ResourceName>();
		}

		m_SelectedTab = tab;
		foreach (MUI_Button button : m_TabButtons)
			button.MakeDefault();

		m_TabButtons[tab].MakeAccent();
		RefreshArsenalList();
		if (tab == IBX_EArsenalTab.AMMUNITION)
		{
			if (m_CompatibleAmmunition.IsEmpty())
				m_Status.SetText("No compatible ammunition detected for the equipped weapons.");
			else
				m_Status.SetText(string.Format("%1 compatible ammunition types are highlighted in green.", m_CompatibleAmmunition.Count()));
		}
	}

	protected void SelectWeapons()
	{
		SelectTab(IBX_EArsenalTab.WEAPONS);
	}

	protected void SelectAmmunition()
	{
		SelectTab(IBX_EArsenalTab.AMMUNITION);
	}

	protected void SelectClothing()
	{
		SelectTab(IBX_EArsenalTab.CLOTHING);
	}

	protected void SelectMedical()
	{
		SelectTab(IBX_EArsenalTab.MEDICAL);
	}

	protected void SelectExplosives()
	{
		SelectTab(IBX_EArsenalTab.EXPLOSIVES);
	}

	protected void SelectEquipment()
	{
		SelectTab(IBX_EArsenalTab.EQUIPMENT);
	}

	protected bool MatchesSelectedTab(ResourceName prefab)
	{
		SCR_EArsenalItemType type;
		SCR_EArsenalItemMode mode;
		if (!m_ArsenalTypes.Find(prefab, type) || !m_ArsenalModes.Find(prefab, mode))
			return false;

		if (mode & SCR_EArsenalItemMode.AMMUNITION)
			return m_SelectedTab == IBX_EArsenalTab.AMMUNITION;

		if (type & (SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.LEGS | SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR | SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK))
			return m_SelectedTab == IBX_EArsenalTab.CLOTHING;

		if (type & SCR_EArsenalItemType.HEAL)
			return m_SelectedTab == IBX_EArsenalTab.MEDICAL;

		if (type & (SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.EXPLOSIVES))
			return m_SelectedTab == IBX_EArsenalTab.EXPLOSIVES;

		if (type & (SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MORTARS))
			return m_SelectedTab == IBX_EArsenalTab.WEAPONS;

		return m_SelectedTab == IBX_EArsenalTab.EQUIPMENT;
	}

	protected bool IsWeapon(ResourceName prefab)
	{
		SCR_EArsenalItemType type;
		SCR_EArsenalItemMode mode;
		if (!m_ArsenalTypes.Find(prefab, type) || !m_ArsenalModes.Find(prefab, mode) || mode & SCR_EArsenalItemMode.AMMUNITION)
			return false;

		return type & (SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MORTARS);
	}

	protected bool MatchesSelectedFaction(ResourceName prefab)
	{
		if (m_SelectedFaction <= 0 || m_GeneralPrefabs.Contains(prefab))
			return true;

		return m_FactionPrefabs[m_SelectedFaction - 1].Contains(prefab);
	}

	protected void LoadCompatibleAmmunition()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(player.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;

		set<typename> weaponWells = new set<typename>();
		array<IEntity> weapons = {};
		weaponManager.GetWeaponsList(weapons);
		foreach (IEntity weapon : weapons)
			AddWeaponWells(weapon, weaponWells);

		FindCompatibleAmmunition(weaponWells, m_CompatibleAmmunition);
	}

	protected void LoadWeaponAmmunition(ResourceName prefab)
	{
		m_SelectedWeaponAmmunition = new set<ResourceName>();
		Resource resource = Resource.Load(prefab);
		if (!resource || !resource.IsValid())
			return;

		IEntity weapon = GetGame().SpawnEntityPrefabLocal(resource, GetGame().GetWorld());
		if (!weapon)
			return;

		set<typename> weaponWells = new set<typename>();
		AddWeaponWells(weapon, weaponWells);
		SCR_EntityHelper.DeleteEntityAndChildren(weapon);
		FindCompatibleAmmunition(weaponWells, m_SelectedWeaponAmmunition);
	}

	protected static void AddWeaponWells(notnull IEntity weapon, notnull set<typename> weaponWells)
	{
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
		if (!weaponComponent)
			return;

		array<BaseMuzzleComponent> muzzles = {};
		weaponComponent.GetMuzzlesList(muzzles);
		foreach (BaseMuzzleComponent muzzle : muzzles)
		{
			BaseMagazineWell well = muzzle.GetMagazineWell();
			if (well)
				weaponWells.Insert(well.Type());
		}
	}

	protected void FindCompatibleAmmunition(notnull set<typename> weaponWells, notnull set<ResourceName> compatibleAmmunition)
	{
		if (weaponWells.IsEmpty())
			return;

		foreach (ResourceName prefab : m_ArsenalPrefabs)
		{
			SCR_EArsenalItemMode mode;
			if (!m_ArsenalModes.Find(prefab, mode) || !(mode & SCR_EArsenalItemMode.AMMUNITION))
				continue;

			Resource resource = Resource.Load(prefab);
			if (!resource || !resource.IsValid())
				continue;

			IEntity preview = GetGame().SpawnEntityPrefabLocal(resource, GetGame().GetWorld());
			if (!preview)
				continue;

			BaseMagazineComponent magazine = BaseMagazineComponent.Cast(preview.FindComponent(BaseMagazineComponent));
			if (magazine && IsCompatibleWell(magazine.GetMagazineWell(), weaponWells))
				compatibleAmmunition.Insert(prefab);

			SCR_EntityHelper.DeleteEntityAndChildren(preview);
		}
	}

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

	protected int GetQuantity()
	{
		return Math.ClampInt(m_Quantity.GetText().ToInt(), 1, IBX_GMInventoryEditorComponent.MAX_MUTATION_QUANTITY);
	}

	protected void FinishAdd()
	{
		m_AddPending = false;
		if (m_AddButton)
			m_AddButton.SetEnabled(true);
	}

	protected string GetLabel(ResourceName prefab)
	{
		string label;
		if (m_ArsenalLabels.Find(prefab, label) && !label.IsEmpty())
			return label;

		return "Unnamed item";
	}

	protected static UIInfo GetItemInfo(notnull SCR_ArsenalItem item)
	{
		Resource resource = item.GetItemResource();
		if (!resource || !resource.IsValid())
			return null;

		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(resource, InventoryItemComponent);
		if (!componentSource)
			return null;

		SCR_ItemAttributeCollection attributes = SCR_ComponentHelper.GetInventoryItemInfo(componentSource);
		if (!attributes)
			return null;

		UIInfo info = attributes.GetUIInfo();
		return info;
	}

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

	void Close()
	{
		GetGame().GetCallqueue().Remove(FinishAdd);
		GetGame().GetCallqueue().Remove(TickMikesUI);
		if (m_MikesUI)
			m_MikesUI.Unmount();
		m_MikesUI = null;
		if (m_Root)
		{
			GetGame().GetWorkspace().RemoveModal(m_Root);
			m_Root.RemoveFromHierarchy();
		}

		m_Root = null;
		m_Component = null;
		s_Instance = null;
	}

	static bool CloseIfOpen()
	{
		if (!s_Instance)
			return false;

		s_Instance.Close();
		return true;
	}
}

modded class EditorMenuUI
{
	override void OpenPauseMenu()
	{
		if (IBX_GMInventoryEditorUI.ConsumePauseMenuSuppression())
			return;

		if (IBX_GMInventoryEditorUI.CloseIfOpen())
			return;

		super.OpenPauseMenu();
	}
}
