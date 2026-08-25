$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$prefabRoot = Join-Path $root 'Prefabs\Items\Medicine\SalineBag_01'
$volumes = 250, 500, 1000, 1250, 1500

foreach ($faction in 'US', 'USSR') {
    foreach ($volume in $volumes) {
        $content = Get-Content -Raw -Encoding utf8 (Join-Path $prefabRoot "SalineBag_${faction}_${volume}.et")
        if ($content -notmatch "Name `"Saline $volume`"" -or
            $content -notmatch "m_fItemAbsoluteRegenerationAmount $volume" -or
            $content -match 'ESB_ConsumableSalineBag|m_sRemainingPrefab') {
            throw "Invalid saline prefab: ${faction} ${volume} ml"
        }
    }
}

$script = Get-Content -Raw -Encoding utf8 (Join-Path $root 'Scripts\Game\ESB_ConsumableSalineBag.c')
foreach ($volume in 250, 500, 750, 1000, 1250, 1500) {
    if ($script -notmatch "case $volume") {
        throw "Missing remaining-prefab mapping: $volume ml"
    }
}

if ($script -notmatch '/ 250\) \* 250' -or
    $script -notmatch 'Saline stoppen' -or
    $script -notmatch 'Stop saline' -or
    $script -notmatch 'GetActionProgressScript') {
    throw 'Manual 250 ml stop logic is incomplete'
}

$character = Get-Content -Raw -Encoding utf8 (Join-Path $root 'Prefabs\Characters\Core\Character_Base.et')
if ($character -notmatch 'SCR_SalineBagUserAction' -or $character -notmatch 'Duration 3') {
    throw 'Saline stop action must use the native three-second progress circle'
}

if (-not (Test-Path (Join-Path $root 'Prefabs\Characters\Core\Character_Base.et.meta'))) {
    throw 'Character_Base.et must be registered in Workbench'
}

foreach ($faction in 'US', 'USSR') {
    $catalog = Get-Content -Raw -Encoding utf8 (Join-Path $root "Configs\EntityCatalog\$faction\InventoryItems_EntityCatalog_${faction}.conf")
    foreach ($volume in 250, 500, 1000, 1250, 1500) {
        $matches = [regex]::Matches($catalog, "SalineBag_${faction}_${volume}\.et")
        if ($matches.Count -ne 1) {
            throw "Expected one arsenal entry for ${faction} ${volume} ml, found $($matches.Count)"
        }
    }

    foreach ($volume in 250, 1250) {
        if ($catalog -notmatch "SalineBag_${faction}_${volume}\.et[\s\S]{0,300}m_eItemType HEAL[\s\S]{0,100}m_eItemMode CONSUMABLE") {
            throw "Missing arsenal metadata for ${faction} ${volume} ml"
        }
    }
}

Write-Output 'Manual 250 ml saline stop configuration OK'
