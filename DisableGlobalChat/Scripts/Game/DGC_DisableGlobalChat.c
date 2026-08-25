enum DGC_EChatChannel
{
	GLOBAL = 1,
	FACTION = 2,
	GROUP = 4,
	LOCAL = 8,
	VEHICLE = 16,
	DIRECT = 32,
	ALL = 63
}

modded class SCR_AdditionalGameModeSettingsComponent
{
	[RplProp()]
	protected int m_iDGCAllowedChannels = DGC_EChatChannel.GROUP;

	int DGC_GetAllowedChannels()
	{
		return m_iDGCAllowedChannels;
	}

	bool DGC_IsChannelAllowed(DGC_EChatChannel channel)
	{
		return (m_iDGCAllowedChannels & channel) != 0;
	}

	void DGC_SetAllowedChannels_S(int channels)
	{
		channels &= DGC_EChatChannel.ALL;
		if (!GetGameMode().IsMaster() || m_iDGCAllowedChannels == channels)
			return;

		m_iDGCAllowedChannels = channels;
		Replication.BumpMe();
	}
}

class DGC_GameMasterAccess
{
	static bool CanSend(BaseChatComponent sender)
	{
		if (!sender)
			return false;

		PlayerController controller = PlayerController.Cast(sender.GetOwner());
		return controller && HasFullAccess(controller.GetPlayerId());
	}

	static bool CanLocalPlayerSend()
	{
		PlayerController controller = GetGame().GetPlayerController();
		return controller && HasFullAccess(controller.GetPlayerId());
	}

	static bool CanUseChannel(BaseChatComponent sender, DGC_EChatChannel channel)
	{
		if (CanSend(sender))
			return true;

		if (!IsChannelAllowed(channel))
			return false;

		return channel != DGC_EChatChannel.GROUP || CanSendToGroup(sender);
	}

	static bool CanLocalPlayerUseChannel(BaseChatChannel channel)
	{
		if (CanLocalPlayerSend())
			return true;

		DGC_EChatChannel channelFlag = GetChannelFlag(channel);
		return channelFlag && IsChannelAllowed(channelFlag);
	}

	static bool CanSendToGroup(BaseChatComponent sender)
	{
		if (!sender)
			return false;

		PlayerController controller = PlayerController.Cast(sender.GetOwner());
		if (!controller)
			return false;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		return groupController && groupController.GetGroupID() != -1;
	}

	static bool IsAdminLoginCommand(string message)
	{
		message.TrimInPlace();
		message.ToLower();
		return message == "#login" || message.StartsWith("#login ");
	}

	static bool IsChatControlCommand(string message)
	{
		message.TrimInPlace();
		message.ToLower();
		return message == "#chat" || message.StartsWith("#chat ");
	}

	static bool ProcessChannelMessage(BaseChatComponent sender, string message, bool isAuthority, DGC_EChatChannel channel)
	{
		if (IsChatControlCommand(message))
		{
			if (isAuthority)
				HandleChatControlCommand_S(sender, message);

			return !isAuthority;
		}

		if (IsAdminLoginCommand(message) || CanUseChannel(sender, channel))
			return true;

		if (isAuthority)
			SendPrivateFeedback_S(sender, "#DGC-Feedback_ChannelLocked", GetChannelLabelByFlag(channel));

		return false;
	}

	static void HandleChatControlCommand_S(BaseChatComponent sender, string message)
	{
		if (!CanSend(sender))
		{
			SendPrivateFeedback_S(sender, "#DGC-Feedback_Unauthorized");
			return;
		}

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
		{
			SendPrivateFeedback_S(sender, "#DGC-Feedback_SettingsUnavailable");
			return;
		}

		message.TrimInPlace();
		message.ToLower();

		string argument;
		if (message.Length() > 5)
			argument = message.Substring(5, message.Length() - 5);
		argument.TrimInPlace();

		int currentChannels = settings.DGC_GetAllowedChannels();
		int newChannels = currentChannels;
		string feedback;
		string feedbackParam1;
		string feedbackParam2;
		string feedbackParam3;
		string feedbackParam4;
		string feedbackParam5;
		string feedbackParam6;

		if (argument == "status")
		{
			feedback = "#DGC-Feedback_Status";
			feedbackParam1 = GetChannelState(currentChannels, DGC_EChatChannel.GLOBAL);
			feedbackParam2 = GetChannelState(currentChannels, DGC_EChatChannel.FACTION);
			feedbackParam3 = GetChannelState(currentChannels, DGC_EChatChannel.GROUP);
			feedbackParam4 = GetChannelState(currentChannels, DGC_EChatChannel.LOCAL);
			feedbackParam5 = GetChannelState(currentChannels, DGC_EChatChannel.VEHICLE);
			feedbackParam6 = GetChannelState(currentChannels, DGC_EChatChannel.DIRECT);
		}
		else if (argument == "all")
		{
			newChannels = DGC_EChatChannel.ALL;
			feedback = "#DGC-Feedback_AllEnabled";
		}
		else if (argument == "none")
		{
			newChannels = 0;
			feedback = "#DGC-Feedback_AllDisabled";
		}
		else
		{
			DGC_EChatChannel channel = GetChannelFlagByName(argument);
			if (!channel)
			{
				SendPrivateFeedback_S(sender, "#DGC-Feedback_Usage");
				return;
			}

			newChannels = currentChannels ^ channel;
			feedbackParam1 = GetChannelLabelByFlag(channel);
			if ((newChannels & channel) != 0)
				feedback = "#DGC-Feedback_ChannelEnabled";
			else
				feedback = "#DGC-Feedback_ChannelDisabled";
		}

		settings.DGC_SetAllowedChannels_S(newChannels);
		SendPrivateFeedback_S(sender, feedback, feedbackParam1, feedbackParam2, feedbackParam3, feedbackParam4, feedbackParam5, feedbackParam6);
	}

	protected static void SendPrivateFeedback_S(BaseChatComponent sender, string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "")
	{
		if (!sender)
			return;

		PlayerController controller = PlayerController.Cast(sender.GetOwner());
		if (!controller)
			return;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (groupController)
		{
			groupController.DGC_SendChatFeedback_S(message, param1, param2, param3, param4, param5, param6);
			return;
		}

		sender.SendPrivateMessage("[Disable Chat] " + Localize(message, param1, param2, param3, param4, param5, param6), controller.GetPlayerId());
	}

	static string Localize(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "")
	{
		return WidgetManager.Translate(message,
			WidgetManager.Translate(param1),
			WidgetManager.Translate(param2),
			WidgetManager.Translate(param3),
			WidgetManager.Translate(param4),
			WidgetManager.Translate(param5),
			WidgetManager.Translate(param6));
	}

	static DGC_EChatChannel GetChannelFlagByName(string channelName)
	{
		switch (channelName)
		{
			case "global": return DGC_EChatChannel.GLOBAL;
			case "faction":
			case "fraktion": return DGC_EChatChannel.FACTION;
			case "group":
			case "gruppe": return DGC_EChatChannel.GROUP;
			case "local":
			case "lokal": return DGC_EChatChannel.LOCAL;
			case "vehicle":
			case "fahrzeug": return DGC_EChatChannel.VEHICLE;
			case "direct":
			case "direkt": return DGC_EChatChannel.DIRECT;
		}

		return 0;
	}

	protected static string GetChannelState(int channels, DGC_EChatChannel channel)
	{
		if ((channels & channel) != 0)
			return "#DGC-State_On";

		return "#DGC-State_Off";
	}

	static bool IsChannelAllowed(DGC_EChatChannel channel)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (settings)
			return settings.DGC_IsChannelAllowed(channel);

		return channel == DGC_EChatChannel.GROUP;
	}

	static DGC_EChatChannel GetChannelFlag(BaseChatChannel channel)
	{
		if (channel && channel.Type() == BaseChatChannel && channel.GetName() == "Global")
			return DGC_EChatChannel.GLOBAL;
		if (FactionChatChannel.Cast(channel))
			return DGC_EChatChannel.FACTION;
		if (GroupChatChannel.Cast(channel))
			return DGC_EChatChannel.GROUP;
		if (LocalChatChannel.Cast(channel))
			return DGC_EChatChannel.LOCAL;
		if (SCR_VehicleChatChannel.Cast(channel))
			return DGC_EChatChannel.VEHICLE;
		if (PrivateMessageChannel.Cast(channel))
			return DGC_EChatChannel.DIRECT;
		if (SCR_ChatChannel.Cast(channel))
			return DGC_EChatChannel.GLOBAL;

		return 0;
	}

	static bool CanReceiveMessage(int channelId, int senderId)
	{
		if (channelId == 0 || HasFullAccess(senderId))
			return true;

		ScriptedChatEntity chat = ScriptedChatEntity.Cast(GetGame().GetChat());
		return chat && IsChannelAllowed(GetChannelFlag(chat.GetChannel(channelId)));
	}

	static string GetChannelLabel(BaseChatChannel channel)
	{
		return GetChannelLabelByFlag(GetChannelFlag(channel));
	}

	static string GetChannelLabelByFlag(DGC_EChatChannel channel)
	{
		switch (channel)
		{
			case DGC_EChatChannel.GLOBAL: return "#DGC-Channel_Global";
			case DGC_EChatChannel.FACTION: return "#DGC-Channel_Faction";
			case DGC_EChatChannel.GROUP: return "#DGC-Channel_Group";
			case DGC_EChatChannel.LOCAL: return "#DGC-Channel_Local";
			case DGC_EChatChannel.VEHICLE: return "#DGC-Channel_Vehicle";
			case DGC_EChatChannel.DIRECT: return "#DGC-Channel_Direct";
		}

		return "#DGC-Channel_Text";
	}

	static bool HasFullAccess(int playerId)
	{
		return playerId > 0 && (SCR_Global.IsAdmin(playerId) || IsGameMaster(playerId));
	}

	static bool IsGameMaster(int playerId)
	{
		if (playerId <= 0)
			return false;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return false;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerId);
		return editorManager && !editorManager.IsLimited();
	}
}

modded class SCR_ChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, DGC_EChatChannel.GLOBAL))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class FactionChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, DGC_EChatChannel.FACTION))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class GroupChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, DGC_EChatChannel.GROUP))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class LocalChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, DGC_EChatChannel.LOCAL))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class ServerChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, 0))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class SCR_VehicleChatChannel
{
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (!DGC_GameMasterAccess.ProcessChannelMessage(sender, message, isAuthority, DGC_EChatChannel.VEHICLE))
			return false;

		return super.ProcessMessage(sender, message, isAuthority);
	}
}

modded class SCR_ChatComponent
{
	override void OnNewMessage(string msg, int channelId, int senderId)
	{
		if (DGC_GameMasterAccess.CanReceiveMessage(channelId, senderId))
			super.OnNewMessage(msg, channelId, senderId);
	}

	override void OnNewPrivateMessage(string msg, int senderId, int receiverId)
	{
		if (senderId == receiverId || DGC_GameMasterAccess.HasFullAccess(senderId) || DGC_GameMasterAccess.IsChannelAllowed(DGC_EChatChannel.DIRECT))
			super.OnNewPrivateMessage(msg, senderId, receiverId);
	}
}

modded class SCR_PlayerControllerGroupComponent
{
	void DGC_SendChatFeedback_S(string message, string param1 = "", string param2 = "", string param3 = "", string param4 = "", string param5 = "", string param6 = "")
	{
		if (Replication.IsServer())
			Rpc(RpcDo_DGC_ChatFeedback, message, param1, param2, param3, param4, param5, param6);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_DGC_ChatFeedback(string message, string param1, string param2, string param3, string param4, string param5, string param6)
	{
		SCR_ChatPanelManager manager = SCR_ChatPanelManager.GetInstance();
		if (manager)
			manager.ShowHelpMessage("[Disable Chat] " + DGC_GameMasterAccess.Localize(message, param1, param2, param3, param4, param5, param6));
	}

	void DGC_RequestChatControlCommand(string message)
	{
		if (Replication.IsServer())
		{
			RpcAsk_DGC_ChatControlCommand(message);
			return;
		}

		Rpc(RpcAsk_DGC_ChatControlCommand, message);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DGC_ChatControlCommand(string message)
	{
		PlayerController controller = PlayerController.Cast(GetOwner());
		SCR_ChatComponent chatComponent;
		if (controller)
			chatComponent = SCR_ChatComponent.Cast(controller.FindComponent(SCR_ChatComponent));

		DGC_GameMasterAccess.HandleChatControlCommand_S(chatComponent, message);
	}
}

modded class SCR_ChatPanel
{
	override void SendMessage()
	{
		string input;
		if (m_Widgets.m_MessageEditBox)
			input = m_Widgets.m_MessageEditBox.GetText();

		if (DGC_GameMasterAccess.IsChatControlCommand(input))
		{
			if (!DGC_GameMasterAccess.CanLocalPlayerSend())
			{
				DGC_ShowHelpMessage("#DGC-Feedback_Unauthorized");
				return;
			}

			if (!DGC_SendChatControlCommand(input))
				DGC_ShowHelpMessage("#DGC-Feedback_CommandSendFailed");
			return;
		}

		if (!DGC_GameMasterAccess.CanLocalPlayerUseChannel(m_ActiveChannel)
			&& !DGC_GameMasterAccess.IsAdminLoginCommand(input))
		{
			DGC_ShowHelpMessage("#DGC-Feedback_ChannelLocked", DGC_GameMasterAccess.GetChannelLabel(m_ActiveChannel));
			return;
		}

		super.SendMessage();
	}

	protected bool DGC_SendChatControlCommand(string message)
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;

		groupController.DGC_RequestChatControlCommand(message);
		return true;
	}

	protected void DGC_ShowHelpMessage(string message, string param1 = "")
	{
		message = "[Disable Chat] " + DGC_GameMasterAccess.Localize(message, param1);
		SCR_ChatPanelManager manager = SCR_ChatPanelManager.GetInstance();
		if (manager)
			manager.ShowHelpMessage(message);
		else
			ShowChannelWarning(null, message);
	}
}
