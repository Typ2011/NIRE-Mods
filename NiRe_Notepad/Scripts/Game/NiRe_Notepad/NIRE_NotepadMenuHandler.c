class NIRE_NotepadMenuHandler : ScriptedWidgetEventHandler
{
	protected NIRE_NotepadController m_Controller;

	void NIRE_NotepadMenuHandler(NIRE_NotepadController controller)
	{
		m_Controller = controller;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w.GetName() == "NoteEditor" || w.GetName() == "EditorScroll")
			return false;

		if (w.GetName() == "MissionRow")
		{
			m_Controller.SelectMission(w.GetUserID());
			m_Controller.ScheduleEditorDeactivation();
			return true;
		}
		switch (w.GetName())
		{
			case "TabGeneral": m_Controller.SelectTab(0); break;
			case "TabCAS": m_Controller.SelectTab(1); break;
			case "TabArtillery": m_Controller.SelectTab(2); break;
			case "TabLogistics": m_Controller.OpenLogisticsScreen(); break;
			case "TabMedevac": m_Controller.SelectTab(4); break;
			case "TabEvacTransport": m_Controller.SelectTab(5); break;
			case "NewMission": m_Controller.NewMission(); break;
			case "DeleteMission": m_Controller.DeleteMission(); break;
			case "Template5": m_Controller.RequestTemplate(5); break;
			case "Template9": m_Controller.RequestSecondaryTemplate(); break;
			case "TemplateFree": m_Controller.SelectFreeText(); break;
			case "TerrainPen": m_Controller.SetDrawingTool(false); break;
			case "TerrainEraser": m_Controller.SetDrawingTool(true); break;
			case "TerrainClear": m_Controller.ClearDrawing(); break;
			case "TerrainColorWhite": m_Controller.SetDrawingColor(0); break;
			case "TerrainColorRed": m_Controller.SetDrawingColor(1); break;
			case "TerrainColorBlue": m_Controller.SetDrawingColor(2); break;
			case "TerrainColorGreen": m_Controller.SetDrawingColor(3); break;
			case "TerrainColorYellow": m_Controller.SetDrawingColor(4); break;
			case "CloseButton": m_Controller.Close(); return true;
		}

		m_Controller.ScheduleEditorDeactivation();
		return true;
	}

	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (w.GetName() == "TerrainInput")
		{
			m_Controller.StartDrawing(x, y, button);
			return true;
		}
		if (w.GetName() == "NoteEditor" || w.GetName() == "EditorScroll")
			m_Controller.FocusEditor(button);

		m_Controller.HandlePointerDown(x, y);
		return false;
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (w.GetName() != "TerrainInput")
			return false;

		m_Controller.StopDrawing(x, y, button);
		return true;
	}

	override bool OnChange(Widget w, bool finished)
	{
		if (w.GetName() == "MissionName")
			m_Controller.OnMissionNameChanged(finished);
		else if (w.GetName() == "NoteEditor")
			m_Controller.OnEditorChanged();

		return false;
	}

	override bool OnController(Widget w, ControlID control, int value)
	{
		if (control != ControlID.BACK || value <= 0)
			return false;

		return m_Controller.HandleBackControl();
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		if (w.GetName() == "NoteEditor")
			m_Controller.SetEditorFocused(true);

		return false;
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (w.GetName() == "NoteEditor")
			m_Controller.SetEditorFocused(false);

		return false;
	}

	override bool OnWriteModeLeave(Widget w)
	{
		if (w.GetName() == "NoteEditor")
			m_Controller.SetEditorFocused(false);

		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w.GetName() == "MissionRow")
			m_Controller.HoverMission(w.GetUserID(), true);
		else if (w.GetName() == "CloseButton")
			m_Controller.HoverDone(true);

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w.GetName() == "MissionRow")
			m_Controller.HoverMission(w.GetUserID(), false);
		else if (w.GetName() == "CloseButton")
			m_Controller.HoverDone(false);
		else if (w.GetName() == "NotepadPanel")
			m_Controller.ReleaseMapFocus();

		return false;
	}
}
