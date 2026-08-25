$ErrorActionPreference = 'Stop'

$projectRoot = Join-Path $PSScriptRoot '..'
$source = Get-Content -Raw (Join-Path $projectRoot 'Scripts\Game\DGC_DisableGlobalChat.c')
$project = Get-Content -Raw (Join-Path $projectRoot 'DisableGlobalChat.gproj')
$stringTable = Get-Content -Raw (Join-Path $projectRoot 'Language\DGC_localization.st')
$requiredHooks = @(
	'modded class SCR_ChatChannel',
	'modded class FactionChatChannel',
	'modded class GroupChatChannel',
	'modded class LocalChatChannel',
	'modded class ServerChatChannel',
	'modded class SCR_VehicleChatChannel',
	'modded class SCR_ChatComponent',
	'modded class SCR_ChatPanel',
	'modded class SCR_AdditionalGameModeSettingsComponent',
	'!editorManager.IsLimited()',
	'SCR_Global.IsAdmin(playerId)',
	'[RplProp()]',
	'protected int m_iDGCAllowedChannels = DGC_EChatChannel.GROUP;',
	'if (!GetGameMode().IsMaster() || m_iDGCAllowedChannels == channels)',
	'Replication.BumpMe();',
	'static bool CanSendToGroup(BaseChatComponent sender)',
	'groupController && groupController.GetGroupID() != -1',
	'static bool CanUseChannel(BaseChatComponent sender, DGC_EChatChannel channel)',
	'static bool ProcessChannelMessage(BaseChatComponent sender, string message, bool isAuthority, DGC_EChatChannel channel)',
	'static bool IsChatControlCommand(string message)',
	'HandleChatControlCommand_S(sender, message);',
	'newChannels = currentChannels ^ channel;',
	'newChannels = DGC_EChatChannel.ALL;',
	'newChannels = 0;',
	'argument == "status"',
	'GetChannelFlagByName(argument)',
	'sender.SendPrivateMessage("[Disable Chat] " + Localize(message,',
	'void DGC_SendChatFeedback_S(string message, string param1 = ""',
	'[RplRpc(RplChannel.Reliable, RplRcver.Owner)]',
	'Rpc(RpcDo_DGC_ChatFeedback, message, param1,',
	'DGC_GameMasterAccess.Localize(message, param1,',
	'WidgetManager.Translate(message,',
	'"#DGC-Feedback_ChannelLocked"',
	'"#DGC-State_On"',
	'static DGC_EChatChannel GetChannelFlag(BaseChatChannel channel)',
	'channel.Type() == BaseChatChannel && channel.GetName() == "Global"',
	'static bool CanReceiveMessage(int channelId, int senderId)',
	'static bool IsAdminLoginCommand(string message)',
	'return message == "#login" || message.StartsWith("#login ");',
	'DGC_GameMasterAccess.IsChannelAllowed(DGC_EChatChannel.DIRECT)',
	'senderId == receiverId',
	'!DGC_GameMasterAccess.CanLocalPlayerUseChannel(m_ActiveChannel)',
	'DGC_SendChatControlCommand(input)',
	'modded class SCR_PlayerControllerGroupComponent',
	'void DGC_RequestChatControlCommand(string message)',
	'if (Replication.IsServer())',
	'RpcAsk_DGC_ChatControlCommand(message);',
	'Rpc(RpcAsk_DGC_ChatControlCommand, message);',
	'[RplRpc(RplChannel.Reliable, RplRcver.Server)]',
	'PlayerController controller = PlayerController.Cast(GetOwner());',
	'chatComponent = SCR_ChatComponent.Cast(controller.FindComponent(SCR_ChatComponent));',
	'DGC_GameMasterAccess.HandleChatControlCommand_S(chatComponent, message);',
	'SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();',
	'groupController.DGC_RequestChatControlCommand(message);',
	'manager.ShowHelpMessage(message);'
)

foreach ($hook in $requiredHooks)
{
	if (!$source.Contains($hook))
	{
		throw "Missing required chat guard: $hook"
	}
}

$languages = @('cs_cz', 'de_de', 'en_us', 'es_es', 'fr_fr', 'it_it', 'ja_jp', 'ko_kr', 'pl_pl', 'pt_br', 'ru_ru', 'uk_ua', 'zh_cn')
foreach ($language in $languages)
{
	if (!$stringTable.Contains("Target_$language"))
	{
		throw "Missing string-table target: $language"
	}

	if (!$project.Contains("Code `"$language`""))
	{
		throw "Missing project language registration: $language"
	}

	$runtimePath = Join-Path $projectRoot "Language\DGC_localization.$language.conf"
	if (!(Test-Path -LiteralPath $runtimePath))
	{
		throw "Missing runtime string table: $language"
	}

	$runtime = Get-Content -Raw -LiteralPath $runtimePath
	$idBlock = [regex]::Match($runtime, 'Ids\s*\{(?<Values>.*?)\}\s*Texts', 'Singleline').Groups['Values'].Value
	$textBlock = [regex]::Match($runtime, 'Texts\s*\{(?<Values>.*?)\}\s*\}', 'Singleline').Groups['Values'].Value
	if ([regex]::Matches($idBlock, '"DGC-[^"]+"').Count -ne 19 -or [regex]::Matches($textBlock, '"(?:[^"\\]|\\.)*"').Count -ne 19)
	{
		throw "Runtime string table does not contain 19 matching IDs and texts: $language"
	}
}

if ([regex]::Matches($stringTable, 'Id "DGC-').Count -ne 19)
{
	throw 'Source string table must contain exactly 19 localization IDs.'
}

if ($source.Contains('Nur Game Master und Server-Admins') -or $source.Contains('Chat-Befehl konnte nicht gesendet werden'))
{
	throw 'Localized feedback must not remain hard-coded in the script.'
}

$sourceKeys = [regex]::Matches($source, '#DGC-[A-Za-z_]+') | ForEach-Object Value | Sort-Object -Unique
$tableKeys = [regex]::Matches($stringTable, 'Id "(?<Id>DGC-[^"]+)"') | ForEach-Object { '#' + $_.Groups['Id'].Value } | Sort-Object -Unique
if (Compare-Object $sourceKeys $tableKeys)
{
	throw 'Script localization keys and source string-table IDs do not match.'
}

if ([regex]::Matches($source, 'DGC_GameMasterAccess\.ProcessChannelMessage\(sender, message, isAuthority,').Count -ne 6)
{
	throw 'Every script-extensible player channel must use the shared authoritative message guard.'
}

if ($source.Contains('DGC_ChatChannelsEditorAttribute') -or $source.Contains('SCR_AttributesManagerEditorComponentClass'))
{
	throw 'Legacy Game Master attribute configuration must stay removed.'
}

if ($source.Contains('m_ChatEntity.GetChannelsCount()') -and $source.Contains('protected bool DGC_SendChatControlCommand(string message)'))
{
	throw 'Chat command transport must not depend on a configured channel instance.'
}

Write-Output 'DisableGlobalChat static checks passed.'
