[CmdletBinding()]
param(
	[switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path $PSScriptRoot -Parent
$workbenchAddonsRoot = Split-Path (Split-Path $projectRoot -Parent) -Parent
$aceSourceRoot = Join-Path $workbenchAddonsRoot 'ACE-Anvil-dev\addons'
$workbenchExecutable = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe'
$gameAddons = 'C:\Program Files (x86)\Steam\steamapps\common\Arma Reforger\addons'
$expandedSaline = Join-Path (Split-Path $projectRoot -Parent) 'Expanded Saline Bags'
$projectPath = Join-Path $projectRoot 'RAMI_AdvancedMedicalInterface.gproj'
$aceAddons = @('core', 'medical_core', 'medical_circulation', 'medical_hitzones', 'carrying')
$addonDirectories = @($gameAddons)

foreach ($addon in $aceAddons)
{
	$addonPath = Join-Path $aceSourceRoot $addon
	if (-not (Test-Path -LiteralPath (Join-Path $addonPath 'dev.gproj')))
	{
		throw "Missing official ACE dev project: $addonPath"
	}

	$addonDirectories += $addonPath
}

foreach ($requiredPath in @($workbenchExecutable, $gameAddons, (Join-Path $expandedSaline 'addon.gproj'), $projectPath))
{
	if (-not (Test-Path -LiteralPath $requiredPath))
	{
		throw "Missing Workbench dependency: $requiredPath"
	}
}

if ($ValidateOnly)
{
	Write-Output 'RAMI Workbench source dependencies are valid.'
	return
}

if (Get-Process -Name 'ArmaReforgerWorkbenchSteamDiag' -ErrorAction SilentlyContinue)
{
	throw 'Close Enfusion Workbench before using this launcher.'
}

$addonDirectories += $expandedSaline
$arguments = '-gproj "' + $projectPath + '" -addonsDir "' + ($addonDirectories -join ',') + '"'
Start-Process -FilePath $workbenchExecutable -ArgumentList $arguments
