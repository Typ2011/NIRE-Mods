modded class RAMI_MedicalMenuUI
{
	protected static const string AMI_ACE_BREATHING_TOGGLE = "AMIACEBreathingToggle";
	protected static const string AMI_ACE_TILT_HEAD = "AMIACETiltHead";
	protected static const string AMI_ACE_CLEAR_AIRWAY = "AMIACEClearAirway";
	protected static const string AMI_ACE_RESPIRATORY_RATE = "AMIACERespiratoryRate";
	protected static const string AMI_ACE_SPO2 = "AMIACESpO2";
	protected static const ResourceName AMI_ACE_STATUS_ICON_LAYOUT = "{42333020C90DFC23}UI/layouts/Menus/Inventory/Medical/ACE_Medical_PneumothoraxInfo.layout";
	protected static const ResourceName AMI_ACE_STATUS_ICON_SET = "{B9199157B90D6217}UI/Textures/InventoryIcons/Medical/ACE_Medical_Icons.imageset";

	protected ref array<SCR_EConsumableType> m_AMIACEBreathingTypes = {
		SCR_EConsumableType.ACE_MEDICAL_CHEST_SEAL,
		SCR_EConsumableType.ACE_MEDICAL_NCD_KIT,
		SCR_EConsumableType.ACE_MEDICAL_LARYNGEAL_TUBE,
		SCR_EConsumableType.ACE_MEDICAL_OXYGEN_MASK
	};
	protected ref array<string> m_AMIACEBreathingNames = {
		"AMIACEBreathingChestSeal",
		"AMIACEBreathingNCDKit",
		"AMIACEBreathingKingLT",
		"AMIACEBreathingOxygenMask"
	};
	protected ref array<string> m_AMIACEBreathingLabels = {
		"#ACE_Medical-Item_ChestSeal_Name",
		"#ACE_Medical-Item_NCDKit_Name",
		"#ACE_Medical-Item_KingLT_Name",
		"#ACE_Medical-Item_OxygenMask_Name"
	};
	protected ref array<Widget> m_AMIACEBreathingRows = {};
	protected ref array<ButtonWidget> m_AMIACEBreathingButtons = {};
	protected ref array<Widget> m_AMIACEBreathingContents = {};
	protected ref array<ItemPreviewWidget> m_AMIACEBreathingIcons = {};
	protected ref array<TextWidget> m_AMIACEBreathingLabelsWidgets = {};
	protected ref array<TextWidget> m_AMIACEBreathingCounts = {};
	protected ref array<Widget> m_AMIACEStandardRows = {};
	protected ButtonWidget m_AMIACEBreathingToggleButton;
	protected Widget m_AMIACEBreathingToggleContent;
	protected TextWidget m_AMIACEBreathingToggleLabel;
	protected bool m_AMIACEBreathingOpen;
	protected ButtonWidget m_AMIACETiltHeadButton;
	protected ButtonWidget m_AMIACEClearAirwayButton;
	protected TextWidget m_AMIACETiltHeadText;
	protected TextWidget m_AMIACEClearAirwayText;
	protected ButtonWidget m_AMIACERespiratoryRateButton;
	protected ButtonWidget m_AMIACESpO2Button;
	protected TextWidget m_AMIACERespiratoryRateText;
	protected TextWidget m_AMIACESpO2Text;
	protected TextWidget m_AMIACERespiratoryRateValue;
	protected TextWidget m_AMIACESpO2Value;
	protected bool m_AMIACEIsRespiratoryRateMonitored;
	protected bool m_AMIACEIsSpO2Monitored;
	protected Widget m_AMIACEAirwayStatusIcon;
	protected Widget m_AMIACEPneumothoraxStatusIcon;

	override void ShowPage(string pageName)
	{
		AMIACEClearBreathingWidgets();
		AMIACEClearPageWidgets();
		super.ShowPage(pageName);
		AMIACECreateBreathingStatusIcons();
		if (pageName == "PageDiagnose")
		{
			AMIACECreateDiagnoseControls();
			return;
		}
		if (pageName == "PageAdvanced")
		{
			AMIACECreateAirwayControls();
			return;
		}
		if (pageName != "PageBandages" || !m_InventoryRoot)
			return;

		AMIACECreateBreathingControls();
		AMIACEUpdateBreathingItems();
		UpdateTreatmentButtons();
	}

	protected void AMIACEClearBreathingWidgets()
	{
		m_AMIACEBreathingRows.Clear();
		m_AMIACEBreathingButtons.Clear();
		m_AMIACEBreathingContents.Clear();
		m_AMIACEBreathingIcons.Clear();
		m_AMIACEBreathingLabelsWidgets.Clear();
		m_AMIACEBreathingCounts.Clear();
		m_AMIACEStandardRows.Clear();
		m_AMIACEBreathingToggleButton = null;
		m_AMIACEBreathingToggleContent = null;
		m_AMIACEBreathingToggleLabel = null;
		m_AMIACEBreathingOpen = false;
	}

	protected void AMIACEClearPageWidgets()
	{
		m_AMIACETiltHeadButton = null;
		m_AMIACEClearAirwayButton = null;
		m_AMIACETiltHeadText = null;
		m_AMIACEClearAirwayText = null;
		m_AMIACERespiratoryRateButton = null;
		m_AMIACESpO2Button = null;
		m_AMIACERespiratoryRateText = null;
		m_AMIACESpO2Text = null;
		m_AMIACERespiratoryRateValue = null;
		m_AMIACESpO2Value = null;
		m_AMIACEAirwayStatusIcon = null;
		m_AMIACEPneumothoraxStatusIcon = null;
	}

	protected void AMIACECreateBreathingStatusIcons()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_PageRoot)
			return;
		Widget bodyModel = m_PageRoot.FindAnyWidget("BodyModel");
		if (!bodyModel)
			return;

		m_AMIACEAirwayStatusIcon = workspace.CreateWidgets(AMI_ACE_STATUS_ICON_LAYOUT, bodyModel);
		m_AMIACEPneumothoraxStatusIcon = workspace.CreateWidgets(AMI_ACE_STATUS_ICON_LAYOUT, bodyModel);
		AMIACEPrepareBreathingStatusIcon(m_AMIACEAirwayStatusIcon, "AMIACEAirwayStatus", "Lungs_UI", 0.7, 0.1);
		AMIACEPrepareBreathingStatusIcon(m_AMIACEPneumothoraxStatusIcon, "AMIACEPneumothoraxStatus", "TPTX_UI", 0.8, 0.1);
		AMIACEUpdateBreathingStatusIcons();
	}

	protected void AMIACEPrepareBreathingStatusIcon(Widget root, string name, string imageName, float anchorX, float anchorY)
	{
		if (!root)
			return;

		root.SetName(name);
		FrameSlot.SetAnchorMin(root, anchorX, anchorY);
		FrameSlot.SetAnchorMax(root, anchorX, anchorY);
		FrameSlot.SetPos(root, -64, 0);
		FrameSlot.SetSize(root, 64, 64);

		array<string> removeNames = {"Background", "Button", "Outline"};
		foreach (string removeName : removeNames)
		{
			Widget widget = root.FindAnyWidget(removeName);
			if (widget)
				widget.RemoveFromHierarchy();
		}

		ImageWidget icon = ImageWidget.Cast(root.FindAnyWidget("Icon"));
		if (icon)
		{
			icon.SetName(name + "Icon");
			icon.LoadImageFromSet(0, AMI_ACE_STATUS_ICON_SET, imageName);
			icon.SetColor(Color.FromSRGBA(255, 255, 0, 255));
		}
		root.SetVisible(false);
	}

	override protected void UpdatePatientVitals()
	{
		super.UpdatePatientVitals();
		AMIACEUpdateBreathingStatusIcons();
	}

	protected void AMIACEUpdateBreathingStatusIcons()
	{
		ACE_Medical_VitalsComponent vitals;
		if (s_Patient)
			vitals = ACE_Medical_VitalsComponent.Cast(s_Patient.FindComponent(ACE_Medical_VitalsComponent));

		if (m_AMIACEAirwayStatusIcon)
			m_AMIACEAirwayStatusIcon.SetVisible(vitals && (vitals.IsAirwayObstructed() || vitals.IsAirwayOccluded()));
		if (m_AMIACEPneumothoraxStatusIcon)
			m_AMIACEPneumothoraxStatusIcon.SetVisible(vitals && (vitals.GetPneumothoraxScale() > 0 || vitals.HasTensionPneumothorax()));
	}

	protected void AMIACECreateAirwayControls()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_PageRoot)
			return;

		Widget root = workspace.CreateWidgets(PAGE_ADVANCED_LAYOUT, m_PageRoot);
		if (!root)
			return;

		m_AMIACETiltHeadButton = ButtonWidget.Cast(root.FindAnyWidget("AdvancedBackButton"));
		m_AMIACEClearAirwayButton = ButtonWidget.Cast(root.FindAnyWidget("AdvancedRightSideButton"));
		m_AMIACETiltHeadText = TextWidget.Cast(root.FindAnyWidget("AdvancedBackText"));
		m_AMIACEClearAirwayText = TextWidget.Cast(root.FindAnyWidget("AdvancedRightSideText"));
		array<string> removeNames = {
			"PageTitle",
			"AdvancedCPRButton",
			"AdvancedCarryButton",
			"AdvancedDragButton",
			"AdvancedLeftSideButton",
			"AdvancedLoadVehicleButton"
		};
		foreach (string removeName : removeNames)
		{
			Widget widget = root.FindAnyWidget(removeName);
			if (widget)
				widget.RemoveFromHierarchy();
		}

		root.SetName("AMIACEAirwayControlsRoot");
		if (m_AMIACETiltHeadButton)
		{
			m_AMIACETiltHeadButton.SetName(AMI_ACE_TILT_HEAD + "Button");
			FrameSlot.SetAnchorMin(m_AMIACETiltHeadButton, 0.27, 0.78);
			FrameSlot.SetAnchorMax(m_AMIACETiltHeadButton, 0.49, 0.89);
			FrameSlot.SetOffsets(m_AMIACETiltHeadButton, 0, 0, 0, 0);
			m_AMIACETiltHeadButton.AddHandler(m_Handler);
		}
		if (m_AMIACEClearAirwayButton)
		{
			m_AMIACEClearAirwayButton.SetName(AMI_ACE_CLEAR_AIRWAY + "Button");
			FrameSlot.SetAnchorMin(m_AMIACEClearAirwayButton, 0.51, 0.78);
			FrameSlot.SetAnchorMax(m_AMIACEClearAirwayButton, 0.73, 0.89);
			FrameSlot.SetOffsets(m_AMIACEClearAirwayButton, 0, 0, 0, 0);
			m_AMIACEClearAirwayButton.AddHandler(m_Handler);
		}
		if (m_AMIACETiltHeadText)
		{
			m_AMIACETiltHeadText.SetName(AMI_ACE_TILT_HEAD + "Text");
			m_AMIACETiltHeadText.SetText("#ACE_Medical-UserAction_TiltHead");
		}
		if (m_AMIACEClearAirwayText)
		{
			m_AMIACEClearAirwayText.SetName(AMI_ACE_CLEAR_AIRWAY + "Text");
			m_AMIACEClearAirwayText.SetText("#ACE_Medical-UserAction_ClearVomit");
		}

		UpdateAdvancedButtons();
		AMIACEConfigureAdvancedNavigation();
		GetGame().GetCallqueue().CallLater(AMIACEConfigureAdvancedNavigation, 2);
	}

	protected void AMIACECreateDiagnoseControls()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_PageRoot)
			return;

		Widget widget = m_PageRoot.FindAnyWidget("DiagnoseHeartRateGraph");
		if (widget)
		{
			FrameSlot.SetAnchorMin(widget, 0.04, 0.35);
			FrameSlot.SetAnchorMax(widget, 0.49, 0.67);
			FrameSlot.SetOffsets(widget, 0, 0, 0, 0);
		}
		widget = m_PageRoot.FindAnyWidget("DiagnoseBloodPressureGraph");
		if (widget)
		{
			FrameSlot.SetAnchorMin(widget, 0.58, 0.36);
			FrameSlot.SetAnchorMax(widget, 0.96, 0.67);
			FrameSlot.SetOffsets(widget, 0, 0, 0, 0);
		}
		widget = m_PageRoot.FindAnyWidget("DiagnoseBloodPressureMiddle");
		if (widget)
		{
			FrameSlot.SetAnchorMin(widget, 0.54, 0.49);
			FrameSlot.SetAnchorMax(widget, 0.575, 0.53);
			FrameSlot.SetOffsets(widget, 0, 0, 0, 0);
		}
		widget = m_PageRoot.FindAnyWidget("DiagnoseBloodPressureLow");
		if (widget)
		{
			FrameSlot.SetAnchorMin(widget, 0.54, 0.63);
			FrameSlot.SetAnchorMax(widget, 0.575, 0.67);
			FrameSlot.SetOffsets(widget, 0, 0, 0, 0);
		}
		widget = m_PageRoot.FindAnyWidget("DiagnoseBloodPressureXAxis");
		if (widget)
		{
			FrameSlot.SetAnchorMin(widget, 0.79, 0.68);
			FrameSlot.SetAnchorMax(widget, 0.96, 0.73);
			FrameSlot.SetOffsets(widget, 0, 0, 0, 0);
		}

		Widget root = workspace.CreateWidgets(PAGE_DIAGNOSE_LAYOUT, m_PageRoot);
		if (!root)
			return;

		m_AMIACERespiratoryRateButton = ButtonWidget.Cast(root.FindAnyWidget("DiagnoseHeartRateButton"));
		m_AMIACESpO2Button = ButtonWidget.Cast(root.FindAnyWidget("DiagnoseBloodPressureButton"));
		m_AMIACERespiratoryRateText = TextWidget.Cast(root.FindAnyWidget("DiagnoseHeartRateText"));
		m_AMIACESpO2Text = TextWidget.Cast(root.FindAnyWidget("DiagnoseBloodPressureText"));
		m_AMIACERespiratoryRateValue = TextWidget.Cast(root.FindAnyWidget("DiagnoseHeartRateValue"));
		m_AMIACESpO2Value = TextWidget.Cast(root.FindAnyWidget("DiagnoseBloodPressureValue"));
		array<string> removeNames = {
			"PageTitle",
			"DiagnosePatientName",
			"DiagnoseHeartRateGraph",
			"DiagnoseHeartRhythmLabel",
			"DiagnoseSystolicLegend",
			"DiagnoseDiastolicLegend",
			"DiagnoseBloodPressureGraph",
			"DiagnoseBloodPressureHigh",
			"DiagnoseBloodPressureMiddle",
			"DiagnoseBloodPressureLow",
			"DiagnoseBloodPressureYAxis",
			"DiagnoseBloodPressureXAxis"
		};
		foreach (string removeName : removeNames)
		{
			Widget removeWidget = root.FindAnyWidget(removeName);
			if (removeWidget)
				removeWidget.RemoveFromHierarchy();
		}

		root.SetName("AMIACEDiagnoseControlsRoot");
		m_AMIACERespiratoryRateButton.SetName(AMI_ACE_RESPIRATORY_RATE + "Button");
		m_AMIACESpO2Button.SetName(AMI_ACE_SPO2 + "Button");
		m_AMIACERespiratoryRateText.SetName(AMI_ACE_RESPIRATORY_RATE + "Text");
		m_AMIACESpO2Text.SetName(AMI_ACE_SPO2 + "Text");
		m_AMIACERespiratoryRateValue.SetName(AMI_ACE_RESPIRATORY_RATE + "Value");
		m_AMIACESpO2Value.SetName(AMI_ACE_SPO2 + "Value");

		FrameSlot.SetAnchorMin(m_AMIACERespiratoryRateButton, 0.04, 0.76);
		FrameSlot.SetAnchorMax(m_AMIACERespiratoryRateButton, 0.3, 0.84);
		FrameSlot.SetOffsets(m_AMIACERespiratoryRateButton, 0, 0, 0, 0);
		FrameSlot.SetAnchorMin(m_AMIACERespiratoryRateValue, 0.31, 0.76);
		FrameSlot.SetAnchorMax(m_AMIACERespiratoryRateValue, 0.49, 0.84);
		FrameSlot.SetOffsets(m_AMIACERespiratoryRateValue, 0, 0, 0, 0);
		FrameSlot.SetAnchorMin(m_AMIACESpO2Button, 0.54, 0.76);
		FrameSlot.SetAnchorMax(m_AMIACESpO2Button, 0.8, 0.84);
		FrameSlot.SetOffsets(m_AMIACESpO2Button, 0, 0, 0, 0);
		FrameSlot.SetAnchorMin(m_AMIACESpO2Value, 0.81, 0.76);
		FrameSlot.SetAnchorMax(m_AMIACESpO2Value, 0.96, 0.84);
		FrameSlot.SetOffsets(m_AMIACESpO2Value, 0, 0, 0, 0);
		m_AMIACERespiratoryRateButton.AddHandler(m_Handler);
		m_AMIACESpO2Button.AddHandler(m_Handler);

		AMIACEUpdateDiagnoseButtonText();
		AMIACEUpdateBreathingMonitor();
		AMIACEConfigureDiagnoseNavigation();
		GetGame().GetCallqueue().CallLater(AMIACEConfigureDiagnoseNavigation, 2);
	}

	protected void AMIACECreateBreathingControls()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		m_AMIACEStandardRows.Insert(m_InventoryRoot.FindAnyWidget("BandageRow"));
		m_AMIACEStandardRows.Insert(m_InventoryRoot.FindAnyWidget("TourniquetRow"));
		m_AMIACEStandardRows.Insert(m_InventoryRoot.FindAnyWidget("MedicalKitRow"));

		Widget toggleRow = workspace.CreateWidgets(PAGE_TREATMENT_CONTROLS_LAYOUT, m_InventoryRoot);
		if (!toggleRow)
			return;
		AMIACEPrepareRowTemplate(toggleRow);
		FrameSlot.SetAnchorMin(toggleRow, 0, 0);
		FrameSlot.SetAnchorMax(toggleRow, 1, 0.068493);
		FrameSlot.SetOffsets(toggleRow, 0, 0, 0, 0);
		m_AMIACEBreathingToggleButton = ButtonWidget.Cast(toggleRow.FindAnyWidget("BandageButton"));
		m_AMIACEBreathingToggleContent = toggleRow.FindAnyWidget("BandageButtonContent");
		m_AMIACEBreathingToggleLabel = TextWidget.Cast(toggleRow.FindAnyWidget("BandageLabel"));
		ItemPreviewWidget toggleIcon = ItemPreviewWidget.Cast(toggleRow.FindAnyWidget("BandageIcon"));
		TextWidget toggleCount = TextWidget.Cast(toggleRow.FindAnyWidget("BandageCount"));
		AMIACERenameAndBind(toggleRow, AMI_ACE_BREATHING_TOGGLE);
		if (toggleIcon)
			toggleIcon.SetVisible(false);
		if (toggleCount)
			toggleCount.SetVisible(false);
		if (m_AMIACEBreathingToggleLabel)
		{
			FrameSlot.SetAnchorMin(m_AMIACEBreathingToggleLabel, 0, 0);
			FrameSlot.SetAnchorMax(m_AMIACEBreathingToggleLabel, 1, 1);
			FrameSlot.SetOffsets(m_AMIACEBreathingToggleLabel, 0, 0, 0, 0);
		}
		TextWidget selfTitle = TextWidget.Cast(m_InventoryRoot.FindAnyWidget("SelfinventoryTitle"));
		if (selfTitle)
			selfTitle.SetVisible(false);

		for (int index = 0; index < m_AMIACEBreathingTypes.Count(); index++)
		{
			Widget row = workspace.CreateWidgets(PAGE_TREATMENT_CONTROLS_LAYOUT, m_InventoryRoot);
			if (!row)
				continue;
			AMIACEPrepareRowTemplate(row);

			float minY = 0.082192 + index * 0.107877;
			FrameSlot.SetAnchorMin(row, 0, minY);
			FrameSlot.SetAnchorMax(row, 1, minY + 0.097877);
			FrameSlot.SetOffsets(row, 0, 0, 0, 0);
			ButtonWidget button = ButtonWidget.Cast(row.FindAnyWidget("BandageButton"));
			Widget content = row.FindAnyWidget("BandageButtonContent");
			ItemPreviewWidget icon = ItemPreviewWidget.Cast(row.FindAnyWidget("BandageIcon"));
			TextWidget label = TextWidget.Cast(row.FindAnyWidget("BandageLabel"));
			TextWidget count = TextWidget.Cast(row.FindAnyWidget("BandageCount"));
			AMIACERenameAndBind(row, m_AMIACEBreathingNames[index]);
			if (label)
				label.SetText(m_AMIACEBreathingLabels[index]);
			row.SetVisible(false);
			m_AMIACEBreathingRows.Insert(row);
			m_AMIACEBreathingButtons.Insert(button);
			m_AMIACEBreathingContents.Insert(content);
			m_AMIACEBreathingIcons.Insert(icon);
			m_AMIACEBreathingLabelsWidgets.Insert(label);
			m_AMIACEBreathingCounts.Insert(count);
		}
	}

	protected void AMIACEPrepareRowTemplate(Widget root)
	{
		array<string> removeNames = {
			"SelfinventoryTitle",
			"TourniquetRow",
			"MedicalKitRow",
			"ForeigninventoryTitle",
			"BandageRowForeign",
			"TourniquetRowForeign"
		};
		foreach (string removeName : removeNames)
		{
			Widget widget = root.FindAnyWidget(removeName);
			if (widget)
				widget.RemoveFromHierarchy();
		}

		Widget row = root.FindAnyWidget("BandageRow");
		if (row)
		{
			FrameSlot.SetAnchorMin(row, 0, 0);
			FrameSlot.SetAnchorMax(row, 1, 1);
			FrameSlot.SetOffsets(row, 0, 0, 0, 0);
		}
	}

	protected void AMIACERenameAndBind(Widget row, string prefix)
	{
		Widget sourceRow = row.FindAnyWidget("BandageRow");
		array<string> sourceNames = {"BandageButton", "BandageButtonContent", "BandageIcon", "BandageLabel", "BandageCount"};
		array<string> suffixes = {"Button", "Content", "Icon", "Label", "Count"};
		for (int index = 0; index < sourceNames.Count(); index++)
		{
			Widget widget = row.FindAnyWidget(sourceNames[index]);
			if (!widget)
				continue;
			widget.SetName(prefix + suffixes[index]);
			widget.AddHandler(m_Handler);
		}
		if (sourceRow)
			sourceRow.SetName(prefix + "Row");
		row.SetName(prefix + "Root");
	}

	override protected void UpdateTreatmentItems()
	{
		super.UpdateTreatmentItems();
		AMIACEUpdateBreathingItems();
	}

	protected void AMIACEUpdateBreathingItems()
	{
		if (m_AMIACEBreathingButtons.Count() != m_AMIACEBreathingTypes.Count())
			return;

		array<IEntity> items = {};
		GetTreatmentInventoryItems(items);
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		bool canTreat = CanTreatPatient();
		for (int index = 0; index < m_AMIACEBreathingTypes.Count(); index++)
		{
			int count;
			bool canApply;
			IEntity previewItem;
			foreach (IEntity item : items)
			{
				SCR_ConsumableItemComponent consumable;
				SCR_ConsumableEffectHealthItems effect = GetTreatmentEffect(item, consumable);
				if (!consumable || consumable.GetConsumableType() != m_AMIACEBreathingTypes[index])
					continue;

				count++;
				if (!previewItem)
					previewItem = item;
				int failReason;
				if (canTreat && AMIACECanApplyBreathingTypeToSelectedRegion(m_AMIACEBreathingTypes[index]) && effect && effect.CanApplyEffectToHZ(s_Patient, user, m_SelectedRegion, failReason))
					canApply = true;
			}

			m_AMIACEBreathingCounts[index].SetText(count.ToString());
			SetTreatmentPreview(m_AMIACEBreathingIcons[index], previewItem);
			SetTreatmentButtonState(
				m_AMIACEBreathingButtons[index],
				m_AMIACEBreathingContents[index],
				m_AMIACEBreathingLabelsWidgets[index],
				m_AMIACEBreathingCounts[index],
				m_AMIACEBreathingIcons[index],
				canTreat && count > 0 && canApply
			);
		}
		UpdateRequestedButtonStyles();
	}

	protected IEntity AMIACEFindBreathingItem(SCR_EConsumableType type)
	{
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user || !CanTreatPatient() || !AMIACECanApplyBreathingTypeToSelectedRegion(type))
			return null;

		array<IEntity> items = {};
		GetTreatmentInventoryItems(items);
		foreach (IEntity item : items)
		{
			SCR_ConsumableItemComponent consumable;
			SCR_ConsumableEffectHealthItems effect = GetTreatmentEffect(item, consumable);
			if (!consumable || consumable.GetConsumableType() != type)
				continue;

			int failReason;
			if (effect && effect.CanApplyEffectToHZ(s_Patient, user, m_SelectedRegion, failReason))
				return item;
		}
		return null;
	}

	protected bool AMIACECanApplyBreathingTypeToSelectedRegion(SCR_EConsumableType type)
	{
		if (type == SCR_EConsumableType.ACE_MEDICAL_LARYNGEAL_TUBE || type == SCR_EConsumableType.ACE_MEDICAL_OXYGEN_MASK)
			return m_SelectedRegion == ECharacterHitZoneGroup.HEAD;
		return true;
	}

	override protected BaseUserAction FindAdvancedAction(string widgetName)
	{
		if (widgetName != AMI_ACE_TILT_HEAD + "Button" && widgetName != AMI_ACE_CLEAR_AIRWAY + "Button")
			return super.FindAdvancedAction(widgetName);
		if (!s_Patient)
			return null;

		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(s_Patient.FindComponent(ActionsManagerComponent));
		if (!actionsManager)
			return null;

		array<BaseUserAction> actions = {};
		actionsManager.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			if (widgetName == AMI_ACE_TILT_HEAD + "Button" && ACE_Medical_TiltHeadUserAction.Cast(action))
				return action;
			if (widgetName == AMI_ACE_CLEAR_AIRWAY + "Button" && ACE_Medical_ClearVomitAction.Cast(action))
				return action;
		}
		return null;
	}

	override protected void UpdateAdvancedButtons()
	{
		super.UpdateAdvancedButtons();
		SetAdvancedButtonState(m_AMIACETiltHeadButton, CanUseAdvancedAction(AMI_ACE_TILT_HEAD + "Button"));
		SetAdvancedButtonState(m_AMIACEClearAirwayButton, CanUseAdvancedAction(AMI_ACE_CLEAR_AIRWAY + "Button"));
		AMIACEConfigureAdvancedNavigation();
		UpdateRequestedButtonStyles();
	}

	protected void AMIACEToggleDiagnoseMonitor(string widgetName)
	{
		if (widgetName.StartsWith(AMI_ACE_RESPIRATORY_RATE))
			m_AMIACEIsRespiratoryRateMonitored = !m_AMIACEIsRespiratoryRateMonitored;
		else if (widgetName.StartsWith(AMI_ACE_SPO2))
			m_AMIACEIsSpO2Monitored = !m_AMIACEIsSpO2Monitored;

		AMIACEUpdateDiagnoseButtonText();
		GetGame().GetCallqueue().Remove(AMIACEUpdateBreathingMonitor);
		if (!m_AMIACEIsRespiratoryRateMonitored && !m_AMIACEIsSpO2Monitored)
			return;

		AMIACEUpdateBreathingMonitor();
		GetGame().GetCallqueue().CallLater(AMIACEUpdateBreathingMonitor, DIAGNOSE_UPDATE_INTERVAL_MS, true);
	}

	protected void AMIACEUpdateDiagnoseButtonText()
	{
		if (m_AMIACERespiratoryRateText)
		{
			if (m_AMIACEIsRespiratoryRateMonitored)
				m_AMIACERespiratoryRateText.SetText("STOP RESPIRATORY RATE");
			else
				m_AMIACERespiratoryRateText.SetText("MONITOR RESPIRATORY RATE");
		}
		if (m_AMIACESpO2Text)
		{
			if (m_AMIACEIsSpO2Monitored)
				m_AMIACESpO2Text.SetText("STOP OXYGEN SATURATION");
			else
				m_AMIACESpO2Text.SetText("MONITOR OXYGEN SATURATION");
		}
		if (!m_AMIACEIsRespiratoryRateMonitored && m_AMIACERespiratoryRateValue)
			m_AMIACERespiratoryRateValue.SetText("NOT MONITORED");
		if (!m_AMIACEIsSpO2Monitored && m_AMIACESpO2Value)
			m_AMIACESpO2Value.SetText("NOT MONITORED");
		UpdateRequestedButtonStyles();
	}

	protected void AMIACEUpdateBreathingMonitor()
	{
		if (!m_AMIACEIsRespiratoryRateMonitored && !m_AMIACEIsSpO2Monitored)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		ACE_Medical_NetworkComponent networkComponent;
		if (playerController)
			networkComponent = ACE_Medical_NetworkComponent.Cast(playerController.FindComponent(ACE_Medical_NetworkComponent));
		if (!networkComponent || !s_Patient)
		{
			if (m_AMIACEIsRespiratoryRateMonitored && m_AMIACERespiratoryRateValue)
				m_AMIACERespiratoryRateValue.SetText("UNAVAILABLE");
			if (m_AMIACEIsSpO2Monitored && m_AMIACESpO2Value)
				m_AMIACESpO2Value.SetText("UNAVAILABLE");
			return;
		}

		float respiratoryRate;
		float spO2;
		if (networkComponent.AMIACEGetBreathingVitals(s_Patient, respiratoryRate, spO2))
		{
			if (m_AMIACEIsRespiratoryRateMonitored && m_AMIACERespiratoryRateValue)
				m_AMIACERespiratoryRateValue.SetText(string.Format("%1 /MIN", Math.Round(respiratoryRate)));
			if (m_AMIACEIsSpO2Monitored && m_AMIACESpO2Value)
				m_AMIACESpO2Value.SetText(string.Format("%1 %%", Math.Round(spO2)));
		}
		else
		{
			if (m_AMIACEIsRespiratoryRateMonitored && m_AMIACERespiratoryRateValue)
				m_AMIACERespiratoryRateValue.SetText("READING...");
			if (m_AMIACEIsSpO2Monitored && m_AMIACESpO2Value)
				m_AMIACESpO2Value.SetText("READING...");
		}
		networkComponent.AMIACERequestBreathingVitals(s_Patient);
	}

	override bool UseMedicationWidget(string widgetName)
	{
		if (widgetName.StartsWith(AMI_ACE_TILT_HEAD) || widgetName.StartsWith(AMI_ACE_CLEAR_AIRWAY))
		{
			UseAdvancedAction(widgetName);
			return true;
		}
		if (widgetName.StartsWith(AMI_ACE_RESPIRATORY_RATE) || widgetName.StartsWith(AMI_ACE_SPO2))
		{
			AMIACEToggleDiagnoseMonitor(widgetName);
			return true;
		}
		if (widgetName.StartsWith(AMI_ACE_BREATHING_TOGGLE))
		{
			m_AMIACEBreathingOpen = !m_AMIACEBreathingOpen;
			AMIACEApplyBreathingVisibility();
			UpdateTreatmentButtons();
			return true;
		}

		for (int index = 0; index < m_AMIACEBreathingNames.Count(); index++)
		{
			if (!widgetName.StartsWith(m_AMIACEBreathingNames[index]))
				continue;
			if (m_CurrentPageName == "PageBandages" && m_AMIACEBreathingOpen)
				StartTreatmentWithItem(AMIACEFindBreathingItem(m_AMIACEBreathingTypes[index]));
			return true;
		}

		return super.UseMedicationWidget(widgetName);
	}

	override protected void UpdateTreatmentButtons()
	{
		super.UpdateTreatmentButtons();
		AMIACEApplyBreathingVisibility();
	}

	protected void AMIACEApplyBreathingVisibility()
	{
		if (!m_AMIACEBreathingToggleButton)
			return;

		if (m_AMIACEStandardRows.Count() > 0 && m_AMIACEStandardRows[0])
			m_AMIACEStandardRows[0].SetVisible(!m_AMIACEBreathingOpen);
		if (m_AMIACEStandardRows.Count() > 1 && m_AMIACEStandardRows[1])
			m_AMIACEStandardRows[1].SetVisible(!m_AMIACEBreathingOpen);
		if (m_AMIACEBreathingOpen && m_AMIACEStandardRows.Count() > 2 && m_AMIACEStandardRows[2])
			m_AMIACEStandardRows[2].SetVisible(false);
		foreach (Widget row : m_AMIACEBreathingRows)
			row.SetVisible(m_AMIACEBreathingOpen);
		if (m_AMIACEBreathingToggleLabel)
		{
			if (m_AMIACEBreathingOpen)
				m_AMIACEBreathingToggleLabel.SetText("STANDARD ITEMS");
			else
				m_AMIACEBreathingToggleLabel.SetText("BREATHING ITEMS");
		}
		ApplyRequestedButtonStyle(m_AMIACEBreathingToggleButton, m_AMIACEBreathingToggleLabel, null, true);
	}

	override protected ButtonWidget GetRequestedButton(Widget widget)
	{
		ButtonWidget button = super.GetRequestedButton(widget);
		if (button || !widget)
			return button;

		string widgetName = widget.GetName();
		if (widgetName.StartsWith(AMI_ACE_TILT_HEAD))
			return m_AMIACETiltHeadButton;
		if (widgetName.StartsWith(AMI_ACE_CLEAR_AIRWAY))
			return m_AMIACEClearAirwayButton;
		if (widgetName.StartsWith(AMI_ACE_RESPIRATORY_RATE))
			return m_AMIACERespiratoryRateButton;
		if (widgetName.StartsWith(AMI_ACE_SPO2))
			return m_AMIACESpO2Button;
		if (widgetName.StartsWith(AMI_ACE_BREATHING_TOGGLE))
			return m_AMIACEBreathingToggleButton;
		for (int index = 0; index < m_AMIACEBreathingNames.Count(); index++)
		{
			if (widgetName.StartsWith(m_AMIACEBreathingNames[index]) && index < m_AMIACEBreathingButtons.Count())
				return m_AMIACEBreathingButtons[index];
		}
		return null;
	}

	override protected void UpdateRequestedButtonStyles()
	{
		super.UpdateRequestedButtonStyles();
		ApplyRequestedButtonStyle(m_AMIACETiltHeadButton, m_AMIACETiltHeadText, null, true);
		ApplyRequestedButtonStyle(m_AMIACEClearAirwayButton, m_AMIACEClearAirwayText, null, true);
		ApplyRequestedButtonStyle(m_AMIACERespiratoryRateButton, m_AMIACERespiratoryRateText, null, true);
		ApplyRequestedButtonStyle(m_AMIACESpO2Button, m_AMIACESpO2Text, null, true);
		if (m_AMIACEIsRespiratoryRateMonitored && m_AMIACERespiratoryRateButton)
		{
			m_AMIACERespiratoryRateButton.SetColor(Color.FromSRGBA(51, 179, 255, 153));
			ApplyRequestedButtonText(m_AMIACERespiratoryRateText, true, true);
		}
		if (m_AMIACEIsSpO2Monitored && m_AMIACESpO2Button)
		{
			m_AMIACESpO2Button.SetColor(Color.FromSRGBA(51, 179, 255, 153));
			ApplyRequestedButtonText(m_AMIACESpO2Text, true, true);
		}
		ApplyRequestedButtonStyle(m_AMIACEBreathingToggleButton, m_AMIACEBreathingToggleLabel, null, true);
		for (int index = 0; index < m_AMIACEBreathingButtons.Count(); index++)
			ApplyRequestedButtonStyle(m_AMIACEBreathingButtons[index], m_AMIACEBreathingLabelsWidgets[index], m_AMIACEBreathingCounts[index], true);
	}

	override protected void UpdateTreatmentItemNavigation()
	{
		super.UpdateTreatmentItemNavigation();
		if (!m_AMIACEBreathingToggleButton || m_CurrentPageName != "PageBandages")
			return;

		SetControllerNavigation("PageBandages", WidgetNavigationDirection.DOWN, AMI_ACE_BREATHING_TOGGLE + "Button");
		SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.DOWN, AMI_ACE_BREATHING_TOGGLE + "Button");
		SetControllerNavigation(AMI_ACE_BREATHING_TOGGLE + "Button", WidgetNavigationDirection.UP, "PageBandages");

		array<ButtonWidget> availableItems = {};
		if (m_AMIACEBreathingOpen)
		{
			foreach (ButtonWidget breathingButton : m_AMIACEBreathingButtons)
			{
				if (breathingButton && breathingButton.IsEnabled())
					availableItems.Insert(breathingButton);
			}
			if (m_BandageButtonForeign && m_BandageButtonForeign.IsEnabled())
				availableItems.Insert(m_BandageButtonForeign);
			if (m_TourniquetButtonForeign && m_TourniquetButtonForeign.IsEnabled())
				availableItems.Insert(m_TourniquetButtonForeign);
		}
		else
		{
			array<ButtonWidget> standardButtons = {m_BandageButton, m_TourniquetButton, m_MedicalKitButton, m_BandageButtonForeign, m_TourniquetButtonForeign};
			foreach (ButtonWidget standardButton : standardButtons)
			{
				if (standardButton && standardButton.IsEnabled())
					availableItems.Insert(standardButton);
			}
		}

		string firstItem;
		if (!availableItems.IsEmpty())
			firstItem = availableItems[0].GetName();
		SetControllerNavigation(AMI_ACE_BREATHING_TOGGLE + "Button", WidgetNavigationDirection.DOWN, firstItem);
		SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.LEFT, AMI_ACE_BREATHING_TOGGLE + "Button");
		SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.LEFT, AMI_ACE_BREATHING_TOGGLE + "Button");
		for (int index = 0; index < availableItems.Count(); index++)
		{
			string upTarget = AMI_ACE_BREATHING_TOGGLE + "Button";
			if (index > 0)
				upTarget = availableItems[index - 1].GetName();
			string downTarget;
			if (index + 1 < availableItems.Count())
				downTarget = availableItems[index + 1].GetName();
			SetControllerNavigation(availableItems[index].GetName(), WidgetNavigationDirection.UP, upTarget);
			SetControllerNavigation(availableItems[index].GetName(), WidgetNavigationDirection.DOWN, downTarget);
		}
	}

	protected void AMIACEConfigureAdvancedNavigation()
	{
		if (!m_AMIACETiltHeadButton || m_CurrentPageName != "PageAdvanced")
			return;

		array<string> movementButtons = {"AdvancedCarryButton", "AdvancedDragButton", "AdvancedLoadVehicleButton"};
		foreach (string movementButton : movementButtons)
			SetControllerNavigation(movementButton, WidgetNavigationDirection.DOWN, AMI_ACE_TILT_HEAD + "Button");
		SetControllerNavigation(AMI_ACE_TILT_HEAD + "Button", WidgetNavigationDirection.LEFT, AMI_ACE_CLEAR_AIRWAY + "Button");
		SetControllerNavigation(AMI_ACE_TILT_HEAD + "Button", WidgetNavigationDirection.RIGHT, AMI_ACE_CLEAR_AIRWAY + "Button");
		SetControllerNavigation(AMI_ACE_TILT_HEAD + "Button", WidgetNavigationDirection.UP, "AdvancedCarryButton");
		SetControllerNavigation(AMI_ACE_TILT_HEAD + "Button", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation(AMI_ACE_CLEAR_AIRWAY + "Button", WidgetNavigationDirection.LEFT, AMI_ACE_TILT_HEAD + "Button");
		SetControllerNavigation(AMI_ACE_CLEAR_AIRWAY + "Button", WidgetNavigationDirection.RIGHT, AMI_ACE_TILT_HEAD + "Button");
		SetControllerNavigation(AMI_ACE_CLEAR_AIRWAY + "Button", WidgetNavigationDirection.UP, "AdvancedLoadVehicleButton");
		SetControllerNavigation(AMI_ACE_CLEAR_AIRWAY + "Button", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, AMI_ACE_CLEAR_AIRWAY + "Button");
	}

	protected void AMIACEConfigureDiagnoseNavigation()
	{
		if (!m_AMIACERespiratoryRateButton || m_CurrentPageName != "PageDiagnose")
			return;

		SetControllerNavigation("DiagnoseHeartRateButton", WidgetNavigationDirection.DOWN, AMI_ACE_RESPIRATORY_RATE + "Button");
		SetControllerNavigation("DiagnoseBloodPressureButton", WidgetNavigationDirection.DOWN, AMI_ACE_SPO2 + "Button");
		SetControllerNavigation(AMI_ACE_RESPIRATORY_RATE + "Button", WidgetNavigationDirection.LEFT, AMI_ACE_SPO2 + "Button");
		SetControllerNavigation(AMI_ACE_RESPIRATORY_RATE + "Button", WidgetNavigationDirection.RIGHT, AMI_ACE_SPO2 + "Button");
		SetControllerNavigation(AMI_ACE_RESPIRATORY_RATE + "Button", WidgetNavigationDirection.UP, "DiagnoseHeartRateButton");
		SetControllerNavigation(AMI_ACE_RESPIRATORY_RATE + "Button", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation(AMI_ACE_SPO2 + "Button", WidgetNavigationDirection.LEFT, AMI_ACE_RESPIRATORY_RATE + "Button");
		SetControllerNavigation(AMI_ACE_SPO2 + "Button", WidgetNavigationDirection.RIGHT, AMI_ACE_RESPIRATORY_RATE + "Button");
		SetControllerNavigation(AMI_ACE_SPO2 + "Button", WidgetNavigationDirection.UP, "DiagnoseBloodPressureButton");
		SetControllerNavigation(AMI_ACE_SPO2 + "Button", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, AMI_ACE_SPO2 + "Button");
	}

	override protected void OnMenuClose()
	{
		GetGame().GetCallqueue().Remove(AMIACEUpdateBreathingMonitor);
		m_AMIACEIsRespiratoryRateMonitored = false;
		m_AMIACEIsSpO2Monitored = false;
		AMIACEClearPageWidgets();
		super.OnMenuClose();
	}
}

modded class ACE_Medical_NetworkComponent
{
	protected SCR_ChimeraCharacter m_AMIACEBreathingPatient;
	protected float m_AMIACERespiratoryRate;
	protected float m_AMIACESpO2;

	void AMIACERequestBreathingVitals(SCR_ChimeraCharacter patient)
	{
		if (patient)
			Rpc(AMIACERpcAskBreathingVitals, Replication.FindItemId(patient));
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void AMIACERpcAskBreathingVitals(RplId patientId)
	{
		SCR_ChimeraCharacter patient = SCR_ChimeraCharacter.Cast(Replication.FindItem(patientId));
		if (!patient)
			return;
		ACE_Medical_VitalsComponent vitals = ACE_Medical_VitalsComponent.Cast(patient.FindComponent(ACE_Medical_VitalsComponent));
		if (vitals)
			Rpc(AMIACERpcDoBreathingVitals, patientId, vitals.GetRespiratoryRate(), vitals.GetSpO2());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void AMIACERpcDoBreathingVitals(RplId patientId, float respiratoryRate, float spO2)
	{
		m_AMIACEBreathingPatient = SCR_ChimeraCharacter.Cast(Replication.FindItem(patientId));
		m_AMIACERespiratoryRate = respiratoryRate;
		m_AMIACESpO2 = spO2;
	}

	bool AMIACEGetBreathingVitals(SCR_ChimeraCharacter patient, out float respiratoryRate, out float spO2)
	{
		if (!patient || patient != m_AMIACEBreathingPatient)
			return false;
		respiratoryRate = m_AMIACERespiratoryRate;
		spO2 = m_AMIACESpO2;
		return true;
	}
}
