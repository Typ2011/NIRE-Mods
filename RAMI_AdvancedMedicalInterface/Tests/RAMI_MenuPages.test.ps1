$ErrorActionPreference = 'Stop'

$root = Split-Path $PSScriptRoot -Parent
$layout = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalGlobal.layout') -Raw
$treatment = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalTreatment.layout') -Raw
$treatmentControls = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalTreatmentControls.layout') -Raw
$medication = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalMedication.layout') -Raw
$advanced = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalAdvanced.layout') -Raw
$diagnose = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalDiagnose.layout') -Raw
$triage = Get-Content (Join-Path $root 'UI/layouts/RAMI_MedicalTriage.layout') -Raw
$controllerFocus = Get-Content (Join-Path $root 'UI/layouts/RAMI_ControllerFocus.layout') -Raw
$patientRow = Get-Content (Join-Path $root 'UI/layouts/RAMI_PatientRow.layout') -Raw
$interfaceSettings = Get-Content (Join-Path $root 'UI/layouts/RAMI_InterfaceSettings.layout') -Raw
$script = (Get-ChildItem (Join-Path $root 'Scripts/Game/RAMI') -Filter '*.c' | Get-Content -Raw) -join "`n"
$project = Get-Content (Join-Path $root 'RAMI_AdvancedMedicalInterface.gproj') -Raw
$inputConfig = Get-Content (Join-Path $root 'Configs/System/chimeraInputCommon.conf') -Raw
$keyBindingMenu = Get-Content (Join-Path $root 'Configs/System/keyBindingMenu.conf') -Raw
$menuContext = Get-Content (Join-Path $root 'Configs/System/ActionContext/MenuContext.conf') -Raw
$menuConfig = Get-Content (Join-Path $root 'Configs/System/chimeraMenus.conf') -Raw
$workbenchLauncherPath = Join-Path $root 'Tools/Launch-RAMI-Workbench.ps1'
$workbenchLauncher = Get-Content $workbenchLauncherPath -Raw
$pages = 'PageTriage', 'PageDiagnose', 'PageBandages', 'PageMedication', 'PageAdvanced', 'PageToggleSelf'

$launcherTokens = $null
$launcherErrors = $null
[System.Management.Automation.Language.Parser]::ParseFile($workbenchLauncherPath, [ref]$launcherTokens, [ref]$launcherErrors) | Out-Null
if ($launcherErrors)
{
	throw "RAMI Workbench launcher has syntax errors: $launcherErrors"
}
foreach ($binding in '[switch]$ValidateOnly', 'ACE-Anvil-dev\addons', "@('core', 'medical_core', 'medical_circulation', 'medical_hitzones', 'carrying')", "'Expanded Saline Bags'", '-addonsDir')
{
	if (-not $workbenchLauncher.Contains($binding))
	{
		throw "RAMI Workbench launcher is missing: $binding"
	}
}

foreach ($page in $pages) {
	if (-not $layout.Contains("Name `"$page`"") -or -not $script.Contains("`"$page`"")) {
		throw "RAMI page is not wired: $page"
	}
}

foreach ($icon in 'PageTriageIcon', 'PageDiagnoseIcon', 'PageBandagesIcon', 'PageMedicationIcon', 'PageAdvancedIcon', 'PageToggleSelfIcon') {
	if (-not $layout.Contains("Name `"$icon`"")) {
		throw "RAMI page icon is missing: $icon"
	}
}

foreach ($icon in 'Triage', 'Diagnose', 'Bandages', 'Medication', 'Advanced', 'TogglePatient') {
	$resourcePath = "UI/Textures/RAMI_ACE3PageIcons/RAMI_ACE3_$icon.edds"
	if (-not $layout.Contains($resourcePath)) {
		throw "ACE3 page icon is not wired: $icon"
	}
	foreach ($extension in 'png', 'edds', 'edds.meta') {
		$assetPath = Join-Path $root "UI/Textures/RAMI_ACE3PageIcons/RAMI_ACE3_$icon.$extension"
		if (-not (Test-Path $assetPath)) {
			throw "ACE3 page icon asset is missing: RAMI_ACE3_$icon.$extension"
		}
	}
	$meta = Get-Content (Join-Path $root "UI/Textures/RAMI_ACE3PageIcons/RAMI_ACE3_$icon.edds.meta") -Raw
	$resourceName = [regex]::Match($meta, 'Name "([^"]+)"').Groups[1].Value
	if (-not $layout.Contains($resourceName)) {
		throw "ACE3 page icon GUID does not match its layout reference: $icon"
	}
}
if (-not [regex]::IsMatch($layout, '(?s)Name "PageDiagnoseIcon".{0,300}RAMI_ACE3_Advanced\.edds') -or -not [regex]::IsMatch($layout, '(?s)Name "PageAdvancedIcon".{0,300}RAMI_ACE3_Diagnose\.edds')) {
	throw 'Diagnose and Actions page icons are not swapped'
}
if ($script.Contains('LoadPageIcons(root)')) {
	throw 'Page icons still depend on runtime image-set loading'
}
if ([regex]::Matches($layout, 'Padding 10 5 10 5').Count -ne 6) {
	throw 'RAMI page icons do not keep their compact padding'
}

$triageColorMethod = [regex]::Match($script, '(?s)protected Color GetTriageLevelColor.*?protected Color GetTriageLevelReadableTextColor').Value
if (-not $triageColorMethod.Contains('RAMI_ETriageLevel.NONE') -or -not $triageColorMethod.Contains('Color.FromSRGBA(128, 128, 128, 255)')) {
	throw 'Selected NONE triage button is not gray'
}

foreach ($widget in 'RAMISettings', 'RAMIButtonTextColor', 'RAMIColorblindMode') {
	if (-not $interfaceSettings.Contains("Name `"$widget`"") -or -not $script.Contains($widget)) {
		throw "RAMI interface setting is not wired: $widget"
	}
}

foreach ($removedWidget in 'TextColorButton', 'ColorblindModeButton') {
	if ($layout.Contains("Name `"$removedWidget`"")) {
		throw "RAMI appearance control remains in the medical menu: $removedWidget"
	}
}

foreach ($color in 'Default', 'White', 'Yellow', 'Cyan', 'Black', 'Red', 'Orange', 'Green', 'Blue', 'Magenta') {
	if (-not $interfaceSettings.Contains("`"$color`"")) {
		throw "RAMI button text color is missing: $color"
	}
}

if ([regex]::Matches($interfaceSettings, 'SCR_SpinBoxComponent "\{5472C6CBC0640458\}"').Count -ne 2) {
	throw 'RAMI settings do not override the inherited spinbox components'
}

foreach ($binding in 'class RAMI_UISettingsModule : ModuleGameSettings', 'GetModule("RAMI_UISettingsModule")', 'BUTTON_TEXT_COLOR_COUNT = 10', 'modded class SCR_InterfaceSettingsSubMenu', 'override void OnTabShow()', 'content = m_wScroll.GetChildren()', 'workspace.CreateWidgets(RAMI_SETTINGS_LAYOUT, content)', 'm_OnChanged.Remove(RAMI_SetButtonTextColor)', 'GetGame().UserSettingsChanged()', 'GetGame().SaveUserSettings()', 'RAMI_SetButtonTextColor(', 'RAMI_SetColorblindMode(', 'GetButtonTextColor(', 'm_ColorblindMode', 'Color.FromSRGBA(0, 114, 178, 255)', 'Color.FromSRGBA(213, 94, 0, 255)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI persisted accessibility setting is missing: $binding"
	}
}

foreach ($widget in 'BodyModel', 'PatientVitals', 'SelectedRegion', 'PageTitle') {
	if ($layout.Contains("Name `"$widget`"")) {
		throw "Page widget remains in global layout: $widget"
	}
}

foreach ($widget in 'BandageButton', 'BandageIcon', 'BandageCount', 'TourniquetButton', 'TourniquetIcon', 'TourniquetCount') {
	if (-not $treatmentControls.Contains("Name `"$widget`"")) {
		throw "Treatment item UI is missing: $widget"
	}
}

foreach ($widget in 'TriageLevelNone', 'TriageLevelMinimal', 'TriageLevelDelayed', 'TriageLevelImmediate', 'TriageLevelExpectant', 'TriageActivityTitle', 'TriageActivityScroll', 'TriageActivityContent', 'TriageActivityLog') {
	if (-not $triage.Contains("Name `"$widget`"")) {
		throw "Triage UI is missing: $widget"
	}
}

if (-not $triage.Contains('ScrollLayoutWidgetClass') -or -not $triage.Contains('VerticalLayoutWidgetClass')) {
	throw 'Triage activity log is not scrollable'
}

if (-not $triage.Contains('SizeToContent 1') -or -not $script.Contains('RAMI_ACTIVITY_LIMIT = 50')) {
	throw 'Triage activity log cannot grow beyond the visible area'
}

foreach ($widget in 'TriageLevelNone', 'TriageLevelMinimal', 'TriageLevelDelayed', 'TriageLevelImmediate', 'TriageLevelExpectant', 'TriageActivityLog') {
	if (-not $script.Contains($widget)) {
		throw "Triage UI is not wired: $widget"
	}
}

foreach ($level in 'NONE', 'MINIMAL', 'DELAYED', 'IMMEDIATE', 'EXPECTANT') {
	if (-not $script.Contains("RAMI_ETriageLevel.$level")) {
		throw "Triage level is missing: $level"
	}
}

foreach ($binding in 'RAMI_RequestTriage', 'RAMI_GetTriage', 'RAMI_SetPatientTriage', 'RAMI_RpcAsk_GetTriage', 'RAMI_RpcAsk_SetTriage', 'RAMI_RpcDo_SetTriage', 'RplRcver.Server', 'RplRcver.Owner', 'RAMI_SetTriageSnapshot', 'm_RAMI_TriageTargetId', 'RAMI_IsTreatmentTargetInRange') {
	if (-not $script.Contains($binding)) {
		throw "Server-authoritative triage flow is missing: $binding"
	}
}

foreach ($binding in 'm_RAMI_ActivityTimes.Insert(System.GetUnixTime())', 'System.GetHourMinuteSecond(localHour, localMinute, localSecond)', 'System.GetHourMinuteSecondUTC(utcHour, utcMinute, utcSecond)', 'SCR_DateTimeHelper.ConvertDateIntoMinutes', 'unixTime % 86400 + localOffsetSeconds', 'FormatLocalActivityTime(times[index])') {
	if (-not $script.Contains($binding)) {
		throw "UTC activity storage or native local-time display is missing: $binding"
	}
}

foreach ($binding in 'override void AddLogEntry(', 'override void SetTourniquettedGroup(', 'TREATMENT //', 'MEDICATION //', 'TRIAGE SET //', 'TOURNIQUET %1') {
	if (-not $script.Contains($binding)) {
		throw "Central patient activity hook is missing: $binding"
	}
}

if (-not $script.Contains('for (int index = 0; index < messages.Count(); index++)')) {
	throw 'Triage activity is not ordered from oldest to newest'
}

foreach ($excludedActivity in 'INJURY //', 'VITAL STATE //', 'CPR STARTED', 'CPR STOPPED', 'LIFE STATE //') {
	if ($script.Contains($excludedActivity)) {
		throw "Excluded patient activity remains: $excludedActivity"
	}
}

foreach ($binding in 'RAMI_IsMedicationItem(item)', 'override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)', 'TREATMENT // MEDICAL KIT //', '!message.StartsWith("TRIAGE SET //")', '!message.StartsWith("MEDICATION //")', '!message.StartsWith("TREATMENT //")', 'messages[index].Length().ToString() + ":" + messages[index]', 'remainingMessages.Substring(') {
	if (-not $script.Contains($binding)) {
		throw "Triage activity deduplication or safe snapshot packing is missing: $binding"
	}
}

if ($layout.Contains('Name "PageDragCarry"') -or $script.Contains('"PageDragCarry"')) {
	throw 'Separate Drag/Carry page remains after Advanced/Misc merge'
}

if ($treatment.Contains('Name "PageContent"') -or $script.Contains('"PageContent"')) {
	throw 'Obsolete treatment PageContent remains wired'
}

foreach ($widget in 'AdvancedCPRButton', 'AdvancedCarryButton', 'AdvancedDragButton', 'AdvancedBackButton', 'AdvancedRightSideButton', 'AdvancedLeftSideButton', 'AdvancedLoadVehicleButton') {
	if (-not $advanced.Contains("Name `"$widget`"") -or -not $script.Contains("`"$widget`"")) {
		throw "Advanced ACE action is not wired: $widget"
	}
}

foreach ($widget in 'DiagnosePatientName', 'DiagnoseHeartRateButton', 'DiagnoseHeartRateValue', 'DiagnoseHeartRateGraph', 'DiagnoseBloodPressureButton', 'DiagnoseBloodPressureValue', 'DiagnoseBloodPressureGraph') {
	if (-not $diagnose.Contains("Name `"$widget`"") -or -not $script.Contains("$widget")) {
		throw "Diagnose monitor is not wired: $widget"
	}
}

foreach ($widget in 'DiagnoseSystolicLegend', 'DiagnoseDiastolicLegend', 'DiagnoseHeartRhythmLabel', 'DiagnoseBloodPressureHigh', 'DiagnoseBloodPressureMiddle', 'DiagnoseBloodPressureLow', 'DiagnoseBloodPressureYAxis', 'DiagnoseBloodPressureXAxis') {
	if (-not $diagnose.Contains("Name `"$widget`"")) {
		throw "Diagnose graph legend is missing: $widget"
	}
}

foreach ($anchor in 'Anchor 0.04 0.35 0.49 0.88', 'Anchor 0.58 0.36 0.96 0.88') {
	if (-not $diagnose.Contains($anchor)) {
		throw "Diagnose page is not arranged for the 1804.8 x 745.2 content area: $anchor"
	}
}

if (-not $script.Contains('m_DiagnosePatientName.SetText(string.Format("VITALS FROM: %1", GetPatientName(s_Patient)))')) {
	throw 'Diagnose monitor does not show the active patient name'
}

foreach ($binding in 'DIAGNOSE_UPDATE_INTERVAL_MS = 1000', 'RequestUpdatePatientData(s_Patient)', 'vitals.GetHeartRate()', 'vitals.GetBloodPressures()', 'ACE_PhysicalConstants.KPA2MMHG', 'Remove(UpdateDiagnoseMonitor)') {
	if (-not $script.Contains($binding)) {
		throw "ACE diagnose polling is missing: $binding"
	}
}

foreach ($binding in 'LineDrawCommand', 'SetDrawCommands(', 'DrawDiagnoseHeartRateGraph()', 'GetHeartRhythmAmplitude(float phase)', 'DrawDiagnoseBloodPressureGraph()', 'DIAGNOSE_BLOOD_PRESSURE_HISTORY_COUNT = 30', 'm_DiagnoseSystolicHistory.Count() >= DIAGNOSE_BLOOD_PRESSURE_HISTORY_COUNT', 'axes.m_Vertices', 'middleGridLine.m_Vertices', 'systolicDot.m_Vertices', 'diastolicDot.m_Vertices') {
	if (-not $script.Contains($binding)) {
		throw "Diagnose graph is not wired: $binding"
	}
}

if (-not $script.Contains('m_DiagnoseHeartRateTarget - m_DiagnoseHeartRate')) {
	throw 'Heart rhythm does not smooth ACE sample changes'
}
foreach ($binding in 'DIAGNOSE_HEART_TRACE_SAMPLE_COUNT = 360', 'DIAGNOSE_HEART_TRACE_SAMPLE_INTERVAL = 1.0 / 60', 'm_DiagnoseHeartTrace.Insert(GetHeartRhythmAmplitude(m_DiagnoseHeartPhase))', 'm_DiagnoseHeartTrace.Count() >= DIAGNOSE_HEART_TRACE_SAMPLE_COUNT', 'TessellateCircle(', 'plotLeft = 12', 'plotRight = width - 12') {
	if (-not $script.Contains($binding)) {
		throw "Heart rhythm is not a rolling fixed-rate trace: $binding"
	}
}
foreach ($binding in 'InitializeDiagnoseHeartRateGraph()', 'm_DiagnoseHeartRateLine.m_Vertices.Clear()', 'm_DiagnoseHeartRateGraph.Update()') {
	if (-not $script.Contains($binding)) {
		throw "Heart rhythm does not retain its Canvas draw command: $binding"
	}
}

$showPageMethod = [regex]::Match($script, '(?s)void ShowPage\(string pageName\).*?protected void BindPageWidgets').Value
foreach ($stoppedState in 'Remove(UpdateDiagnoseMonitor)', 'm_IsHeartRateMonitored = false', 'm_IsBloodPressureMonitored = false') {
	if ($showPageMethod.Contains($stoppedState)) {
		throw "Page switching stops Diagnose monitoring: $stoppedState"
	}
}
foreach ($binding in 'RestoreDiagnosePage()', 'm_IsHeartRateMonitored = false', 'm_IsBloodPressureMonitored = false', 'Remove(UpdateDiagnoseMonitor)') {
	if (-not $script.Contains($binding)) {
		throw "Menu-lifetime Diagnose monitoring is missing: $binding"
	}
}

foreach ($binding in 'protected bool HasMedicalKit()', 'return FindMedicalKit(count) != null', 'protected string GetCoarseLevel(float value, float lowThreshold, float highThreshold)', 'LOW_HEART_RATE_BPM = 60', 'HIGH_HEART_RATE_BPM = 100', 'LOW_SYSTOLIC_PRESSURE_MMHG = 90', 'HIGH_SYSTOLIC_PRESSURE_MMHG = 140', 'if (hasMedicalKit)', 'UpdateDiagnoseGraphVisibility(hasMedicalKit)', 'widget.SetVisible(visible)', 'm_DiagnoseHeartRateValue.SetText("NONE")', 'm_DiagnoseHeartRateValue.SetText(GetCoarseLevel(', 'm_DiagnoseBloodPressureValue.SetText("LOW")', 'm_DiagnoseBloodPressureValue.SetText("MODERATE")', 'm_DiagnoseBloodPressureValue.SetText("HIGH")') {
	if (-not $script.Contains($binding)) {
		throw "Medical-kit Diagnose precision gate is missing: $binding"
	}
}
$toggleDiagnoseMethod = [regex]::Match($script, '(?s)void ToggleDiagnoseMonitor\(string widgetName\).*?protected void UpdateDiagnoseButtonText').Value
if (-not [regex]::IsMatch($toggleDiagnoseMethod, 'Remove\(UpdateDiagnoseMonitor\);\s*UpdateDiagnoseMonitor\(\);\s*GetGame\(\)\.GetCallqueue\(\)\.CallLater\(UpdateDiagnoseMonitor')) {
	throw 'Diagnose click does not show the current exact or coarse reading immediately'
}
foreach ($widget in 'DiagnoseHeartRateGraph', 'DiagnoseHeartRhythmLabel', 'DiagnoseSystolicLegend', 'DiagnoseDiastolicLegend', 'DiagnoseBloodPressureGraph', 'DiagnoseBloodPressureHigh', 'DiagnoseBloodPressureMiddle', 'DiagnoseBloodPressureLow', 'DiagnoseBloodPressureYAxis', 'DiagnoseBloodPressureXAxis') {
	if (-not [regex]::IsMatch($script, "(?s)protected void UpdateDiagnoseGraphVisibility\(bool visible\).*?`"$widget`"")) {
		throw "Exact Diagnose graph detail is not gated by medical kit: $widget"
	}
}

foreach ($binding in 'ACE_Medical_CPRUserAction.Cast(action)', 'ACE_Carrying_CarryUserAction.Cast(action)', 'ACE_Carrying_DragUserAction.Cast(action)', 'ACE_Medical_RepositionUserAction.Cast(action)', 'SCR_LoadCasualtySupportStationUserAction.Cast(action)', '#ACE_Medical-UserAction_BackPosition', '#ACE_Medical-UserAction_RightRecoveryPosition', '#ACE_Medical-UserAction_LeftRecoveryPosition', 'action.CanBeShown(user)', 'action.CanBePerformed(user)', 'vitals.IsCPRPerformed()', 'm_AdvancedCPRText.SetText("STOP CPR")', 'ACE_Medical_CPRHelperCompartment.Cast(ACE_AnimationTools.GetHelperCompartment(user))', 'cprHelper.Terminate()', 'performer.PerformAction(action)') {
	if (-not $script.Contains($binding)) {
		throw "Advanced page does not delegate to ACE action: $binding"
	}
}

$advancedActionMethod = [regex]::Match($script, '(?s)void UseAdvancedAction\(string widgetName\).*?protected void UpdateSelectedRegionText').Value
if ($advancedActionMethod -notmatch 'performer\.PerformAction\(action\);') {
	throw 'Actions page no longer performs its selected action'
}
if ($advancedActionMethod -match 'CloseMenu\(\);') {
	throw 'An Actions button still closes RAMI'
}
foreach ($binding in 'SetAdvancedButtonState(', 'button.SetEnabled(enabled)', 'Color buttonColor = Color.FromSRGBA(128, 128, 128, 230)', 'buttonColor = Color.FromSRGBA(255, 255, 255, 230)', 'button.SetColor(buttonColor)') {
	if (-not $script.Contains($binding)) {
		throw "Advanced button disabled styling is missing: $binding"
	}
}

if ([regex]::Matches($script, '\.SetBold\(true\)').Count -lt 7) {
	throw 'Treatment, medication, and advanced button text must be bold'
}

foreach ($binding in 'widgetName.StartsWith("Diagnose")', 'triageLevelText.SetBold(true)', 'buttonColor.SetA(0.9)', 'Color.FromSRGBA(255, 255, 255, 230)') {
	if (-not $script.Contains($binding)) {
		throw "Diagnose or triage button styling is missing: $binding"
	}
}

foreach ($binding in 'ApplyButtonTextContrast(TextWidget text)', 'text.SetOutline(0)', 'text.SetShadow(1, shadowColor, 0.65, 1, 1)', 'ApplyButtonTextContrast(label)', 'ApplyButtonTextContrast(count)', 'ApplyButtonTextContrast(text)') {
	if (-not $script.Contains($binding)) {
		throw "Button text contrast styling is missing: $binding"
	}
}

if (-not $project.Contains('"65AD7C379CBD394D"')) {
	throw 'ACE Carrying Dev is not a direct project dependency'
}

foreach ($widget in 'BandageButtonForeign', 'BandageIconForeign', 'BandageCountForeign', 'TourniquetButtonForeign', 'TourniquetIconForeign', 'TourniquetCountForeign') {
	if (-not $treatmentControls.Contains("Name `"$widget`"") -or -not $script.Contains("m_$widget")) {
		throw "Foreign treatment item UI is not bound: $widget"
	}
}

foreach ($name in 'Epinephrine', 'Morphine', 'Naloxone', 'Phenylephrine', 'Metoprolol', 'Ammonia', 'Saline500', 'Saline750', 'Saline1000', 'Saline1500', 'Saline250', 'Saline1250') {
	foreach ($suffix in 'Button', 'Icon', 'Label', 'Count', 'ButtonForeign', 'IconForeign', 'LabelForeign', 'CountForeign') {
		if (-not $medication.Contains("Name `"Medication$name$suffix`"") -or -not $script.Contains("Medication$name")) {
			throw "Medication control is not wired: Medication$name$suffix"
		}
	}
}

$controlLayouts = @{
	Treatment = $treatmentControls
	Medication = $medication
}
foreach ($controlLayout in $controlLayouts.GetEnumerator()) {
	$controlSlots = [regex]::Matches($controlLayout.Value, '(?ms)Slot FrameWidgetSlot "[^"]+" \{(.*?)\}')
	foreach ($slot in $controlSlots) {
		foreach ($zeroOffset in 'PositionX 0', 'OffsetLeft 0', 'PositionY 0', 'OffsetTop 0', 'SizeX 0', 'OffsetRight 0', 'SizeY 0', 'OffsetBottom 0') {
			if (-not $slot.Groups[1].Value.Contains($zeroOffset)) {
				throw "$($controlLayout.Key) slot is missing explicit zero offset: $zeroOffset in $($slot.Value)"
			}
		}
	}
}
foreach ($binding in @{
	TreatmentTop = 'Anchor 0 0 1 0.068493'
	TreatmentBottom = 'Anchor 0 0.835616 1 1'
	MedicationTop = 'Anchor 0 0 1 0.069444'
}.GetEnumerator()) {
	$source = $treatmentControls
	if ($binding.Key.StartsWith('Medication')) {
		$source = $medication
	}
	if (-not $source.Contains($binding.Value)) {
		throw "Control layout does not use its full canvas: $($binding.Key)"
	}
}

$medicationIconSlots = [regex]::Matches($medication, '(?ms)Name "Medication[^"]+Icon(?:Foreign)?"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0 0\.22 1\s+PositionX 0\s+OffsetLeft 0\s+PositionY 0\s+OffsetTop 0\s+SizeX 0\s+OffsetRight 0\s+SizeY 0\s+OffsetBottom 0\s+\}')
if ($medicationIconSlots.Count -ne 26) {
	throw "Medication icons do not share the required slot geometry: $($medicationIconSlots.Count)/26"
}

foreach ($binding in '(?s)Name "MedicationSalineToggleRow"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.097222 1 0\.173611', '(?s)Name "MedicationSalineToggleRowForeign"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.652778 1 0\.713') {
	if (-not [regex]::IsMatch($medication, $binding)) {
		throw 'Saline category button is not first or full-width'
	}
}

foreach ($binding in 'case "PageMedication": layout = PAGE_TREATMENT_LAYOUT; hasInventoryControls = true; m_IsMedicationPage = true;', 'm_PageRoot.FindAnyWidget("InventoryFrame")', 'workspace.CreateWidgets(inventoryLayout, inventoryFrame)', 'PAGE_TREATMENT_CONTROLS_LAYOUT') {
	if (-not $script.Contains($binding)) {
		throw "Medication page no longer preserves the treatment body and vitals: $binding"
	}
}

foreach ($binding in 'RAMI_ItemMatchesTreatment', 'RAMI_RequestMedicationInventory', 'RAMI_RpcAsk_GetMedicationInventory', 'RAMI_RpcDo_SetMedicationInventory', 'RAMI_GetMedicationInventory', 'inventory.Split("|", values, false)', 'effect.CanApplyEffectToHZ(s_Patient, user, m_SelectedRegion, failReason)', 'UseMedicationWidget(string widgetName)') {
	if (-not $script.Contains($binding)) {
		throw "Medication treatment flow is missing: $binding"
	}
}

foreach ($binding in 'RAMI_CanApplyMedicationToRegion', 'treatmentType == RAMI_ETreatmentType.AMMONIUM_CARBONATE', 'region == ECharacterHitZoneGroup.HEAD && controller && controller.IsUnconscious()', 'treatmentType >= RAMI_ETreatmentType.SALINE_500 && treatmentType <= RAMI_ETreatmentType.SALINE_1250', 'region == ECharacterHitZoneGroup.LEFTARM || region == ECharacterHitZoneGroup.RIGHTARM', 'region == ECharacterHitZoneGroup.LEFTLEG', 'region == ECharacterHitZoneGroup.RIGHTLEG', 'canTreat && count > 0 && canApply', 'foreignCount > 0 && canApplyForeign', 'label.SetColor(GetButtonTextColor(Color.Black))', 'count.SetColor(GetButtonTextColor(Color.Black))', 'icon.SetColor(widgetColor)') {
	if (-not $script.Contains($binding)) {
		throw "Medication availability rule is missing: $binding"
	}
}
foreach ($prefab in 'ACE_Medical_EpinephrineInjection.et', 'MorphineInjection_01.et', 'ACE_Medical_NaloxoneInjection.et', 'ACE_Medical_PhenylephrineInjection.et', 'ACE_Medical_MetoprololInjection.et', 'ACE_Medical_AmmoniumCarbonatePackage.et', 'SalineBag_US_250.et', 'SalineBag_US_500.et', 'SalineBag_US_1000.et', 'SalineBag_US_1250.et', 'SalineBag_US_1500.et', 'SalineBag_US_01.et', 'SalineBag_USSR_250.et', 'SalineBag_USSR_1250.et', 'SalineBag_USSR_01.et') {
	if (-not $script.Contains($prefab)) {
		throw "Medication prefab is missing: $prefab"
	}
}

foreach ($binding in 'SetTreatmentButtonState(m_MedicationButtons[index]', 'SetTreatmentButtonState(m_MedicationButtonsForeign[index]') {
	if (-not $script.Contains($binding)) {
		throw 'Medication text does not use Treatment button color grading'
	}
}

if (-not [regex]::IsMatch($layout, '(?s)CanvasWidgetClass.*?Name "Background"') -or -not [regex]::IsMatch($layout, '(?s)CanvasWidgetClass.*?Name "PatientWindowBackground"') -or -not $script.Contains('DrawRoundedBackground(background, m_MenuBackgroundCommands)') -or -not $script.Contains('DrawRoundedBackground(patientBackground, m_PatientWindowBackgroundCommands)') -or -not $script.Contains('fill.m_iColor = 0xBF000000') -or -not $script.Contains('float outlineThickness = 6') -or -not $script.Contains('ref TriMeshDrawCommand outlineCorners = new TriMeshDrawCommand()') -or -not $script.Contains('for (int cornerIndex = 0; cornerIndex < 4; cornerIndex++)') -or -not $script.Contains('bool mirrorX = cornerIndex == 1 || cornerIndex == 3') -or -not $script.Contains('bool mirrorY = cornerIndex >= 2') -or -not $script.Contains('outlineTop.m_Vertices = {cornerRadius, 0, width - cornerRadius, 0, width - cornerRadius, outlineThickness, cornerRadius, outlineThickness}') -or -not $script.Contains('outlineBottom.m_Vertices = {cornerRadius, height - outlineThickness, width - cornerRadius, height - outlineThickness, width - cornerRadius, height, cornerRadius, height}') -or -not $script.Contains('outlineLeft.m_Vertices = {0, cornerRadius, outlineThickness, cornerRadius, outlineThickness, height - cornerRadius, 0, height - cornerRadius}') -or -not $script.Contains('outlineRight.m_Vertices = {width - outlineThickness, cornerRadius, width, cornerRadius, width, height - cornerRadius, width - outlineThickness, height - cornerRadius}') -or -not $script.Contains('background.SetDrawCommands(commands)') -or -not $script.Contains('m_IsPatientWindowOpen && m_PatientWindowBackgroundCommands.IsEmpty()')) {
	throw 'Medical interface background does not copy one shared outline thickness to all four sides'
}

foreach ($region in 'HEAD', 'CHEST', 'ABDOMEN', 'LEFT ARM', 'RIGHT ARM', 'LEFT LEG', 'RIGHT LEG') {
	$binding = 'm_SelectedRegionText.SetText("' + $region + '")'
	if (-not $script.Contains($binding)) {
		throw "Selected body-zone label is missing plain region text: $region"
	}
}
if ($script.Contains('SELECTED //') -or -not $treatment.Contains('Text "CHEST"')) {
	throw 'Selected body-zone label still contains the SELECTED prefix'
}

if ([regex]::Matches($layout, 'Color 1 1 1 0\.9').Count -ne 9 -or [regex]::Matches($diagnose, 'Color 1 1 1 0\.9').Count -ne 2 -or [regex]::Matches($advanced, 'Color 1 1 1 0\.9').Count -ne 7) {
	throw 'Global, Diagnose, or Actions buttons do not use 90% opacity'
}

if ([regex]::Matches($triage, 'Color 1 1 1 0\.9').Count -ne 5 -or [regex]::Matches($treatmentControls, 'Color 0\.35 0\.35 0\.35 0\.9').Count -ne 5 -or [regex]::Matches($medication, 'Color 0\.35 0\.35 0\.35 0\.9').Count -ne 28) {
	throw 'Triage, Treatment, or Medication buttons do not use 90% opacity'
}

foreach ($widget in 'MedicationSalineToggleButton', 'MedicationSalineToggleIcon', 'MedicationSalineToggleLabel', 'MedicationSalineToggleCount', 'MedicationSalineToggleButtonForeign', 'MedicationSalineToggleIconForeign', 'MedicationSalineToggleLabelForeign', 'MedicationSalineToggleCountForeign') {
	if (-not $medication.Contains("Name `"$widget`"") -or -not $script.Contains($widget)) {
		throw "Persistent Saline toggle is missing: $widget"
	}
}

foreach ($volume in 250, 500, 750, 1000, 1250, 1500) {
	foreach ($suffix in 'Button', 'Icon', 'Label', 'Count', 'ButtonForeign', 'IconForeign', 'LabelForeign', 'CountForeign') {
		if (-not $medication.Contains("Name `"MedicationSaline$volume$suffix`"")) {
			throw "Saline volume option is missing: MedicationSaline$volume$suffix"
		}
	}
}

if (-not $project.Contains('"6A05287AF042071B"') -or -not $script.Contains('values.Count() != 24')) {
	throw 'Expanded Saline Bags dependency or twelve-item medication inventory payload is missing'
}

foreach ($binding in @{
	ForeigninventoryTitle = 'm_ForeignInventoryTitle.SetVisible(foreignInteraction)'
	BandageRowForeign = 'm_BandageRowForeign.SetVisible(showTreatmentForeignInventory)'
	TourniquetRowForeign = 'm_TourniquetRowForeign.SetVisible(showTreatmentForeignInventory)'
}.GetEnumerator()) {
	if (-not $treatmentControls.Contains("Name `"$($binding.Key)`"") -or -not $script.Contains($binding.Value)) {
		throw "Foreign inventory visibility is missing: $($binding.Key)"
	}
}

if (-not $script.Contains('bool foreignInteraction = s_ForeignPatient && s_Patient == s_ForeignPatient;')) {
	throw 'Foreign inventory is not tied to the active foreign interaction'
}
if (-not $script.Contains('bool showTreatmentForeignInventory = foreignInteraction && !m_IsMedicationPage;')) {
	throw 'Treatment inventory rows can leak onto the medication page'
}

foreach ($binding in 'TextWidget m_ForeignInventoryTitle', 'm_ForeignInventoryTitle.SetText(string.Format("Inventory from %1", GetPatientName(s_Patient)))', 'string patientName = GetPatientName(patient)') {
	if (-not $script.Contains($binding)) {
		throw "Foreign inventory patient name is missing: $binding"
	}
}

foreach ($itemType in 'StartTreatment(1, foreignInventory)', 'StartTreatment(4, foreignInventory)', 'GetCommonType()', 'ItemPreviewWidget', 'SetPreviewItem(preview, item)') {
	if (-not $script.Contains($itemType)) {
		throw "Treatment inventory binding is missing: $itemType"
	}
}

foreach ($widget in 'BodyTourniquetLeftArm', 'BodyTourniquetRightArm', 'BodyTourniquetLeftLeg', 'BodyTourniquetRightLeg') {
	if (-not $treatment.Contains("Name `"$widget`"") -or -not $script.Contains("m_$widget")) {
		throw "Tourniquet body visualization is missing: $widget"
	}
}

if (-not $treatment.Contains('Texture "{177061F4F0CE8F2C}UI/Textures/RAMI_BodyZones/RAMI_Tourniquet_v2.edds"') -or
	-not $script.Contains('ImageWidget m_BodyTourniquetLeftArm') -or
	$treatment.Contains('Text "TQ"')) {
	throw 'Registered tourniquet image has not replaced the provisional TQ text'
}

if (-not $script.Contains('GetGroupTourniquetted(group)') -or -not $script.Contains('state.HasTourniquet(group)')) {
	throw 'ACE tourniquet state is not wired to the body visualization'
}

foreach ($widget in 'BodyInjuryHead', 'BodyInjuryChest', 'BodyInjuryAbdomen', 'BodyInjuryLeftArm', 'BodyInjuryRightArm', 'BodyInjuryLeftLeg', 'BodyInjuryRightLeg') {
	if (-not $treatment.Contains("Name `"$widget`"") -or -not $script.Contains("m_$widget")) {
		throw "Injury body-zone indicator is missing: $widget"
	}
}

foreach ($binding in 'GetInjurySeverity(group, damageManager)', '1 - characterHitZone.GetHealthScaled()', 'healthScaled < WOUND_4_HEALTH_THRESHOLD', 'healthScaled < WOUND_3_HEALTH_THRESHOLD', 'healthScaled < WOUND_2_HEALTH_THRESHOLD', 'string.Format("Wound_%1_UI", woundLevel)', 'injury.SetVisible(severity > 0)', 'Math.Round((1 - severity) * 255)') {
	if (-not $script.Contains($binding)) {
		throw "Injury severity visualization is missing: $binding"
	}
}

foreach ($binding in 'Name "BodySalineBag"', '"Saline-bag_UI"', 'Color.FromSRGBA(255, 255, 0, 255)', 'RAMI_GetSalineBagStatus(damageManager', 'FindAllDamageEffectsOfType(SCR_SalineDamageEffect, effects)', 'SCR_SalineDamageEffect.Cast(persistentEffect)', 'GetMaxDuration() - salineEffect.GetCurrentDuration()', 'Math.AbsFloat(salineEffect.GetDPS()) * remainingTime', 'salineTimeRemainingSeconds > 0', 'SALINE BAG\nVOLUME REMAINING\nTIME REMAINING', 'm_BodySalineBag.SetOpacity') {
	if (-not $treatment.Contains($binding) -and -not $script.Contains($binding)) {
		throw "Saline bag status is missing: $binding"
	}
}

if (-not [regex]::IsMatch($script, '(?s)int salineTimeRemainingSeconds;\s*bool salineApplied;\s*bool medicationSummaryAvailable;\s*if \(m_PatientMedications\).*?RAMI_RequestMedicationSummary\(damageManager\)')) {
	throw 'Saline bag status is not requested for both Treatment and Medication'
}

foreach ($binding in '!m_IsMedicationPage && bodyZoneState', 'bodyZoneState.GetInjurySeverity(m_SelectedRegion)', 'selectedZoneLabelBlock = "\n\nINJURY"', 'Math.Round(injurySeverity * 100)', '(selectedArm || selectedLeg) && bodyZoneState.HasFracture(m_SelectedRegion)', 'selectedZoneLabelBlock += "\nAIMING"', 'selectedZoneLabelBlock += "\nMOVEMENT"', 'selectedZoneValueBlock += "\nIMPAIRED"') {
	if (-not $script.Contains($binding)) {
		throw "Selected-zone treatment vitals are missing: $binding"
	}
}

foreach ($binding in 'Name "MedicalKitRow"', 'Name "MedicalKitButton"', 'Name "MedicalKitIcon"', 'Name "MedicalKitLabel"', 'Name "MedicalKitCount"') {
	if (-not $treatmentControls.Contains($binding)) {
		throw "Medical-kit treatment control is missing: $binding"
	}
}

if (-not $script.Contains('m_MedicalKitRow.SetVisible(medicalKitCount > 0)')) {
	throw 'Empty medical-kit control is not hidden'
}

foreach ($binding in 'm_Menu.UseMedicalKit()', 'void UseMedicalKit()', 'FindComponent(SCR_HealSupportStationComponent)', 'SCR_HealSupportStationAction FindMedicalKitAction', 'healAction.GetHitZoneGroup() == region', 'StartTreatmentWithItem(medicalKit)', 'FinishPendingMedicalKit(user)', 'action.CanBeShown(user)', 'action.CanBePerformed(user)', 'performer.StartAction(action)', 'm_IsMedicalKitTreatmentActive = true', '!m_IsMedicalKitTreatmentActive', 'UpdatePendingMedicalKit(tDelta)', 'performer.PerformContinuousAction(action, timeSlice)', 'performer.CancelAction(action)', 'characterStorage.UnequipCurrentItem()', 'MEDICAL KIT TREATMENT COMPLETE') {
	if (-not $script.Contains($binding)) {
		throw "Native medical-kit flow is missing: $binding"
	}
}

foreach ($binding in 'OnMenuUpdate(float tDelta)', 'Math.Sin(m_BleedingPulseTime', 'UpdateBodyZonePulse(', 'zone.SetOpacity(opacity)') {
	if (-not $script.Contains($binding)) {
		throw "Bleeding body-zone pulse is missing: $binding"
	}
}

if (-not $script.Contains('controller.GetLifeState() == ECharacterLifeState.DEAD') -or -not $script.Contains('state = null;')) {
	throw 'Bleeding body-zone pulse must stop for deceased patients'
}

foreach ($binding in 'm_TourniquetAppliedAt.Set(group, System.GetTickCount())', 'm_TourniquetAppliedAt.Remove(group)', 'GetTourniquetElapsedSeconds(m_SelectedRegion)', 'TOURNIQUET TIME') {
	if (-not $script.Contains($binding)) {
		throw "Selected-region tourniquet stopwatch is missing: $binding"
	}
}

foreach ($binding in 'UseOrRemoveTourniquet(bool foreignInventory)', 'REMOVE TOURNIQUET', 'SCR_TourniquetStorageComponent', 'RemoveTourniquetFromSlot(m_SelectedRegion, user)') {
	if (-not $script.Contains($binding)) {
		throw "Tourniquet removal is missing: $binding"
	}
}

foreach ($binding in 'UseBandage(bool foreignInventory)', 'IsSelectedRegionBleeding()', 'ACE_Medical_CalculateBleedingRate() > 0') {
	if (-not $script.Contains($binding)) {
		throw "Region-specific bandage guard is missing: $binding"
	}
}

foreach ($binding in 'FindApplicableTreatmentItem(int commonType)', 'CanStartTreatment(int commonType, bool foreignInventory)', 'SCR_ConsumableEffectHealthItems', 'effect.ActivateEffect(', 'SetTreatmentButtonState(', 'button.SetEnabled(enabled)') {
	if (-not $script.Contains($binding)) {
		throw "Direct generic treatment is missing: $binding"
	}
}

foreach ($binding in 'CanTreatPatient()', 'controller.GetLifeState() != ECharacterLifeState.DEAD', 'if (!user || !CanTreatPatient())', 'if (!CanTreatPatient())', 'bool canTreat = CanTreatPatient();', 'if (canTreat && tourniquetApplied)') {
	if (-not $script.Contains($binding)) {
		throw "Deceased patient treatment guard is missing: $binding"
	}
}

if (-not $script.Contains('label.SetColor(GetButtonTextColor(Color.Black))') -or -not $script.Contains('count.SetColor(GetButtonTextColor(Color.Black))')) {
	throw 'Treatment button text and count must stay white'
}

if ($script.Contains('SetIconTo(icon)') -or -not $treatmentControls.Contains('ItemPreviewWidgetClass') -or -not $script.Contains('world.GetItemPreviewManager()')) {
	throw 'Treatment items are not rendered as inventory previews'
}

foreach ($binding in 'if (!item)', 'preview.SetVisible(false)', 'preview.SetVisible(true)') {
	if (-not $script.Contains($binding)) {
		throw "Empty treatment preview flicker guard is missing: $binding"
	}
}

foreach ($binding in 'characterStorage.CanUseItem(item, ESlotFunction.TYPE_GADGET)', 'characterStorage.UseItem(item, ESlotFunction.TYPE_GADGET, SCR_EUseContext.FROM_INVENTORY)', 'FinishPendingTreatment()', 'controller.IsChangingItem()', 'characterStorage.GetCurrentItem() != m_PendingTreatmentItem', 'ClearPendingTreatment()') {
	if (-not $script.Contains($binding)) {
		throw "One-click equip-and-treat flow is missing: $binding"
	}
}

foreach ($binding in 'GetTreatmentInventoryItems(out array<IEntity> items)', 'inventoryOwner.FindComponent(SCR_InventoryStorageManagerComponent)') {
	if (-not $script.Contains($binding)) {
		throw "Local treatment inventory is missing: $binding"
	}
}

foreach ($binding in 'RAMI_SetTreatmentTarget(consumable, m_PendingTreatmentPatient, m_PendingTreatmentRegion)', 'RAMI_RpcAsk_SetTreatmentTarget', 'RplRcver.Server', 'RAMI_InventoryContains(user, item)', 'effect.CanApplyEffectToHZ(target, user, region, failReason)') {
	if (-not $script.Contains($binding)) {
		throw "Dedicated-server treatment target is missing: $binding"
	}
}

foreach ($binding in 'RAMI_RequestTreatmentInventory', 'RAMI_RpcAsk_GetTreatmentInventory', 'RAMI_RpcDo_SetTreatmentInventory', 'RAMI_GetTreatmentInventory', 'item.GetPrefabData().GetPrefabName()', 'SetPreviewItemFromPrefab(preview, prefab)', 'RAMI_RequestForeignTreatment', 'RAMI_RpcAsk_PrepareForeignTreatment', 'RAMI_RpcDo_PrepareForeignTreatment', 'TryMoveItemToStorage(item, destination)', 'FinishForeignTreatmentRequest') {
	if (-not $script.Contains($binding)) {
		throw "Dedicated-server foreign inventory flow is missing: $binding"
	}
}

foreach ($obsolete in 'SetTreatmentLootStorage', 'm_LootStoragePatient', 'vicinity.SetItemOfInterest(patient)') {
	if ($script.Contains($obsolete)) {
		throw "Obsolete client-side foreign inventory path remains: $obsolete"
	}
}

foreach ($binding in 'RAMI_RequestPain(damageManager)', 'RAMI_RpcAsk_GetPain', 'RAMI_RpcDo_SetPain', 'RplRcver.Owner', 'RAMI_GetPain(damageManager, painPercent)') {
	if (-not $script.Contains($binding)) {
		throw "Dedicated-server pain read is missing: $binding"
	}
}

foreach ($binding in 'RAMI_RequestMedicationSummary(damageManager)', 'RAMI_RpcAsk_GetMedicationSummary', 'RAMI_RpcDo_SetMedicationSummary', 'ACE_SettingsHelperT<ACE_Medical_Medication_Settings>.GetModSettings()', 'settings.GetPharmacokineticsConfig(drugs[index])', 'medication.GetMedications(drugs, allDoses)', 'for (int doseIndex = 0; doseIndex < allDoses[index].Count(); doseIndex++)', 'bolus.GetAdministeredConcentration()', 'RAMI_GetMedicationConcentrationScale(config, elapsedTime)', 'RAMI_GetMedicationPeakTime(config)', 'RAMI_GetMedicationHalfTime(config, peakTime)', 'RAMI_GetMedicationEffectConfig(settings, drugs[index])', 'RAMI_GetMedicationAntagonistScale(effectConfig, concentrations)', 'antagonistConfig.ComputeChi(concentrations)', 'strengthValue += " / " + antagonistNames', 'labels += "\n" + medicationName + "\n"', 'PEAK IN ', 'PEAK ACTIVE', 'HALF IN ', 'HALF PASSED', 'STRENGTH %1 %%') {
	if (-not $script.Contains($binding)) {
		throw "Config-driven medication summary is missing: $binding"
	}
}

foreach ($copiedDefault in '0.05193617', '0.03441518', '0.03872958', '0.03090123', '0.0009', '0.02911635', '0.0022', '0.03007057', '0.0386053') {
	if ($script.Contains($copiedDefault)) {
		throw "ACE medication setting was hardcoded into RAMI: $copiedDefault"
	}
}

if (-not $treatment.Contains('Name "PatientVitalValues"') -or -not $treatment.Contains('"Horizontal Alignment" Right') -or -not $script.Contains('m_PatientVitalValues')) {
	throw 'Patient vital values are not wired to the right-aligned column'
}
if (-not $treatment.Contains('Text "MEDICATIONS"')) {
	throw 'Medication summary heading is missing'
}

foreach ($widget in 'PatientVitalsBackground', 'PatientMedicationsBackground', 'PatientMedicationsHeader', 'PatientMedicationsDivider', 'PatientMedicationsScroll', 'PatientMedicationsContent', 'PatientMedicationsColumns', 'PatientMedications', 'PatientMedicationValues', 'PatientMedicationsFocus') {
	if (-not $treatment.Contains("Name `"$widget`"") -or -not $script.Contains("m_$widget") -and $widget -in 'PatientMedications', 'PatientMedicationValues', 'PatientMedicationsFocus') {
		throw "Separate medication window is missing: $widget"
	}
}
if (-not $treatment.Contains('SCR_GamepadScrollComponent') -or -not [regex]::IsMatch($treatment, '(?s)VerticalLayoutWidgetClass.*?Name "PatientMedicationsContent".*?OverlayWidgetClass.*?Name "PatientMedicationsColumns".*?SizeToContent 1.*?HorizontalAlign 3') -or -not [regex]::IsMatch($treatment, '(?s)Name "PatientMedicationValues".*?Slot OverlayWidgetSlot.*?HorizontalAlign 3\s+VerticalAlign 3\s+Padding 0 0 6 0.*?"Font Size" 16\s+"Min Font Size" 16.*?"Horizontal Alignment" Right') -or -not [regex]::IsMatch($treatment, '(?s)Name "PatientVitals".*?"Font Size" 16\s+"Min Font Size" 16')) {
	throw 'Vitals and medication windows do not keep a fixed readable font with native scrolling'
}
if ($script.Contains('m_PatientMedicationValues.SetTextOffset(')) {
	throw 'Medication values must be aligned by the full-width overlay instead of a text offset'
}
if (-not [regex]::IsMatch($treatment, '(?s)Name "PatientVitalsBackground".*?Anchor 0\.71 0\.05 0\.985 0\.47') -or -not [regex]::IsMatch($treatment, '(?s)Name "PatientMedicationsBackground".*?Anchor 0\.71 0\.5 0\.985 0\.93') -or -not [regex]::IsMatch($treatment, '(?s)Name "PatientMedicationsScroll".*?Anchor 0\.73 0\.6 0\.965 0\.9')) {
	throw 'Raised vitals card does not leave the medication list enough vertical space'
}
foreach ($binding in 'm_PatientMedications.SetText(medicationListLabels)', 'm_PatientMedicationValues.SetText(medicationListValues)', 'medicationListLabels = "LOADING..."', 'focused.GetName() == "PatientMedicationsFocus"', 'SetControllerNavigation("PatientMedicationsFocus"') {
	if (-not $script.Contains($binding)) {
		throw "Scrollable medication window is not wired: $binding"
	}
}
foreach ($obsolete in 'medicationLabelBlock', 'medicationValueBlock', 'salineLabelBlock', 'salineValueBlock') {
	if ($script.Contains($obsolete)) {
		throw "Medication data remains embedded in the vitals window: $obsolete"
	}
}

foreach ($binding in 'HemorrhageClassLabel(bloodScaled)', 'bloodScaled <= 0.2', 'bloodScaled <= 0.4', 'bloodScaled <= 0.7', 'return "CLASS IV"', 'return "CLASS III"', 'return "CLASS II"', 'return "CLASS I"', 'return "NORMAL"', 'return "FATAL"') {
	if (-not $script.Contains($binding)) {
		throw "ACE hemorrhage range is missing: $binding"
	}
}

foreach ($binding in 'bool hasMedicalKit = HasMedicalKit()', 'if (hasMedicalKit)', 'tourniquetValue = "\nAPPLIED"', 'selectedZoneValueBlock = "\n\n" + GetCoarseLevel(', 'string bleedingEstimate = "NONE"', 'string painEstimate = "NONE"', 'GetCoarseLevel(healthPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD)', 'GetCoarseLevel(resiliencePercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD)', 'GetCoarseLevel(bloodPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD)', 'GetCoarseLevel(100 - bloodPercent, COARSE_LOW_PERCENT_THRESHOLD, COARSE_HIGH_PERCENT_THRESHOLD)') {
	if (-not $script.Contains($binding)) {
		throw "Medical-kit patient-vitals precision gate is missing: $binding"
	}
}

foreach ($binding in 'm_Menu.UseBandage(true)', 'm_Menu.UseOrRemoveTourniquet(true)', 'm_BandageButtonForeign', 'm_TourniquetButtonForeign', 'm_BandageCountForeign', 'm_TourniquetCountForeign', 'm_BandageIconForeign', 'm_TourniquetIconForeign') {
	if (-not $script.Contains($binding)) {
		throw "Foreign treatment controls are not wired: $binding"
	}
}

if ($script.Contains('characterStorage.CanUseItem_Inventory(item)')) {
	throw 'Foreign treatment must not be blocked by the self-treatment inventory check'
}

foreach ($binding in 'm_PendingTreatmentPatient != user', 'effect.GetApplyToOtherAnimCmnd(user)', 'effect.GetReviveAnimCmnd(user)', 'effect.GetApplyToOtherDuraction()') {
	if (-not $script.Contains($binding)) {
		throw "Foreign-patient treatment animation is missing: $binding"
	}
}

foreach ($binding in 'RAMI_IsTreatmentTargetInRange(IEntity target)', 'vector.Distance(user.GetOrigin(), target.GetOrigin()) <= 5', 's_ForeignPatient == s_Patient || !IsForeignPatientInRange()', 's_Patient == s_ForeignPatient && !IsForeignPatientInRange()', 'if (!IsForeignPatientInRange())') {
	if (-not $script.Contains($binding)) {
		throw "Foreign-patient menu range validation is missing: $binding"
	}
}

foreach ($widget in 'PatientWindow', 'PatientWindowTitle', 'PatientWindowSubtitle', 'PatientWindowDivider', 'PatientListSurface', 'PatientListScroll', 'PatientList', 'PatientListStatus', 'PatientActions', 'PatientActionTreatButton', 'PatientActionTreatText', 'PatientActionUnloadButton', 'PatientActionUnloadText') {
	if (-not $layout.Contains(('Name "{0}"' -f $widget))) {
		throw "Patient window is missing: $widget"
	}
}
if (-not $layout.Contains('Padding 12 0 12 0')) {
	throw 'Patient-list buttons do not use equal left and right spacing'
}

foreach ($binding in 'SALINE_FIRST_INDEX = 6', 'if (!m_IsMedicationPage || m_MedicationRows.Count()', 'UpdateSalineMenuVisibility()', 'saline == m_IsSelfSalineMenuOpen', 'foreignInteraction && saline == m_IsForeignSalineMenuOpen', 'm_SalineToggleRowForeign.SetVisible(foreignInteraction)', 'm_IsForeignSalineMenuOpen = false', 'm_SalineToggleRowForeign.IsVisible()', 'm_SalineToggleLabel.SetText("SALINE ITEMS")', 'm_SalineToggleLabel.SetText("STANDARD ITEMS")', 'm_SalineToggleLabelForeign.SetText("SALINE ITEMS")', 'm_SalineToggleLabelForeign.SetText("STANDARD ITEMS")', 'salineCount += count', 'salineCountForeign += foreignCount', 'm_IsSelfSalineMenuOpen = !m_IsSelfSalineMenuOpen', 'm_IsForeignSalineMenuOpen = !m_IsForeignSalineMenuOpen', 'for (int index = 0; index < m_MedicationTypes.Count(); index++)') {
	if (-not $script.Contains($binding)) {
		throw "Saline submenu is not wired: $binding"
	}
}
foreach ($binding in '(?s)Name "MedicationSaline250Label".*?Text "Saline 250 ml"', '(?s)Name "MedicationSaline250LabelForeign".*?Text "Saline 250 ml"', '(?s)Name "MedicationSaline500Label".*?Text "Saline 500 ml"', '(?s)Name "MedicationSaline500LabelForeign".*?Text "Saline 500 ml"', '(?s)Name "MedicationSaline750Label".*?Text "Saline 750 ml"', '(?s)Name "MedicationSaline750LabelForeign".*?Text "Saline 750 ml"', '(?s)Name "MedicationSaline1250Label".*?Text "Saline 1250 ml"', '(?s)Name "MedicationSaline1250LabelForeign".*?Text "Saline 1250 ml"') {
	if (-not [regex]::IsMatch($medication, $binding)) {
		throw 'Self or foreign Saline volume label is missing'
	}
}
foreach ($binding in '(?s)Name "MedicationSaline750Row"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.277778 0\.477273 0\.354167', '(?s)Name "MedicationSaline750RowForeign"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.7963 0\.477273 0\.8565') {
	if (-not [regex]::IsMatch($medication, $binding)) {
		throw 'Saline 750 ml is not first in the second self or foreign Saline row'
	}
}
foreach ($binding in '(?s)Name "MedicationSaline250Row"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.1875 0\.477273 0\.263889', '(?s)Name "MedicationSaline1250Row"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.368056 0\.477273 0\.444444', '(?s)Name "MedicationSaline250RowForeign"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.7245 0\.477273 0\.7847', '(?s)Name "MedicationSaline1250RowForeign"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.8681 0\.477273 0\.9283') {
	if (-not [regex]::IsMatch($medication, $binding)) {
		throw 'Saline 250/1250 ml is not ordered by volume in self or foreign submenu'
	}
}
foreach ($binding in '(?s)RAMI_ETreatmentType\.SALINE_250,\s*RAMI_ETreatmentType\.SALINE_500,\s*RAMI_ETreatmentType\.SALINE_750,\s*RAMI_ETreatmentType\.SALINE_1000,\s*RAMI_ETreatmentType\.SALINE_1250,\s*RAMI_ETreatmentType\.SALINE_1500', '(?s)"MedicationSaline250",\s*"MedicationSaline500",\s*"MedicationSaline750",\s*"MedicationSaline1000",\s*"MedicationSaline1250",\s*"MedicationSaline1500"') {
	if (-not [regex]::IsMatch($script, $binding)) {
		throw 'Saline controller order does not match visible Saline rows'
	}
}
$medicationNavigationMethod = [regex]::Match($script, '(?s)protected void UpdateTreatmentItemNavigation\(\).*?protected void ConfigureAdvancedNavigation').Value
if (-not [regex]::IsMatch($medicationNavigationMethod, '(?s)if \(m_SalineToggleButton\).*?itemButtons\.Insert\(m_SalineToggleButton\);.*?for \(int medicationIndex')) {
	throw 'Saline category is not first in controller navigation'
}
foreach ($binding in 'm_SalineToggleIcon, true)', 'm_SalineToggleIconForeign, foreignInteraction)', '!m_SalineToggleButton || !m_SalineToggleButton.IsEnabled()', '!m_SalineToggleButtonForeign || !m_SalineToggleButtonForeign.IsEnabled()') {
	if (-not $script.Contains($binding)) {
		throw "Saline category availability is missing: $binding"
	}
}
$useMedicationWidgetMethod = [regex]::Match($script, '(?s)bool UseMedicationWidget\(string widgetName\).*?protected void StopSaline').Value
foreach ($obsolete in 'if (index >= SALINE_FIRST_INDEX)', 'm_IsSelfSalineMenuOpen = false', 'm_IsForeignSalineMenuOpen = false') {
	if ($useMedicationWidgetMethod.Contains($obsolete)) {
		throw "Saline submenu closes after administering saline: $obsolete"
	}
}
foreach ($widget in 'MedicationStopSalineRow', 'MedicationStopSalineButton', 'MedicationStopSalineLabel', 'MedicationStopSalineRowForeign', 'MedicationStopSalineButtonForeign', 'MedicationStopSalineLabelForeign') {
	if (-not $medication.Contains(('Name "{0}"' -f $widget))) {
		throw "Stop-saline control is missing: $widget"
	}
}
foreach ($binding in 'm_StopSalineRow.SetVisible(m_IsSelfSalineMenuOpen)', 'm_StopSalineRowForeign.SetVisible(foreignInteraction && m_IsForeignSalineMenuOpen)', 'ESB_PartialSaline.CanStop(s_Patient)', 'RAMI_RequestStopSaline(damageManager)', 'RAMI_RpcAsk_StopSaline', 'RAMI_IsTreatmentTargetInRange(damageManager.GetOwner())', 'ESB_PartialSaline.Stop(damageManager.GetOwner(), GetControlledEntity())') {
	if (-not $script.Contains($binding)) {
		throw "Stop-saline action is not wired: $binding"
	}
}
$stopSalineMenuMethod = [regex]::Match($script, '(?s)protected void StopSaline\(\).*?protected void UpdateTreatmentItemsForInventory').Value
if ($stopSalineMenuMethod.Contains('performer.StartAction(action)')) {
	throw 'Stop-saline must execute directly without a user action'
}
foreach ($obsolete in 'RAMI_GetTreatmentAnimationParameters', 'm_RAMI_StopSalineAnimationParameters', 'RAMI_OnStopSalineAnimationEnded', 'RAMI_FinishStopSalineAnimation', 'animationComponent.CallCommand') {
	if ($script.Contains($obsolete)) {
		throw "Stop-saline animation code remains: $obsolete"
	}
}
foreach ($binding in '(?s)Name "MedicationStopSalineRow"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.458333 1 0\.534722', '(?s)Name "MedicationStopSalineRowForeign"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0 0\.9399 1 0\.9999') {
	if (-not [regex]::IsMatch($medication, $binding)) {
		throw 'Stop-saline control is not full-width in both Saline submenus'
	}
}
$closeMenuMethod = [regex]::Match($script, '(?s)void CloseMenu\(\).*?void TogglePatient').Value
foreach ($binding in 'm_IsSelfSalineMenuOpen || m_IsForeignSalineMenuOpen', 'm_IsSelfSalineMenuOpen = false', 'm_IsForeignSalineMenuOpen = false', 'UpdateMedicationItems()', 'return;') {
	if (-not $closeMenuMethod.Contains($binding)) {
		throw "Close action does not collapse Saline submenu: $binding"
	}
}
$bindPageWidgetsMethod = [regex]::Match($script, '(?s)protected void BindPageWidgets\(\).*?protected void UpdateTriagePage').Value
if ($bindPageWidgetsMethod.Contains('UpdateSalineMenuVisibility();')) {
	throw 'Saline visibility runs while non-medication page widgets are being bound'
}
foreach ($widget in 'PatientEntry', 'PatientOverlay', 'PatientName', 'PatientCareOutline') {
	if (-not $patientRow.Contains(('Name "{0}"' -f $widget))) {
		throw "Patient row is missing: $widget"
	}
}
foreach ($binding in 'Slot LayoutSlot', 'Padding 0 5 0 5', 'OverlayWidgetClass', 'Slot ButtonWidgetSlot', 'Padding 14 14 14 14', 'Color 1 1 1 0.9', '"Horizontal Alignment" Center') {
	if (-not $patientRow.Contains($binding)) {
		throw "Patient-list spacing is missing: $binding"
	}
}
if ($patientRow.Contains('Name "PatientMarker"') -or $patientRow.Contains('Text "+"')) {
	throw 'Patient-list marker must not displace the centered patient name'
}
foreach ($binding in 'Color 1 0 0 1', 'style outline_4px') {
	if (-not $patientRow.Contains($binding)) {
		throw "Patient-list medical-care outline is missing: $binding"
	}
}
if ($patientRow.Contains('Visible 0')) {
	throw 'Patient-list medical-care outline uses unsupported layout visibility serialization'
}
if ($patientRow -notmatch '(?s)Name "PatientOverlay"\s+Slot ButtonWidgetSlot.*?HorizontalAlign 3.*?VerticalAlign 3') {
	throw 'Patient-list medical-care overlay does not fill the patient button'
}
if ($patientRow -notmatch '(?s)Name "PatientCareOutline"\s+Slot OverlayWidgetSlot.*?HorizontalAlign 3.*?VerticalAlign 3') {
	throw 'Patient-list medical-care outline does not fill the patient-button overlay'
}
if ($patientRow.Contains('VerticalLayoutWidgetSlot')) {
	throw 'Patient row uses unsupported VerticalLayoutWidgetSlot'
}
foreach ($binding in 'OnMouseEnter(Widget w, int x, int y)', 'OnMouseLeave(Widget w, Widget enterW, int x, int y)', 'SetPatientButtonHovered(w, true)', 'SetPatientButtonHovered(w, false)', 'UpdatePatientButtonColors()', 'm_ListedPatients[index] == s_Patient', 'm_ListedPatients[index] == m_PatientActionTarget', 'PatientIsBleeding(m_ListedPatients[index])', 'careOutline.SetVisible(false)', 'careOutline.SetVisible(isBleeding && !isDead)', 'controller.GetLifeState() == ECharacterLifeState.INCAPACITATED', 'controller.GetLifeState() == ECharacterLifeState.DEAD', 'Color.FromSRGBA(51, 179, 255, 153)', 'button.SetColor(Color.Black)', 'Color.FromSRGBA(128, 128, 128, 230)', 'Color.FromSRGBA(255, 255, 255, 230)', 'Color.FromSRGBA(192, 192, 192, 230)', 'nameText.SetBold(highlighted || isUnconscious)', 'if (highlighted || isUnconscious)', 'nameText.SetColor(Color.White)') {
	if (-not $script.Contains($binding)) {
		throw "Patient-list hover or selection color is missing: $binding"
	}
}
foreach ($binding in 'protected bool PatientIsBleeding(SCR_ChimeraCharacter patient)', 'damageManager.GetBloodHitZone()', 'blood.GetTotalBleedingAmount() > 0') {
	if (-not $script.Contains($binding)) {
		throw "Patient-list bleeding detection is missing: $binding"
	}
}
$patientColorMethod = [regex]::Match($script, '(?s)protected void UpdatePatientButtonColors\(\).*?protected bool PatientIsBleeding').Value
if ($patientColorMethod -notmatch 'else if \(isUnconscious\)\s*button\.SetColor\(Color\.FromSRGBA\(128, 128, 128, 230\)\);' -or $patientColorMethod.Contains('Color.FromSRGBA(0, 0, 0, 150)')) {
	throw 'Unconscious patient buttons are not grey'
}
$outlineState = $patientColorMethod.IndexOf('careOutline.SetVisible(isBleeding && !isDead)')
$deadState = $patientColorMethod.IndexOf('if (isDead)', $outlineState)
$unconsciousState = $patientColorMethod.IndexOf('else if (isUnconscious)', $deadState)
$highlightedState = $patientColorMethod.IndexOf('else if (highlighted)', $unconsciousState)
if ($outlineState -lt 0 -or $deadState -lt 0 -or $unconsciousState -lt 0 -or $highlightedState -lt 0) {
	throw 'Patient-list dead, unconscious, bleeding, and highlighted state priority is incorrect'
}
if ($script -notmatch 'if \(!changed\)\s*\{\s*UpdatePatientButtonColors\(\);\s*UpdatePatientActionButtons\(\);\s*return;') {
	throw 'Patient-list medical-state color is not refreshed while the patient set remains unchanged'
}
foreach ($binding in 'SetRequestedButtonHovered(w, true)', 'SetRequestedButtonHovered(w, false)', 'GetRequestedButton(Widget widget)', 'UpdateRequestedButtonStyles()', 'ApplyRequestedButtonStyle(m_CloseButton, m_CloseText, null, false)', 'ApplyRequestedButtonStyle(m_DiagnoseHeartRateButton, m_DiagnoseHeartRateText, null, true)', 'ApplyRequestedButtonStyle(m_DiagnoseBloodPressureButton, m_DiagnoseBloodPressureText, null, true)', 'ApplyRequestedButtonStyle(m_AdvancedCPRButton, m_AdvancedCPRText, null, true)', 'ApplyRequestedButtonStyle(m_MedicationButtons[index], m_MedicationLabels[index], m_MedicationCounts[index], true)', 'ApplyRequestedButtonStyle(m_SalineToggleButton, m_SalineToggleLabel, m_SalineToggleCount, true)', 'button == m_DiagnoseHeartRateButton && m_IsHeartRateMonitored', 'button == m_DiagnoseBloodPressureButton && m_IsBloodPressureMonitored', 'text.SetBold(baseBold || highlighted)', 'text.SetColor(Color.White)') {
	if (-not $script.Contains($binding)) {
		throw "Requested cyan button hover or active styling is missing: $binding"
	}
}
foreach ($binding in 'CompartmentAccessComponent.GetVehicleIn(user)', 'vehicle == CompartmentAccessComponent.GetVehicleIn(target)', 'SCR_CharacterRegistrationComponent.GetChimeraCharacters()', 'playerController.RAMI_IsTreatmentTargetInRange(character)', 'workspace.CreateWidgets(PATIENT_ROW_LAYOUT, m_PatientList)', 'm_Menu.SelectListedPatient(w)', 's_Patient = patient') {
	if (-not $script.Contains($binding)) {
		throw "Patient-list treatment is not wired: $binding"
	}
}

if (-not $script.Contains('m_TreatmentStatus.SetText("TREATMENT STARTED")') -or $script.Contains("ClearPendingTreatment();`n`t`t`tCloseMenu();")) {
	throw 'RAMI must stay open after a treatment starts'
}

if ($script.Contains('GetCurrentItemInHands()') -or $script.Contains('GetItemInHandSlot()')) {
	throw 'Medical gadgets must be checked through the active inventory item'
}

if ($treatmentControls.Contains('Name "TreatButton"') -or $script.Contains('TreatSelectedRegion()')) {
	throw 'Obsolete Treat selected region flow remains'
}

foreach ($widget in 'BodyModel', 'PatientVitals', 'SelectedRegion', 'PageTitle') {
	if (-not $treatment.Contains("Name `"$widget`"")) {
		throw "Treatment layout is missing: $widget"
	}
}

foreach ($pageLayout in 'RAMI_MedicalTriage.layout', 'RAMI_MedicalDiagnose.layout', 'RAMI_MedicalTreatment.layout', 'RAMI_MedicalTreatmentControls.layout', 'RAMI_MedicalMedication.layout', 'RAMI_MedicalAdvanced.layout') {
	if (-not (Test-Path (Join-Path $root "UI/layouts/$pageLayout")) -or -not $script.Contains($pageLayout)) {
		throw "RAMI page layout is not loadable: $pageLayout"
	}
}

if (-not $layout.Contains('Name "ContentArea"') -or -not $script.Contains('workspace.CreateWidgets(layout, m_ContentArea)')) {
	throw 'ContentArea page loading is not wired'
}
if ($layout -notmatch 'Name "ContentArea"\s+Slot FrameWidgetSlot "[^"]+" \{\s+Anchor 0\.03 0\.18 0\.97 0\.87\s+PositionX 0\s+OffsetLeft 0\s+PositionY 0\s+OffsetTop 0\s+SizeX 0\s+OffsetRight 0\s+SizeY 0\s+OffsetBottom 0') {
	throw 'ContentArea does not provide the exact 1804.8 x 745.2 layout area at 1920 x 1080'
}
if (-not $treatment.Contains('Name "InventoryFrame"') -or $treatment.Contains('Name "BandageButton"') -or $script.Contains('FrameSlot.SetAnchorMax(m_InventoryRoot')) {
	throw 'Inventory controls are not isolated in the layout-owned frame'
}

foreach ($binding in 'if (widgetName == "PageToggleSelf")', 'm_Menu.TogglePatient();', 'm_IsPatientWindowOpen = !m_IsPatientWindowOpen', 'm_PatientWindow.SetVisible(m_IsPatientWindowOpen)', 'm_PatientWindow.SetVisible(false)') {
	if (-not $script.Contains($binding)) {
		throw "PageToggleSelf does not toggle the patient window: $binding"
	}
}

foreach ($binding in 'Action RAMI_ToggleMedicalMenu', 'Action RAMI_CloseMedicalMenu', 'Input "keyboard:KC_ESCAPE"', 'Input "keyboard:KC_H"', 'InputSourceCombo', 'FilterPreset "click"', 'FilterPreset "gamepad:click"', 'InputFilterClick', 'Input "gamepad0:shoulder_right"', 'Input "gamepad0:pad_right"', 'Input "gamepad0:b"') {
	if (-not $inputConfig.Contains($binding)) {
		throw "RAMI menu close input is missing: $binding"
	}
}
$toggleAction = [regex]::Match($inputConfig, '(?s)Action RAMI_ToggleMedicalMenu \{.*?\n  \}').Value
if ($toggleAction.Contains('InputFilterHold')) {
	throw 'RAMI opening combo must click D-pad Right while Right Shoulder is held'
}
if (-not $keyBindingMenu.Contains('m_sPreset "click"') -or $keyBindingMenu.Contains('m_sPresetGamepadOptional') -or $keyBindingMenu.Contains('m_sActionNameGamepadOptional')) {
	throw 'RAMI controls entry does not use the native shared click preset'
}
$closeAction = [regex]::Match($inputConfig, '(?s)Action RAMI_CloseMedicalMenu \{.*?\n  \}').Value
if ($closeAction.Contains('keyboard:KC_H')) {
	throw 'RAMI close action still hardcodes H instead of using the mapped toggle action'
}
foreach ($binding in 'Action RAMI_PreviousMedicalPage', 'Input "gamepad0:left_trigger"', 'Action RAMI_NextMedicalPage', 'Input "gamepad0:right_trigger"') {
	if (-not $inputConfig.Contains($binding)) {
		throw "RAMI controller page-cycle input is missing: $binding"
	}
}
foreach ($binding in 'Action RAMI_MedicalMenuModifier', '"RAMI_MedicalMenuModifier"') {
	if (-not $inputConfig.Contains($binding)) {
		throw "RAMI shoulder modifier input is missing: $binding"
	}
}
if (-not $inputConfig.Contains('ActionContext IngameContext')) {
	throw 'RAMI opening input is not available in vehicles through IngameContext'
}
if ($menuContext.Contains('"RAMI_MedicalMenuModifier"')) {
	throw 'RAMI shoulder modifier must not remain active inside the medical menu'
}
foreach ($binding in 'class RAMI_AvailableActionContext : SCR_AvailableActionContext', 'm_sAction = "RAMI_ToggleMedicalMenu"', 'm_sName = "', 'GetActionValue("RAMI_MedicalMenuModifier")', 'modded class SCR_AvailableActionsDisplay', 'm_aActions.Insert(m_RAMI_AvailableAction)', 'FindMenuByPreset(ChimeraMenuPreset.RAMI_MedicalMenu)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI shoulder control hint is missing: $binding"
	}
}
foreach ($obsoleteBinding in 'SCR_HintUIInfo.CreateInfo(', 'SCR_HintManagerComponent.ShowHint(', 'SCR_HintManagerComponent.HideHint(') {
	if ($script.Contains($obsoleteBinding)) {
		throw "RAMI shoulder control hint still uses normal hint UI: $obsoleteBinding"
	}
}
foreach ($binding in '"RAMI_PreviousMedicalPage"', '"RAMI_NextMedicalPage"') {
	if (-not $menuContext.Contains($binding)) {
		throw "RAMI controller page-cycle MenuContext action is missing: $binding"
	}
}
foreach ($binding in 'AddActionListener("RAMI_PreviousMedicalPage", EActionTrigger.DOWN, ShowPreviousPage)', 'AddActionListener("RAMI_NextMedicalPage", EActionTrigger.DOWN, ShowNextPage)', 'RemoveActionListener("RAMI_PreviousMedicalPage", EActionTrigger.DOWN, ShowPreviousPage)', 'RemoveActionListener("RAMI_NextMedicalPage", EActionTrigger.DOWN, ShowNextPage)', 'pageNames.Find(m_CurrentPageName)', 'm_CurrentPageName = pageName', 'nextIndex = pageNames.Count() - 1', 'nextIndex = 0', 'workspace.SetFocusedWidget(pageButton)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI controller page cycling is missing: $binding"
	}
}
if (-not $triage.Contains('SCR_GamepadScrollComponent')) {
	throw 'RAMI triage activity scroll is missing its native gamepad component'
}

if (-not $script.Contains('workspace.SetFocusedWidget(GetRootWidget().FindAnyWidget("PageBandages"))')) {
	throw 'RAMI menu initial controller focus is missing'
}

foreach ($binding in 'GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD', 'workspace.GetFocusedWidget()', 'FindAnyWidget("MedicalPanel")', 'workspace.CreateWidgets(CONTROLLER_FOCUS_LAYOUT, focusParent)', 'focused.GetScreenPos(focusedX, focusedY)', 'focusParent.GetScreenSize(parentWidth, parentHeight)', 'FrameSlot.SetAnchorMin(m_ControllerFocus', 'FrameSlot.SetAnchorMax(m_ControllerFocus', 'm_ControllerFocus.SetVisible(true)', 'WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS', 'm_ControllerFocus.SetZOrder(100)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI controller focus indicator is missing: $binding"
	}
}
if (-not $controllerFocus.Contains('Slot FrameWidgetSlot') -or $controllerFocus.Contains('Slot ButtonWidgetSlot')) {
	throw 'RAMI controller focus indicator must attach to the global frame'
}
if (($controllerFocus | Select-String -Pattern 'PanelWidgetClass' -AllMatches).Matches.Count -ne 1) {
	throw 'RAMI controller focus indicator must use one highlight'
}
foreach ($binding in 'ConfigureControllerNavigation(pageName)', 'WidgetNavigationDirection.LEFT', 'WidgetNavigationDirection.RIGHT', 'WidgetNavigationDirection.UP', 'WidgetNavigationDirection.DOWN', 'WidgetNavigationRuleType.EXPLICIT', 'WidgetNavigationRuleType.STOP', 'SetControllerNavigation("BodyHead"', 'SetControllerNavigation("BodyRightLeg"') {
	if (-not $script.Contains($binding)) {
		throw "RAMI explicit controller navigation is missing: $binding"
	}
}
if (-not $script.Contains('CallLater(ConfigureControllerNavigation, 1, false, pageName)') -or -not $script.Contains('case "PageDiagnose": firstPageControl = "DiagnoseHeartRateButton"')) {
	throw 'RAMI diagnose navigation is not refreshed after dynamic page creation'
}
if ($script.Contains('override bool OnFocus(Widget w, int x, int y)')) {
	throw 'RAMI controller page tabs must not switch pages merely from focus'
}
if ($script.Contains('FocusControllerPageContent')) {
	throw 'RAMI controller page selection must not move focus on OK'
}
foreach ($binding in 'AddActionListener("MenuDown", EActionTrigger.DOWN, NavigateDownFromPageTab)', 'case "PageDiagnose": targetName = "DiagnoseHeartRateButton"', 'case "PageTriage": targetName = "TriageLevelNone"', 'case "PageAdvanced":', 'actionButton.IsEnabled()', 'targetName = "CloseButton"', 'FindAnyWidget(targetName)', 'RemoveActionListener("MenuDown", EActionTrigger.DOWN, NavigateDownFromPageTab)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI dynamic page tab down navigation is missing: $binding"
	}
}
if (-not $triage.Contains('Name "TriageActivityFocus"')) {
	throw 'RAMI triage activity scroll focus proxy is missing'
}
foreach ($binding in 'ButtonWidget m_TriageActivityFocus', 'SetControllerNavigationRow(triageButtons, "PageTriage", "TriageActivityFocus")', 'm_TriageActivityFocus.SetFlags(m_TriageActivityFocus.GetFlags() | WidgetFlags.IGNORE_CURSOR)', 'm_ControllerFocus.SetOpacity(0.35)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI triage activity controller scrolling is missing: $binding"
	}
}
foreach ($unsafeBinding in 'new SCR_GamepadScrollComponent()', 'm_TriageActivityGamepadScroll.Update') {
	if ($script.Contains($unsafeBinding)) {
		throw "RAMI triage activity scroll must use an attached widget component: $unsafeBinding"
	}
}
foreach ($binding in 'SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.RIGHT, "BodyHead")', 'SetControllerNavigation("BodyHead", WidgetNavigationDirection.UP, "PageToggleSelf")', 'SetControllerNavigation("BodyHead", WidgetNavigationDirection.LEFT, "PageToggleSelf")', 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.LEFT, "PageToggleSelf")', 'UpdateTreatmentItemNavigation()', 'itemButton.IsEnabled()', 'SetControllerNavigation(pageButtonName, WidgetNavigationDirection.DOWN, firstItem)', 'SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.LEFT, firstItem)', 'SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.LEFT, firstItem)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI extended controller navigation is missing: $binding"
	}
}
foreach ($binding in 'SetControllerNavigation("PageToggleSelf", WidgetNavigationDirection.DOWN, firstItem)', 'SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.DOWN, "CloseButton")', 'SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.RIGHT, "CloseButton")', 'SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.RIGHT, "CloseButton")', 'SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.DOWN, "CloseButton")') {
	if (-not $script.Contains($binding)) {
		throw "RAMI close-edge controller navigation is missing: $binding"
	}
}
foreach ($binding in 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.LEFT, "BodyLeftLeg")', 'SetControllerNavigation("TriageActivityFocus", WidgetNavigationDirection.DOWN, "CloseButton")', 'SetControllerNavigationRow(diagnoseButtons, "PageDiagnose", "CloseButton")', 'SetControllerNavigationRow(movementButtons, string.Empty, "CloseButton")') {
	if (-not $script.Contains($binding)) {
		throw "RAMI page-end close navigation is missing: $binding"
	}
}
foreach ($binding in 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "BodyRightLeg")', 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "TriageActivityFocus")', 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "DiagnoseBloodPressureButton")', 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, "AdvancedLoadVehicleButton")') {
	if (-not $script.Contains($binding)) {
		throw "RAMI close return navigation is missing: $binding"
	}
}
foreach ($binding in 'string closeUpTarget = "PageAdvanced"', 'actionButton && actionButton.IsEnabled()', 'closeUpTarget = actionButton.GetName()', 'SetControllerNavigation("CloseButton", WidgetNavigationDirection.UP, closeUpTarget)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI Actions close-up fallback is missing: $binding"
	}
}
foreach ($binding in 'SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.LEFT, string.Empty)', 'SetControllerNavigation("BodyAbdomen", WidgetNavigationDirection.RIGHT, string.Empty)', 'SetControllerNavigation("BodyLeftLeg", WidgetNavigationDirection.UP, "BodyAbdomen")', 'SetControllerNavigation("BodyRightLeg", WidgetNavigationDirection.UP, "BodyAbdomen")') {
	if (-not $script.Contains($binding)) {
		throw "RAMI abdomen controller navigation is missing: $binding"
	}
}
foreach ($binding in 'SetControllerNavigation("BodyLeftArm", WidgetNavigationDirection.DOWN, string.Empty)', 'SetControllerNavigation("BodyRightArm", WidgetNavigationDirection.DOWN, string.Empty)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI arm controller navigation is missing: $binding"
	}
}
if (-not $controllerFocus.Contains('Color 0 1 1 0.5') -or -not $script.Contains('FrameSlot.SetOffsets(m_ControllerFocus, 0, 0, 0, 0)')) {
	throw 'RAMI controller focus highlight is not prominent enough'
}
foreach ($binding in 'm_Menu.TreatListedPatient()', 'm_Menu.UnloadListedPatient()', 'm_PatientActions.SetVisible(true)', 'm_PatientActions.SetVisible(false)', 'm_PatientActions.IsVisible() && m_PatientActionTarget == patient', 'HidePatientActions()', 'UpdatePatientButtonVisibility()', 'm_PatientButtons[index].SetVisible(!showSelectedOnly || m_ListedPatients[index] == m_PatientActionTarget)', 'protected SCR_RemoveCasualtyUserAction FindRemoveCasualtyAction()', 'compartmentAccess.GetCompartment()', 'compartment.GetOccupant() != m_PatientActionTarget', 'SCR_RemoveCasualtyUserAction.Cast(compartment.GetGetOutAction())', 'FindRemoveCasualtyActionInManager(compartment.GetOwner(), compartment)', 'actionsManager.GetActionsList(actions)', 'action.GetCompartmentSlot() == compartment', 'compartment.GetAddUserActions(actions)', 'SCR_RemoveCasualtyUserAction.Cast(baseAction)', 'PatientIsUnconscious(m_PatientActionTarget)', 'controller.GetLifeState() == ECharacterLifeState.INCAPACITATED', 'performer.PerformAction(action)') {
	if (-not $script.Contains($binding)) {
		throw "Patient-list action choice is not wired: $binding"
	}
}
foreach ($binding in 'downTarget = "PatientActionTreatButton"', 'string treatDownTarget = "CloseButton"', 'if (m_PatientUnloadButton.IsEnabled())', 'treatDownTarget = "PatientActionUnloadButton"', 'SetControllerNavigation("PatientActionTreatButton", WidgetNavigationDirection.DOWN, treatDownTarget)') {
	if (-not $script.Contains($binding)) {
		throw "Patient-list controller action navigation is missing: $binding"
	}
}
foreach ($obsoleteBinding in 'RAMI_RequestUnloadPatient', 'RAMI_RpcAsk_UnloadPatient', 'RAMI_UnloadPatient', 'GetOutVehicle_NoDoor(exitTransform', 'ACE_MoveOutVehicle(exitTransform') {
	if ($script.Contains($obsoleteBinding)) {
		throw "Custom patient unloading path remains: $obsoleteBinding"
	}
}
foreach ($binding in 'Text "TREAT"', 'Text "UNLOAD FROM VEHICLE"') {
	if (-not $layout.Contains($binding)) {
		throw "Patient-list action choice label is missing: $binding"
	}
}
if (-not $layout.Contains('Name "ActivePageHighlight"') -or -not $layout.Contains('Color 0.2 0.7 1 1') -or -not $layout.Contains('style outline_4px') -or -not $script.Contains('Color.FromSRGBA(51, 179, 255, 153)')) {
	throw 'RAMI active page outline is missing'
}
foreach ($binding in 'UpdateActivePageHighlight(pageName)', 'FrameSlot.SetAnchorMin(m_ActivePageHighlight', 'FrameSlot.SetAnchorMax(m_ActivePageHighlight', 'FrameSlot.SetOffsets(m_ActivePageHighlight, 0, 0, 0, 0)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI active page outline is not wired: $binding"
	}
}
foreach ($outline in 'BodyZoneOutlineLeft', 'BodyZoneOutlineRight', 'BodyZoneOutlineTop', 'BodyZoneOutlineBottom', 'BodyZoneOutlineMask') {
	if (-not $treatment.Contains("Name `"$outline`"")) {
		throw "RAMI selected body-zone outline is missing: $outline"
	}
}
if ([regex]::Matches($treatment, '(?s)Name "BodyZoneOutline(?:Left|Right|Top|Bottom)".*?Color 0\.2 0\.7 1 1').Count -ne 4) {
	throw 'RAMI selected body-zone outline does not match the vitals and medications headings'
}
foreach ($binding in 'UpdateSelectedBodyRegionHighlight()', 'FrameSlot.Move(m_BodyZoneOutlines[0], -3, 0)', 'm_BodyZoneOutlineMask.LoadImageTexture(0, texture)', 'outline.LoadImageTexture(0, texture)', 'outline.SetOpacity(0.9)', 'm_BodyZoneOutlineMask.SetColor(selectedZone.GetColor())') {
	if (-not $script.Contains($binding)) {
		throw "RAMI selected body-zone outline is not wired: $binding"
	}
}
foreach ($binding in 'string upTarget = "PageBandages"', 'upTarget = "PageMedication"') {
	if (-not $script.Contains($binding)) {
		throw "RAMI item navigation does not return to its page tab: $binding"
	}
}

foreach ($binding in 'AddActionListener("RAMI_CloseMedicalMenu"', 'RemoveActionListener("RAMI_CloseMedicalMenu"', 'CloseMenuByPreset(ChimeraMenuPreset.RAMI_MedicalMenu)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI menu close listener is missing: $binding"
	}
}
foreach ($binding in 'AddActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.UP, CloseMenu)', 'RemoveActionListener("RAMI_ToggleMedicalMenu", EActionTrigger.UP, CloseMenu)') {
	if (-not $script.Contains($binding)) {
		throw "RAMI mapped close listener is missing: $binding"
	}
}
foreach ($binding in 'CallLater(RegisterCloseActions, 250)', 'Remove(RegisterCloseActions)', 'AddActionListener("RAMI_CloseMedicalMenu", EActionTrigger.UP, CloseMenu)') {
	if (-not $script.Contains($binding)) {
		throw "Menu key-release close wiring is missing: $binding"
	}
}

if (-not $menuConfig.Contains('ActionContext "MenuContext"') -or -not $menuContext.Contains('"RAMI_CloseMedicalMenu"') -or -not $menuContext.Contains('"RAMI_ToggleMedicalMenu"')) {
	throw 'RAMI close action is not registered in MenuContext'
}

if ($inputConfig.Contains('Action MenuBack') -or $script.Contains('modded class PauseMenuUI')) {
	throw 'RAMI must not modify global menu-back or pause-menu behavior'
}

if ($inputConfig.Contains('ActionContext RAMI_MedicalMenuContext')) {
	throw 'Unused RAMI menu context remains'
}

if (-not $script.Contains('RAMI_MedicalMenuUI.OpenMedicalMenu();') -or $script.Contains('RAMI_MedicalMenuUI.ToggleMedicalMenu();') -or $script.Contains('s_CurrentMenu')) {
	throw 'Unused RAMI toggle state remains'
}

foreach ($binding in 'workspace.GetScreenSize(width, height)', 'GetMouseDeviceHandler().SetCursorPosition(Math.Round(width * 0.5), Math.Round(height * 0.5))') {
	if (-not $script.Contains($binding)) {
		throw "Menu cursor centering is missing: $binding"
	}
}

Write-Output 'RAMI menu page wiring passed.'
