modded class SCR_BaseGameMode
{
	protected bool m_bNIRE_NotepadInputRegistered;

	override void OnGameStart()
	{
		super.OnGameStart();
		NIRE_RegisterNotepadInput();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		SCR_PlayerController localController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (localController && localController.GetPlayerId() == playerId)
			NIRE_RegisterNotepadInput();
	}

	//------------------------------------------------------------------------------------------------
	protected void NIRE_RegisterNotepadInput()
	{
		if (m_bNIRE_NotepadInputRegistered)
			return;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("NIRE_ToggleNotepad", EActionTrigger.DOWN, NIRE_ToggleNotepad);
			m_bNIRE_NotepadInputRegistered = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager && m_bNIRE_NotepadInputRegistered)
			inputManager.RemoveActionListener("NIRE_ToggleNotepad", EActionTrigger.DOWN, NIRE_ToggleNotepad);
		m_bNIRE_NotepadInputRegistered = false;

		NIRE_NotepadMenu.CloseIfOpen();
		super.OnGameEnd();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		SCR_PlayerController.NIRE_RemoveLogisticsAccess(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void NIRE_ToggleNotepad()
	{
		NIRE_NotepadMenu.Toggle();
	}
}

//! The Game Master pause action does not check for an open menu the way ArmaReforgerScripted.OnMenuOpen
//! does, so in the editor Escape would open the pause menu straight on top of the notepad. The same
//! interception InventoryBoxes uses for its crate editor: answer the press and stop, or pass it on.
modded class EditorMenuUI
{
	override void OpenPauseMenu()
	{
		if (NIRE_LogisticsScreen.ConsumeBack())
			return;

		if (NIRE_NotepadMenu.CloseIfOpen())
			return;

		super.OpenPauseMenu();
	}
}

modded class PauseMenuUI
{
	//! Escape does not reach NIRE_CloseNotepad while a notepad menu holds focus, because that action
	//! lives in the ingame, map and text-edit contexts only. The pause menu is therefore where a
	//! notepad back press actually lands, and exactly one layer may answer it: the logistics
	//! workspace first, the notepad only once the workspace is gone.
	override void OnMenuOpen()
	{
		bool suppressPause = true;
		if (NIRE_LogisticsScreen.ConsumePauseSuppression())
		{
			// The workspace already closed itself through its own back handler.
		}
		else if (NIRE_NotepadMenu.ConsumePauseSuppression())
		{
			// The notepad already closed itself through NIRE_CloseNotepad.
		}
		else if (!NIRE_LogisticsScreen.CloseIfOpen() && !NIRE_NotepadMenu.CloseIfOpen())
		{
			suppressPause = false;
		}

		super.OnMenuOpen();
		if (suppressPause)
			Close();
	}
}
