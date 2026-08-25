class RAMI_MedicalMenuUI : MenuBase
{
	static const float REFERENCE_BLOOD_VOLUME_ML = 6000;
	static const int DIAGNOSE_UPDATE_INTERVAL_MS = 1000;
	static const int DIAGNOSE_BLOOD_PRESSURE_HISTORY_COUNT = 30;
	static const int DIAGNOSE_HEART_TRACE_SAMPLE_COUNT = 360;
	static const int SALINE_FIRST_INDEX = 6;
	static const float DIAGNOSE_HEART_TRACE_SAMPLE_INTERVAL = 1.0 / 60;
	protected static const float LOW_HEART_RATE_BPM = 60;
	protected static const float HIGH_HEART_RATE_BPM = 100;
	protected static const float LOW_SYSTOLIC_PRESSURE_MMHG = 90;
	protected static const float HIGH_SYSTOLIC_PRESSURE_MMHG = 140;
	protected static const float LOW_DIASTOLIC_PRESSURE_MMHG = 60;
	protected static const float HIGH_DIASTOLIC_PRESSURE_MMHG = 90;
	protected static const float COARSE_LOW_PERCENT_THRESHOLD = 33;
	protected static const float COARSE_HIGH_PERCENT_THRESHOLD = 67;
	protected static const float SEVERE_BLEEDING_ML_PER_SECOND = 20;
	protected static const float WOUND_2_HEALTH_THRESHOLD = 0.75;
	protected static const float WOUND_3_HEALTH_THRESHOLD = 0.5;
	protected static const float WOUND_4_HEALTH_THRESHOLD = 0.25;
	protected static const ResourceName PAGE_TRIAGE_LAYOUT = "{94737247F9A1C9B1}UI/layouts/RAMI_MedicalTriage.layout";
	protected static const ResourceName PAGE_DIAGNOSE_LAYOUT = "{BBFBFF3AC26E2834}UI/layouts/RAMI_MedicalDiagnose.layout";
	protected static const ResourceName PAGE_TREATMENT_LAYOUT = "{39A5405EECF7C2F0}UI/layouts/RAMI_MedicalTreatment.layout";
	protected static const ResourceName PAGE_TREATMENT_CONTROLS_LAYOUT = "{A8D3F21C6B9047E1}UI/layouts/RAMI_MedicalTreatmentControls.layout";
	protected static const ResourceName PAGE_MEDICATION_LAYOUT = "{646C95CE11764777}UI/layouts/RAMI_MedicalMedication.layout";
	protected static const ResourceName PAGE_ADVANCED_LAYOUT = "{A4D73E19B52C680F}UI/layouts/RAMI_MedicalAdvanced.layout";
	protected static const ResourceName CONTROLLER_FOCUS_LAYOUT = "{BEF91F2C607E4C5D}UI/layouts/RAMI_ControllerFocus.layout";
	protected static const ResourceName PATIENT_ROW_LAYOUT = "{8C21A4F79D6B30E2}UI/layouts/RAMI_PatientRow.layout";
	protected static const ResourceName MEDICAL_ICON_IMAGE_SET = "{B9199157B90D6216}UI/Textures/InventoryIcons/Medical/Medical-icons.imageset";
	protected static SCR_ChimeraCharacter s_Patient;
	protected static SCR_ChimeraCharacter s_ForeignPatient;
	protected static ref map<SCR_ChimeraCharacter, ref RAMI_BodyZoneState> s_BodyZoneStates = new map<SCR_ChimeraCharacter, ref RAMI_BodyZoneState>();

	protected ref RAMI_MedicalMenuHandler m_Handler;
	protected Widget m_ContentArea;
	protected Widget m_PageRoot;
	protected Widget m_InventoryRoot;
	protected Widget m_ControllerFocus;
	protected Widget m_ActivePageHighlight;
	protected Widget m_PatientWindow;
	protected Widget m_PatientList;
	protected TextWidget m_PatientListStatus;
	protected Widget m_PatientActions;
	protected ButtonWidget m_PatientTreatButton;
	protected TextWidget m_PatientTreatText;
	protected ButtonWidget m_PatientUnloadButton;
	protected TextWidget m_PatientUnloadText;
	protected SCR_ChimeraCharacter m_PatientActionTarget;
	protected ref array<ButtonWidget> m_PatientButtons = {};
	protected ref array<SCR_ChimeraCharacter> m_ListedPatients = {};
	protected ButtonWidget m_HoveredPatientButton;
	protected ButtonWidget m_HoveredRequestedButton;
	protected ButtonWidget m_CloseButton;
	protected TextWidget m_CloseText;
	protected bool m_IsPatientWindowOpen;
	protected string m_CurrentPageName;
	protected int m_ButtonTextColor;
	protected bool m_ColorblindMode;
	protected bool m_IsMedicationPage;
	protected bool m_IsHeartRateMonitored;
	protected bool m_IsBloodPressureMonitored;
	protected ButtonWidget m_DiagnoseHeartRateButton;
	protected ButtonWidget m_DiagnoseBloodPressureButton;
	protected TextWidget m_DiagnoseHeartRateText;
	protected TextWidget m_DiagnoseBloodPressureText;
	protected TextWidget m_DiagnoseHeartRateValue;
	protected TextWidget m_DiagnoseBloodPressureValue;
	protected TextWidget m_DiagnosePatientName;
	protected CanvasWidget m_DiagnoseHeartRateGraph;
	protected CanvasWidget m_DiagnoseBloodPressureGraph;
	protected ref array<ref CanvasWidgetCommand> m_DiagnoseHeartRateGraphCommands = {};
	protected ref array<ref CanvasWidgetCommand> m_DiagnoseBloodPressureGraphCommands = {};
	protected ref array<ref CanvasWidgetCommand> m_MenuBackgroundCommands = {};
	protected ref array<ref CanvasWidgetCommand> m_PatientWindowBackgroundCommands = {};
	protected ref LineDrawCommand m_DiagnoseHeartRateLine;
	protected ref PolygonDrawCommand m_DiagnoseHeartRateDot;
	protected ref array<float> m_DiagnoseSystolicHistory = {};
	protected ref array<float> m_DiagnoseDiastolicHistory = {};
	protected ref array<float> m_DiagnoseHeartTrace = {};
	protected float m_DiagnoseHeartRate;
	protected float m_DiagnoseHeartRateTarget;
	protected float m_DiagnoseHeartPhase;
	protected float m_DiagnoseHeartSampleTime;
	protected TextWidget m_TriageActivityLog;
	protected ButtonWidget m_TriageActivityFocus;
	protected ref array<ButtonWidget> m_TriageLevelButtons = {};
	protected ref array<TextWidget> m_TriageLevelTexts = {};
	protected TextWidget m_PatientVitals;
	protected TextWidget m_PatientVitalValues;
	protected TextWidget m_PatientMedications;
	protected TextWidget m_PatientMedicationValues;
	protected ButtonWidget m_PatientMedicationsFocus;
	protected TextWidget m_SelectedRegionText;
	protected TextWidget m_TreatmentStatus;
	protected TextWidget m_ForeignInventoryTitle;
	protected Widget m_BandageRowForeign;
	protected Widget m_TourniquetRowForeign;
	protected Widget m_MedicalKitRow;
	protected ButtonWidget m_BandageButton;
	protected ButtonWidget m_TourniquetButton;
	protected ButtonWidget m_MedicalKitButton;
	protected Widget m_BandageButtonContent;
	protected Widget m_TourniquetButtonContent;
	protected Widget m_MedicalKitButtonContent;
	protected TextWidget m_BandageCount;
	protected TextWidget m_TourniquetCount;
	protected TextWidget m_MedicalKitCount;
	protected TextWidget m_BandageLabel;
	protected TextWidget m_TourniquetLabel;
	protected TextWidget m_MedicalKitLabel;
	protected ItemPreviewWidget m_BandageIcon;
	protected ItemPreviewWidget m_TourniquetIcon;
	protected ItemPreviewWidget m_MedicalKitIcon;
	protected ButtonWidget m_BandageButtonForeign;
	protected ButtonWidget m_TourniquetButtonForeign;
	protected ButtonWidget m_AdvancedCPRButton;
	protected TextWidget m_AdvancedCPRText;
	protected ButtonWidget m_AdvancedCarryButton;
	protected ButtonWidget m_AdvancedDragButton;
	protected ButtonWidget m_AdvancedBackButton;
	protected ButtonWidget m_AdvancedRightSideButton;
	protected ButtonWidget m_AdvancedLeftSideButton;
	protected ButtonWidget m_AdvancedLoadVehicleButton;
	protected Widget m_BandageButtonContentForeign;
	protected Widget m_TourniquetButtonContentForeign;
	protected TextWidget m_BandageCountForeign;
	protected TextWidget m_TourniquetCountForeign;
	protected TextWidget m_BandageLabelForeign;
	protected TextWidget m_TourniquetLabelForeign;
	protected ItemPreviewWidget m_BandageIconForeign;
	protected ItemPreviewWidget m_TourniquetIconForeign;
	protected ref array<RAMI_ETreatmentType> m_MedicationTypes = {
		RAMI_ETreatmentType.EPINEPHRINE,
		RAMI_ETreatmentType.MORPHINE,
		RAMI_ETreatmentType.NALOXONE,
		RAMI_ETreatmentType.PHENYLEPHRINE,
		RAMI_ETreatmentType.METOPROLOL,
		RAMI_ETreatmentType.AMMONIUM_CARBONATE,
		RAMI_ETreatmentType.SALINE_250,
		RAMI_ETreatmentType.SALINE_500,
		RAMI_ETreatmentType.SALINE_750,
		RAMI_ETreatmentType.SALINE_1000,
		RAMI_ETreatmentType.SALINE_1250,
		RAMI_ETreatmentType.SALINE_1500
	};
	protected ref array<string> m_MedicationWidgetNames = {
		"MedicationEpinephrine",
		"MedicationMorphine",
		"MedicationNaloxone",
		"MedicationPhenylephrine",
		"MedicationMetoprolol",
		"MedicationAmmonia",
		"MedicationSaline250",
		"MedicationSaline500",
		"MedicationSaline750",
		"MedicationSaline1000",
		"MedicationSaline1250",
		"MedicationSaline1500"
	};
	protected ref array<ButtonWidget> m_MedicationButtons = {};
	protected ref array<Widget> m_MedicationButtonContents = {};
	protected ref array<TextWidget> m_MedicationCounts = {};
	protected ref array<TextWidget> m_MedicationLabels = {};
	protected ref array<ItemPreviewWidget> m_MedicationIcons = {};
	protected ref array<Widget> m_MedicationRows = {};
	protected ref array<ButtonWidget> m_MedicationButtonsForeign = {};
	protected ref array<Widget> m_MedicationButtonContentsForeign = {};
	protected ref array<TextWidget> m_MedicationCountsForeign = {};
	protected ref array<TextWidget> m_MedicationLabelsForeign = {};
	protected ref array<ItemPreviewWidget> m_MedicationIconsForeign = {};
	protected ref array<Widget> m_MedicationRowsForeign = {};
	protected bool m_IsSelfSalineMenuOpen;
	protected bool m_IsForeignSalineMenuOpen;
	protected ButtonWidget m_SalineToggleButton;
	protected TextWidget m_SalineToggleCount;
	protected TextWidget m_SalineToggleLabel;
	protected ItemPreviewWidget m_SalineToggleIcon;
	protected Widget m_SalineToggleRowForeign;
	protected ButtonWidget m_SalineToggleButtonForeign;
	protected TextWidget m_SalineToggleCountForeign;
	protected TextWidget m_SalineToggleLabelForeign;
	protected ItemPreviewWidget m_SalineToggleIconForeign;
	protected Widget m_StopSalineRow;
	protected ButtonWidget m_StopSalineButton;
	protected TextWidget m_StopSalineLabel;
	protected Widget m_StopSalineRowForeign;
	protected ButtonWidget m_StopSalineButtonForeign;
	protected TextWidget m_StopSalineLabelForeign;
	protected ImageWidget m_BodyZoneHead;
	protected ImageWidget m_BodyZoneChest;
	protected ImageWidget m_BodyZoneAbdomen;
	protected ImageWidget m_BodyZoneLeftArm;
	protected ImageWidget m_BodyZoneRightArm;
	protected ImageWidget m_BodyZoneLeftLeg;
	protected ImageWidget m_BodyZoneRightLeg;
	protected ref array<ImageWidget> m_BodyZoneOutlines = {};
	protected ImageWidget m_BodyZoneOutlineMask;
	protected ImageWidget m_BodyInjuryHead;
	protected ImageWidget m_BodyInjuryChest;
	protected ImageWidget m_BodyInjuryAbdomen;
	protected ImageWidget m_BodyInjuryLeftArm;
	protected ImageWidget m_BodyInjuryRightArm;
	protected ImageWidget m_BodyInjuryLeftLeg;
	protected ImageWidget m_BodyInjuryRightLeg;
	protected ImageWidget m_BodySalineBag;
	protected ImageWidget m_BodyBoneLeftArm;
	protected ImageWidget m_BodyBoneRightArm;
	protected ImageWidget m_BodyBoneLeftLeg;
	protected ImageWidget m_BodyBoneRightLeg;
	protected ImageWidget m_BodyTourniquetLeftArm;
	protected ImageWidget m_BodyTourniquetRightArm;
	protected ImageWidget m_BodyTourniquetLeftLeg;
	protected ImageWidget m_BodyTourniquetRightLeg;
	protected IEntity m_PendingTreatmentItem;
	protected SCR_ChimeraCharacter m_PendingTreatmentPatient;
	protected ECharacterHitZoneGroup m_PendingTreatmentRegion;
	protected int m_PendingTreatmentAttempts;
	protected bool m_IsMedicalKitTreatmentActive;
	protected int m_PendingForeignCommonType;
	protected int m_PendingForeignAttempts;
	protected float m_BleedingPulseTime;
	protected ECharacterHitZoneGroup m_SelectedRegion = ECharacterHitZoneGroup.UPPERTORSO;

	static void OpenMedicalMenu()
	{
		s_Patient = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!s_Patient)
			return;

		s_ForeignPatient = null;
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (camera)
		{
			IEntity cursorTarget = camera.GetCursorTarget();
			if (cursorTarget)
				s_ForeignPatient = SCR_ChimeraCharacter.Cast(cursorTarget.GetRootParent());
		}

		if (s_ForeignPatient == s_Patient || !IsForeignPatientInRange())
			s_ForeignPatient = null;

		if (s_ForeignPatient)
			s_Patient = s_ForeignPatient;

		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager.FindMenuByPreset(ChimeraMenuPreset.RAMI_MedicalMenu))
			menuManager.OpenMenu(ChimeraMenuPreset.RAMI_MedicalMenu, 0, true);
	}

	protected static bool IsForeignPatientInRange()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		return playerController && playerController.RAMI_IsTreatmentTargetInRange(s_ForeignPatient);
	}

	static void CaptureBodyZoneState(SCR_ChimeraCharacter patient)
	{
		if (!patient)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(patient.GetCharacterController());
		if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(patient.GetDamageManager());
		if (!damageManager)
			return;

		SCR_CharacterBloodHitZone blood = damageManager.GetBloodHitZone();
		if (!blood)
			return;

		RAMI_BodyZoneState state = s_BodyZoneStates.Get(patient);
		if (!state)
		{
			state = new RAMI_BodyZoneState();
			s_BodyZoneStates.Set(patient, state);
		}

		state.Capture(damageManager, blood);
	}

	protected override void OnMenuOpen()
	{
		Widget root = GetRootWidget();
		if (!root)
			return;

		m_CloseButton = ButtonWidget.Cast(root.FindAnyWidget("CloseButton"));
		m_CloseText = TextWidget.Cast(root.FindAnyWidget("Text0"));
		m_ActivePageHighlight = root.FindAnyWidget("ActivePageHighlight");
		if (!m_CloseButton || !m_CloseText || !m_ActivePageHighlight)
		{
			Print("RAMI: Global menu controls were not found", LogLevel.ERROR);
			Close();
			return;
		}
		m_ActivePageHighlight.SetFlags(WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS);

		m_Handler = new RAMI_MedicalMenuHandler(this);
		m_CloseButton.AddHandler(m_Handler);
		LoadUISettings();

		array<string> pageButtons = {
			"PageTriage",
			"PageDiagnose",
			"PageBandages",
			"PageMedication",
			"PageAdvanced",
			"PageToggleSelf"
		};

		foreach (string pageButtonName : pageButtons)
		{
			ButtonWidget pageButton = ButtonWidget.Cast(root.FindAnyWidget(pageButtonName));
			if (!pageButton)
			{
				Print(string.Format("RAMI: %1 was not found", pageButtonName), LogLevel.ERROR);
				Close();
				return;
			}

			pageButton.AddHandler(m_Handler);
		}

		m_ContentArea = root.FindAnyWidget("ContentArea");
		if (!m_ContentArea)
		{
			Print("RAMI: ContentArea was not found", LogLevel.ERROR);
			Close();
			return;
		}
		m_PatientWindow = root.FindAnyWidget("PatientWindow");
		m_PatientList = root.FindAnyWidget("PatientList");
		m_PatientListStatus = TextWidget.Cast(root.FindAnyWidget("PatientListStatus"));
		m_PatientActions = root.FindAnyWidget("PatientActions");
		m_PatientTreatButton = ButtonWidget.Cast(root.FindAnyWidget("PatientActionTreatButton"));
		m_PatientTreatText = TextWidget.Cast(root.FindAnyWidget("PatientActionTreatText"));
		m_PatientUnloadButton = ButtonWidget.Cast(root.FindAnyWidget("PatientActionUnloadButton"));
		m_PatientUnloadText = TextWidget.Cast(root.FindAnyWidget("PatientActionUnloadText"));
		if (!m_PatientWindow || !m_PatientList || !m_PatientListStatus || !m_PatientActions || !m_PatientTreatButton || !m_PatientUnloadButton)
		{
			Print("RAMI: Patient window was not found", LogLevel.ERROR);
			Close();
			return;
		}
		m_PatientTreatButton.AddHandler(m_Handler);
		m_PatientUnloadButton.AddHandler(m_Handler);
		m_PatientActions.SetVisible(false);
		m_IsPatientWindowOpen = false;
		m_PatientWindow.SetVisible(false);

		ShowPage("PageBandages");
		GetGame().GetCallqueue().CallLater(UpdatePatientVitals, 250, true);
		GetGame().GetCallqueue().CallLater(RegisterCloseActions, 250);
	}

	protected void LoadUISettings()
	{
		BaseContainer settings = RAMI_UISettingsModule.GetInstance();
		if (!settings)
			return;

		settings.Get(RAMI_UISettingsModule.BUTTON_TEXT_COLOR, m_ButtonTextColor);
		settings.Get(RAMI_UISettingsModule.COLORBLIND_MODE, m_ColorblindMode);
		if (m_ButtonTextColor < 0 || m_ButtonTextColor >= RAMI_UISettingsModule.BUTTON_TEXT_COLOR_COUNT)
			m_ButtonTextColor = 0;
	}

	protected Color GetButtonTextColor(Color defaultColor)
	{
		switch (m_ButtonTextColor)
		{
			case 1: return Color.White;
			case 2: return Color.FromSRGBA(255, 220, 0, 255);
			case 3: return Color.FromSRGBA(86, 180, 233, 255);
			case 4: return Color.Black;
			case 5: return Color.FromSRGBA(229, 57, 53, 255);
			case 6: return Color.FromSRGBA(255, 152, 0, 255);
			case 7: return Color.FromSRGBA(67, 160, 71, 255);
			case 8: return Color.FromSRGBA(30, 136, 229, 255);
			case 9: return Color.FromSRGBA(216, 27, 96, 255);
		}
		return defaultColor;
	}

	protected void ApplyButtonTextContrast(TextWidget text)
	{
		if (!text)
			return;

		Color textColor = text.GetColor();
		int shadowColor = 0xFFFFFFFF;
		if (textColor.R() + textColor.G() + textColor.B() > 1.5)
			shadowColor = 0xFF000000;
		text.SetOutline(0);
		text.SetShadow(1, shadowColor, 0.65, 1, 1);
	}

	protected void ApplyButtonTextColors()
	{
		Widget root = GetRootWidget();
		if (!root)
			return;
		array<string> darkButtonTexts = {
			"Text0",
			"DiagnoseHeartRateText",
			"DiagnoseBloodPressureText",
			"AdvancedCPRText",
			"AdvancedBackText",
			"AdvancedRightSideText",
			"AdvancedLeftSideText",
			"AdvancedCarryText",
			"AdvancedDragText",
			"AdvancedLoadVehicleText"
		};
		foreach (string widgetName : darkButtonTexts)
		{
			TextWidget text = TextWidget.Cast(root.FindAnyWidget(widgetName));
			if (text && (widgetName.StartsWith("Advanced") || widgetName.StartsWith("Diagnose")))
				text.SetBold(true);
			if (text)
			{
				text.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(text);
			}
		}

		array<string> lightButtonTextParts = {
			"BandageLabel",
			"BandageCount",
			"TourniquetLabel",
			"TourniquetCount",
			"MedicalKitLabel",
			"MedicalKitCount",
			"MedicationSalineToggleLabel",
			"MedicationSalineToggleCount"
		};
		foreach (string widgetName : lightButtonTextParts)
		{
			TextWidget text = TextWidget.Cast(root.FindAnyWidget(widgetName));
			if (text)
			{
				text.SetBold(true);
				text.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(text);
			}
			text = TextWidget.Cast(root.FindAnyWidget(widgetName + "Foreign"));
			if (text)
			{
				text.SetBold(true);
				text.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(text);
			}
		}

		foreach (string medicationName : m_MedicationWidgetNames)
		{
			TextWidget label = TextWidget.Cast(root.FindAnyWidget(medicationName + "Label"));
			TextWidget count = TextWidget.Cast(root.FindAnyWidget(medicationName + "Count"));
			TextWidget labelForeign = TextWidget.Cast(root.FindAnyWidget(medicationName + "LabelForeign"));
			TextWidget countForeign = TextWidget.Cast(root.FindAnyWidget(medicationName + "CountForeign"));
			if (label)
			{
				label.SetBold(true);
				label.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(label);
			}
			if (count)
			{
				count.SetBold(true);
				count.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(count);
			}
			if (labelForeign)
			{
				labelForeign.SetBold(true);
				labelForeign.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(labelForeign);
			}
			if (countForeign)
			{
				countForeign.SetBold(true);
				countForeign.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(countForeign);
			}
		}
	}

	protected override void OnMenuOpened()
	{
		DrawMenuBackground();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		InputManager inputManager = GetGame().GetInputManager();
		if (!workspace || !inputManager)
			return;

		float width, height;
		workspace.GetScreenSize(width, height);
		inputManager.GetMouseDeviceHandler().SetCursorPosition(Math.Round(width * 0.5), Math.Round(height * 0.5));
		workspace.SetFocusedWidget(GetRootWidget().FindAnyWidget("PageBandages"));
	}

	protected void DrawMenuBackground()
	{
		CanvasWidget background = CanvasWidget.Cast(GetRootWidget().FindAnyWidget("Background"));
		if (background)
			DrawRoundedBackground(background, m_MenuBackgroundCommands);
		CanvasWidget patientBackground = CanvasWidget.Cast(GetRootWidget().FindAnyWidget("PatientWindowBackground"));
		if (patientBackground)
			DrawRoundedBackground(patientBackground, m_PatientWindowBackgroundCommands);
	}

	protected void DrawRoundedBackground(CanvasWidget background, array<ref CanvasWidgetCommand> commands)
	{
		if (!background || !commands)
			return;

		float width, height;
		background.GetScreenSize(width, height);
		if (width <= 0 || height <= 0)
			return;

		float outlineThickness = 6;
		float cornerRadius = 18;
		ref array<float> outerVertices = {};
		ref array<float> innerVertices = {};
		background.TessellateRoundedRectangle(Vector(0, 0, 0), Vector(width, height, 0), cornerRadius, 6, RectangleCorner.All, outerVertices);
		background.TessellateRoundedRectangle(Vector(outlineThickness, outlineThickness, 0), Vector(width - outlineThickness, height - outlineThickness, 0), cornerRadius - outlineThickness, 6, RectangleCorner.All, innerVertices);
		ref PolygonDrawCommand fill = new PolygonDrawCommand();
		fill.m_iColor = 0xBF000000;
		fill.m_Vertices = outerVertices;
		ref TriMeshDrawCommand outlineCorners = new TriMeshDrawCommand();
		outlineCorners.m_iColor = 0xFF000000;
		outlineCorners.m_Vertices = {};
		outlineCorners.m_Indices = {};
		int outlineVertexCount = outerVertices.Count() / 2;
		int vertexIndex;
		for (vertexIndex = 0; vertexIndex < outlineVertexCount; vertexIndex++)
		{
			int nextVertexIndex = (vertexIndex + 1) % outlineVertexCount;
			float outerX = outerVertices[vertexIndex * 2];
			float outerY = outerVertices[vertexIndex * 2 + 1];
			float nextOuterX = outerVertices[nextVertexIndex * 2];
			float nextOuterY = outerVertices[nextVertexIndex * 2 + 1];
			if (outerX > cornerRadius || outerY > cornerRadius || nextOuterX > cornerRadius || nextOuterY > cornerRadius)
				continue;

			float innerX = innerVertices[vertexIndex * 2];
			float innerY = innerVertices[vertexIndex * 2 + 1];
			float nextInnerX = innerVertices[nextVertexIndex * 2];
			float nextInnerY = innerVertices[nextVertexIndex * 2 + 1];
			for (int cornerIndex = 0; cornerIndex < 4; cornerIndex++)
			{
				bool mirrorX = cornerIndex == 1 || cornerIndex == 3;
				bool mirrorY = cornerIndex >= 2;
				float cornerOuterX = outerX;
				float cornerOuterY = outerY;
				float cornerNextOuterX = nextOuterX;
				float cornerNextOuterY = nextOuterY;
				float cornerInnerX = innerX;
				float cornerInnerY = innerY;
				float cornerNextInnerX = nextInnerX;
				float cornerNextInnerY = nextInnerY;
				if (mirrorX)
				{
					cornerOuterX = width - cornerOuterX;
					cornerNextOuterX = width - cornerNextOuterX;
					cornerInnerX = width - cornerInnerX;
					cornerNextInnerX = width - cornerNextInnerX;
				}
				if (mirrorY)
				{
					cornerOuterY = height - cornerOuterY;
					cornerNextOuterY = height - cornerNextOuterY;
					cornerInnerY = height - cornerInnerY;
					cornerNextInnerY = height - cornerNextInnerY;
				}

				int cornerVertexIndex = outlineCorners.m_Vertices.Count() / 2;
				outlineCorners.m_Vertices.Insert(cornerOuterX);
				outlineCorners.m_Vertices.Insert(cornerOuterY);
				outlineCorners.m_Vertices.Insert(cornerNextOuterX);
				outlineCorners.m_Vertices.Insert(cornerNextOuterY);
				outlineCorners.m_Vertices.Insert(cornerInnerX);
				outlineCorners.m_Vertices.Insert(cornerInnerY);
				outlineCorners.m_Vertices.Insert(cornerNextInnerX);
				outlineCorners.m_Vertices.Insert(cornerNextInnerY);
				if (mirrorX != mirrorY)
				{
					outlineCorners.m_Indices.Insert(cornerVertexIndex);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 2);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 1);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 1);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 2);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 3);
				}
				else
				{
					outlineCorners.m_Indices.Insert(cornerVertexIndex);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 1);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 2);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 1);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 3);
					outlineCorners.m_Indices.Insert(cornerVertexIndex + 2);
				}
			}
		}
		ref PolygonDrawCommand outlineTop = new PolygonDrawCommand();
		outlineTop.m_iColor = 0xFF000000;
		outlineTop.m_Vertices = {cornerRadius, 0, width - cornerRadius, 0, width - cornerRadius, outlineThickness, cornerRadius, outlineThickness};
		ref PolygonDrawCommand outlineBottom = new PolygonDrawCommand();
		outlineBottom.m_iColor = 0xFF000000;
		outlineBottom.m_Vertices = {cornerRadius, height - outlineThickness, width - cornerRadius, height - outlineThickness, width - cornerRadius, height, cornerRadius, height};
		ref PolygonDrawCommand outlineLeft = new PolygonDrawCommand();
		outlineLeft.m_iColor = 0xFF000000;
		outlineLeft.m_Vertices = {0, cornerRadius, outlineThickness, cornerRadius, outlineThickness, height - cornerRadius, 0, height - cornerRadius};
		ref PolygonDrawCommand outlineRight = new PolygonDrawCommand();
		outlineRight.m_iColor = 0xFF000000;
		outlineRight.m_Vertices = {width - outlineThickness, cornerRadius, width, cornerRadius, width, height - cornerRadius, width - outlineThickness, height - cornerRadius};
		commands.Clear();
		commands.Insert(fill);
		commands.Insert(outlineCorners);
		commands.Insert(outlineTop);
		commands.Insert(outlineBottom);
		commands.Insert(outlineLeft);
		commands.Insert(outlineRight);
		background.SetDrawCommands(commands);
	}

	protected override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (m_MenuBackgroundCommands.IsEmpty() || (m_IsPatientWindowOpen && m_PatientWindowBackgroundCommands.IsEmpty()))
			DrawMenuBackground();
		UpdateControllerFocus();
		if (s_Patient == s_ForeignPatient && !IsForeignPatientInRange())
		{
			CloseMenu();
			return;
		}
		UpdatePendingMedicalKit(tDelta);

		m_BleedingPulseTime += tDelta;
		if (m_IsHeartRateMonitored)
		{
			m_DiagnoseHeartRate += (m_DiagnoseHeartRateTarget - m_DiagnoseHeartRate) * Math.Min(tDelta * 4, 1);
			m_DiagnoseHeartSampleTime += tDelta;
			while (m_DiagnoseHeartRate > 0 && m_DiagnoseHeartSampleTime >= DIAGNOSE_HEART_TRACE_SAMPLE_INTERVAL)
			{
				m_DiagnoseHeartSampleTime -= DIAGNOSE_HEART_TRACE_SAMPLE_INTERVAL;
				m_DiagnoseHeartPhase += DIAGNOSE_HEART_TRACE_SAMPLE_INTERVAL * m_DiagnoseHeartRate / 60;
				m_DiagnoseHeartPhase -= Math.Floor(m_DiagnoseHeartPhase);
				if (m_DiagnoseHeartTrace.Count() >= DIAGNOSE_HEART_TRACE_SAMPLE_COUNT)
					m_DiagnoseHeartTrace.Clear();
				m_DiagnoseHeartTrace.Insert(GetHeartRhythmAmplitude(m_DiagnoseHeartPhase));
			}
			if (m_DiagnoseHeartRateGraph)
				DrawDiagnoseHeartRateGraph();
		}

		RAMI_BodyZoneState state = s_BodyZoneStates.Get(s_Patient);
		SCR_CharacterControllerComponent controller;
		if (s_Patient)
			controller = SCR_CharacterControllerComponent.Cast(s_Patient.GetCharacterController());
		if (controller && controller.GetLifeState() == ECharacterLifeState.DEAD)
			state = null;

		float opacity = 0.8 + Math.Sin(m_BleedingPulseTime * 4.2) * 0.2;
		UpdateBodyZonePulse(ECharacterHitZoneGroup.HEAD, m_BodyZoneHead, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.UPPERTORSO, m_BodyZoneChest, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.LOWERTORSO, m_BodyZoneAbdomen, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.LEFTARM, m_BodyZoneLeftArm, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.RIGHTARM, m_BodyZoneRightArm, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.LEFTLEG, m_BodyZoneLeftLeg, state, opacity);
		UpdateBodyZonePulse(ECharacterHitZoneGroup.RIGHTLEG, m_BodyZoneRightLeg, state, opacity);
		if (m_BodySalineBag && m_BodySalineBag.IsVisible())
			m_BodySalineBag.SetOpacity(0.65 + Math.Sin(m_BleedingPulseTime * 4.2) * 0.35);
	}

	protected void UpdateControllerFocus()
	{
		InputManager inputManager = GetGame().GetInputManager();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!inputManager || !workspace || inputManager.GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD)
		{
			if (m_ControllerFocus)
				m_ControllerFocus.SetVisible(false);
			return;
		}

		Widget focused = workspace.GetFocusedWidget();
		if (!ButtonWidget.Cast(focused))
		{
			if (m_ControllerFocus)
				m_ControllerFocus.SetVisible(false);
			return;
		}

		Widget focusParent = GetRootWidget().FindAnyWidget("MedicalPanel");
		if (!focusParent)
			return;

		if (!m_ControllerFocus)
		{
			m_ControllerFocus = workspace.CreateWidgets(CONTROLLER_FOCUS_LAYOUT, focusParent);
			if (!m_ControllerFocus)
				return;
			m_ControllerFocus.SetFlags(WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS);
			m_ControllerFocus.SetZOrder(100);
		}

		float focusedX, focusedY, focusedWidth, focusedHeight;
		float parentX, parentY, parentWidth, parentHeight;
		focused.GetScreenPos(focusedX, focusedY);
		focused.GetScreenSize(focusedWidth, focusedHeight);
		focusParent.GetScreenPos(parentX, parentY);
		focusParent.GetScreenSize(parentWidth, parentHeight);
		if (parentWidth <= 0 || parentHeight <= 0)
			return;

		FrameSlot.SetAnchorMin(m_ControllerFocus, (focusedX - parentX) / parentWidth, (focusedY - parentY) / parentHeight);
		FrameSlot.SetAnchorMax(m_ControllerFocus, (focusedX + focusedWidth - parentX) / parentWidth, (focusedY + focusedHeight - parentY) / parentHeight);
		FrameSlot.SetOffsets(m_ControllerFocus, 0, 0, 0, 0);
		if (focused.GetName() == "TriageActivityFocus" || focused.GetName() == "PatientMedicationsFocus")
			m_ControllerFocus.SetOpacity(0.35);
		else
			m_ControllerFocus.SetOpacity(1);
		m_ControllerFocus.SetVisible(true);
	}

	protected void RegisterCloseActions()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		inputManager.AddActionListener("RAMI_CloseMedicalMenu", EActionTrigger.UP, CloseMenu);
		inputManager.AddActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.UP, CloseMenu);
		inputManager.AddActionListener("RAMI_PreviousMedicalPage", EActionTrigger.DOWN, ShowPreviousPage);
		inputManager.AddActionListener("RAMI_NextMedicalPage", EActionTrigger.DOWN, ShowNextPage);
		inputManager.AddActionListener("MenuDown", EActionTrigger.DOWN, NavigateDownFromPageTab);
	}

	protected void ShowPreviousPage()
	{
		CyclePage(-1);
	}

	protected void ShowNextPage()
	{
		CyclePage(1);
	}

	protected void CyclePage(int direction)
	{
		array<string> pageNames = {
			"PageTriage",
			"PageDiagnose",
			"PageBandages",
			"PageMedication",
			"PageAdvanced"
		};
		int currentIndex = pageNames.Find(m_CurrentPageName);
		if (currentIndex < 0)
			return;

		int nextIndex = currentIndex + direction;
		if (nextIndex < 0)
			nextIndex = pageNames.Count() - 1;
		else if (nextIndex >= pageNames.Count())
			nextIndex = 0;

		string pageName = pageNames[nextIndex];
		ShowPage(pageName);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget pageButton = GetRootWidget().FindAnyWidget(pageName);
		if (workspace && pageButton)
			workspace.SetFocusedWidget(pageButton);
	}

	protected void NavigateDownFromPageTab()
	{
		InputManager inputManager = GetGame().GetInputManager();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!inputManager || !workspace || inputManager.GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD)
			return;

		Widget focused = workspace.GetFocusedWidget();
		if (!focused)
			return;

		string targetName;
		switch (focused.GetName())
		{
			case "PageDiagnose": targetName = "DiagnoseHeartRateButton"; break;
			case "PageTriage": targetName = "TriageLevelNone"; break;
			case "PageAdvanced":
			{
				array<string> actionButtons = {
					"AdvancedCPRButton",
					"AdvancedBackButton",
					"AdvancedRightSideButton",
					"AdvancedLeftSideButton",
					"AdvancedCarryButton",
					"AdvancedDragButton",
					"AdvancedLoadVehicleButton"
				};
				foreach (string actionButtonName : actionButtons)
				{
					ButtonWidget actionButton = ButtonWidget.Cast(GetRootWidget().FindAnyWidget(actionButtonName));
					if (!actionButton || !actionButton.IsEnabled())
						continue;

					targetName = actionButtonName;
					break;
				}
				if (targetName.IsEmpty())
					targetName = "CloseButton";
				break;
			}
			default: return;
		}

		Widget target = GetRootWidget().FindAnyWidget(targetName);
		if (target)
			workspace.SetFocusedWidget(target);
	}

	protected override void OnMenuClose()
	{
		GetGame().GetCallqueue().Remove(UpdatePatientVitals);
		GetGame().GetCallqueue().Remove(UpdateDiagnoseMonitor);
		m_IsHeartRateMonitored = false;
		m_IsBloodPressureMonitored = false;
		ClearDiagnoseGraphs();
		GetGame().GetCallqueue().Remove(RegisterCloseActions);
		ClearPendingTreatment();
		m_ControllerFocus = null;
		m_ActivePageHighlight = null;
		m_HoveredRequestedButton = null;
		m_CloseButton = null;
		m_CloseText = null;
		m_PageRoot = null;
		m_PatientWindow = null;
		m_PatientList = null;
		m_PatientListStatus = null;
		m_PatientActions = null;
		m_PatientTreatButton = null;
		m_PatientTreatText = null;
		m_PatientUnloadButton = null;
		m_PatientUnloadText = null;
		m_PatientActionTarget = null;
		m_PatientButtons.Clear();
		m_ListedPatients.Clear();
		s_Patient = null;
		s_ForeignPatient = null;

		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		inputManager.RemoveActionListener("RAMI_CloseMedicalMenu", EActionTrigger.UP, CloseMenu);
		inputManager.RemoveActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.UP, CloseMenu);
		inputManager.RemoveActionListener("RAMI_PreviousMedicalPage", EActionTrigger.DOWN, ShowPreviousPage);
		inputManager.RemoveActionListener("RAMI_NextMedicalPage", EActionTrigger.DOWN, ShowNextPage);
		inputManager.RemoveActionListener("MenuDown", EActionTrigger.DOWN, NavigateDownFromPageTab);
	}

	void CloseMenu()
	{
		if (m_IsMedicationPage && (m_IsSelfSalineMenuOpen || m_IsForeignSalineMenuOpen))
		{
			m_IsSelfSalineMenuOpen = false;
			m_IsForeignSalineMenuOpen = false;
			UpdateMedicationItems();
			return;
		}

		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RAMI_MedicalMenu);
	}

	void TogglePatient()
	{
		m_IsPatientWindowOpen = !m_IsPatientWindowOpen;
		UpdatePatientList();
	}

	void SelectListedPatient(Widget patientButton)
	{
		int index = m_PatientButtons.Find(ButtonWidget.Cast(patientButton));
		if (index < 0 || index >= m_ListedPatients.Count())
			return;

		SCR_ChimeraCharacter patient = m_ListedPatients[index];
		if (m_PatientActions.IsVisible() && m_PatientActionTarget == patient)
		{
			HidePatientActions();
			return;
		}

		m_PatientActionTarget = patient;
		m_PatientActions.SetVisible(true);
		UpdatePatientButtonVisibility();
		UpdatePatientActionButtons();
		UpdatePatientButtonColors();
	}

	void TreatListedPatient()
	{
		SCR_ChimeraCharacter patient = m_PatientActionTarget;
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!patient || !user)
			return;

		if (patient == user)
			s_ForeignPatient = null;
		else
		{
			s_ForeignPatient = patient;
			if (!IsForeignPatientInRange())
			{
				s_ForeignPatient = null;
				return;
			}
		}

		s_Patient = patient;
		HidePatientActions();
		UpdatePatientSelection();
	}

	void UnloadListedPatient()
	{
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		SCR_RemoveCasualtyUserAction action = FindRemoveCasualtyAction();
		if (!user || !action || !PatientIsUnconscious(m_PatientActionTarget))
			return;

		ActionsPerformerComponent performer = ActionsPerformerComponent.Cast(user.FindComponent(ActionsPerformerComponent));
		if (!performer)
			return;

		performer.PerformAction(action);
		HidePatientActions();
	}

	void SetPatientButtonHovered(Widget patientButton, bool hovered)
	{
		ButtonWidget button = ButtonWidget.Cast(patientButton);
		if (!button || m_PatientButtons.Find(button) < 0)
			return;

		if (hovered)
			m_HoveredPatientButton = button;
		else if (m_HoveredPatientButton == button)
			m_HoveredPatientButton = null;
		UpdatePatientButtonColors();
	}

	void SetRequestedButtonHovered(Widget widget, bool hovered)
	{
		ButtonWidget button = GetRequestedButton(widget);
		if (!button || hovered && !button.IsEnabled())
			return;

		if (hovered)
			m_HoveredRequestedButton = button;
		else if (m_HoveredRequestedButton == button)
			m_HoveredRequestedButton = null;
		UpdateRequestedButtonStyles();
	}

	protected ButtonWidget GetRequestedButton(Widget widget)
	{
		if (!widget)
			return null;

		string widgetName = widget.GetName();
		ButtonWidget button = ButtonWidget.Cast(widget);
		if (button && (widgetName == "CloseButton" || widgetName == "DiagnoseHeartRateButton" || widgetName == "DiagnoseBloodPressureButton" || widgetName.StartsWith("Advanced") || widgetName.StartsWith("Medication") || widgetName.StartsWith("PatientAction")))
			return button;

		bool foreign = widgetName.Contains("Foreign");
		if (widgetName.StartsWith("MedicationSalineToggle"))
		{
			if (foreign)
				return m_SalineToggleButtonForeign;
			return m_SalineToggleButton;
		}
		if (widgetName.StartsWith("MedicationStopSaline"))
		{
			if (foreign)
				return m_StopSalineButtonForeign;
			return m_StopSalineButton;
		}

		for (int index = 0; index < m_MedicationWidgetNames.Count(); index++)
		{
			if (!widgetName.StartsWith(m_MedicationWidgetNames[index]))
				continue;
			if (foreign)
				return m_MedicationButtonsForeign[index];
			return m_MedicationButtons[index];
		}

		return null;
	}

	protected void UpdateRequestedButtonStyles()
	{
		ApplyRequestedButtonStyle(m_CloseButton, m_CloseText, null, false);
		ApplyRequestedButtonStyle(m_DiagnoseHeartRateButton, m_DiagnoseHeartRateText, null, true);
		ApplyRequestedButtonStyle(m_DiagnoseBloodPressureButton, m_DiagnoseBloodPressureText, null, true);
		ApplyRequestedButtonStyle(m_AdvancedCPRButton, m_AdvancedCPRText, null, true);
		ApplyRequestedButtonStyle(m_PatientTreatButton, m_PatientTreatText, null, true);
		ApplyRequestedButtonStyle(m_PatientUnloadButton, m_PatientUnloadText, null, true);

		Widget root = GetRootWidget();
		if (root)
		{
			ApplyRequestedButtonStyle(m_AdvancedBackButton, TextWidget.Cast(root.FindAnyWidget("AdvancedBackText")), null, true);
			ApplyRequestedButtonStyle(m_AdvancedRightSideButton, TextWidget.Cast(root.FindAnyWidget("AdvancedRightSideText")), null, true);
			ApplyRequestedButtonStyle(m_AdvancedLeftSideButton, TextWidget.Cast(root.FindAnyWidget("AdvancedLeftSideText")), null, true);
			ApplyRequestedButtonStyle(m_AdvancedCarryButton, TextWidget.Cast(root.FindAnyWidget("AdvancedCarryText")), null, true);
			ApplyRequestedButtonStyle(m_AdvancedDragButton, TextWidget.Cast(root.FindAnyWidget("AdvancedDragText")), null, true);
			ApplyRequestedButtonStyle(m_AdvancedLoadVehicleButton, TextWidget.Cast(root.FindAnyWidget("AdvancedLoadVehicleText")), null, true);
		}

		for (int index = 0; index < m_MedicationButtons.Count(); index++)
		{
			ApplyRequestedButtonStyle(m_MedicationButtons[index], m_MedicationLabels[index], m_MedicationCounts[index], true);
			ApplyRequestedButtonStyle(m_MedicationButtonsForeign[index], m_MedicationLabelsForeign[index], m_MedicationCountsForeign[index], true);
		}
		ApplyRequestedButtonStyle(m_SalineToggleButton, m_SalineToggleLabel, m_SalineToggleCount, true);
		ApplyRequestedButtonStyle(m_SalineToggleButtonForeign, m_SalineToggleLabelForeign, m_SalineToggleCountForeign, true);
		ApplyRequestedButtonStyle(m_StopSalineButton, m_StopSalineLabel, null, true);
		ApplyRequestedButtonStyle(m_StopSalineButtonForeign, m_StopSalineLabelForeign, null, true);
	}

	protected void ApplyRequestedButtonStyle(ButtonWidget button, TextWidget label, TextWidget count, bool baseBold)
	{
		if (!button)
			return;

		bool highlighted = button == m_HoveredRequestedButton && button.IsEnabled();
		if (button == m_DiagnoseHeartRateButton && m_IsHeartRateMonitored)
			highlighted = true;
		if (button == m_DiagnoseBloodPressureButton && m_IsBloodPressureMonitored)
			highlighted = true;

		if (highlighted)
			button.SetColor(Color.FromSRGBA(51, 179, 255, 153));
		else if (button.IsEnabled())
			button.SetColor(Color.FromSRGBA(255, 255, 255, 230));
		else
			button.SetColor(Color.FromSRGBA(128, 128, 128, 230));

		ApplyRequestedButtonText(label, highlighted, baseBold);
		ApplyRequestedButtonText(count, highlighted, baseBold);
	}

	protected void ApplyRequestedButtonText(TextWidget text, bool highlighted, bool baseBold)
	{
		if (!text)
			return;

		text.SetBold(baseBold || highlighted);
		if (highlighted)
			text.SetColor(Color.White);
		else
			text.SetColor(GetButtonTextColor(Color.Black));
		ApplyButtonTextContrast(text);
	}

	protected void UpdatePatientSelection()
	{
		UpdatePatientButtonColors();
		UpdatePatientVitals();
		if (m_IsHeartRateMonitored || m_IsBloodPressureMonitored)
		{
			ClearDiagnoseGraphs();
			if (m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
				m_DiagnoseHeartRateValue.SetText("READING...");
			if (m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
				m_DiagnoseBloodPressureValue.SetText("READING...");
			RequestDiagnoseVitals();
		}
	}

	protected void UpdatePatientList()
	{
		if (!m_PatientWindow || !m_PatientList)
			return;

		m_PatientWindow.SetVisible(m_IsPatientWindowOpen);
		if (!m_IsPatientWindowOpen)
		{
			if (!m_ListedPatients.IsEmpty())
				ClearPatientList();
			return;
		}

		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		array<SCR_ChimeraCharacter> patients = {};
		if (user)
			patients.Insert(user);
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (characters && playerController)
		{
			foreach (SCR_ChimeraCharacter character : characters)
			{
				if (character != user && playerController.RAMI_IsTreatmentTargetInRange(character))
					patients.Insert(character);
			}
		}

		bool changed = patients.Count() != m_ListedPatients.Count();
		for (int index = 0; !changed && index < patients.Count(); index++)
			changed = patients[index] != m_ListedPatients[index];
		if (!changed)
		{
			UpdatePatientButtonColors();
			UpdatePatientActionButtons();
			return;
		}

		ClearPatientList();
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		for (int index = 0; index < patients.Count(); index++)
		{
			ButtonWidget button = ButtonWidget.Cast(workspace.CreateWidgets(PATIENT_ROW_LAYOUT, m_PatientList));
			if (!button)
				continue;

			button.SetName("PatientEntry" + index.ToString());
			button.AddHandler(m_Handler);
			Widget careOutline = button.FindAnyWidget("PatientCareOutline");
			if (careOutline)
			{
				careOutline.SetVisible(false);
				careOutline.SetFlags(WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS);
			}
			m_PatientButtons.Insert(button);
			m_ListedPatients.Insert(patients[index]);
			TextWidget nameText = TextWidget.Cast(button.FindAnyWidget("PatientName"));
			if (nameText)
				nameText.SetText(GetPatientName(patients[index]));
		}
		if (m_PatientListStatus)
			m_PatientListStatus.SetVisible(m_PatientButtons.IsEmpty());
		UpdatePatientButtonColors();
		ConfigurePatientListNavigation();
	}

	protected SCR_RemoveCasualtyUserAction FindRemoveCasualtyAction()
	{
		if (!m_PatientActionTarget)
			return null;

		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(m_PatientActionTarget.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return null;

		BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
		if (!compartment || compartment.GetOccupant() != m_PatientActionTarget)
			return null;

		SCR_RemoveCasualtyUserAction registeredAction = SCR_RemoveCasualtyUserAction.Cast(compartment.GetGetOutAction());
		if (!registeredAction)
			registeredAction = FindRemoveCasualtyActionInManager(compartment.GetOwner(), compartment);
		if (registeredAction)
			return registeredAction;

		array<CompartmentUserAction> actions = {};
		compartment.GetAddUserActions(actions);
		foreach (CompartmentUserAction baseAction : actions)
		{
			SCR_RemoveCasualtyUserAction action = SCR_RemoveCasualtyUserAction.Cast(baseAction);
			if (action)
				return action;
		}

		return null;
	}

	protected SCR_RemoveCasualtyUserAction FindRemoveCasualtyActionInManager(IEntity owner, BaseCompartmentSlot compartment)
	{
		if (!owner)
			return null;

		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!actionsManager)
			return null;

		array<BaseUserAction> actions = {};
		actionsManager.GetActionsList(actions);
		foreach (BaseUserAction baseAction : actions)
		{
			SCR_RemoveCasualtyUserAction action = SCR_RemoveCasualtyUserAction.Cast(baseAction);
			if (action && action.GetCompartmentSlot() == compartment)
				return action;
		}

		return null;
	}

	protected bool PatientIsUnconscious(SCR_ChimeraCharacter patient)
	{
		if (!patient)
			return false;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(patient.GetCharacterController());
		return controller && controller.GetLifeState() == ECharacterLifeState.INCAPACITATED;
	}

	protected void UpdatePatientActionButtons()
	{
		if (!m_PatientActions || !m_PatientActions.IsVisible())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		bool canTreat = playerController && m_PatientActionTarget && m_ListedPatients.Contains(m_PatientActionTarget);
		bool canUnload = playerController && PatientIsUnconscious(m_PatientActionTarget) && FindRemoveCasualtyAction();
		m_PatientTreatButton.SetEnabled(canTreat);
		m_PatientUnloadButton.SetEnabled(canUnload);
		ConfigurePatientListNavigation();
		UpdateRequestedButtonStyles();
	}

	protected void HidePatientActions()
	{
		m_PatientActionTarget = null;
		if (m_PatientActions)
			m_PatientActions.SetVisible(false);
		if (m_HoveredRequestedButton == m_PatientTreatButton || m_HoveredRequestedButton == m_PatientUnloadButton)
			m_HoveredRequestedButton = null;
		UpdatePatientButtonVisibility();
		UpdatePatientButtonColors();
		ConfigurePatientListNavigation();
	}

	protected void UpdatePatientButtonVisibility()
	{
		bool showSelectedOnly = m_PatientActions && m_PatientActions.IsVisible() && m_PatientActionTarget;
		for (int index = 0; index < m_PatientButtons.Count() && index < m_ListedPatients.Count(); index++)
			m_PatientButtons[index].SetVisible(!showSelectedOnly || m_ListedPatients[index] == m_PatientActionTarget);
	}

	protected void UpdatePatientButtonColors()
	{
		for (int index = 0; index < m_PatientButtons.Count() && index < m_ListedPatients.Count(); index++)
		{
			ButtonWidget button = m_PatientButtons[index];
			if (!button)
				continue;

			bool highlighted = button == m_HoveredPatientButton || m_ListedPatients[index] == s_Patient || m_ListedPatients[index] == m_PatientActionTarget;
			bool isBleeding = PatientIsBleeding(m_ListedPatients[index]);
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_ListedPatients[index].GetCharacterController());
			bool isUnconscious = controller && controller.GetLifeState() == ECharacterLifeState.INCAPACITATED;
			bool isDead = controller && controller.GetLifeState() == ECharacterLifeState.DEAD;
			Widget careOutline = button.FindAnyWidget("PatientCareOutline");
			if (careOutline)
				careOutline.SetVisible(isBleeding && !isDead);
			if (isDead)
				button.SetColor(Color.Black);
			else if (isUnconscious)
				button.SetColor(Color.FromSRGBA(128, 128, 128, 230));
			else if (highlighted)
				button.SetColor(Color.FromSRGBA(51, 179, 255, 153));
			else
				button.SetColor(Color.FromSRGBA(255, 255, 255, 230));

			TextWidget nameText = TextWidget.Cast(button.FindAnyWidget("PatientName"));
			if (nameText)
			{
				nameText.SetBold(highlighted || isUnconscious);
				if (highlighted || isUnconscious)
					nameText.SetColor(Color.White);
				else if (isDead)
					nameText.SetColor(Color.FromSRGBA(192, 192, 192, 230));
				else
					nameText.SetColor(GetButtonTextColor(Color.Black));
				ApplyButtonTextContrast(nameText);
			}
		}
	}

	protected bool PatientIsBleeding(SCR_ChimeraCharacter patient)
	{
		if (!patient)
			return false;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(patient.GetDamageManager());
		if (!damageManager)
			return false;

		SCR_CharacterBloodHitZone blood = damageManager.GetBloodHitZone();
		return blood && blood.GetTotalBleedingAmount() > 0;
	}

	protected void ClearPatientList()
	{
		foreach (ButtonWidget button : m_PatientButtons)
		{
			if (button)
				button.RemoveFromHierarchy();
		}
		m_PatientButtons.Clear();
		m_ListedPatients.Clear();
		m_HoveredPatientButton = null;
		m_PatientActionTarget = null;
		if (m_PatientActions)
			m_PatientActions.SetVisible(false);
		if (m_PatientListStatus)
			m_PatientListStatus.SetVisible(true);
		if (m_CurrentPageName == "PageBandages" || m_CurrentPageName == "PageMedication")
			SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, "BodyHead");
		else
			SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, "CloseButton");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.RIGHT, string.Empty);
	}

	protected void ConfigurePatientListNavigation()
	{
		array<ButtonWidget> visibleButtons = {};
		foreach (ButtonWidget patientButton : m_PatientButtons)
		{
			if (patientButton && patientButton.IsVisible())
				visibleButtons.Insert(patientButton);
		}
		if (visibleButtons.IsEmpty())
			return;

		SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, visibleButtons[0].GetName());
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.RIGHT, visibleButtons[0].GetName());
		for (int index = 0; index < visibleButtons.Count(); index++)
		{
			string upTarget = "PageToggleSelf";
			if (index > 0)
				upTarget = visibleButtons[index - 1].GetName();
			string downTarget = "CloseButton";
			if (m_PatientActions && m_PatientActions.IsVisible())
				downTarget = "PatientActionTreatButton";
			else if (index + 1 < visibleButtons.Count())
				downTarget = visibleButtons[index + 1].GetName();
			SetControllerNavigation(visibleButtons[index].GetName(), WidgetNavigationDirection.UP, upTarget);
			SetControllerNavigation(visibleButtons[index].GetName(), WidgetNavigationDirection.DOWN, downTarget);
			SetControllerNavigation(visibleButtons[index].GetName(), WidgetNavigationDirection.LEFT, "PageToggleSelf");
			string rightTarget;
			if (m_PatientActions && m_PatientActions.IsVisible())
				rightTarget = "PatientActionTreatButton";
			SetControllerNavigation(visibleButtons[index].GetName(), WidgetNavigationDirection.RIGHT, rightTarget);
		}

		if (!m_PatientActions || !m_PatientActions.IsVisible())
			return;

		string patientTarget = visibleButtons[0].GetName();
		string treatDownTarget = "CloseButton";
		if (m_PatientUnloadButton.IsEnabled())
			treatDownTarget = "PatientActionUnloadButton";
		SetControllerNavigation("PatientActionTreatButton", WidgetNavigationDirection.UP, patientTarget);
		SetControllerNavigation("PatientActionTreatButton", WidgetNavigationDirection.DOWN, treatDownTarget);
		SetControllerNavigation("PatientActionTreatButton", WidgetNavigationDirection.LEFT, patientTarget);
		SetControllerNavigation("PatientActionTreatButton", WidgetNavigationDirection.RIGHT, "PatientActionUnloadButton");
		SetControllerNavigation("PatientActionUnloadButton", WidgetNavigationDirection.UP, "PatientActionTreatButton");
		SetControllerNavigation("PatientActionUnloadButton", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation("PatientActionUnloadButton", WidgetNavigationDirection.LEFT, "PatientActionTreatButton");
		SetControllerNavigation("PatientActionUnloadButton", WidgetNavigationDirection.RIGHT, "CloseButton");
	}

	void ShowPage(string pageName)
	{
		m_HoveredRequestedButton = null;
		m_IsMedicationPage = false;
		bool hasInventoryControls;
		ResourceName layout;
		switch (pageName)
		{
			case "PageTriage": layout = PAGE_TRIAGE_LAYOUT; break;
			case "PageDiagnose": layout = PAGE_DIAGNOSE_LAYOUT; break;
			case "PageBandages": layout = PAGE_TREATMENT_LAYOUT; hasInventoryControls = true; break;
			case "PageMedication": layout = PAGE_TREATMENT_LAYOUT; hasInventoryControls = true; m_IsMedicationPage = true; break;
			case "PageAdvanced": layout = PAGE_ADVANCED_LAYOUT; break;
			default: return;
		}

		if (m_InventoryRoot)
			m_InventoryRoot.RemoveFromHierarchy();
		m_InventoryRoot = null;
		if (m_PageRoot)
			m_PageRoot.RemoveFromHierarchy();

		m_PatientVitals = null;
		m_PatientVitalValues = null;
		m_PatientMedications = null;
		m_PatientMedicationValues = null;
		m_PatientMedicationsFocus = null;
		m_DiagnoseHeartRateButton = null;
		m_DiagnoseBloodPressureButton = null;
		m_DiagnoseHeartRateText = null;
		m_DiagnoseBloodPressureText = null;
		m_DiagnoseHeartRateValue = null;
		m_DiagnoseBloodPressureValue = null;
		m_DiagnosePatientName = null;
		m_DiagnoseHeartRateGraph = null;
		m_DiagnoseBloodPressureGraph = null;
		m_TriageActivityLog = null;
		m_TriageActivityFocus = null;
		m_TriageLevelButtons.Clear();
		m_TriageLevelTexts.Clear();
		m_SelectedRegionText = null;
		m_TreatmentStatus = null;
		m_ForeignInventoryTitle = null;
		m_BandageRowForeign = null;
		m_TourniquetRowForeign = null;
		m_MedicalKitRow = null;
		m_BandageButton = null;
		m_TourniquetButton = null;
		m_MedicalKitButton = null;
		m_BandageButtonContent = null;
		m_TourniquetButtonContent = null;
		m_MedicalKitButtonContent = null;
		m_BandageCount = null;
		m_TourniquetCount = null;
		m_MedicalKitCount = null;
		m_BandageLabel = null;
		m_TourniquetLabel = null;
		m_MedicalKitLabel = null;
		m_BandageIcon = null;
		m_TourniquetIcon = null;
		m_MedicalKitIcon = null;
		m_BandageButtonForeign = null;
		m_TourniquetButtonForeign = null;
		m_AdvancedCPRButton = null;
		m_AdvancedCPRText = null;
		m_AdvancedCarryButton = null;
		m_AdvancedDragButton = null;
		m_AdvancedBackButton = null;
		m_AdvancedRightSideButton = null;
		m_AdvancedLeftSideButton = null;
		m_AdvancedLoadVehicleButton = null;
		m_BandageButtonContentForeign = null;
		m_TourniquetButtonContentForeign = null;
		m_BandageCountForeign = null;
		m_TourniquetCountForeign = null;
		m_BandageLabelForeign = null;
		m_TourniquetLabelForeign = null;
		m_BandageIconForeign = null;
		m_TourniquetIconForeign = null;
		m_BodyZoneHead = null;
		m_BodyZoneChest = null;
		m_BodyZoneAbdomen = null;
		m_BodyZoneLeftArm = null;
		m_BodyZoneRightArm = null;
		m_BodyZoneLeftLeg = null;
		m_BodyZoneRightLeg = null;
		m_BodyZoneOutlines.Clear();
		m_BodyZoneOutlineMask = null;
		m_BodyInjuryHead = null;
		m_BodyInjuryChest = null;
		m_BodyInjuryAbdomen = null;
		m_BodyInjuryLeftArm = null;
		m_BodyInjuryRightArm = null;
		m_BodyInjuryLeftLeg = null;
		m_BodyInjuryRightLeg = null;
		m_BodySalineBag = null;
		m_BodyBoneLeftArm = null;
		m_BodyBoneRightArm = null;
		m_BodyBoneLeftLeg = null;
		m_BodyBoneRightLeg = null;
		m_BodyTourniquetLeftArm = null;
		m_BodyTourniquetRightArm = null;
		m_BodyTourniquetLeftLeg = null;
		m_BodyTourniquetRightLeg = null;
		m_MedicationButtons.Clear();
		m_MedicationButtonContents.Clear();
		m_MedicationCounts.Clear();
		m_MedicationLabels.Clear();
		m_MedicationIcons.Clear();
		m_MedicationRows.Clear();
		m_MedicationButtonsForeign.Clear();
		m_MedicationButtonContentsForeign.Clear();
		m_MedicationCountsForeign.Clear();
		m_MedicationLabelsForeign.Clear();
		m_MedicationIconsForeign.Clear();
		m_MedicationRowsForeign.Clear();
		m_IsSelfSalineMenuOpen = false;
		m_IsForeignSalineMenuOpen = false;
		m_SalineToggleButton = null;
		m_SalineToggleCount = null;
		m_SalineToggleLabel = null;
		m_SalineToggleIcon = null;
		m_SalineToggleRowForeign = null;
		m_SalineToggleButtonForeign = null;
		m_SalineToggleCountForeign = null;
		m_SalineToggleLabelForeign = null;
		m_SalineToggleIconForeign = null;
		m_StopSalineRow = null;
		m_StopSalineButton = null;
		m_StopSalineLabel = null;
		m_StopSalineRowForeign = null;
		m_StopSalineButtonForeign = null;
		m_StopSalineLabelForeign = null;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		m_PageRoot = workspace.CreateWidgets(layout, m_ContentArea);
		if (!m_PageRoot)
		{
			Print(string.Format("RAMI: Could not load page layout %1", layout), LogLevel.ERROR);
			return;
		}

		FrameSlot.SetAnchorMin(m_PageRoot, 0, 0);
		FrameSlot.SetAnchorMax(m_PageRoot, 1, 1);
		FrameSlot.SetOffsets(m_PageRoot, 0, 0, 0, 0);
		if (hasInventoryControls)
		{
			Widget inventoryFrame = m_PageRoot.FindAnyWidget("InventoryFrame");
			ResourceName inventoryLayout = PAGE_TREATMENT_CONTROLS_LAYOUT;
			if (m_IsMedicationPage)
				inventoryLayout = PAGE_MEDICATION_LAYOUT;
			if (inventoryFrame)
				m_InventoryRoot = workspace.CreateWidgets(inventoryLayout, inventoryFrame);
			if (!m_InventoryRoot)
			{
				Print(string.Format("RAMI: Could not load inventory controls %1", inventoryLayout), LogLevel.ERROR);
				return;
			}

			if (m_IsMedicationPage)
			{
				TextWidget pageTitle = TextWidget.Cast(m_PageRoot.FindAnyWidget("PageTitle"));
				if (pageTitle)
					pageTitle.SetText("Medication");
			}
		}

		m_CurrentPageName = pageName;
		UpdateActivePageHighlight(pageName);
		BindPageWidgets();
		ConfigureControllerNavigation(pageName);
		GetGame().GetCallqueue().CallLater(ConfigureControllerNavigation, 1, false, pageName);
		ApplyButtonTextColors();
		UpdateRequestedButtonStyles();
		UpdatePatientVitals();
	}

	protected void UpdateActivePageHighlight(string pageName)
	{
		if (!m_ActivePageHighlight)
			return;

		switch (pageName)
		{
			case "PageTriage":
				FrameSlot.SetAnchorMin(m_ActivePageHighlight, 0.03, 0.1);
				FrameSlot.SetAnchorMax(m_ActivePageHighlight, 0.08, 0.17);
				break;
			case "PageDiagnose":
				FrameSlot.SetAnchorMin(m_ActivePageHighlight, 0.085, 0.1);
				FrameSlot.SetAnchorMax(m_ActivePageHighlight, 0.135, 0.17);
				break;
			case "PageBandages":
				FrameSlot.SetAnchorMin(m_ActivePageHighlight, 0.14, 0.1);
				FrameSlot.SetAnchorMax(m_ActivePageHighlight, 0.19, 0.17);
				break;
			case "PageMedication":
				FrameSlot.SetAnchorMin(m_ActivePageHighlight, 0.195, 0.1);
				FrameSlot.SetAnchorMax(m_ActivePageHighlight, 0.245, 0.17);
				break;
			case "PageAdvanced":
				FrameSlot.SetAnchorMin(m_ActivePageHighlight, 0.25, 0.1);
				FrameSlot.SetAnchorMax(m_ActivePageHighlight, 0.3, 0.17);
				break;
		}

		FrameSlot.SetOffsets(m_ActivePageHighlight, 0, 0, 0, 0);
	}

	protected void ConfigureControllerNavigation(string pageName)
	{
		string firstPageControl;
		switch (pageName)
		{
			case "PageTriage": firstPageControl = "TriageLevelNone"; break;
			case "PageDiagnose": firstPageControl = "DiagnoseHeartRateButton"; break;
			case "PageBandages": break;
			case "PageMedication": break;
			case "PageAdvanced": firstPageControl = "AdvancedCPRButton"; break;
		}

		array<string> pageButtons = {
			"PageTriage",
			"PageDiagnose",
			"PageBandages",
			"PageMedication",
			"PageAdvanced",
			"PageToggleSelf"
		};
		SetControllerNavigationRow(pageButtons, string.Empty, firstPageControl);
		SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.DOWN, firstPageControl);
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.LEFT, "PageToggleSelf");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "PageToggleSelf");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.RIGHT, string.Empty);
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.DOWN, string.Empty);
		if (pageName == "PageBandages" || pageName == "PageMedication")
			SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, "BodyHead");
		else
			SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, "CloseButton");

		switch (pageName)
		{
			case "PageTriage":
			{
				array<string> triageButtons = {
					"TriageLevelNone",
					"TriageLevelMinimal",
					"TriageLevelDelayed",
					"TriageLevelImmediate",
					"TriageLevelExpectant"
				};
				SetControllerNavigationRow(triageButtons, "PageTriage", "TriageActivityFocus");
				SetControllerNavigation("TriageActivityFocus", WidgetNavigationDirection.UP, "TriageLevelNone");
				SetControllerNavigation("TriageActivityFocus", WidgetNavigationDirection.LEFT, string.Empty);
				SetControllerNavigation("TriageActivityFocus", WidgetNavigationDirection.RIGHT, string.Empty);
				SetControllerNavigation("TriageActivityFocus", WidgetNavigationDirection.DOWN, "CloseButton");
				SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "TriageActivityFocus");
				break;
			}
			case "PageDiagnose":
			{
				array<string> diagnoseButtons = {"DiagnoseHeartRateButton", "DiagnoseBloodPressureButton"};
				SetControllerNavigationRow(diagnoseButtons, "PageDiagnose", "CloseButton");
				SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "DiagnoseBloodPressureButton");
				break;
			}
			case "PageBandages":
			case "PageMedication": ConfigureBodyZoneNavigation(); break;
			case "PageAdvanced": ConfigureAdvancedNavigation(); break;
		}
		if (m_IsPatientWindowOpen)
			ConfigurePatientListNavigation();
	}

	protected void SetControllerNavigationRow(array<string> widgetNames, string upTarget, string downTarget)
	{
		for (int index = 0; index < widgetNames.Count(); index++)
		{
			int leftIndex = index - 1;
			if (leftIndex < 0)
				leftIndex = widgetNames.Count() - 1;
			int rightIndex = index + 1;
			if (rightIndex >= widgetNames.Count())
				rightIndex = 0;

			SetControllerNavigation(widgetNames[index], WidgetNavigationDirection.LEFT, widgetNames[leftIndex]);
			SetControllerNavigation(widgetNames[index], WidgetNavigationDirection.RIGHT, widgetNames[rightIndex]);
			SetControllerNavigation(widgetNames[index], WidgetNavigationDirection.UP, upTarget);
			SetControllerNavigation(widgetNames[index], WidgetNavigationDirection.DOWN, downTarget);
		}
	}

	protected void ConfigureBodyZoneNavigation()
	{
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.LEFT, "BodyLeftLeg");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "BodyRightLeg");
		SetControllerNavigation("BodyHead", WidgetNavigationDirection.UP, "PageToggleSelf");
		SetControllerNavigation("BodyHead", WidgetNavigationDirection.LEFT, "PageToggleSelf");
		SetControllerNavigation("BodyHead", WidgetNavigationDirection.RIGHT, "BodyRightArm");
		SetControllerNavigation("BodyHead", WidgetNavigationDirection.DOWN, "BodyChest");
		SetControllerNavigation("BodyChest", WidgetNavigationDirection.UP, "BodyHead");
		SetControllerNavigation("BodyChest", WidgetNavigationDirection.LEFT, "BodyLeftArm");
		SetControllerNavigation("BodyChest", WidgetNavigationDirection.RIGHT, "BodyRightArm");
		SetControllerNavigation("BodyChest", WidgetNavigationDirection.DOWN, "BodyAbdomen");
		SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.UP, "BodyChest");
		SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.LEFT, string.Empty);
		SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.RIGHT, string.Empty);
		SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.DOWN, "BodyLeftLeg");
		SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.UP, "BodyHead");
		SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.RIGHT, "BodyChest");
		SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.DOWN, string.Empty);
		SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.UP, "BodyHead");
		SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.LEFT, "BodyChest");
		SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.RIGHT, "CloseButton");
		SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.DOWN, string.Empty);
		SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.UP, "BodyAbdomen");
		SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.RIGHT, "BodyRightLeg");
		SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.UP, "BodyAbdomen");
		SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.LEFT, "BodyLeftLeg");
		SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.RIGHT, "CloseButton");
		SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.DOWN, "CloseButton");
		string pageButton = "PageBandages";
		if (m_IsMedicationPage)
			pageButton = "PageMedication";
		SetControllerNavigation("PatientMedicationsFocus", WidgetNavigationDirection.UP, pageButton);
		SetControllerNavigation("PatientMedicationsFocus", WidgetNavigationDirection.LEFT, "BodyRightArm");
		SetControllerNavigation("PatientMedicationsFocus", WidgetNavigationDirection.RIGHT, string.Empty);
		SetControllerNavigation("PatientMedicationsFocus", WidgetNavigationDirection.DOWN, "CloseButton");
		SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.RIGHT, "PatientMedicationsFocus");
		SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.RIGHT, "PatientMedicationsFocus");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "PatientMedicationsFocus");
	}

	protected void UpdateTreatmentItemNavigation()
	{
		array<ButtonWidget> itemButtons = {};
		if (m_IsMedicationPage)
		{
			if (m_SalineToggleButton)
				itemButtons.Insert(m_SalineToggleButton);
			for (int medicationIndex = 0; medicationIndex < m_MedicationButtons.Count(); medicationIndex++)
			{
				if (m_MedicationRows[medicationIndex] && m_MedicationRows[medicationIndex].IsVisible())
					itemButtons.Insert(m_MedicationButtons[medicationIndex]);
			}
			if (m_StopSalineButton && m_StopSalineRow && m_StopSalineRow.IsVisible())
				itemButtons.Insert(m_StopSalineButton);
			if (m_SalineToggleButtonForeign && m_SalineToggleRowForeign && m_SalineToggleRowForeign.IsVisible())
				itemButtons.Insert(m_SalineToggleButtonForeign);
			for (int foreignMedicationIndex = 0; foreignMedicationIndex < m_MedicationButtonsForeign.Count(); foreignMedicationIndex++)
			{
				if (m_MedicationRowsForeign[foreignMedicationIndex] && m_MedicationRowsForeign[foreignMedicationIndex].IsVisible())
					itemButtons.Insert(m_MedicationButtonsForeign[foreignMedicationIndex]);
			}
			if (m_StopSalineButtonForeign && m_StopSalineRowForeign && m_StopSalineRowForeign.IsVisible())
				itemButtons.Insert(m_StopSalineButtonForeign);
		}
		else
		{
			itemButtons.Insert(m_BandageButton);
			itemButtons.Insert(m_TourniquetButton);
			itemButtons.Insert(m_MedicalKitButton);
			itemButtons.Insert(m_BandageButtonForeign);
			itemButtons.Insert(m_TourniquetButtonForeign);
		}

		array<string> availableItems = {};
		foreach (ButtonWidget itemButton : itemButtons)
		{
			if (itemButton && itemButton.IsEnabled())
				availableItems.Insert(itemButton.GetName());
		}

		string firstItem;
		if (!availableItems.IsEmpty())
			firstItem = availableItems[0];
		array<string> treatmentPageButtons = {
			"PageTriage",
			"PageDiagnose",
			"PageBandages",
			"PageMedication",
			"PageAdvanced"
		};
		foreach (string pageButtonName : treatmentPageButtons)
			SetControllerNavigation(pageButtonName, WidgetNavigationDirection.DOWN, firstItem);
		SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.DOWN, firstItem);
		SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.LEFT, firstItem);
		SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.LEFT, firstItem);
		for (int index = 0; index < availableItems.Count(); index++)
		{
			string upTarget = "PageBandages";
			if (m_IsMedicationPage)
				upTarget = "PageMedication";
			if (index > 0)
				upTarget = availableItems[index - 1];
			string downTarget;
			if (index + 1 < availableItems.Count())
				downTarget = availableItems[index + 1];
			SetControllerNavigation(availableItems[index], WidgetNavigationDirection.UP, upTarget);
			SetControllerNavigation(availableItems[index], WidgetNavigationDirection.DOWN, downTarget);
		}
	}

	protected void ConfigureAdvancedNavigation()
	{
		SetControllerNavigation("AdvancedCPRButton", WidgetNavigationDirection.UP, "PageAdvanced");
		SetControllerNavigation("AdvancedCPRButton", WidgetNavigationDirection.DOWN, "AdvancedRightSideButton");
		array<string> positionButtons = {"AdvancedBackButton", "AdvancedRightSideButton", "AdvancedLeftSideButton"};
		array<string> movementButtons = {"AdvancedCarryButton", "AdvancedDragButton", "AdvancedLoadVehicleButton"};
		SetControllerNavigationRow(positionButtons, "AdvancedCPRButton", string.Empty);
		SetControllerNavigationRow(movementButtons, string.Empty, "CloseButton");
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "AdvancedLoadVehicleButton");
		for (int index = 0; index < positionButtons.Count(); index++)
		{
			SetControllerNavigation(positionButtons[index], WidgetNavigationDirection.DOWN, movementButtons[index]);
			SetControllerNavigation(movementButtons[index], WidgetNavigationDirection.UP, positionButtons[index]);
		}
	}

	protected void SetControllerNavigation(string widgetName, WidgetNavigationDirection direction, string targetName)
	{
		Widget widget = GetRootWidget().FindAnyWidget(widgetName);
		if (!widget)
			return;

		if (targetName.IsEmpty())
			widget.SetNavigation(direction, WidgetNavigationRuleType.STOP);
		else
			widget.SetNavigation(direction, WidgetNavigationRuleType.EXPLICIT, targetName);
	}

	protected void BindPageWidgets()
	{
		Widget pageWidgets = m_ContentArea;
		Widget inventoryWidgets = m_InventoryRoot;
		if (!inventoryWidgets)
			inventoryWidgets = m_PageRoot;
		m_PatientVitals = TextWidget.Cast(pageWidgets.FindAnyWidget("PatientVitals"));
		m_PatientVitalValues = TextWidget.Cast(pageWidgets.FindAnyWidget("PatientVitalValues"));
		m_PatientMedications = TextWidget.Cast(pageWidgets.FindAnyWidget("PatientMedications"));
		m_PatientMedicationValues = TextWidget.Cast(pageWidgets.FindAnyWidget("PatientMedicationValues"));
		m_PatientMedicationsFocus = ButtonWidget.Cast(pageWidgets.FindAnyWidget("PatientMedicationsFocus"));
		if (m_PatientMedicationsFocus)
			m_PatientMedicationsFocus.SetFlags(m_PatientMedicationsFocus.GetFlags() | WidgetFlags.IGNORE_CURSOR);
		m_DiagnoseHeartRateButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseHeartRateButton"));
		m_DiagnoseBloodPressureButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseBloodPressureButton"));
		m_DiagnoseHeartRateText = TextWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseHeartRateText"));
		m_DiagnoseBloodPressureText = TextWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseBloodPressureText"));
		m_DiagnoseHeartRateValue = TextWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseHeartRateValue"));
		m_DiagnoseBloodPressureValue = TextWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseBloodPressureValue"));
		m_DiagnosePatientName = TextWidget.Cast(m_PageRoot.FindAnyWidget("DiagnosePatientName"));
		m_DiagnoseHeartRateGraph = CanvasWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseHeartRateGraph"));
		m_DiagnoseBloodPressureGraph = CanvasWidget.Cast(m_PageRoot.FindAnyWidget("DiagnoseBloodPressureGraph"));
		m_TriageActivityLog = TextWidget.Cast(m_PageRoot.FindAnyWidget("TriageActivityLog"));
		m_TriageActivityFocus = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("TriageActivityFocus"));
		if (m_TriageActivityFocus)
			m_TriageActivityFocus.SetFlags(m_TriageActivityFocus.GetFlags() | WidgetFlags.IGNORE_CURSOR);
		array<string> triageLevelButtonNames = {
			"TriageLevelNone",
			"TriageLevelMinimal",
			"TriageLevelDelayed",
			"TriageLevelImmediate",
			"TriageLevelExpectant"
		};
		m_TriageLevelButtons.Clear();
		m_TriageLevelTexts.Clear();
		foreach (string triageLevelButtonName : triageLevelButtonNames)
		{
			ButtonWidget triageLevelButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget(triageLevelButtonName));
			TextWidget triageLevelText = TextWidget.Cast(m_PageRoot.FindAnyWidget(triageLevelButtonName + "Text"));
			m_TriageLevelButtons.Insert(triageLevelButton);
			m_TriageLevelTexts.Insert(triageLevelText);
			if (triageLevelText)
				triageLevelText.SetBold(true);
			if (triageLevelButton)
				triageLevelButton.AddHandler(m_Handler);
		}
		InitializeDiagnoseHeartRateGraph();
		RestoreDiagnosePage();
		if (m_DiagnoseHeartRateButton)
			m_DiagnoseHeartRateButton.AddHandler(m_Handler);
		if (m_DiagnoseBloodPressureButton)
			m_DiagnoseBloodPressureButton.AddHandler(m_Handler);
		m_SelectedRegionText = TextWidget.Cast(m_PageRoot.FindAnyWidget("SelectedRegion"));
		m_TreatmentStatus = TextWidget.Cast(m_PageRoot.FindAnyWidget("TreatmentStatus"));
		m_ForeignInventoryTitle = TextWidget.Cast(inventoryWidgets.FindAnyWidget("ForeigninventoryTitle"));
		m_BandageRowForeign = inventoryWidgets.FindAnyWidget("BandageRowForeign");
		m_TourniquetRowForeign = inventoryWidgets.FindAnyWidget("TourniquetRowForeign");
		m_MedicalKitRow = inventoryWidgets.FindAnyWidget("MedicalKitRow");
		m_BandageButton = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("BandageButton"));
		m_TourniquetButton = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetButton"));
		m_MedicalKitButton = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("MedicalKitButton"));
		m_BandageButtonContent = inventoryWidgets.FindAnyWidget("BandageButtonContent");
		m_TourniquetButtonContent = inventoryWidgets.FindAnyWidget("TourniquetButtonContent");
		m_MedicalKitButtonContent = inventoryWidgets.FindAnyWidget("MedicalKitButtonContent");
		m_BandageCount = TextWidget.Cast(inventoryWidgets.FindAnyWidget("BandageCount"));
		m_TourniquetCount = TextWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetCount"));
		m_MedicalKitCount = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicalKitCount"));
		m_BandageLabel = TextWidget.Cast(inventoryWidgets.FindAnyWidget("BandageLabel"));
		m_TourniquetLabel = TextWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetLabel"));
		m_MedicalKitLabel = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicalKitLabel"));
		m_BandageIcon = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("BandageIcon"));
		m_TourniquetIcon = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetIcon"));
		m_MedicalKitIcon = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("MedicalKitIcon"));
		m_BandageButtonForeign = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("BandageButtonForeign"));
		m_TourniquetButtonForeign = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetButtonForeign"));
		m_AdvancedCPRButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedCPRButton"));
		m_AdvancedCPRText = TextWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedCPRText"));
		m_AdvancedCarryButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedCarryButton"));
		m_AdvancedDragButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedDragButton"));
		m_AdvancedBackButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedBackButton"));
		m_AdvancedRightSideButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedRightSideButton"));
		m_AdvancedLeftSideButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedLeftSideButton"));
		m_AdvancedLoadVehicleButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget("AdvancedLoadVehicleButton"));
		m_BandageButtonContentForeign = inventoryWidgets.FindAnyWidget("BandageButtonContentForeign");
		m_TourniquetButtonContentForeign = inventoryWidgets.FindAnyWidget("TourniquetButtonContentForeign");
		m_BandageCountForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("BandageCountForeign"));
		m_TourniquetCountForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetCountForeign"));
		m_BandageLabelForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("BandageLabelForeign"));
		m_TourniquetLabelForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetLabelForeign"));
		m_BandageIconForeign = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("BandageIconForeign"));
		m_TourniquetIconForeign = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("TourniquetIconForeign"));
		m_BodyZoneHead = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneHead"));
		m_BodyZoneChest = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneChest"));
		m_BodyZoneAbdomen = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneAbdomen"));
		m_BodyZoneLeftArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneLeftArm"));
		m_BodyZoneRightArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneRightArm"));
		m_BodyZoneLeftLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneLeftLeg"));
		m_BodyZoneRightLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneRightLeg"));
		array<string> bodyZoneOutlineNames = {
			"BodyZoneOutlineLeft",
			"BodyZoneOutlineRight",
			"BodyZoneOutlineTop",
			"BodyZoneOutlineBottom"
		};
		foreach (string bodyZoneOutlineName : bodyZoneOutlineNames)
			m_BodyZoneOutlines.Insert(ImageWidget.Cast(m_PageRoot.FindAnyWidget(bodyZoneOutlineName)));
		if (m_BodyZoneOutlines.Count() == 4)
		{
			if (m_BodyZoneOutlines[0])
				FrameSlot.Move(m_BodyZoneOutlines[0], -3, 0);
			if (m_BodyZoneOutlines[1])
				FrameSlot.Move(m_BodyZoneOutlines[1], 3, 0);
			if (m_BodyZoneOutlines[2])
				FrameSlot.Move(m_BodyZoneOutlines[2], 0, -3);
			if (m_BodyZoneOutlines[3])
				FrameSlot.Move(m_BodyZoneOutlines[3], 0, 3);
		}
		m_BodyZoneOutlineMask = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyZoneOutlineMask"));
		if (m_BodyZoneOutlineMask)
			UpdateSelectedBodyRegionHighlight();
		m_BodyInjuryHead = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryHead"));
		m_BodyInjuryChest = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryChest"));
		m_BodyInjuryAbdomen = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryAbdomen"));
		m_BodyInjuryLeftArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryLeftArm"));
		m_BodyInjuryRightArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryRightArm"));
		m_BodyInjuryLeftLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryLeftLeg"));
		m_BodyInjuryRightLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyInjuryRightLeg"));
		m_BodySalineBag = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodySalineBag"));
		if (m_BodySalineBag)
		{
			m_BodySalineBag.LoadImageFromSet(0, MEDICAL_ICON_IMAGE_SET, "Saline-bag_UI");
			m_BodySalineBag.SetColor(Color.FromSRGBA(255, 255, 0, 255));
			m_BodySalineBag.SetVisible(false);
		}
		m_BodyBoneLeftArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyBoneLeftArm"));
		m_BodyBoneRightArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyBoneRightArm"));
		m_BodyBoneLeftLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyBoneLeftLeg"));
		m_BodyBoneRightLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyBoneRightLeg"));
		m_BodyTourniquetLeftArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyTourniquetLeftArm"));
		m_BodyTourniquetRightArm = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyTourniquetRightArm"));
		m_BodyTourniquetLeftLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyTourniquetLeftLeg"));
		m_BodyTourniquetRightLeg = ImageWidget.Cast(m_PageRoot.FindAnyWidget("BodyTourniquetRightLeg"));
		m_SalineToggleButton = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleButton"));
		m_SalineToggleCount = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleCount"));
		m_SalineToggleLabel = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleLabel"));
		m_SalineToggleIcon = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleIcon"));
		m_SalineToggleRowForeign = inventoryWidgets.FindAnyWidget("MedicationSalineToggleRowForeign");
		m_SalineToggleButtonForeign = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleButtonForeign"));
		m_SalineToggleCountForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleCountForeign"));
		m_SalineToggleLabelForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleLabelForeign"));
		m_SalineToggleIconForeign = ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationSalineToggleIconForeign"));
		m_StopSalineRow = inventoryWidgets.FindAnyWidget("MedicationStopSalineRow");
		m_StopSalineButton = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationStopSalineButton"));
		m_StopSalineLabel = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationStopSalineLabel"));
		m_StopSalineRowForeign = inventoryWidgets.FindAnyWidget("MedicationStopSalineRowForeign");
		m_StopSalineButtonForeign = ButtonWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationStopSalineButtonForeign"));
		m_StopSalineLabelForeign = TextWidget.Cast(inventoryWidgets.FindAnyWidget("MedicationStopSalineLabelForeign"));
		array<string> salineToggleWidgets = {
			"MedicationSalineToggleButton",
			"MedicationSalineToggleIcon",
			"MedicationSalineToggleLabel",
			"MedicationSalineToggleCount",
			"MedicationSalineToggleButtonForeign",
			"MedicationSalineToggleIconForeign",
			"MedicationSalineToggleLabelForeign",
			"MedicationSalineToggleCountForeign",
			"MedicationStopSalineButton",
			"MedicationStopSalineLabel",
			"MedicationStopSalineButtonForeign",
			"MedicationStopSalineLabelForeign"
		};
		foreach (string salineToggleWidgetName : salineToggleWidgets)
		{
			Widget salineToggleWidget = inventoryWidgets.FindAnyWidget(salineToggleWidgetName);
			if (salineToggleWidget)
				salineToggleWidget.AddHandler(m_Handler);
		}
		for (int index = 0; index < m_MedicationWidgetNames.Count(); index++)
		{
			string widgetName = m_MedicationWidgetNames[index];
			m_MedicationButtons.Insert(ButtonWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "Button")));
			m_MedicationButtonContents.Insert(inventoryWidgets.FindAnyWidget(widgetName + "Content"));
			m_MedicationCounts.Insert(TextWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "Count")));
			m_MedicationLabels.Insert(TextWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "Label")));
			m_MedicationIcons.Insert(ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "Icon")));
			m_MedicationRows.Insert(inventoryWidgets.FindAnyWidget(widgetName + "Row"));
			m_MedicationButtonsForeign.Insert(ButtonWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "ButtonForeign")));
			m_MedicationButtonContentsForeign.Insert(inventoryWidgets.FindAnyWidget(widgetName + "ContentForeign"));
			m_MedicationCountsForeign.Insert(TextWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "CountForeign")));
			m_MedicationLabelsForeign.Insert(TextWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "LabelForeign")));
			m_MedicationIconsForeign.Insert(ItemPreviewWidget.Cast(inventoryWidgets.FindAnyWidget(widgetName + "IconForeign")));
			m_MedicationRowsForeign.Insert(inventoryWidgets.FindAnyWidget(widgetName + "RowForeign"));

			array<string> suffixes = {"Button", "Content", "Icon", "Label", "Count", "ButtonForeign", "ContentForeign", "IconForeign", "LabelForeign", "CountForeign"};
			foreach (string suffix : suffixes)
			{
				Widget medicationWidget = inventoryWidgets.FindAnyWidget(widgetName + suffix);
				if (medicationWidget)
					medicationWidget.AddHandler(m_Handler);
			}
		}
		array<string> treatmentItemWidgets = {
			"BandageButton",
			"BandageButtonContent",
			"BandageIcon",
			"BandageLabel",
			"BandageCount",
			"TourniquetButton",
			"TourniquetButtonContent",
			"TourniquetIcon",
			"TourniquetLabel",
			"TourniquetCount",
			"MedicalKitButton",
			"MedicalKitButtonContent",
			"MedicalKitIcon",
			"MedicalKitLabel",
			"MedicalKitCount",
			"BandageButtonForeign",
			"BandageButtonContentForeign",
			"BandageIconForeign",
			"BandageLabelForeign",
			"BandageCountForeign",
			"TourniquetButtonForeign",
			"TourniquetButtonContentForeign",
			"TourniquetIconForeign",
			"TourniquetLabelForeign",
			"TourniquetCountForeign"
		};
		foreach (string treatmentItemWidgetName : treatmentItemWidgets)
		{
			Widget treatmentItemWidget = m_PageRoot.FindAnyWidget(treatmentItemWidgetName);
			if (treatmentItemWidget)
				treatmentItemWidget.AddHandler(m_Handler);
		}

		array<string> bodyRegionButtons = {
			"BodyHead",
			"BodyChest",
			"BodyAbdomen",
			"BodyLeftArm",
			"BodyRightArm",
			"BodyLeftLeg",
			"BodyRightLeg"
		};

		foreach (string bodyRegionButtonName : bodyRegionButtons)
		{
			ButtonWidget bodyRegionButton = ButtonWidget.Cast(m_PageRoot.FindAnyWidget(bodyRegionButtonName));
			if (bodyRegionButton)
				bodyRegionButton.AddHandler(m_Handler);
		}

		UpdateSelectedRegionText();
		if (m_IsMedicationPage)
			UpdateMedicationItems();
		else
			UpdateTreatmentButtons();
		if (m_TreatmentStatus)
		{
			if (m_IsMedicationPage)
				m_TreatmentStatus.SetText("SELECT AN AVAILABLE MEDICATION");
			else
				m_TreatmentStatus.SetText("SELECT AN AVAILABLE TREATMENT");
		}

		array<ButtonWidget> advancedActionButtons = {
			m_AdvancedCPRButton,
			m_AdvancedCarryButton,
			m_AdvancedDragButton,
			m_AdvancedBackButton,
			m_AdvancedRightSideButton,
			m_AdvancedLeftSideButton,
			m_AdvancedLoadVehicleButton
		};
		foreach (ButtonWidget advancedActionButton : advancedActionButtons)
		{
			if (advancedActionButton)
				advancedActionButton.AddHandler(m_Handler);
		}
		UpdatePatientVitals();
	}

	void SetTriageLevel(string widgetName)
	{
		RAMI_ETriageLevel triageLevel;
		switch (widgetName)
		{
			case "TriageLevelMinimal": triageLevel = RAMI_ETriageLevel.MINIMAL; break;
			case "TriageLevelDelayed": triageLevel = RAMI_ETriageLevel.DELAYED; break;
			case "TriageLevelImmediate": triageLevel = RAMI_ETriageLevel.IMMEDIATE; break;
			case "TriageLevelExpectant": triageLevel = RAMI_ETriageLevel.EXPECTANT; break;
			case "TriageLevelNone": triageLevel = RAMI_ETriageLevel.NONE; break;
			default: return;
		}

		SCR_CharacterDamageManagerComponent damageManager;
		if (s_Patient)
			damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (damageManager && playerController)
			playerController.RAMI_SetPatientTriage(damageManager, triageLevel);
	}

	protected void UpdateTriagePage()
	{
		if (!m_TriageActivityLog || !s_Patient)
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!damageManager || !playerController)
			return;

		playerController.RAMI_RequestTriage(damageManager);
		RAMI_ETriageLevel triageLevel;
		array<int> times = {};
		array<string> messages = {};
		if (!playerController.RAMI_GetTriage(damageManager, triageLevel, times, messages))
		{
			m_TriageActivityLog.SetText("READING ACTIVITY...");
			return;
		}

		UpdateTriageButtonSelection(triageLevel);
		if (messages.IsEmpty())
		{
			m_TriageActivityLog.SetText("NO ACTIVITY RECORDED");
			return;
		}

		string activityText;
		for (int index = 0; index < messages.Count(); index++)
		{
			if (!activityText.IsEmpty())
				activityText += "\n";
			activityText += string.Format("[%1] %2", FormatLocalActivityTime(times[index]), messages[index]);
		}
		m_TriageActivityLog.SetText(activityText);
	}

	protected Color GetTriageLevelColor(RAMI_ETriageLevel level)
	{
		if (level == RAMI_ETriageLevel.NONE)
			return Color.FromSRGBA(128, 128, 128, 255);

		if (m_ColorblindMode)
		{
			switch (level)
			{
				case RAMI_ETriageLevel.MINIMAL: return Color.FromSRGBA(0, 114, 178, 255);
				case RAMI_ETriageLevel.DELAYED: return Color.FromSRGBA(240, 228, 66, 255);
				case RAMI_ETriageLevel.IMMEDIATE: return Color.FromSRGBA(213, 94, 0, 255);
			}
			return Color.FromSRGBA(0, 0, 0, 255);
		}

		switch (level)
		{
			case RAMI_ETriageLevel.MINIMAL: return Color.FromSRGBA(0, 115, 0, 255);
			case RAMI_ETriageLevel.DELAYED: return Color.FromSRGBA(255, 220, 0, 255);
			case RAMI_ETriageLevel.IMMEDIATE: return Color.FromSRGBA(204, 0, 0, 255);
		}
		return Color.FromSRGBA(0, 0, 0, 255);
	}

	protected Color GetTriageLevelReadableTextColor(RAMI_ETriageLevel level)
	{
		if (level == RAMI_ETriageLevel.DELAYED || level == RAMI_ETriageLevel.NONE)
			return Color.FromSRGBA(0, 0, 0, 255);
		return Color.FromSRGBA(255, 255, 255, 255);
	}

	protected void UpdateTriageButtonSelection(RAMI_ETriageLevel triageLevel)
	{
		for (int index = 0; index < m_TriageLevelButtons.Count(); index++)
		{
			ButtonWidget button = m_TriageLevelButtons[index];
			TextWidget text = m_TriageLevelTexts[index];
			Color buttonColor = GetTriageLevelColor(index);
			buttonColor.SetA(0.9);
			if (index == triageLevel)
			{
				if (button)
					button.SetColor(buttonColor);
				if (text)
					text.SetColor(GetTriageLevelReadableTextColor(index));
			}
			else
			{
				if (button)
					button.SetColor(Color.FromSRGBA(255, 255, 255, 230));
				if (text)
					text.SetColor(GetButtonTextColor(Color.Black));
			}
			ApplyButtonTextContrast(text);
		}
	}

	protected string FormatLocalActivityTime(int unixTime)
	{
		int localYear, localMonth, localDay, localHour, localMinute, localSecond;
		int utcYear, utcMonth, utcDay, utcHour, utcMinute, utcSecond;
		System.GetYearMonthDay(localYear, localMonth, localDay);
		System.GetHourMinuteSecond(localHour, localMinute, localSecond);
		System.GetYearMonthDayUTC(utcYear, utcMonth, utcDay);
		System.GetHourMinuteSecondUTC(utcHour, utcMinute, utcSecond);
		int localOffsetSeconds = (SCR_DateTimeHelper.ConvertDateIntoMinutes(localYear, localMonth, localDay, localHour, localMinute)
			- SCR_DateTimeHelper.ConvertDateIntoMinutes(utcYear, utcMonth, utcDay, utcHour, utcMinute)) * 60
			+ localSecond - utcSecond;
		int localSeconds = unixTime % 86400 + localOffsetSeconds;
		while (localSeconds < 0)
			localSeconds += 86400;
		localSeconds = localSeconds % 86400;

		return string.Format("%1:%2:%3", PadTime(localSeconds / 3600), PadTime(localSeconds / 60 % 60), PadTime(localSeconds % 60));
	}

	protected string PadTime(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}

	void ToggleDiagnoseMonitor(string widgetName)
	{
		if (widgetName == "DiagnoseHeartRateButton")
			m_IsHeartRateMonitored = !m_IsHeartRateMonitored;
		else if (widgetName == "DiagnoseBloodPressureButton")
			m_IsBloodPressureMonitored = !m_IsBloodPressureMonitored;

		UpdateDiagnoseButtonText();
		if (!m_IsHeartRateMonitored && !m_IsBloodPressureMonitored)
		{
			GetGame().GetCallqueue().Remove(UpdateDiagnoseMonitor);
			return;
		}

		if (widgetName == "DiagnoseHeartRateButton" && m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
		{
			m_DiagnoseHeartRate = 0;
			m_DiagnoseHeartRateTarget = 0;
			m_DiagnoseHeartPhase = 0;
			m_DiagnoseHeartSampleTime = 0;
			m_DiagnoseHeartTrace.Clear();
			if (m_DiagnoseHeartRateLine)
				m_DiagnoseHeartRateLine.m_Vertices.Clear();
			if (m_DiagnoseHeartRateDot)
				m_DiagnoseHeartRateDot.m_Vertices.Clear();
			if (m_DiagnoseHeartRateGraph)
				m_DiagnoseHeartRateGraph.Update();
			m_DiagnoseHeartRateValue.SetText("READING...");
		}
		if (widgetName == "DiagnoseBloodPressureButton" && m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
		{
			m_DiagnoseSystolicHistory.Clear();
			m_DiagnoseDiastolicHistory.Clear();
			m_DiagnoseBloodPressureGraphCommands.Clear();
			if (m_DiagnoseBloodPressureGraph)
				m_DiagnoseBloodPressureGraph.SetDrawCommands(m_DiagnoseBloodPressureGraphCommands);
			m_DiagnoseBloodPressureValue.SetText("READING...");
		}
		GetGame().GetCallqueue().Remove(UpdateDiagnoseMonitor);
		UpdateDiagnoseMonitor();
		GetGame().GetCallqueue().CallLater(UpdateDiagnoseMonitor, DIAGNOSE_UPDATE_INTERVAL_MS, true);
	}

	protected void UpdateDiagnoseButtonText()
	{
		if (m_DiagnoseHeartRateText)
		{
			if (m_IsHeartRateMonitored)
				m_DiagnoseHeartRateText.SetText("STOP HEART RATE");
			else
				m_DiagnoseHeartRateText.SetText("MONITOR HEART RATE");
		}

		if (m_DiagnoseBloodPressureText)
		{
			if (m_IsBloodPressureMonitored)
				m_DiagnoseBloodPressureText.SetText("STOP BLOOD PRESSURE");
			else
				m_DiagnoseBloodPressureText.SetText("MONITOR BLOOD PRESSURE");
		}

		if (!m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
			m_DiagnoseHeartRateValue.SetText("NOT MONITORED");
		if (!m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
			m_DiagnoseBloodPressureValue.SetText("NOT MONITORED");
		UpdateRequestedButtonStyles();
	}

	protected void RequestDiagnoseVitals()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController || !s_Patient)
			return;

		ACE_Medical_NetworkComponent networkComponent = ACE_Medical_NetworkComponent.Cast(playerController.FindComponent(ACE_Medical_NetworkComponent));
		if (networkComponent)
			networkComponent.RequestUpdatePatientData(s_Patient);
	}

	protected void UpdateDiagnoseMonitor()
	{
		if (!m_IsHeartRateMonitored && !m_IsBloodPressureMonitored)
			return;

		bool hasMedicalKit = HasMedicalKit();
		UpdateDiagnoseGraphVisibility(hasMedicalKit);

		if (!s_Patient)
		{
			if (m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
				m_DiagnoseHeartRateValue.SetText("UNAVAILABLE");
			if (m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
				m_DiagnoseBloodPressureValue.SetText("UNAVAILABLE");
			return;
		}

		ACE_Medical_VitalsComponent vitals = ACE_Medical_VitalsComponent.Cast(s_Patient.FindComponent(ACE_Medical_VitalsComponent));
		if (!vitals)
		{
			if (m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
				m_DiagnoseHeartRateValue.SetText("UNAVAILABLE");
			if (m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
				m_DiagnoseBloodPressureValue.SetText("UNAVAILABLE");
			return;
		}

		if (m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
		{
			m_DiagnoseHeartRateTarget = vitals.GetHeartRate();
			if (m_DiagnoseHeartRate <= 0)
				m_DiagnoseHeartRate = m_DiagnoseHeartRateTarget;
			if (m_DiagnoseHeartRateTarget <= 0)
				m_DiagnoseHeartRateValue.SetText("NONE");
			else if (hasMedicalKit)
				m_DiagnoseHeartRateValue.SetText(string.Format("%1 BPM", Math.Round(m_DiagnoseHeartRateTarget)));
			else
				m_DiagnoseHeartRateValue.SetText(GetCoarseLevel(m_DiagnoseHeartRateTarget, LOW_HEART_RATE_BPM, HIGH_HEART_RATE_BPM));
		}
		if (m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue)
		{
			Tuple2<float, float> pressures = vitals.GetBloodPressures();
			float systolicPressure = pressures.param2 * ACE_PhysicalConstants.KPA2MMHG;
			float diastolicPressure = pressures.param1 * ACE_PhysicalConstants.KPA2MMHG;
			if (hasMedicalKit)
			{
				m_DiagnoseBloodPressureValue.SetText(string.Format(
					"%1 / %2 mmHg",
					Math.Round(systolicPressure),
					Math.Round(diastolicPressure)
				));
			}
			else if (systolicPressure < LOW_SYSTOLIC_PRESSURE_MMHG || diastolicPressure < LOW_DIASTOLIC_PRESSURE_MMHG)
				m_DiagnoseBloodPressureValue.SetText("LOW");
			else if (systolicPressure > HIGH_SYSTOLIC_PRESSURE_MMHG || diastolicPressure > HIGH_DIASTOLIC_PRESSURE_MMHG)
				m_DiagnoseBloodPressureValue.SetText("HIGH");
			else
				m_DiagnoseBloodPressureValue.SetText("MODERATE");
			if (m_DiagnoseSystolicHistory.Count() >= DIAGNOSE_BLOOD_PRESSURE_HISTORY_COUNT)
			{
				m_DiagnoseSystolicHistory.Clear();
				m_DiagnoseDiastolicHistory.Clear();
			}
			m_DiagnoseSystolicHistory.Insert(systolicPressure);
			m_DiagnoseDiastolicHistory.Insert(diastolicPressure);
			if (m_DiagnoseBloodPressureGraph)
				DrawDiagnoseBloodPressureGraph();
		}

		RequestDiagnoseVitals();
	}

	protected void RestoreDiagnosePage()
	{
		UpdateDiagnoseButtonText();
		bool hasMedicalKit = HasMedicalKit();
		UpdateDiagnoseGraphVisibility(hasMedicalKit);
		if (m_IsHeartRateMonitored && m_DiagnoseHeartRateValue)
		{
			if (m_DiagnoseHeartRateTarget <= 0)
				m_DiagnoseHeartRateValue.SetText("NONE");
			else if (hasMedicalKit)
				m_DiagnoseHeartRateValue.SetText(string.Format("%1 BPM", Math.Round(m_DiagnoseHeartRateTarget)));
			else
				m_DiagnoseHeartRateValue.SetText(GetCoarseLevel(m_DiagnoseHeartRateTarget, LOW_HEART_RATE_BPM, HIGH_HEART_RATE_BPM));
		}

		if (m_IsBloodPressureMonitored && m_DiagnoseBloodPressureValue && !m_DiagnoseSystolicHistory.IsEmpty())
		{
			int lastSample = m_DiagnoseSystolicHistory.Count() - 1;
			if (hasMedicalKit)
			{
				m_DiagnoseBloodPressureValue.SetText(string.Format(
					"%1 / %2 mmHg",
					Math.Round(m_DiagnoseSystolicHistory[lastSample]),
					Math.Round(m_DiagnoseDiastolicHistory[lastSample])
				));
			}
			else if (m_DiagnoseSystolicHistory[lastSample] < LOW_SYSTOLIC_PRESSURE_MMHG || m_DiagnoseDiastolicHistory[lastSample] < LOW_DIASTOLIC_PRESSURE_MMHG)
				m_DiagnoseBloodPressureValue.SetText("LOW");
			else if (m_DiagnoseSystolicHistory[lastSample] > HIGH_SYSTOLIC_PRESSURE_MMHG || m_DiagnoseDiastolicHistory[lastSample] > HIGH_DIASTOLIC_PRESSURE_MMHG)
				m_DiagnoseBloodPressureValue.SetText("HIGH");
			else
				m_DiagnoseBloodPressureValue.SetText("MODERATE");
		}

		if (m_DiagnoseHeartRateGraph && !m_DiagnoseHeartTrace.IsEmpty())
			DrawDiagnoseHeartRateGraph();
		if (m_DiagnoseBloodPressureGraph && !m_DiagnoseSystolicHistory.IsEmpty())
			DrawDiagnoseBloodPressureGraph();
	}

	protected void UpdateDiagnoseGraphVisibility(bool visible)
	{
		if (!m_PageRoot || m_CurrentPageName != "PageDiagnose")
			return;

		array<string> widgetNames = {
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
		foreach (string widgetName : widgetNames)
		{
			Widget widget = m_PageRoot.FindAnyWidget(widgetName);
			if (widget)
				widget.SetVisible(visible);
		}
	}

	protected void ClearDiagnoseGraphs()
	{
		m_DiagnoseHeartRate = 0;
		m_DiagnoseHeartRateTarget = 0;
		m_DiagnoseHeartPhase = 0;
		m_DiagnoseHeartSampleTime = 0;
		m_DiagnoseHeartTrace.Clear();
		m_DiagnoseSystolicHistory.Clear();
		m_DiagnoseDiastolicHistory.Clear();
		if (m_DiagnoseHeartRateLine)
			m_DiagnoseHeartRateLine.m_Vertices.Clear();
		if (m_DiagnoseHeartRateDot)
			m_DiagnoseHeartRateDot.m_Vertices.Clear();
		m_DiagnoseBloodPressureGraphCommands.Clear();
		if (m_DiagnoseHeartRateGraph)
			m_DiagnoseHeartRateGraph.Update();
		if (m_DiagnoseBloodPressureGraph)
			m_DiagnoseBloodPressureGraph.SetDrawCommands(m_DiagnoseBloodPressureGraphCommands);
	}

	protected void InitializeDiagnoseHeartRateGraph()
	{
		m_DiagnoseHeartRateLine = null;
		m_DiagnoseHeartRateDot = null;
		m_DiagnoseHeartRateGraphCommands.Clear();
		if (!m_DiagnoseHeartRateGraph)
			return;

		m_DiagnoseHeartRateLine = new LineDrawCommand();
		m_DiagnoseHeartRateLine.m_iColor = 0xFFFF5050;
		m_DiagnoseHeartRateLine.m_fWidth = 2;
		m_DiagnoseHeartRateLine.m_Vertices = {};
		m_DiagnoseHeartRateDot = new PolygonDrawCommand();
		m_DiagnoseHeartRateDot.m_iColor = 0xFFFFFFFF;
		m_DiagnoseHeartRateDot.m_Vertices = {};
		m_DiagnoseHeartRateGraphCommands.Insert(m_DiagnoseHeartRateLine);
		m_DiagnoseHeartRateGraphCommands.Insert(m_DiagnoseHeartRateDot);
		m_DiagnoseHeartRateGraph.SetDrawCommands(m_DiagnoseHeartRateGraphCommands);
	}

	protected void DrawDiagnoseHeartRateGraph()
	{
		float width, height;
		m_DiagnoseHeartRateGraph.GetScreenSize(width, height);
		if (width <= 0 || height <= 0 || m_DiagnoseHeartTrace.IsEmpty())
			return;

		if (!m_DiagnoseHeartRateLine)
			InitializeDiagnoseHeartRateGraph();
		if (!m_DiagnoseHeartRateLine)
			return;

		m_DiagnoseHeartRateLine.m_Vertices.Clear();
		m_DiagnoseHeartRateDot.m_Vertices.Clear();
		float plotLeft = 12;
		float plotRight = width - 12;
		int sampleCount = m_DiagnoseHeartTrace.Count();
		for (int index = 0; index < sampleCount; index++)
		{
			float x = plotLeft + (plotRight - plotLeft) * index / (DIAGNOSE_HEART_TRACE_SAMPLE_COUNT - 1);
			m_DiagnoseHeartRateLine.m_Vertices.Insert(x);
			m_DiagnoseHeartRateLine.m_Vertices.Insert(height * (0.55 - m_DiagnoseHeartTrace[index] * 0.4));
		}

		if (sampleCount == 1)
		{
			m_DiagnoseHeartRateLine.m_Vertices.Insert(plotLeft + 2);
			m_DiagnoseHeartRateLine.m_Vertices.Insert(m_DiagnoseHeartRateLine.m_Vertices[1]);
		}

		int lastVertex = m_DiagnoseHeartRateLine.m_Vertices.Count() - 2;
		m_DiagnoseHeartRateGraph.TessellateCircle(
			Vector(m_DiagnoseHeartRateLine.m_Vertices[lastVertex], m_DiagnoseHeartRateLine.m_Vertices[lastVertex + 1], 0),
			4,
			12,
			m_DiagnoseHeartRateDot.m_Vertices
		);

		m_DiagnoseHeartRateGraph.Update();
	}

	// Rhythm shape visualizes rate only; ACE remains authoritative for medical state.
	protected float GetHeartRhythmAmplitude(float phase)
	{
		if (phase < 0.12)
			return 0;
		if (phase < 0.15)
			return (phase - 0.12) / 0.03 * 0.15;
		if (phase < 0.18)
			return (0.18 - phase) / 0.03 * 0.15;
		if (phase < 0.30)
			return 0;
		if (phase < 0.33)
			return -(phase - 0.30) / 0.03 * 0.2;
		if (phase < 0.36)
			return -0.2 + (phase - 0.33) / 0.03 * 1.2;
		if (phase < 0.40)
			return 1 - (phase - 0.36) / 0.04 * 1.35;
		if (phase < 0.44)
			return -0.35 + (phase - 0.40) / 0.04 * 0.35;
		if (phase < 0.56)
			return 0;
		if (phase < 0.64)
			return (phase - 0.56) / 0.08 * 0.3;
		if (phase < 0.72)
			return (0.72 - phase) / 0.08 * 0.3;

		return 0;
	}

	protected void DrawDiagnoseBloodPressureGraph()
	{
		float width, height;
		m_DiagnoseBloodPressureGraph.GetScreenSize(width, height);
		if (width <= 0 || height <= 0 || m_DiagnoseSystolicHistory.IsEmpty())
			return;

		ref LineDrawCommand axes = new LineDrawCommand();
		ref LineDrawCommand middleGridLine = new LineDrawCommand();
		ref LineDrawCommand systolicLine = new LineDrawCommand();
		ref LineDrawCommand diastolicLine = new LineDrawCommand();
		ref PolygonDrawCommand systolicDot = new PolygonDrawCommand();
		ref PolygonDrawCommand diastolicDot = new PolygonDrawCommand();
		axes.m_iColor = 0xFF808080;
		middleGridLine.m_iColor = 0x40808080;
		systolicLine.m_iColor = 0xFFFFA040;
		diastolicLine.m_iColor = 0xFF40C0FF;
		systolicDot.m_iColor = 0xFFFFA040;
		diastolicDot.m_iColor = 0xFF40C0FF;
		axes.m_fWidth = 1;
		middleGridLine.m_fWidth = 1;
		systolicLine.m_fWidth = 2;
		diastolicLine.m_fWidth = 2;
		float plotLeft = 12;
		float plotRight = width - 12;
		float plotTop = 8;
		float plotBottom = height - 12;
		axes.m_Vertices = {plotLeft, plotTop, plotLeft, plotBottom, plotRight, plotBottom};
		middleGridLine.m_Vertices = {plotLeft, (plotTop + plotBottom) * 0.5, plotRight, (plotTop + plotBottom) * 0.5};
		systolicLine.m_Vertices = {};
		diastolicLine.m_Vertices = {};
		systolicDot.m_Vertices = {};
		diastolicDot.m_Vertices = {};

		int sampleCount = m_DiagnoseSystolicHistory.Count();
		for (int index = 0; index < sampleCount; index++)
		{
			float x = plotLeft + (plotRight - plotLeft) * index / (DIAGNOSE_BLOOD_PRESSURE_HISTORY_COUNT - 1);
			systolicLine.m_Vertices.Insert(x);
			systolicLine.m_Vertices.Insert(plotBottom - (plotBottom - plotTop) * (Math.Clamp(m_DiagnoseSystolicHistory[index], 40, 200) - 40) / 160);
			diastolicLine.m_Vertices.Insert(x);
			diastolicLine.m_Vertices.Insert(plotBottom - (plotBottom - plotTop) * (Math.Clamp(m_DiagnoseDiastolicHistory[index], 40, 200) - 40) / 160);
		}

		if (sampleCount == 1)
		{
			systolicLine.m_Vertices.Insert(plotLeft + 2);
			systolicLine.m_Vertices.Insert(systolicLine.m_Vertices[1]);
			diastolicLine.m_Vertices.Insert(plotLeft + 2);
			diastolicLine.m_Vertices.Insert(diastolicLine.m_Vertices[1]);
		}

		int lastVertex = systolicLine.m_Vertices.Count() - 2;
		m_DiagnoseBloodPressureGraph.TessellateCircle(Vector(systolicLine.m_Vertices[lastVertex], systolicLine.m_Vertices[lastVertex + 1], 0), 4, 12, systolicDot.m_Vertices);
		m_DiagnoseBloodPressureGraph.TessellateCircle(Vector(diastolicLine.m_Vertices[lastVertex], diastolicLine.m_Vertices[lastVertex + 1], 0), 4, 12, diastolicDot.m_Vertices);

		m_DiagnoseBloodPressureGraphCommands.Clear();
		m_DiagnoseBloodPressureGraphCommands.Insert(axes);
		m_DiagnoseBloodPressureGraphCommands.Insert(middleGridLine);
		m_DiagnoseBloodPressureGraphCommands.Insert(systolicLine);
		m_DiagnoseBloodPressureGraphCommands.Insert(diastolicLine);
		m_DiagnoseBloodPressureGraphCommands.Insert(systolicDot);
		m_DiagnoseBloodPressureGraphCommands.Insert(diastolicDot);
		m_DiagnoseBloodPressureGraph.SetDrawCommands(m_DiagnoseBloodPressureGraphCommands);
	}

	protected void GetTreatmentInventoryItems(out array<IEntity> items)
	{
		SCR_ChimeraCharacter inventoryOwner = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!inventoryOwner)
			return;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(inventoryOwner.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager)
			storageManager.GetItems(items);
	}

	protected bool GetForeignTreatmentInventory(out int bandageCount, out int tourniquetCount, out bool canBandage, out bool canTourniquet, out ResourceName bandagePrefab, out ResourceName tourniquetPrefab)
	{
		if (!s_ForeignPatient || s_Patient != s_ForeignPatient)
			return false;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!damageManager || !playerController)
			return false;

		playerController.RAMI_RequestTreatmentInventory(damageManager, m_SelectedRegion);
		return playerController.RAMI_GetTreatmentInventory(damageManager, m_SelectedRegion, bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab);
	}

	protected bool CanTreatPatient()
	{
		if (!s_Patient)
			return false;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(s_Patient.GetCharacterController());
		return controller && controller.GetLifeState() != ECharacterLifeState.DEAD;
	}

	protected IEntity FindMedicalKit(out int count)
	{
		array<IEntity> items = {};
		GetTreatmentInventoryItems(items);
		IEntity medicalKit;
		foreach (IEntity item : items)
		{
			if (!item.FindComponent(SCR_HealSupportStationComponent))
				continue;

			count++;
			if (!medicalKit)
				medicalKit = item;
		}

		return medicalKit;
	}

	protected bool HasMedicalKit()
	{
		int count;
		return FindMedicalKit(count) != null;
	}

	protected string GetCoarseLevel(float value, float lowThreshold, float highThreshold)
	{
		if (value < lowThreshold)
			return "LOW";
		if (value > highThreshold)
			return "HIGH";

		return "MODERATE";
	}

	protected SCR_HealSupportStationAction FindMedicalKitAction(SCR_ChimeraCharacter patient, ECharacterHitZoneGroup region)
	{
		if (!patient)
			return null;

		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(patient.FindComponent(ActionsManagerComponent));
		if (!actionsManager)
			return null;

		array<BaseUserAction> actions = {};
		actionsManager.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			SCR_HealSupportStationAction healAction = SCR_HealSupportStationAction.Cast(action);
			if (healAction && healAction.GetHitZoneGroup() == region)
				return healAction;
		}

		return null;
	}

	void UseMedicalKit()
	{
		int count;
		IEntity medicalKit = FindMedicalKit(count);
		if (!medicalKit || !CanTreatPatient() || !FindMedicalKitAction(s_Patient, m_SelectedRegion))
			return;

		StartTreatmentWithItem(medicalKit);
	}

	protected void FinishPendingMedicalKit(SCR_ChimeraCharacter user)
	{
		SCR_HealSupportStationAction action = FindMedicalKitAction(m_PendingTreatmentPatient, m_PendingTreatmentRegion);
		ActionsPerformerComponent performer = ActionsPerformerComponent.Cast(user.FindComponent(ActionsPerformerComponent));
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!action || !performer || !characterStorage || !action.CanBeShown(user) || !action.CanBePerformed(user))
		{
			if (m_TreatmentStatus)
				m_TreatmentStatus.SetText("MEDICAL KIT CANNOT HEAL REGION");
			ClearPendingTreatment();
			return;
		}

		performer.StartAction(action);
		m_IsMedicalKitTreatmentActive = true;
		if (m_TreatmentStatus)
			m_TreatmentStatus.SetText("MEDICAL KIT TREATMENT IN PROGRESS");
	}

	protected void UpdatePendingMedicalKit(float timeSlice)
	{
		if (!m_IsMedicalKitTreatmentActive || !m_PendingTreatmentItem || !m_PendingTreatmentItem.FindComponent(SCR_HealSupportStationComponent))
			return;

		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		SCR_HealSupportStationAction action = FindMedicalKitAction(m_PendingTreatmentPatient, m_PendingTreatmentRegion);
		ActionsPerformerComponent performer;
		SCR_CharacterInventoryStorageComponent characterStorage;
		if (user)
		{
			performer = ActionsPerformerComponent.Cast(user.FindComponent(ActionsPerformerComponent));
			characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		}

		if (!action || !performer || !characterStorage || characterStorage.GetCurrentItem() != m_PendingTreatmentItem || !action.CanBePerformed(user))
		{
			ClearPendingTreatment();
			if (m_TreatmentStatus)
				m_TreatmentStatus.SetText("MEDICAL KIT TREATMENT COMPLETE");
			return;
		}

		performer.PerformContinuousAction(action, timeSlice);
	}

	protected IEntity FindApplicableTreatmentItem(int commonType)
	{
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user || !CanTreatPatient())
			return null;
		if (!SCR_PlayerController.RAMI_CanApplyMedicationToRegion(s_Patient, commonType, m_SelectedRegion))
			return null;

		array<IEntity> items = {};
		GetTreatmentInventoryItems(items);
		foreach (IEntity item : items)
		{
			if (!SCR_PlayerController.RAMI_ItemMatchesTreatment(item, commonType))
				continue;

			SCR_ConsumableItemComponent consumable;
			SCR_ConsumableEffectHealthItems effect = GetTreatmentEffect(item, consumable);
			int failReason;
			if (effect && effect.CanApplyEffectToHZ(s_Patient, user, m_SelectedRegion, failReason))
				return item;
		}

		return null;
	}

	protected SCR_ConsumableEffectHealthItems GetTreatmentEffect(IEntity item, out SCR_ConsumableItemComponent consumable)
	{
		consumable = null;
		if (!item)
			return null;

		consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!consumable)
			return null;

		return SCR_ConsumableEffectHealthItems.Cast(consumable.GetConsumableEffect());
	}

	protected bool CanStartTreatment(int commonType, bool foreignInventory)
	{
		if (foreignInventory)
		{
			int bandageCount;
			int tourniquetCount;
			bool canBandage;
			bool canTourniquet;
			ResourceName bandagePrefab;
			ResourceName tourniquetPrefab;
			if (!GetForeignTreatmentInventory(bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab))
				return false;
			return (commonType == 1 && canBandage) || (commonType == 4 && canTourniquet);
		}

		return FindApplicableTreatmentItem(commonType) != null;
	}

	protected void StartTreatment(int commonType, bool foreignInventory)
	{
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!user || !s_Patient)
			return;

		if (foreignInventory)
		{
			SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!damageManager || !playerController)
				return;

			ClearPendingTreatment();
			m_PendingTreatmentPatient = s_Patient;
			m_PendingTreatmentRegion = m_SelectedRegion;
			m_PendingForeignCommonType = commonType;
			playerController.RAMI_RequestForeignTreatment(damageManager, m_SelectedRegion, commonType);
			if (m_TreatmentStatus)
				m_TreatmentStatus.SetText("REQUESTING PATIENT ITEM");
			GetGame().GetCallqueue().CallLater(FinishForeignTreatmentRequest, 50, false);
			return;
		}

		IEntity item = FindApplicableTreatmentItem(commonType);
		if (!item)
			return;
		StartTreatmentWithItem(item);
	}

	protected void StartTreatmentWithItem(IEntity item)
	{
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!user || !s_Patient || !item)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(user.GetCharacterController());
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!controller || !characterStorage)
			return;

		ClearPendingTreatment();
		m_PendingTreatmentItem = item;
		m_PendingTreatmentPatient = s_Patient;
		m_PendingTreatmentRegion = m_SelectedRegion;

		if (characterStorage.GetCurrentItem() == item)
		{
			FinishPendingTreatment();
			return;
		}

		if (!characterStorage.CanUseItem(item, ESlotFunction.TYPE_GADGET))
		{
			ClearPendingTreatment();
			return;
		}

		characterStorage.UseItem(item, ESlotFunction.TYPE_GADGET, SCR_EUseContext.FROM_INVENTORY);
		if (m_TreatmentStatus)
			m_TreatmentStatus.SetText("PREPARING TREATMENT");
		GetGame().GetCallqueue().CallLater(FinishPendingTreatment, 50, false);
	}

	protected void FinishForeignTreatmentRequest()
	{
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		SCR_CharacterDamageManagerComponent damageManager;
		if (m_PendingTreatmentPatient)
			damageManager = SCR_CharacterDamageManagerComponent.Cast(m_PendingTreatmentPatient.GetDamageManager());
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		RplId itemId;
		bool succeeded;
		bool ready = playerController && playerController.RAMI_GetPreparedForeignTreatment(damageManager, m_PendingTreatmentRegion, m_PendingForeignCommonType, itemId, succeeded);
		SCR_ConsumableItemComponent consumable;
		if (ready && succeeded)
			consumable = SCR_ConsumableItemComponent.Cast(Replication.FindItem(itemId));
		IEntity item;
		if (consumable)
			item = consumable.GetOwner();

		bool itemArrived;
		if (user && item)
		{
			SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
			array<IEntity> items = {};
			if (storageManager)
				storageManager.GetItems(items);
			itemArrived = items.Contains(item);
		}

		if (ready && !succeeded)
		{
			if (m_TreatmentStatus)
				m_TreatmentStatus.SetText("PATIENT ITEM UNAVAILABLE");
			ClearPendingTreatment();
			return;
		}
		if (!itemArrived)
		{
			m_PendingForeignAttempts++;
			if (m_PendingForeignAttempts < 80)
				GetGame().GetCallqueue().CallLater(FinishForeignTreatmentRequest, 50, false);
			else
			{
				if (m_TreatmentStatus)
					m_TreatmentStatus.SetText("PATIENT ITEM TRANSFER TIMED OUT");
				ClearPendingTreatment();
			}
			return;
		}

		s_Patient = m_PendingTreatmentPatient;
		m_SelectedRegion = m_PendingTreatmentRegion;
		StartTreatmentWithItem(item);
	}

	protected void FinishPendingTreatment()
	{
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!user || !m_PendingTreatmentPatient || !m_PendingTreatmentItem)
		{
			ClearPendingTreatment();
			return;
		}

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(user.GetCharacterController());
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!controller || !characterStorage || controller.IsChangingItem() || characterStorage.GetCurrentItem() != m_PendingTreatmentItem)
		{
			m_PendingTreatmentAttempts++;
			if (m_PendingTreatmentAttempts < 40)
				GetGame().GetCallqueue().CallLater(FinishPendingTreatment, 50, false);
			else
			{
				if (m_TreatmentStatus)
					m_TreatmentStatus.SetText("ITEM COULD NOT BE EQUIPPED");
				ClearPendingTreatment();
			}
			return;
		}
		if (m_PendingTreatmentItem.FindComponent(SCR_HealSupportStationComponent))
		{
			FinishPendingMedicalKit(user);
			return;
		}

		SCR_ConsumableItemComponent consumable;
		SCR_ConsumableEffectHealthItems effect = GetTreatmentEffect(m_PendingTreatmentItem, consumable);
		int failReason;
		if (!effect || !effect.CanApplyEffectToHZ(m_PendingTreatmentPatient, user, m_PendingTreatmentRegion, failReason))
		{
			ClearPendingTreatment();
			return;
		}

		ItemUseParameters parameters = effect.GetAnimationParameters(m_PendingTreatmentItem, m_PendingTreatmentPatient, m_PendingTreatmentRegion);
		if (!parameters)
		{
			ClearPendingTreatment();
			return;
		}

		if (m_PendingTreatmentPatient != user)
		{
			SCR_CharacterControllerComponent patientController = SCR_CharacterControllerComponent.Cast(m_PendingTreatmentPatient.GetCharacterController());
			if (patientController && patientController.IsUnconscious())
				parameters.SetCommandID(effect.GetReviveAnimCmnd(user));
			else
				parameters.SetCommandID(effect.GetApplyToOtherAnimCmnd(user));

			parameters.SetMaxAnimLength(effect.GetApplyToOtherDuraction());
		}

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
		{
			ClearPendingTreatment();
			return;
		}

		playerController.RAMI_SetTreatmentTarget(consumable, m_PendingTreatmentPatient, m_PendingTreatmentRegion);
		if (effect.ActivateEffect(m_PendingTreatmentPatient, user, m_PendingTreatmentItem, parameters))
		{
			ClearPendingTreatment();
			if (m_TreatmentStatus)
				m_TreatmentStatus.SetText("TREATMENT STARTED");
			return;
		}

		if (m_TreatmentStatus)
			m_TreatmentStatus.SetText("TREATMENT COULD NOT START");
		ClearPendingTreatment();
	}

	protected void ClearPendingTreatment()
	{
		GetGame().GetCallqueue().Remove(FinishPendingTreatment);
		GetGame().GetCallqueue().Remove(FinishForeignTreatmentRequest);
		if (m_PendingTreatmentItem && m_PendingTreatmentItem.FindComponent(SCR_HealSupportStationComponent))
		{
			SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			SCR_HealSupportStationAction action = FindMedicalKitAction(m_PendingTreatmentPatient, m_PendingTreatmentRegion);
			ActionsPerformerComponent performer;
			SCR_CharacterInventoryStorageComponent characterStorage;
			if (user)
			{
				performer = ActionsPerformerComponent.Cast(user.FindComponent(ActionsPerformerComponent));
				characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
			}

			if (action && performer)
				performer.CancelAction(action);
			if (characterStorage && characterStorage.GetCurrentItem() == m_PendingTreatmentItem)
				characterStorage.UnequipCurrentItem();
		}

		m_PendingTreatmentItem = null;
		m_PendingTreatmentPatient = null;
		m_PendingTreatmentAttempts = 0;
		m_IsMedicalKitTreatmentActive = false;
		m_PendingForeignCommonType = 0;
		m_PendingForeignAttempts = 0;
	}

	void UseBandage(bool foreignInventory)
	{
		if (!IsSelectedRegionBleeding() || !CanStartTreatment(1, foreignInventory))
			return;

		StartTreatment(1, foreignInventory);
	}

	protected bool IsSelectedRegionBleeding()
	{
		if (!s_Patient)
			return false;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		if (!damageManager)
			return false;

		array<HitZone> bleedingHitZones = damageManager.GetBleedingHitZones();
		if (!bleedingHitZones)
			return false;

		foreach (HitZone hitZone : bleedingHitZones)
		{
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (
				characterHitZone &&
				characterHitZone.GetHitZoneGroup() == m_SelectedRegion &&
				characterHitZone.ACE_Medical_CalculateBleedingRate() > 0
			)
				return true;
		}

		return false;
	}

	void UseOrRemoveTourniquet(bool foreignInventory)
	{
		if (!CanTreatPatient())
			return;

		if (!IsSelectedRegionTourniquetted())
		{
			if (CanStartTreatment(4, foreignInventory))
				StartTreatment(4, foreignInventory);
			return;
		}

		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user || !s_Patient)
			return;

		SCR_TourniquetStorageComponent tourniquetStorage = SCR_TourniquetStorageComponent.Cast(s_Patient.FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorage)
		{
			m_TreatmentStatus.SetText("TOURNIQUET STORAGE UNAVAILABLE");
			return;
		}

		if (!tourniquetStorage.RemoveTourniquetFromSlot(m_SelectedRegion, user))
		{
			m_TreatmentStatus.SetText("TOURNIQUET COULD NOT BE REMOVED");
			return;
		}

		CaptureBodyZoneState(s_Patient);
		UpdatePatientVitals();
		m_TreatmentStatus.SetText("TOURNIQUET REMOVED");
	}

	protected bool IsSelectedRegionTourniquetted()
	{
		if (!s_Patient)
			return false;

		bool isExtremity =
			m_SelectedRegion == ECharacterHitZoneGroup.LEFTARM ||
			m_SelectedRegion == ECharacterHitZoneGroup.RIGHTARM ||
			m_SelectedRegion == ECharacterHitZoneGroup.LEFTLEG ||
			m_SelectedRegion == ECharacterHitZoneGroup.RIGHTLEG;
		if (!isExtremity)
			return false;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		return damageManager && damageManager.GetGroupTourniquetted(m_SelectedRegion);
	}

	protected void UpdateTreatmentButtons()
	{
		bool foreignInteraction = s_ForeignPatient && s_Patient == s_ForeignPatient;
		bool canTreat = CanTreatPatient();
		bool tourniquetApplied = IsSelectedRegionTourniquetted();
		if (m_TourniquetLabel && tourniquetApplied)
			m_TourniquetLabel.SetText("REMOVE TOURNIQUET");
		else if (m_TourniquetLabel)
			m_TourniquetLabel.SetText("Tourniquet");
		if (m_TourniquetLabelForeign && tourniquetApplied)
			m_TourniquetLabelForeign.SetText("REMOVE TOURNIQUET");
		else if (m_TourniquetLabelForeign)
			m_TourniquetLabelForeign.SetText("Tourniquet");

		bool isBleeding = IsSelectedRegionBleeding();
		bool canBandage = canTreat && isBleeding && CanStartTreatment(1, false);
		bool canBandageForeign = canTreat && foreignInteraction && isBleeding && CanStartTreatment(1, true);
		bool canTourniquet = canTreat && CanStartTreatment(4, false);
		bool canTourniquetForeign = canTreat && foreignInteraction && CanStartTreatment(4, true);
		int medicalKitCount;
		IEntity medicalKit = FindMedicalKit(medicalKitCount);
		if (m_MedicalKitRow)
			m_MedicalKitRow.SetVisible(medicalKitCount > 0);
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		SCR_CharacterInventoryStorageComponent characterStorage;
		if (user)
			characterStorage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		bool canUseMedicalKit = canTreat && medicalKit && characterStorage && FindMedicalKitAction(s_Patient, m_SelectedRegion) && (characterStorage.GetCurrentItem() == medicalKit || characterStorage.CanUseItem(medicalKit, ESlotFunction.TYPE_GADGET));
		if (canTreat && tourniquetApplied)
		{
			canTourniquet = s_Patient.FindComponent(SCR_TourniquetStorageComponent) != null;
			canTourniquetForeign = foreignInteraction && canTourniquet;
		}

		SetTreatmentButtonState(
			m_BandageButton,
			m_BandageButtonContent,
			m_BandageLabel,
			m_BandageCount,
			m_BandageIcon,
			canBandage
		);
		SetTreatmentButtonState(
			m_TourniquetButton,
			m_TourniquetButtonContent,
			m_TourniquetLabel,
			m_TourniquetCount,
			m_TourniquetIcon,
			canTourniquet
		);
		if (m_MedicalKitCount)
			m_MedicalKitCount.SetText(medicalKitCount.ToString());
		SetTreatmentPreview(m_MedicalKitIcon, medicalKit);
		SetTreatmentButtonState(
			m_MedicalKitButton,
			m_MedicalKitButtonContent,
			m_MedicalKitLabel,
			m_MedicalKitCount,
			m_MedicalKitIcon,
			canUseMedicalKit
		);
		SetTreatmentButtonState(
			m_BandageButtonForeign,
			m_BandageButtonContentForeign,
			m_BandageLabelForeign,
			m_BandageCountForeign,
			m_BandageIconForeign,
			canBandageForeign
		);
		SetTreatmentButtonState(
			m_TourniquetButtonForeign,
			m_TourniquetButtonContentForeign,
			m_TourniquetLabelForeign,
			m_TourniquetCountForeign,
			m_TourniquetIconForeign,
			canTourniquetForeign
		);
		UpdateTreatmentItemNavigation();
	}

	protected void SetTreatmentButtonState(
		ButtonWidget button,
		Widget content,
		TextWidget label,
		TextWidget count,
		Widget icon,
		bool enabled
	)
	{
		if (button)
			button.SetEnabled(enabled);
		if (content)
			content.SetEnabled(true);

		Color widgetColor = Color.FromSRGBA(128, 128, 128, 255);
		Color buttonColor = Color.FromSRGBA(128, 128, 128, 230);
		if (enabled)
		{
			widgetColor = Color.White;
			buttonColor = Color.FromSRGBA(255, 255, 255, 230);
		}

		if (button)
			button.SetColor(buttonColor);
		if (label)
			label.SetColor(GetButtonTextColor(Color.Black));
		if (count)
			count.SetColor(GetButtonTextColor(Color.Black));
		if (icon)
			icon.SetColor(widgetColor);
	}

	protected void UpdateTreatmentItems()
	{
		UpdateTreatmentItemsForInventory(
			false,
			m_BandageCount,
			m_TourniquetCount,
			m_BandageIcon,
			m_TourniquetIcon
		);
		UpdateTreatmentItemsForInventory(
			true,
			m_BandageCountForeign,
			m_TourniquetCountForeign,
			m_BandageIconForeign,
			m_TourniquetIconForeign
		);
	}

	protected void UpdateSalineMenuVisibility()
	{
		if (!m_IsMedicationPage || m_MedicationRows.Count() != m_MedicationTypes.Count() || m_MedicationRowsForeign.Count() != m_MedicationTypes.Count())
			return;

		bool foreignInteraction = s_ForeignPatient && s_Patient == s_ForeignPatient;
		if (!foreignInteraction)
			m_IsForeignSalineMenuOpen = false;
		if (m_SalineToggleRowForeign)
			m_SalineToggleRowForeign.SetVisible(foreignInteraction);
		if (m_StopSalineRow)
			m_StopSalineRow.SetVisible(m_IsSelfSalineMenuOpen);
		if (m_StopSalineRowForeign)
			m_StopSalineRowForeign.SetVisible(foreignInteraction && m_IsForeignSalineMenuOpen);
		for (int index = 0; index < m_MedicationTypes.Count(); index++)
		{
			bool saline = index >= SALINE_FIRST_INDEX;
			if (m_MedicationRows[index])
				m_MedicationRows[index].SetVisible(saline == m_IsSelfSalineMenuOpen);
			if (m_MedicationRowsForeign[index])
				m_MedicationRowsForeign[index].SetVisible(foreignInteraction && saline == m_IsForeignSalineMenuOpen);
		}

		if (m_SalineToggleLabel)
		{
			if (m_IsSelfSalineMenuOpen)
			{
				m_SalineToggleLabel.SetText("STANDARD ITEMS");
				FrameSlot.SetAnchorMin(m_SalineToggleLabel, 0, 0);
				FrameSlot.SetAnchorMax(m_SalineToggleLabel, 1, 1);
			}
			else
			{
				m_SalineToggleLabel.SetText("SALINE ITEMS");
				FrameSlot.SetAnchorMin(m_SalineToggleLabel, 0.22, 0);
				FrameSlot.SetAnchorMax(m_SalineToggleLabel, 0.78, 1);
			}
			FrameSlot.SetOffsets(m_SalineToggleLabel, 0, 0, 0, 0);
		}
		if (m_SalineToggleCount)
			m_SalineToggleCount.SetVisible(!m_IsSelfSalineMenuOpen);
		if (m_IsSelfSalineMenuOpen && m_SalineToggleIcon)
			m_SalineToggleIcon.SetVisible(false);
		if (m_SalineToggleLabelForeign)
		{
			if (m_IsForeignSalineMenuOpen)
			{
				m_SalineToggleLabelForeign.SetText("STANDARD ITEMS");
				FrameSlot.SetAnchorMin(m_SalineToggleLabelForeign, 0, 0);
				FrameSlot.SetAnchorMax(m_SalineToggleLabelForeign, 1, 1);
			}
			else
			{
				m_SalineToggleLabelForeign.SetText("SALINE ITEMS");
				FrameSlot.SetAnchorMin(m_SalineToggleLabelForeign, 0.22, 0);
				FrameSlot.SetAnchorMax(m_SalineToggleLabelForeign, 0.78, 1);
			}
			FrameSlot.SetOffsets(m_SalineToggleLabelForeign, 0, 0, 0, 0);
		}
		if (m_SalineToggleCountForeign)
			m_SalineToggleCountForeign.SetVisible(!m_IsForeignSalineMenuOpen);
		if (m_IsForeignSalineMenuOpen && m_SalineToggleIconForeign)
			m_SalineToggleIconForeign.SetVisible(false);
	}

	protected void UpdateMedicationItems()
	{
		if (!m_IsMedicationPage || m_MedicationButtons.Count() != m_MedicationTypes.Count())
			return;

		array<IEntity> items = {};
		GetTreatmentInventoryItems(items);
		bool canTreat = CanTreatPatient();
		bool foreignInteraction = s_ForeignPatient && s_Patient == s_ForeignPatient;
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		SCR_CharacterDamageManagerComponent damageManager;
		if (s_Patient)
			damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		if (foreignInteraction && playerController && damageManager)
			playerController.RAMI_RequestMedicationInventory(damageManager, m_SelectedRegion);

		int salineCount;
		IEntity salinePreviewItem;
		int salineCountForeign;
		ResourceName salinePrefabForeign;
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		for (int index = 0; index < m_MedicationTypes.Count(); index++)
		{
			int count;
			bool canApply;
			bool regionAllowed = SCR_PlayerController.RAMI_CanApplyMedicationToRegion(s_Patient, m_MedicationTypes[index], m_SelectedRegion);
			IEntity previewItem;
			foreach (IEntity item : items)
			{
				if (!SCR_PlayerController.RAMI_ItemMatchesTreatment(item, m_MedicationTypes[index]))
					continue;

				count++;
				if (!previewItem)
					previewItem = item;
				SCR_ConsumableItemComponent consumable;
				SCR_ConsumableEffectHealthItems effect = GetTreatmentEffect(item, consumable);
				int failReason;
				if (regionAllowed && effect && effect.CanApplyEffectToHZ(s_Patient, user, m_SelectedRegion, failReason))
					canApply = true;
			}

			m_MedicationCounts[index].SetText(count.ToString());
			SetTreatmentPreview(m_MedicationIcons[index], previewItem);
			SetTreatmentButtonState(m_MedicationButtons[index], m_MedicationButtonContents[index], m_MedicationLabels[index], m_MedicationCounts[index], m_MedicationIcons[index], canTreat && count > 0 && canApply);

			int foreignCount;
			bool canApplyForeign;
			if (foreignInteraction && playerController && damageManager)
				playerController.RAMI_GetMedicationInventory(damageManager, m_SelectedRegion, m_MedicationTypes[index], foreignCount, canApplyForeign);
			m_MedicationCountsForeign[index].SetText(foreignCount.ToString());
			ResourceName foreignPrefab;
			if (foreignCount > 0)
				foreignPrefab = SCR_PlayerController.RAMI_GetMedicationPrefab(m_MedicationTypes[index]);
			SetTreatmentPreviewFromPrefab(m_MedicationIconsForeign[index], foreignPrefab);
			SetTreatmentButtonState(m_MedicationButtonsForeign[index], m_MedicationButtonContentsForeign[index], m_MedicationLabelsForeign[index], m_MedicationCountsForeign[index], m_MedicationIconsForeign[index], canTreat && foreignInteraction && foreignCount > 0 && canApplyForeign);

			if (index < SALINE_FIRST_INDEX)
				continue;
			salineCount += count;
			if (!salinePreviewItem && previewItem)
				salinePreviewItem = previewItem;
			salineCountForeign += foreignCount;
			if (salinePrefabForeign.IsEmpty() && !foreignPrefab.IsEmpty())
				salinePrefabForeign = foreignPrefab;
		}

		if (m_SalineToggleCount)
			m_SalineToggleCount.SetText(salineCount.ToString());
		SetTreatmentPreview(m_SalineToggleIcon, salinePreviewItem);
		SetTreatmentButtonState(m_SalineToggleButton, null, m_SalineToggleLabel, m_SalineToggleCount, m_SalineToggleIcon, true);
		if (m_SalineToggleCountForeign)
			m_SalineToggleCountForeign.SetText(salineCountForeign.ToString());
		SetTreatmentPreviewFromPrefab(m_SalineToggleIconForeign, salinePrefabForeign);
		SetTreatmentButtonState(m_SalineToggleButtonForeign, null, m_SalineToggleLabelForeign, m_SalineToggleCountForeign, m_SalineToggleIconForeign, foreignInteraction);
		bool canStopSaline = canTreat && playerController && ESB_PartialSaline.CanStop(s_Patient);
		SetTreatmentButtonState(m_StopSalineButton, null, m_StopSalineLabel, null, null, canStopSaline);
		SetTreatmentButtonState(m_StopSalineButtonForeign, null, m_StopSalineLabelForeign, null, null, canStopSaline);

		UpdateSalineMenuVisibility();
		UpdateTreatmentItemNavigation();
		UpdateRequestedButtonStyles();
	}

	bool UseMedicationWidget(string widgetName)
	{
		if (widgetName.StartsWith("MedicationStopSaline"))
		{
			StopSaline();
			return true;
		}

		array<string> selfSuffixes = {"Button", "Content", "Icon", "Label", "Count"};
		array<string> foreignSuffixes = {"ButtonForeign", "ContentForeign", "IconForeign", "LabelForeign", "CountForeign"};
		array<string> toggleSuffixes = {"Button", "Icon", "Label", "Count"};
		foreach (string suffix : toggleSuffixes)
		{
			if (widgetName == "MedicationSalineToggle" + suffix)
			{
				if (!m_SalineToggleButton || !m_SalineToggleButton.IsEnabled())
					return true;
				m_IsSelfSalineMenuOpen = !m_IsSelfSalineMenuOpen;
				UpdateMedicationItems();
				return true;
			}
		}
		foreach (string suffix : toggleSuffixes)
		{
			if (widgetName == "MedicationSalineToggle" + suffix + "Foreign")
			{
				if (!m_SalineToggleButtonForeign || !m_SalineToggleButtonForeign.IsEnabled())
					return true;
				m_IsForeignSalineMenuOpen = !m_IsForeignSalineMenuOpen;
				UpdateMedicationItems();
				return true;
			}
		}

		for (int index = 0; index < m_MedicationWidgetNames.Count(); index++)
		{
			foreach (string suffix : selfSuffixes)
			{
				if (widgetName == m_MedicationWidgetNames[index] + suffix)
				{
					if (m_IsMedicationPage && FindApplicableTreatmentItem(m_MedicationTypes[index]))
						StartTreatment(m_MedicationTypes[index], false);
					return true;
				}
			}

			foreach (string suffix : foreignSuffixes)
			{
				if (widgetName != m_MedicationWidgetNames[index] + suffix)
					continue;

				int count;
				bool canApply;
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
				SCR_CharacterDamageManagerComponent damageManager;
				if (s_Patient)
					damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
				if (m_IsMedicationPage && playerController && playerController.RAMI_GetMedicationInventory(damageManager, m_SelectedRegion, m_MedicationTypes[index], count, canApply) && count > 0 && canApply)
					StartTreatment(m_MedicationTypes[index], true);
				return true;
			}
		}

		return false;
	}

	protected void StopSaline()
	{
		ButtonWidget button = m_StopSalineButton;
		if (m_IsForeignSalineMenuOpen)
			button = m_StopSalineButtonForeign;
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		SCR_CharacterDamageManagerComponent damageManager;
		if (s_Patient)
			damageManager = SCR_CharacterDamageManagerComponent.Cast(s_Patient.GetDamageManager());
		if (!button || !button.IsEnabled() || !playerController || !damageManager)
			return;

		if (playerController.RAMI_RequestStopSaline(damageManager) && m_TreatmentStatus)
			m_TreatmentStatus.SetText("SALINE STOPPED");
		UpdateMedicationItems();
	}

	protected void UpdateTreatmentItemsForInventory(
		bool foreignInventory,
		TextWidget bandageCountWidget,
		TextWidget tourniquetCountWidget,
		ItemPreviewWidget bandagePreview,
		ItemPreviewWidget tourniquetPreview
	)
	{
		if (!bandageCountWidget || !tourniquetCountWidget)
			return;

		int bandageCount;
		int tourniquetCount;
		if (foreignInventory)
		{
			bool canBandage;
			bool canTourniquet;
			ResourceName bandagePrefab;
			ResourceName tourniquetPrefab;
			GetForeignTreatmentInventory(bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab);
			bandageCountWidget.SetText(bandageCount.ToString());
			tourniquetCountWidget.SetText(tourniquetCount.ToString());
			SetTreatmentPreviewFromPrefab(bandagePreview, bandagePrefab);
			SetTreatmentPreviewFromPrefab(tourniquetPreview, tourniquetPrefab);
			return;
		}

		IEntity bandageItem;
		IEntity tourniquetItem;
		SCR_ChimeraCharacter inventoryOwner = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (inventoryOwner)
		{
			array<IEntity> items = {};
			GetTreatmentInventoryItems(items);
			foreach (IEntity item : items)
			{
				InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
				if (!itemComponent || !itemComponent.GetAttributes())
					continue;

				int commonType = itemComponent.GetAttributes().GetCommonType();
				if (commonType == 1)
				{
					bandageCount++;
					bandageItem = item;
				}
				else if (commonType == 4)
				{
					tourniquetCount++;
					tourniquetItem = item;
				}
			}
		}

		bandageCountWidget.SetText(bandageCount.ToString());
		tourniquetCountWidget.SetText(tourniquetCount.ToString());
		SetTreatmentPreview(bandagePreview, bandageItem);
		SetTreatmentPreview(tourniquetPreview, tourniquetItem);
	}

	protected void SetTreatmentPreview(ItemPreviewWidget preview, IEntity item)
	{
		if (!preview)
			return;
		if (!item)
		{
			preview.SetVisible(false);
			return;
		}

		ChimeraWorld world = GetGame().GetWorld();
		if (world && world.GetItemPreviewManager())
		{
			world.GetItemPreviewManager().SetPreviewItem(preview, item);
			preview.SetVisible(true);
		}
	}

	protected void SetTreatmentPreviewFromPrefab(ItemPreviewWidget preview, ResourceName prefab)
	{
		if (!preview)
			return;
		if (prefab.IsEmpty())
		{
			preview.SetVisible(false);
			return;
		}

		ChimeraWorld world = GetGame().GetWorld();
		if (world && world.GetItemPreviewManager())
		{
			world.GetItemPreviewManager().SetPreviewItemFromPrefab(preview, prefab);
			preview.SetVisible(true);
		}
	}

	void SelectBodyRegion(string widgetName)
	{
		switch (widgetName)
		{
			case "BodyHead":
				m_SelectedRegion = ECharacterHitZoneGroup.HEAD;
				break;
			case "BodyChest":
				m_SelectedRegion = ECharacterHitZoneGroup.UPPERTORSO;
				break;
			case "BodyAbdomen":
				m_SelectedRegion = ECharacterHitZoneGroup.LOWERTORSO;
				break;
			case "BodyLeftArm":
				m_SelectedRegion = ECharacterHitZoneGroup.LEFTARM;
				break;
			case "BodyRightArm":
				m_SelectedRegion = ECharacterHitZoneGroup.RIGHTARM;
				break;
			case "BodyLeftLeg":
				m_SelectedRegion = ECharacterHitZoneGroup.LEFTLEG;
				break;
			case "BodyRightLeg":
				m_SelectedRegion = ECharacterHitZoneGroup.RIGHTLEG;
				break;
		}

		UpdateSelectedBodyRegionHighlight();
		UpdateSelectedRegionText();
		if (m_IsMedicationPage)
			UpdateMedicationItems();
		else
			UpdateTreatmentButtons();
		if (m_TreatmentStatus)
		{
			if (m_IsMedicationPage)
				m_TreatmentStatus.SetText("SELECT AN AVAILABLE MEDICATION");
			else
				m_TreatmentStatus.SetText("SELECT AN AVAILABLE TREATMENT");
		}
	}

	protected void UpdateSelectedBodyRegionHighlight()
	{
		if (!m_BodyZoneOutlineMask)
			return;

		ResourceName texture;
		ImageWidget selectedZone;
		switch (m_SelectedRegion)
		{
			case ECharacterHitZoneGroup.HEAD:
				texture = "{19AAA1C2540968EC}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_Head.edds";
				selectedZone = m_BodyZoneHead;
				break;
			case ECharacterHitZoneGroup.UPPERTORSO:
				texture = "{CC8B90D891CEA314}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_Chest.edds";
				selectedZone = m_BodyZoneChest;
				break;
			case ECharacterHitZoneGroup.LOWERTORSO:
				texture = "{AFC04D886C2BC5C7}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_Abdomen.edds";
				selectedZone = m_BodyZoneAbdomen;
				break;
			case ECharacterHitZoneGroup.LEFTARM:
				texture = "{2A08094A88E952F8}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_LeftArm.edds";
				selectedZone = m_BodyZoneLeftArm;
				break;
			case ECharacterHitZoneGroup.RIGHTARM:
				texture = "{04B0CC6DB75D225E}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_RightArm.edds";
				selectedZone = m_BodyZoneRightArm;
				break;
			case ECharacterHitZoneGroup.LEFTLEG:
				texture = "{FB637A1D66C7AC24}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_LeftLeg.edds";
				selectedZone = m_BodyZoneLeftLeg;
				break;
			case ECharacterHitZoneGroup.RIGHTLEG:
				texture = "{D5DBBF3A5973DC82}UI/Textures/RAMI_BodyZones/RAMI_BodyZone_RightLeg.edds";
				selectedZone = m_BodyZoneRightLeg;
				break;
			default: return;
		}

		if (!selectedZone || !m_BodyZoneOutlineMask.LoadImageTexture(0, texture))
		{
			m_BodyZoneOutlineMask.SetOpacity(0);
			foreach (ImageWidget outline : m_BodyZoneOutlines)
			{
				if (outline)
					outline.SetOpacity(0);
			}
			return;
		}

		foreach (ImageWidget outline : m_BodyZoneOutlines)
		{
			if (!outline)
				continue;
			if (outline.LoadImageTexture(0, texture))
				outline.SetOpacity(0.9);
			else
				outline.SetOpacity(0);
		}

		m_BodyZoneOutlineMask.SetColor(selectedZone.GetColor());
		m_BodyZoneOutlineMask.SetOpacity(selectedZone.GetOpacity());
	}

	protected BaseUserAction FindAdvancedAction(string widgetName)
	{
		if (!s_Patient)
			return null;

		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(s_Patient.FindComponent(ActionsManagerComponent));
		if (!actionsManager)
			return null;

		array<BaseUserAction> actions = {};
		actionsManager.GetActionsList(actions);
		foreach (BaseUserAction action : actions)
		{
			if (widgetName == "AdvancedCPRButton" && ACE_Medical_CPRUserAction.Cast(action))
				return action;
			if (widgetName == "AdvancedCarryButton" && ACE_Carrying_CarryUserAction.Cast(action))
				return action;
			if (widgetName == "AdvancedDragButton" && ACE_Carrying_DragUserAction.Cast(action))
				return action;
			if (widgetName == "AdvancedLoadVehicleButton" && SCR_LoadCasualtySupportStationUserAction.Cast(action))
				return action;

			if (!ACE_Medical_RepositionUserAction.Cast(action) || !action.GetUIInfo())
				continue;

			string actionName = action.GetUIInfo().GetName();
			if (widgetName == "AdvancedBackButton" && actionName == "#ACE_Medical-UserAction_BackPosition")
				return action;
			if (widgetName == "AdvancedRightSideButton" && actionName == "#ACE_Medical-UserAction_RightRecoveryPosition")
				return action;
			if (widgetName == "AdvancedLeftSideButton" && actionName == "#ACE_Medical-UserAction_LeftRecoveryPosition")
				return action;
		}

		return null;
	}

	protected bool CanUseAdvancedAction(string widgetName)
	{
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user)
			return false;
		if (widgetName == "AdvancedCPRButton" && ACE_Medical_CPRHelperCompartment.Cast(ACE_AnimationTools.GetHelperCompartment(user)))
			return true;

		BaseUserAction action = FindAdvancedAction(widgetName);
		if (!action)
			return false;

		return action.CanBeShown(user) && action.CanBePerformed(user);
	}

	protected void UpdateAdvancedButtons()
	{
		ACE_Medical_VitalsComponent vitals;
		if (s_Patient)
			vitals = ACE_Medical_VitalsComponent.Cast(s_Patient.FindComponent(ACE_Medical_VitalsComponent));
		bool isCPRPerformed = vitals && vitals.IsCPRPerformed();
		if (m_AdvancedCPRText)
		{
			if (isCPRPerformed)
				m_AdvancedCPRText.SetText("STOP CPR");
			else
				m_AdvancedCPRText.SetText("START CPR");
		}

		SetAdvancedButtonState(m_AdvancedCPRButton, CanUseAdvancedAction("AdvancedCPRButton"));
		SetAdvancedButtonState(m_AdvancedCarryButton, CanUseAdvancedAction("AdvancedCarryButton"));
		SetAdvancedButtonState(m_AdvancedDragButton, CanUseAdvancedAction("AdvancedDragButton"));
		SetAdvancedButtonState(m_AdvancedBackButton, CanUseAdvancedAction("AdvancedBackButton"));
		SetAdvancedButtonState(m_AdvancedRightSideButton, CanUseAdvancedAction("AdvancedRightSideButton"));
		SetAdvancedButtonState(m_AdvancedLeftSideButton, CanUseAdvancedAction("AdvancedLeftSideButton"));
		SetAdvancedButtonState(m_AdvancedLoadVehicleButton, CanUseAdvancedAction("AdvancedLoadVehicleButton"));

		string closeUpTarget = "PageAdvanced";
		array<ButtonWidget> actionButtons = {
			m_AdvancedCPRButton,
			m_AdvancedBackButton,
			m_AdvancedRightSideButton,
			m_AdvancedLeftSideButton,
			m_AdvancedCarryButton,
			m_AdvancedDragButton,
			m_AdvancedLoadVehicleButton
		};
		foreach (ButtonWidget actionButton : actionButtons)
		{
			if (actionButton && actionButton.IsEnabled())
				closeUpTarget = actionButton.GetName();
		}
		SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, closeUpTarget);
		UpdateRequestedButtonStyles();
	}

	protected void SetAdvancedButtonState(ButtonWidget button, bool enabled)
	{
		if (!button)
			return;

		button.SetEnabled(enabled);
		Color buttonColor = Color.FromSRGBA(128, 128, 128, 230);
		if (enabled)
			buttonColor = Color.FromSRGBA(255, 255, 255, 230);
		button.SetColor(buttonColor);
	}

	void UseAdvancedAction(string widgetName)
	{
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user)
			return;
		if (widgetName == "AdvancedCPRButton")
		{
			ACE_Medical_CPRHelperCompartment cprHelper = ACE_Medical_CPRHelperCompartment.Cast(ACE_AnimationTools.GetHelperCompartment(user));
			if (cprHelper)
			{
				cprHelper.Terminate();
				return;
			}
		}

		BaseUserAction action = FindAdvancedAction(widgetName);
		if (!action)
			return;
		ActionsPerformerComponent performer = ActionsPerformerComponent.Cast(user.FindComponent(ActionsPerformerComponent));
		if (!performer)
			return;
		if (!action.CanBeShown(user) || !action.CanBePerformed(user))
			return;

		performer.PerformAction(action);
	}

	protected void UpdateSelectedRegionText()
	{
		if (!m_SelectedRegionText)
			return;

		switch (m_SelectedRegion)
		{
			case ECharacterHitZoneGroup.HEAD: m_SelectedRegionText.SetText("HEAD"); break;
			case ECharacterHitZoneGroup.UPPERTORSO: m_SelectedRegionText.SetText("CHEST"); break;
			case ECharacterHitZoneGroup.LOWERTORSO: m_SelectedRegionText.SetText("ABDOMEN"); break;
			case ECharacterHitZoneGroup.LEFTARM: m_SelectedRegionText.SetText("LEFT ARM"); break;
			case ECharacterHitZoneGroup.RIGHTARM: m_SelectedRegionText.SetText("RIGHT ARM"); break;
			case ECharacterHitZoneGroup.LEFTLEG: m_SelectedRegionText.SetText("LEFT LEG"); break;
			case ECharacterHitZoneGroup.RIGHTLEG: m_SelectedRegionText.SetText("RIGHT LEG"); break;
		}
	}

	protected void UpdatePatientVitals()
	{
		UpdatePatientList();
		UpdateTriagePage();
		UpdateAdvancedButtons();
		if (m_DiagnosePatientName)
			m_DiagnosePatientName.SetText(string.Format("VITALS FROM: %1", GetPatientName(s_Patient)));

		bool foreignInteraction = s_ForeignPatient && s_Patient == s_ForeignPatient;
		if (m_ForeignInventoryTitle)
		{
			m_ForeignInventoryTitle.SetVisible(foreignInteraction);
			if (foreignInteraction)
				m_ForeignInventoryTitle.SetText(string.Format("Inventory from %1", GetPatientName(s_Patient)));
		}
		bool showTreatmentForeignInventory = foreignInteraction && !m_IsMedicationPage;
		if (m_BandageRowForeign)
			m_BandageRowForeign.SetVisible(showTreatmentForeignInventory);
		if (m_TourniquetRowForeign)
			m_TourniquetRowForeign.SetVisible(showTreatmentForeignInventory);
		foreach (Widget medicationRowForeign : m_MedicationRowsForeign)
		{
			if (medicationRowForeign)
				medicationRowForeign.SetVisible(foreignInteraction);
		}

		if (m_IsMedicationPage)
			UpdateMedicationItems();
		else
		{
			UpdateTreatmentItems();
			UpdateTreatmentButtons();
		}

		if (!m_PatientVitals || !m_PatientVitalValues || !m_PatientMedications || !m_PatientMedicationValues)
			return;

		m_PatientVitalValues.SetText(string.Empty);
		m_PatientMedicationValues.SetText(string.Empty);

		SCR_ChimeraCharacter patient = s_Patient;
		if (!patient)
		{
			ResetBodyZoneVisuals();
			m_PatientVitals.SetText("NO ACE PATIENT");
			m_PatientMedications.SetText("NO MEDICATION DATA");
			return;
		}

		RAMI_BodyZoneState bodyZoneState = s_BodyZoneStates.Get(patient);
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(patient.GetDamageManager());
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(patient.GetCharacterController());
		if (!damageManager || !controller)
		{
			if (bodyZoneState)
				UpdateBodyZoneVisuals(bodyZoneState);
			else
				ResetBodyZoneVisuals();

			m_PatientVitals.SetText("ACE MEDICAL DATA UNAVAILABLE");
			m_PatientMedications.SetText("MEDICATION DATA UNAVAILABLE");
			return;
		}

		SCR_CharacterBloodHitZone blood = damageManager.GetBloodHitZone();
		SCR_CharacterResilienceHitZone resilience = damageManager.GetResilienceHitZone();
		if (!blood || !resilience)
		{
			if (bodyZoneState)
				UpdateBodyZoneVisuals(bodyZoneState);
			else
				ResetBodyZoneVisuals();

			m_PatientVitals.SetText("ACE BLOOD OR RESILIENCE DATA UNAVAILABLE");
			m_PatientMedications.SetText("MEDICATION DATA UNAVAILABLE");
			return;
		}

		int healthPercent = Math.Round(damageManager.ACE_Medical_GetHealthScaled() * 100);
		float bloodScaled = blood.GetHealthScaled();
		int bloodPercent = Math.Round(bloodScaled * 100);
		int bloodVolumeMl = Math.Round(bloodScaled * REFERENCE_BLOOD_VOLUME_ML);
		int bloodLossMl = Math.Round(REFERENCE_BLOOD_VOLUME_ML - bloodVolumeMl);
		int resiliencePercent = Math.Round(resilience.GetHealthScaled() * 100);
		int painPercent = Math.Round(damageManager.ACE_Medical_GetPainIntensity() * 100);
		if (patient == s_ForeignPatient)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
			{
				playerController.RAMI_RequestPain(damageManager);
				playerController.RAMI_GetPain(damageManager, painPercent);
			}
		}
		ECharacterLifeState lifeState = controller.GetLifeState();
		CaptureBodyZoneState(patient);

		bodyZoneState = s_BodyZoneStates.Get(patient);
		if (bodyZoneState)
			UpdateBodyZoneVisuals(bodyZoneState);
		else
			ResetBodyZoneVisuals();

		if (lifeState == ECharacterLifeState.DEAD)
			painPercent = 0;

		float bleedingRateMl = 0;
		if (blood.GetMaxHealth() > 0)
			bleedingRateMl = blood.GetTotalBleedingAmount() / blood.GetMaxHealth() * REFERENCE_BLOOD_VOLUME_ML;

		bleedingRateMl = Math.Round(bleedingRateMl * 10) / 10;
		if (lifeState == ECharacterLifeState.DEAD)
			bleedingRateMl = 0;

		string patientName = GetPatientName(patient);
		bool hasMedicalKit = HasMedicalKit();
		string medicationLabels;
		string medicationValues;
		int salineVolumeRemainingMl;
		int salineTimeRemainingSeconds;
		bool salineApplied;
		bool medicationSummaryAvailable;
		if (m_PatientMedications)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
			{
				playerController.RAMI_RequestMedicationSummary(damageManager);
				medicationSummaryAvailable = playerController.RAMI_GetMedicationSummary(damageManager, medicationLabels, medicationValues, salineVolumeRemainingMl, salineTimeRemainingSeconds);
				salineApplied = salineTimeRemainingSeconds > 0;
			}
		}
		if (m_BodySalineBag)
			m_BodySalineBag.SetVisible(salineApplied);

		string tourniquetLabel;
		string tourniquetValue;
		if (bodyZoneState && bodyZoneState.HasTourniquet(m_SelectedRegion))
		{
			tourniquetLabel = "\nTOURNIQUET TIME";
			if (hasMedicalKit)
			{
				int elapsedSeconds = bodyZoneState.GetTourniquetElapsedSeconds(m_SelectedRegion);
				string seconds = (elapsedSeconds % 60).ToString();
				if (seconds.Length() < 2)
					seconds = "0" + seconds;
				tourniquetValue = string.Format("\n%1:%2", elapsedSeconds / 60, seconds);
			}
			else
				tourniquetValue = "\nAPPLIED";
		}

		string medicationListLabels;
		string medicationListValues;
		if (!medicationSummaryAvailable)
		{
			medicationListLabels = "LOADING...";
		}
		else
		{
			if (salineApplied)
			{
				medicationListLabels = "SALINE BAG\nVOLUME REMAINING\nTIME REMAINING";
				medicationListValues = string.Format("\n%1 ml\n%2", salineVolumeRemainingMl, FormatRemainingTime(salineTimeRemainingSeconds));
			}

			if (!medicationLabels.IsEmpty() && (medicationLabels != "NONE" || !salineApplied))
			{
				if (!medicationListLabels.IsEmpty())
				{
					medicationListLabels += "\n\n";
					medicationListValues += "\n\n";
				}
				medicationListLabels += medicationLabels;
				medicationListValues += "\n" + medicationValues;
			}

			if (medicationListLabels.IsEmpty())
				medicationListLabels = "NONE";
		}
		m_PatientMedications.SetText(medicationListLabels);
		m_PatientMedicationValues.SetText(medicationListValues);

		string selectedZoneLabelBlock;
		string selectedZoneValueBlock;
		if (!m_IsMedicationPage && bodyZoneState)
		{
			float injurySeverity = bodyZoneState.GetInjurySeverity(m_SelectedRegion);
			if (injurySeverity > 0)
			{
				selectedZoneLabelBlock = "\n\nINJURY";
				if (hasMedicalKit)
					selectedZoneValueBlock = string.Format("\n\n%1 %%", Math.Round(injurySeverity * 100));
				else
					selectedZoneValueBlock = "\n\n" + GetCoarseLevel(injurySeverity * 100, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD);
			}

			bool selectedArm = m_SelectedRegion == ECharacterHitZoneGroup.LEFTARM || m_SelectedRegion == ECharacterHitZoneGroup.RIGHTARM;
			bool selectedLeg = m_SelectedRegion == ECharacterHitZoneGroup.LEFTLEG || m_SelectedRegion == ECharacterHitZoneGroup.RIGHTLEG;
			if ((selectedArm || selectedLeg) && bodyZoneState.HasFracture(m_SelectedRegion))
			{
				if (selectedArm)
					selectedZoneLabelBlock += "\nAIMING";
				else
					selectedZoneLabelBlock += "\nMOVEMENT";
				selectedZoneValueBlock += "\nIMPAIRED";
			}
		}

		m_PatientVitals.SetText(string.Format(
			"Vitals from: %1\n\nHEALTH\nRESILIENCE\nBLOOD\nHEMORRHAGE\nBLOOD LOSS\nBLEEDING\nPAIN\nHEALTH STATE",
			patientName
		) + selectedZoneLabelBlock + tourniquetLabel);
		if (hasMedicalKit)
		{
			m_PatientVitalValues.SetText(string.Format(
				"\n\n%1 %%\n%2 %%\n%3 %% (%4 ml)\n%5\n%6 ml\n%7 ml/s\n%8 %%\n%9",
				healthPercent,
				resiliencePercent,
				bloodPercent,
				bloodVolumeMl,
				HemorrhageClassLabel(bloodScaled),
				bloodLossMl,
				bleedingRateMl,
				painPercent,
				LifeStateLabel(lifeState)
			) + selectedZoneValueBlock + tourniquetValue);
			return;
		}

		string bleedingEstimate = "NONE";
		if (bleedingRateMl > 0)
			bleedingEstimate = GetCoarseLevel(bleedingRateMl, SEVERE_BLEEDING_ML_PER_SECOND * 0.5, SEVERE_BLEEDING_ML_PER_SECOND);
		string painEstimate = "NONE";
		if (painPercent > 0)
			painEstimate = GetCoarseLevel(painPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD);

		m_PatientVitalValues.SetText(string.Format(
			"\n\n%1\n%2\n%3\n%4\n%5\n%6\n%7\n%8",
			GetCoarseLevel(healthPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD),
			GetCoarseLevel(resiliencePercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD),
			GetCoarseLevel(bloodPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD),
			HemorrhageClassLabel(bloodScaled),
			GetCoarseLevel(100 - bloodPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD),
			bleedingEstimate,
			painEstimate,
			LifeStateLabel(lifeState)
		) + selectedZoneValueBlock + tourniquetValue);
	}

	protected string FormatRemainingTime(int timeRemainingSeconds)
	{
		string seconds = (timeRemainingSeconds % 60).ToString();
		if (seconds.Length() < 2)
			seconds = "0" + seconds;
		return string.Format("%1:%2", timeRemainingSeconds / 60, seconds);
	}

	protected string GetPatientName(SCR_ChimeraCharacter patient)
	{
		if (!patient)
			return string.Empty;

		SCR_CharacterIdentityComponent identityComponent = SCR_CharacterIdentityComponent.Cast(patient.FindComponent(SCR_CharacterIdentityComponent));
		SCR_ExtendedCharacterIdentityComponent extendedIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(patient.FindComponent(SCR_ExtendedCharacterIdentityComponent));
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(patient);
		if (playerId == 0 && extendedIdentity)
			playerId = extendedIdentity.GetPlayerID();

		string patientName;
		if (playerId > 0)
			patientName = playerManager.GetPlayerName(playerId);
		if (patientName.IsEmpty() && identityComponent && identityComponent.GetIdentity())
			patientName = identityComponent.GetIdentity().GetFullName();
		if (patientName.IsEmpty())
			patientName = patient.GetName();

		return patientName;
	}

	protected string HemorrhageClassLabel(float bloodScaled)
	{
		if (bloodScaled <= 0)
			return "FATAL";
		if (bloodScaled <= 0.2)
			return "CLASS IV";
		if (bloodScaled <= 0.4)
			return "CLASS III";
		if (bloodScaled <= 0.7)
			return "CLASS II";
		if (bloodScaled < 1)
			return "CLASS I";

		return "NORMAL";
	}

	protected void UpdateBodyZoneVisuals(RAMI_BodyZoneState state)
	{
		UpdateBodyZone(ECharacterHitZoneGroup.HEAD, m_BodyZoneHead, m_BodyInjuryHead, null, null, state);
		UpdateBodyZone(ECharacterHitZoneGroup.UPPERTORSO, m_BodyZoneChest, m_BodyInjuryChest, null, null, state);
		UpdateBodyZone(ECharacterHitZoneGroup.LOWERTORSO, m_BodyZoneAbdomen, m_BodyInjuryAbdomen, null, null, state);
		UpdateBodyZone(ECharacterHitZoneGroup.LEFTARM, m_BodyZoneLeftArm, m_BodyInjuryLeftArm, m_BodyBoneLeftArm, m_BodyTourniquetLeftArm, state);
		UpdateBodyZone(ECharacterHitZoneGroup.RIGHTARM, m_BodyZoneRightArm, m_BodyInjuryRightArm, m_BodyBoneRightArm, m_BodyTourniquetRightArm, state);
		UpdateBodyZone(ECharacterHitZoneGroup.LEFTLEG, m_BodyZoneLeftLeg, m_BodyInjuryLeftLeg, m_BodyBoneLeftLeg, m_BodyTourniquetLeftLeg, state);
		UpdateBodyZone(ECharacterHitZoneGroup.RIGHTLEG, m_BodyZoneRightLeg, m_BodyInjuryRightLeg, m_BodyBoneRightLeg, m_BodyTourniquetRightLeg, state);
	}

	protected void UpdateBodyZonePulse(ECharacterHitZoneGroup group, ImageWidget zone, RAMI_BodyZoneState state, float opacity)
	{
		if (!zone)
			return;

		if (!state || state.GetBleedingRate(group) <= 0)
		{
			zone.SetOpacity(1);
			if (group == m_SelectedRegion && m_BodyZoneOutlineMask)
				m_BodyZoneOutlineMask.SetOpacity(1);
			return;
		}

		zone.SetOpacity(opacity);
		if (group == m_SelectedRegion && m_BodyZoneOutlineMask)
			m_BodyZoneOutlineMask.SetOpacity(opacity);
	}

	protected void UpdateBodyZone(ECharacterHitZoneGroup group, ImageWidget zone, ImageWidget injury, ImageWidget bone, ImageWidget tourniquet, RAMI_BodyZoneState state)
	{
		if (zone)
		{
			float bleedingRate = state.GetBleedingRate(group);
			Color zoneColor = Color.White;
			if (bleedingRate >= SEVERE_BLEEDING_ML_PER_SECOND)
			{
				if (m_ColorblindMode)
					zoneColor = Color.FromSRGBA(213, 94, 0, 255);
				else
					zoneColor = Color.FromSRGBA(255, 128, 0, 255);
			}
			else if (bleedingRate > 0)
			{
				if (m_ColorblindMode)
					zoneColor = Color.FromSRGBA(86, 180, 233, 255);
				else
					zoneColor = Color.FromSRGBA(255, 255, 0, 255);
			}

			zone.SetColor(zoneColor);
			if (group == m_SelectedRegion && m_BodyZoneOutlineMask)
				m_BodyZoneOutlineMask.SetColor(zoneColor);
		}

		if (injury)
		{
			float severity = state.GetInjurySeverity(group);
			float healthScaled = 1 - severity;
			int woundLevel = 1;
			if (healthScaled < WOUND_4_HEALTH_THRESHOLD)
				woundLevel = 4;
			else if (healthScaled < WOUND_3_HEALTH_THRESHOLD)
				woundLevel = 3;
			else if (healthScaled < WOUND_2_HEALTH_THRESHOLD)
				woundLevel = 2;
			if (severity > 0)
				injury.LoadImageFromSet(0, MEDICAL_ICON_IMAGE_SET, string.Format("Wound_%1_UI", woundLevel));
			if (m_ColorblindMode)
			{
				int red = Math.Round(86 + severity * 127);
				int green = Math.Round(180 - severity * 86);
				int blue = Math.Round(233 - severity * 233);
				injury.SetColor(Color.FromSRGBA(red, green, blue, 255));
			}
			else
				injury.SetColor(Color.FromSRGBA(255, Math.Round((1 - severity) * 255), 0, 255));
			injury.SetVisible(severity > 0);
		}

		if (bone)
		{
			if (m_ColorblindMode)
				bone.SetColor(Color.FromSRGBA(204, 121, 167, 255));
			else
				bone.SetColor(Color.Red);
			bone.SetVisible(state.HasFracture(group));
		}

		if (tourniquet)
			tourniquet.SetVisible(state.HasTourniquet(group));
	}

	protected void ResetBodyZoneVisuals()
	{
		array<ImageWidget> zones = {
			m_BodyZoneHead,
			m_BodyZoneChest,
			m_BodyZoneAbdomen,
			m_BodyZoneLeftArm,
			m_BodyZoneRightArm,
			m_BodyZoneLeftLeg,
			m_BodyZoneRightLeg
		};
		foreach (ImageWidget zone : zones)
		{
			if (zone)
			{
				zone.SetColor(Color.White);
				zone.SetOpacity(1);
			}
		}
		if (m_BodyZoneOutlineMask)
		{
			m_BodyZoneOutlineMask.SetColor(Color.White);
			m_BodyZoneOutlineMask.SetOpacity(1);
		}

		array<ImageWidget> injuries = {
			m_BodyInjuryHead,
			m_BodyInjuryChest,
			m_BodyInjuryAbdomen,
			m_BodyInjuryLeftArm,
			m_BodyInjuryRightArm,
			m_BodyInjuryLeftLeg,
			m_BodyInjuryRightLeg
		};
		foreach (ImageWidget injury : injuries)
		{
			if (injury)
				injury.SetVisible(false);
		}

		if (m_BodySalineBag)
			m_BodySalineBag.SetVisible(false);

		array<ImageWidget> bones = {
			m_BodyBoneLeftArm,
			m_BodyBoneRightArm,
			m_BodyBoneLeftLeg,
			m_BodyBoneRightLeg
		};
		foreach (ImageWidget bone : bones)
		{
			if (bone)
				bone.SetVisible(false);
		}

		array<ImageWidget> tourniquets = {
			m_BodyTourniquetLeftArm,
			m_BodyTourniquetRightArm,
			m_BodyTourniquetLeftLeg,
			m_BodyTourniquetRightLeg
		};
		foreach (ImageWidget tourniquet : tourniquets)
		{
			if (tourniquet)
				tourniquet.SetVisible(false);
		}
	}

	protected string LifeStateLabel(ECharacterLifeState lifeState)
	{
		switch (lifeState)
		{
			case ECharacterLifeState.ALIVE: return "ALIVE";
			case ECharacterLifeState.INCAPACITATED: return "INCAPACITATED";
			case ECharacterLifeState.DEAD: return "DECEASED";
		}

		return "UNKNOWN";
	}
}
