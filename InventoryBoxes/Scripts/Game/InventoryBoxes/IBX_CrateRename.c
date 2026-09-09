//------------------------------------------------------------------------------------------------
// Crate naming: a world action every player can use, plus the Mike's UI menu it opens. The Game
// Master path is not here - the editor renames inline from its own header (IBX_GMInventoryEditorUI)
// so it never has to stack a second window on top of itself.
//
// The name itself lives on IBX_GMInventoryEditorComponent, which is already on all 42 crates and
// already owns the crate's server-authoritative state.
//------------------------------------------------------------------------------------------------

class IBX_RenameCrateAction : SCR_ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		IBX_GMInventoryEditorComponent component = IBX_GMInventoryEditorComponent.Get(GetOwner());
		if (component && !component.GetCrateName().IsEmpty())
			outName = "Rename Crate (" + component.GetCrateName() + ")";
		else
			outName = "Name Crate";

		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return IBX_GMInventoryEditorComponent.Get(GetOwner()) != null;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	// Deliberately no CanBroadcastScript override. Suppressing the broadcast also stopped
	// PerformAction from running on the performing client on a dedicated server, so the menu never
	// opened. IBX_CarryCrateAction is the confirmed-working shape and does not override it either;
	// the guard in PerformAction is what keeps the server's own copy from doing anything.

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// PerformAction also fires on the server for the performing client; only the machine whose
		// own player did this should get a window. A dedicated server has no local controlled
		// entity, so it drops out here.
		if (pUserEntity != SCR_PlayerController.GetLocalControlledEntity())
			return;

		IBX_GMInventoryEditorComponent component = IBX_GMInventoryEditorComponent.Get(pOwnerEntity);
		if (component)
			IBX_CrateRenameMenu.Open(component);
	}
}

//------------------------------------------------------------------------------------------------

//! The client-to-server leg of a rename. A [RplRpc] declared on IBX_GMInventoryEditorComponent
//! itself never arrived on a dedicated server, while that component's server-to-client broadcast
//! does; the player controller is the channel vanilla uses for every client request and is owned by
//! the requesting client, so it works for a Game Master and a plain player alike.
modded class SCR_PlayerController
{
	void IBX_RequestCrateRename(RplId crateId, string name)
	{
		Rpc(IBX_RpcAsk_CrateRename, crateId, name);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void IBX_RpcAsk_CrateRename(RplId crateId, string name)
	{
		RplComponent crateRpl = RplComponent.Cast(Replication.FindItem(crateId));
		if (!crateRpl)
			return;

		IBX_GMInventoryEditorComponent crate = IBX_GMInventoryEditorComponent.Get(crateRpl.GetEntity());
		if (crate)
			crate.RenameServer(name);
	}
}

//------------------------------------------------------------------------------------------------

//! Per-instance crate name wherever the crate is drawn as an *item* rather than as a storage: the
//! hover info panel in the vicinity list, and its slot once it is loaded into a vehicle.
//!
//! This is the extension point vanilla provides for exactly this - `GetInventoryItemName` takes the
//! specific `InventoryItemComponent`, so a per-item answer is correct even though the UIInfo object
//! it lives on is shared between every instance of the prefab. Set as `ItemDisplayName` on all 42
//! crates in place of the plain `SCR_InventoryUIInfo`.
//!
//! Covers `SCR_InventoryMenuUI.SetFocusedSlotEffects` (hover info) and
//! `SCR_InventorySlotUI.ShowItemDetails` (slot details), the only two callers.
[BaseContainerProps()]
class IBX_CrateInventoryUIInfo : SCR_InventoryUIInfo
{
	override string GetInventoryItemName(InventoryItemComponent item)
	{
		if (item)
		{
			IBX_GMInventoryEditorComponent crate = IBX_GMInventoryEditorComponent.Get(item.GetOwner());
			if (crate)
			{
				string name = crate.GetDisplayName();
				if (!name.IsEmpty())
					return name;
			}
		}

		return super.GetInventoryItemName(item);
	}
}

//------------------------------------------------------------------------------------------------

//! Per-instance crate name in the inventory screen.
//!
//! The obvious place for this - the storage's own ItemAttributeCollection UIInfo, which is what
//! vanilla prints - is shared between every instance of a prefab, so writing into it renamed every
//! crate of that type at once and left the next one placed inheriting the name as its default.
//! Overriding the widgets after vanilla has filled them touches nothing shared.
//!
//! Two sites, because a crate reaches the screen two ways. Opened directly it becomes its own panel
//! with a "StorageName" header, filled in Init. Opened out of the vicinity it is traversed into
//! instead, and gets a traverse title bar filled by FindAndSetTitleName - which is the one the
//! player normally sees, since that is where the world Open action lands.
modded class SCR_InventoryStorageBaseUI
{
	override void Init()
	{
		super.Init();

		if (!m_wStorageName)
			return;

		BaseInventoryStorageComponent storage = GetStorage();
		if (!storage)
			return;

		string name = IBX_GetCrateName(storage);
		if (!name.IsEmpty())
			m_wStorageName.SetText(name);
	}

	override protected void FindAndSetTitleName(Widget targetTitle, BaseInventoryStorageComponent targetStorage)
	{
		super.FindAndSetTitleName(targetTitle, targetStorage);

		if (!targetTitle || !targetStorage)
			return;

		string name = IBX_GetCrateName(targetStorage);
		if (name.IsEmpty())
			return;

		TextWidget titleNameWidget = TextWidget.Cast(targetTitle.FindAnyWidget("TextC"));
		if (titleNameWidget)
			titleNameWidget.SetText(name);
	}

	//! The header bar of an opened crate. Vanilla reports occupied volume only; a crate is equally
	//! full once its weight limit is gone, and the bar on the crate's own inventory slot reports the
	//! same number through IBX_CrateFill. The preview value the inventory menu passes in through
	//! occupiedSpace stays vanilla.
	override float GetOccupiedVolumePercentage(BaseInventoryStorageComponent storage, float occupiedSpace = 0)
	{
		if (occupiedSpace == 0)
		{
			float cratePercentage = IBX_CrateFill.GetPercentage(storage);
			if (cratePercentage >= 0)
				return cratePercentage;
		}

		return super.GetOccupiedVolumePercentage(storage, occupiedSpace);
	}

	override void Refresh()
	{
		super.Refresh();
		IBX_UpdateCrateFillBar();
	}

	override event void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		IBX_UpdateCrateFillBar();
	}

	//! Sets the header bar itself rather than trusting vanilla to ask for a percentage: the opened
	//! crate panel does not reach the UpdateVolumePercentage call in every path it is created
	//! through, and an unasked bar keeps its layout default, which is a full one.
	protected void IBX_UpdateCrateFillBar()
	{
		float percentage = IBX_CrateFill.GetPercentage(m_Storage);
		if (percentage < 0)
			return;

		UpdateVolumePercentage(percentage);
	}

	protected static string IBX_GetCrateName(notnull BaseInventoryStorageComponent storage)
	{
		IBX_GMInventoryEditorComponent crate = IBX_GMInventoryEditorComponent.Get(storage.GetOwner());
		if (!crate)
			return string.Empty;

		return crate.GetDisplayName();
	}
}

//------------------------------------------------------------------------------------------------

modded enum ChimeraMenuPreset
{
	IBX_CrateRenameMenu
}

//! A real menu rather than a workspace modal. A modal added over live gameplay leaves the mouse
//! captured by the character controller, so the field could not be clicked or typed into; going
//! through MenuManager is what frees the cursor and routes keyboard input to the UI. The preset is
//! registered in Configs/System/chimeraMenus.conf against Mike's UI's own blank menu layout.
//!
//! MUI_MenuBase owns mounting, ticking, blur and back/Escape, so only BuildUI is needed here.
class IBX_CrateRenameMenu : MUI_MenuBase
{
	//! OpenMenu carries no payload, so the crate is handed over through a static set immediately
	//! before opening and consumed once in BuildUI.
	protected static IBX_GMInventoryEditorComponent s_PendingComponent;

	protected IBX_GMInventoryEditorComponent m_Component;
	protected MUI_TextField m_Name;

	static void Open(notnull IBX_GMInventoryEditorComponent component)
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;

		s_PendingComponent = component;
		menuManager.OpenMenu(ChimeraMenuPreset.IBX_CrateRenameMenu);
	}

	override string GetMUILogTag()
	{
		return "IBX_CrateRename";
	}

	override void BuildUI(notnull MUI_Runtime runtime)
	{
		m_Component = s_PendingComponent;
		s_PendingComponent = null;
		if (!m_Component)
		{
			Close();
			return;
		}

		MUI_Panel root = runtime.CreatePanel("CrateRenameRoot");
		root.MakeOverlay();
		root.SetFill(Color.FromRGBA(0, 0, 0, 184));

		MUI_Panel frame = runtime.CreatePanel("CrateRenameFrame");
		frame.SetFill(MUI_Theme.Border);
		frame.SetWidth(680);
		frame.SetHeight(340);
		frame.SetAlign(0.5, 0.5);
		frame.SetRadius(18);
		frame.SetPadding(2);
		root.AddChild(frame);

		MUI_Panel card = runtime.CreatePanel("CrateRenameCard");
		card.SetFill(MUI_Theme.DeepFrost);
		card.SetFillWidth();
		card.SetFillHeight();
		card.SetRadius(16);
		card.SetPadding(26);
		card.SetGap(12);
		frame.AddChild(card);

		MUI_Label title = runtime.CreateLabel("NAME CRATE", "CrateRenameTitle");
		title.SetFontSize(MUI_Theme.FONT_TITLE);
		title.SetBold(true);
		title.SetHeight(42);
		card.AddChild(title);

		MUI_Label help = runtime.CreateLabel(string.Format("Shown on the crate and above its inventory. Up to %1 characters; leave empty to restore the original name.", IBX_GMInventoryEditorComponent.MAX_NAME_LENGTH), "CrateRenameHelp");
		help.SetMuted(true);
		help.SetHeight(54);
		card.AddChild(help);

		m_Name = runtime.CreateTextField("CRATE NAME", "CrateRenameField");
		m_Name.SetFillWidth();
		m_Name.SetHeight(74);
		card.AddChild(m_Name);

		MUI_Row actions = runtime.CreateRow("CrateRenameActions");
		actions.SetFillWidth();
		actions.SetHeight(54);
		actions.SetGap(10);
		card.AddChild(actions);

		MUI_Button save = runtime.CreateButton("SAVE", "CrateRenameSave");
		save.MakeAccent();
		save.SetGrow(1);
		save.GetOnClicked().Insert(Save);
		actions.AddChild(save);

		MUI_Button clear = runtime.CreateButton("CLEAR NAME", "CrateRenameClear");
		clear.SetGrow(1);
		clear.GetOnClicked().Insert(ClearName);
		actions.AddChild(clear);

		MUI_Button cancel = runtime.CreateButton("CANCEL", "CrateRenameCancel");
		cancel.SetGrow(1);
		cancel.GetOnClicked().Insert(Cancel);
		actions.AddChild(cancel);

		runtime.SetRoot(root);
		m_Name.SetText(m_Component.GetCrateName());
	}

	protected void Save()
	{
		if (m_Component)
			m_Component.RequestRename(m_Name.GetText());

		Close();
	}

	protected void ClearName()
	{
		if (m_Component)
			m_Component.RequestRename("");

		Close();
	}

	protected void Cancel()
	{
		Close();
	}
}
