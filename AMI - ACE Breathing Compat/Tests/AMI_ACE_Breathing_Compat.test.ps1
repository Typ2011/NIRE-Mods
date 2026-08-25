$ErrorActionPreference = 'Stop'

$root = Split-Path $PSScriptRoot -Parent
$project = Get-Content -Raw (Join-Path $root 'addon.gproj')
$script = Get-Content -Raw (Join-Path $root 'Scripts\Game\AMI_ACE_Breathing_Compat\RAMI_MedicalMenuUI.c')

function Assert-Contains([string]$Text, [string]$Value) {
    if (-not $Text.Contains($Value)) { throw "Missing: $Value" }
}

Assert-Contains $project 'TITLE "AMI ACE Breathing Compat"'
foreach ($dependency in '58D0FB3206B6F859', 'D36C5B996922FFDE', '671F73D99978B4F2') {
    Assert-Contains $project $dependency
}

foreach ($type in @(
    'ACE_MEDICAL_CHEST_SEAL',
    'ACE_MEDICAL_NCD_KIT',
    'ACE_MEDICAL_LARYNGEAL_TUBE',
    'ACE_MEDICAL_OXYGEN_MASK'
)) {
    Assert-Contains $script $type
}

foreach ($binding in @(
    'AMIACECanApplyBreathingTypeToSelectedRegion',
    'type == SCR_EConsumableType.ACE_MEDICAL_LARYNGEAL_TUBE || type == SCR_EConsumableType.ACE_MEDICAL_OXYGEN_MASK',
    'return m_SelectedRegion == ECharacterHitZoneGroup.HEAD;'
)) {
    Assert-Contains $script $binding
}

Assert-Contains $script 'PAGE_TREATMENT_CONTROLS_LAYOUT'
Assert-Contains $script 'AMIACEPrepareRowTemplate'

foreach ($binding in @(
    'ACE_Medical_TiltHeadUserAction',
    'ACE_Medical_ClearVomitAction',
    'AMIACECreateAirwayControls',
    'PAGE_ADVANCED_LAYOUT',
    '#ACE_Medical-UserAction_TiltHead',
    '#ACE_Medical-UserAction_ClearVomit',
    'UseAdvancedAction(widgetName)'
)) {
    Assert-Contains $script $binding
}

foreach ($binding in @(
    'AMIACECreateDiagnoseControls',
    'PAGE_DIAGNOSE_LAYOUT',
    'GetRespiratoryRate()',
    'GetSpO2()',
    'AMIACERequestBreathingVitals',
    'RplRcver.Server',
    'RplRcver.Owner',
    'MONITOR RESPIRATORY RATE',
    'MONITOR OXYGEN SATURATION'
)) {
    Assert-Contains $script $binding
}

foreach ($binding in @(
    'AMIACECreateBreathingStatusIcons',
    'ACE_Medical_PneumothoraxInfo.layout',
    'Lungs_UI',
    'TPTX_UI',
    'IsAirwayObstructed()',
    'IsAirwayOccluded()',
    'GetPneumothoraxScale()',
    'HasTensionPneumothorax()',
    'Color.FromSRGBA(255, 255, 0, 255)',
    'FrameSlot.SetPos(root, -64, 0);',
    'm_PageRoot.FindAnyWidget("BodyModel")',
    '"Lungs_UI", 0.7, 0.1',
    '"TPTX_UI", 0.8, 0.1'
)) {
    Assert-Contains $script $binding
}

foreach ($binding in @(
    'm_AMIACERespiratoryRateButton.SetColor(Color.FromSRGBA(51, 179, 255, 153));',
    'm_AMIACESpO2Button.SetColor(Color.FromSRGBA(51, 179, 255, 153));'
)) {
    Assert-Contains $script $binding
}

$amiRoot = Resolve-Path (Join-Path $root '..\RAMI_AdvancedMedicalInterface')
$amiCode = Get-ChildItem -LiteralPath $amiRoot -Recurse -File | Where-Object Extension -In '.c', '.gproj' |
    Get-Content -Raw
$forbidden = $amiCode | Select-String '671F73D99978B4F2|ACE_Medical_Breathing'
if ($forbidden) { throw 'AMI references ACE Medical Breathing.' }

Write-Output 'AMI ACE Breathing Compat checks passed.'
