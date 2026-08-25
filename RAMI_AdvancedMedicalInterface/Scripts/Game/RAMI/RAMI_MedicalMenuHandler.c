class RAMI_MedicalMenuHandler : ScriptedWidgetEventHandler
{
	protected RAMI_MedicalMenuUI m_Menu;

	void RAMI_MedicalMenuHandler(RAMI_MedicalMenuUI menu)
	{
		m_Menu = menu;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		string widgetName = w.GetName();
		if (widgetName == "PageToggleSelf")
			m_Menu.TogglePatient();
		else if (widgetName == "PatientActionTreatButton")
			m_Menu.TreatListedPatient();
		else if (widgetName == "PatientActionUnloadButton")
			m_Menu.UnloadListedPatient();
		else if (widgetName.StartsWith("PatientEntry"))
			m_Menu.SelectListedPatient(w);
		else if (
			widgetName == "PageTriage" ||
			widgetName == "PageDiagnose" ||
			widgetName == "PageBandages" ||
			widgetName == "PageMedication" ||
			widgetName == "PageAdvanced"
		)
		{
			m_Menu.ShowPage(widgetName);
		}
		else if (
			widgetName == "DiagnoseHeartRateButton" ||
			widgetName == "DiagnoseBloodPressureButton"
		)
			m_Menu.ToggleDiagnoseMonitor(widgetName);
		else if (
			widgetName == "TriageLevelNone" ||
			widgetName == "TriageLevelMinimal" ||
			widgetName == "TriageLevelDelayed" ||
			widgetName == "TriageLevelImmediate" ||
			widgetName == "TriageLevelExpectant"
		)
			m_Menu.SetTriageLevel(widgetName);
		else if (
			widgetName == "AdvancedCPRButton" ||
			widgetName == "AdvancedCarryButton" ||
			widgetName == "AdvancedDragButton" ||
			widgetName == "AdvancedBackButton" ||
			widgetName == "AdvancedRightSideButton" ||
			widgetName == "AdvancedLeftSideButton" ||
			widgetName == "AdvancedLoadVehicleButton"
		)
			m_Menu.UseAdvancedAction(widgetName);
		else if (
			widgetName == "BandageButton" ||
			widgetName == "BandageButtonContent" ||
			widgetName == "BandageIcon" ||
			widgetName == "BandageLabel" ||
			widgetName == "BandageCount"
		)
			m_Menu.UseBandage(false);
		else if (
			widgetName == "BandageButtonForeign" ||
			widgetName == "BandageButtonContentForeign" ||
			widgetName == "BandageIconForeign" ||
			widgetName == "BandageLabelForeign" ||
			widgetName == "BandageCountForeign"
		)
			m_Menu.UseBandage(true);
		else if (
			widgetName == "TourniquetButton" ||
			widgetName == "TourniquetButtonContent" ||
			widgetName == "TourniquetIcon" ||
			widgetName == "TourniquetLabel" ||
			widgetName == "TourniquetCount"
		)
			m_Menu.UseOrRemoveTourniquet(false);
		else if (
			widgetName == "TourniquetButtonForeign" ||
			widgetName == "TourniquetButtonContentForeign" ||
			widgetName == "TourniquetIconForeign" ||
			widgetName == "TourniquetLabelForeign" ||
			widgetName == "TourniquetCountForeign"
		)
			m_Menu.UseOrRemoveTourniquet(true);
		else if (
			widgetName == "MedicalKitButton" ||
			widgetName == "MedicalKitButtonContent" ||
			widgetName == "MedicalKitIcon" ||
			widgetName == "MedicalKitLabel" ||
			widgetName == "MedicalKitCount"
		)
			m_Menu.UseMedicalKit();
		else if (widgetName == "CloseButton")
			m_Menu.CloseMenu();
		else if (!m_Menu.UseMedicationWidget(widgetName))
			m_Menu.SelectBodyRegion(widgetName);

		return true;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w.GetName().StartsWith("PatientEntry"))
			m_Menu.SetPatientButtonHovered(w, true);
		else
			m_Menu.SetRequestedButtonHovered(w, true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w.GetName().StartsWith("PatientEntry"))
			m_Menu.SetPatientButtonHovered(w, false);
		else
			m_Menu.SetRequestedButtonHovered(w, false);
		return false;
	}
}
