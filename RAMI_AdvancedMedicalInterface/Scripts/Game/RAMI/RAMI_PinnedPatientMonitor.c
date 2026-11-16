class RAMI_PinnedPatientMonitor
{
	protected static const ResourceName LAYOUT = "{B918CE5213F049A8}UI/layouts/RAMI_PinnedPatientMonitor.layout";
	protected static const int SAMPLE_TIMEOUT_MS = 5000;
	protected static const float MAX_DISTANCE_METERS = 10;
	protected static const float BACKGROUND_CORNER_RADIUS = 8;
	protected static const int CRITICAL_ALARM_INTERVAL_MS = 5000;
	protected static const int SEVERE_ALARM_INTERVAL_MS = 2000;
	protected static const int ALARM_MIN_GAP_MS = 1000;
	protected static const int ALERT_VISUAL_INTERVAL_MS = 125;
	protected static const ResourceName ALARM_SOUND = "{A7810CDE932F446B}Sounds/UI/RAMI_MonitorAlarm.wav";
	protected static int s_LastWarningTime;
	protected SCR_PlayerController m_PlayerController;
	protected RplId m_PatientId = RplId.Invalid();
	protected Widget m_Root;
	protected CanvasWidget m_Background;
	protected ref PolygonDrawCommand m_BackgroundFill;
	protected ref PolygonDrawCommand m_AlertBorder;
	protected ref array<ref CanvasWidgetCommand> m_BackgroundCommands = {};
	protected TextWidget m_Values;
	protected TextWidget m_State;
	protected TextWidget m_Alarm;
	protected bool m_AlarmEnabled = true;
	protected bool m_HasSample;
	protected int m_LastSampleTime;
	protected int m_LastAlarmState;
	protected int m_LastWarningTime;
	protected bool m_IsCritical;
	protected AudioHandle m_WarningSound;
	protected bool m_WarningSoundActive;

	static bool CanMonitor(SCR_ChimeraCharacter patient)
	{
		return GetUnavailableReason(patient).IsEmpty();
	}

	protected static string GetUnavailableReason(SCR_ChimeraCharacter patient)
	{
		if (!patient)
			return "patient_missing";
		if (!RAMI_MedicalMenuUI.HasMedicalKit())
			return "observer_medical_kit_missing";
		CharacterControllerComponent patientController = patient.GetCharacterController();
		if (!patientController)
			return "patient_controller_missing";
		if (patientController.GetLifeState() == ECharacterLifeState.DEAD)
			return "patient_dead";
		IEntity user = SCR_PlayerController.GetLocalControlledEntity();
		if (!user)
			return "observer_character_missing";
		if (vector.Distance(user.GetOrigin(), patient.GetOrigin()) > MAX_DISTANCE_METERS)
			return "patient_out_of_range";
		return string.Empty;
	}

	bool Start(SCR_PlayerController playerController, SCR_ChimeraCharacter patient, string patientName, int slot)
	{
		if (!playerController || GetGame().GetPlayerController() != playerController || !CanMonitor(patient) || !GetGame().GetWorkspace())
			return false;
		m_PatientId = Replication.FindItemId(patient);
		if (!m_PatientId.IsValid())
			return false;
		m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
		if (!m_Root)
			return false;
		Widget panel = m_Root.FindAnyWidget("PinnedMonitorPanel");
		if (!panel)
		{
			Stop();
			return false;
		}
		SetDisplaySlot(slot);
		m_Values = TextWidget.Cast(m_Root.FindAnyWidget("PinnedValues"));
		m_Background = CanvasWidget.Cast(m_Root.FindAnyWidget("PinnedBackground"));
		m_State = TextWidget.Cast(m_Root.FindAnyWidget("PinnedState"));
		m_Alarm = TextWidget.Cast(m_Root.FindAnyWidget("PinnedAlarm"));
		TextWidget name = TextWidget.Cast(m_Root.FindAnyWidget("PinnedName"));
		if (!m_Background || !m_Values || !m_State || !m_Alarm || !name)
		{
			Stop();
			return false;
		}
		m_PlayerController = playerController;
		name.SetText(patientName);
		SetUnavailable();
		GetGame().GetCallqueue().CallLaterByName(this, "Update", 1000, true);
		Update();
		return IsActive();
	}

	void Stop(string reason = "cleanup")
	{
		SCR_PlayerController playerController = m_PlayerController;
		if (m_Root)
			Print(string.Format("RAMI pinned monitor: stop patient=%1, reason=%2", m_PatientId, reason), LogLevel.NORMAL);
		if (GetGame())
		{
			GetGame().GetCallqueue().RemoveByName(this, "Update");
			GetGame().GetCallqueue().RemoveByName(this, "UpdateAlertVisuals");
		}
		StopWarningSound();
		m_IsCritical = false;
		if (m_Root)
			m_Root.RemoveFromHierarchy();
		m_Root = null;
		m_Background = null;
		m_BackgroundFill = null;
		m_AlertBorder = null;
		m_BackgroundCommands.Clear();
		m_PlayerController = null;
		m_PatientId = RplId.Invalid();
		m_HasSample = false;
		if (playerController)
			playerController.RAMI_RequestPinnedMonitorLayout();
	}

	void ~RAMI_PinnedPatientMonitor()
	{
		Stop();
	}

	bool IsActive()
	{
		return m_Root != null;
	}

	void SetDisplaySlot(int slot)
	{
		if (!m_Root)
			return;
		Widget panel = m_Root.FindAnyWidget("PinnedMonitorPanel");
		if (!panel)
			return;
		FrameSlot.SetAnchorMin(panel, 0.695 + slot * 0.1, 0.015);
		FrameSlot.SetAnchorMax(panel, 0.785 + slot * 0.1, 0.175);
	}

	bool IsPatient(SCR_ChimeraCharacter patient)
	{
		return IsActive() && patient && m_PatientId == Replication.FindItemId(patient);
	}

	bool IsAlarmEnabled()
	{
		return m_AlarmEnabled;
	}

	void ToggleAlarm()
	{
		m_AlarmEnabled = !m_AlarmEnabled;
		m_LastAlarmState = 0;
		if (!m_AlarmEnabled)
			StopWarningSound();
		UpdateAlarmLabel();
	}

	void ReceiveSample(RplId patientId)
	{
		if (!IsActive() || patientId != m_PatientId)
			return;
		string reason = GetUnavailableReason(SCR_ChimeraCharacter.Cast(Replication.FindItem(m_PatientId)));
		if (!reason.IsEmpty())
		{
			Stop(reason);
			return;
		}
		m_LastSampleTime = System.GetTickCount();
		m_HasSample = true;
		DrawSample();
	}

	// Named ScriptCallQueue callbacks require public visibility.
	void Update()
	{
		if (!m_PlayerController || GetGame().GetPlayerController() != m_PlayerController)
		{
			Stop("observer_controller_unavailable");
			return;
		}
		SCR_ChimeraCharacter patient = SCR_ChimeraCharacter.Cast(Replication.FindItem(m_PatientId));
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		string reason = GetUnavailableReason(patient);
		if (!user || !reason.IsEmpty())
		{
			Stop(reason);
			return;
		}
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(user.GetCharacterController());
		if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
		{
			Stop("observer_dead_or_unavailable");
			return;
		}
		DrawBackground();
		m_Root.SetVisible(RAMI_MedicalMenuUI.CanOpenMedicalMenu());
		if (!RAMI_MedicalMenuUI.CanOpenMedicalMenu())
		{
			StopWarningSound();
			return;
		}
		if (!m_HasSample || System.GetTickCount(m_LastSampleTime) > SAMPLE_TIMEOUT_MS)
			SetUnavailable();
		ACE_Medical_NetworkComponent network = ACE_Medical_NetworkComponent.Cast(m_PlayerController.FindComponent(ACE_Medical_NetworkComponent));
		if (network)
			network.RequestUpdatePatientData(patient);
	}

	protected void DrawBackground()
	{
		float width, height;
		m_Background.GetScreenSize(width, height);
		if (width <= 0 || height <= 0)
			return;
		// A panel's default style does not provide a solid fill.
		if (!m_BackgroundFill)
		{
			m_AlertBorder = new PolygonDrawCommand();
			m_BackgroundCommands.Insert(m_AlertBorder);
			m_BackgroundFill = new PolygonDrawCommand();
			m_BackgroundCommands.Insert(m_BackgroundFill);
		}
		m_AlertBorder.m_Vertices = {};
		m_Background.TessellateRoundedRectangle(Vector(0, 0, 0), Vector(width, height, 0), BACKGROUND_CORNER_RADIUS, 6, RectangleCorner.All, m_AlertBorder.m_Vertices);
		m_AlertBorder.m_iColor = 0;
		m_BackgroundFill.m_iColor = 0xBF000000;
		float inset = 0;
		if (m_IsCritical)
		{
			inset = 4;
			m_AlertBorder.m_iColor = 0xFFFF3030;
			m_BackgroundFill.m_iColor = 0xF0400000;
			if ((System.GetTickCount() / 500) % 2 == 0)
				m_BackgroundFill.m_iColor = 0xF0A00000;
		}
		m_BackgroundFill.m_Vertices = {};
		m_Background.TessellateRoundedRectangle(Vector(inset, inset, 0), Vector(width - inset, height - inset, 0), BACKGROUND_CORNER_RADIUS - inset, 6, RectangleCorner.All, m_BackgroundFill.m_Vertices);
		m_Background.SetDrawCommands(m_BackgroundCommands);
	}

	protected void SetCritical(bool critical)
	{
		if (m_IsCritical != critical)
		{
			m_IsCritical = critical;
			GetGame().GetCallqueue().RemoveByName(this, "UpdateAlertVisuals");
			if (critical)
				GetGame().GetCallqueue().CallLaterByName(this, "UpdateAlertVisuals", ALERT_VISUAL_INTERVAL_MS, true);
		}
		m_State.SetBold(critical);
		m_Values.SetBold(critical);
		if (!critical)
		{
			m_LastAlarmState = 0;
			StopWarningSound();
		}
		DrawBackground();
	}

	void UpdateAlertVisuals()
	{
		if (!IsActive() || !m_IsCritical)
			return;
		if (!m_HasSample || System.GetTickCount(m_LastSampleTime) > SAMPLE_TIMEOUT_MS)
		{
			SetUnavailable();
			return;
		}
		if (!RAMI_MedicalMenuUI.CanOpenMedicalMenu())
		{
			StopWarningSound();
			return;
		}
		DrawBackground();
	}

	protected void PlayWarning(int alarmState)
	{
		if (!m_AlarmEnabled || !m_IsCritical)
			return;
		int interval = CRITICAL_ALARM_INTERVAL_MS;
		if (alarmState == ACE_Medical_EVitalStateID.CARDIAC_ARREST)
			interval = SEVERE_ALARM_INTERVAL_MS;
		if (alarmState == m_LastAlarmState && System.GetTickCount(m_LastWarningTime) < interval)
			return;
		// Stagger alarms from multiple patients so the samples do not overlap.
		if (s_LastWarningTime != 0 && System.GetTickCount(s_LastWarningTime) < ALARM_MIN_GAP_MS)
			return;
		StopWarningSound();
		m_WarningSound = AudioSystem.PlaySound(ALARM_SOUND);
		m_WarningSoundActive = true;
		m_LastWarningTime = System.GetTickCount();
		s_LastWarningTime = m_LastWarningTime;
		m_LastAlarmState = alarmState;
	}

	protected void StopWarningSound()
	{
		if (m_WarningSoundActive && AudioSystem.IsSoundPlayed(m_WarningSound))
			AudioSystem.TerminateSound(m_WarningSound);
		m_WarningSoundActive = false;
	}

	protected void DrawSample()
	{
		if (!RAMI_MedicalMenuUI.CanOpenMedicalMenu())
			return;
		SCR_ChimeraCharacter patient = SCR_ChimeraCharacter.Cast(Replication.FindItem(m_PatientId));
		if (!patient)
			return;
		ACE_Medical_VitalsComponent vitals = ACE_Medical_VitalsComponent.Cast(patient.FindComponent(ACE_Medical_VitalsComponent));
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(patient.GetCharacterController());
		if (!vitals || !controller)
		{
			SetUnavailable();
			return;
		}
		Tuple2<float, float> pressures = vitals.GetBloodPressures();
		if (!pressures)
		{
			SetUnavailable();
			return;
		}
		m_Values.SetText(string.Format("%1 BPM\n%2 / %3 mmHg", Math.Round(vitals.GetHeartRate()), Math.Round(pressures.param2 * ACE_PhysicalConstants.KPA2MMHG), Math.Round(pressures.param1 * ACE_PhysicalConstants.KPA2MMHG)));
		ACE_Medical_EVitalStateID state = vitals.GetVitalStateID();
		string stateKey = StateKey(state);
		m_State.SetText(WidgetManager.Translate(stateKey));
		bool critical = state == ACE_Medical_EVitalStateID.CRITICAL || state == ACE_Medical_EVitalStateID.CARDIAC_ARREST || state == ACE_Medical_EVitalStateID.RESUSCITATION;
		SetCritical(critical);
		m_State.SetColor(Color.White);
		int alarmState = state;
		PlayWarning(alarmState);
		UpdateAlarmLabel();
	}

	protected void SetUnavailable()
	{
		SetCritical(false);
		m_Values.SetText(WidgetManager.Translate("#RAMI-UNAVAILABLE"));
		m_State.SetText(string.Empty);
		m_State.SetColor(Color.White);
		UpdateAlarmLabel();
	}

	protected void UpdateAlarmLabel()
	{
		string key = "#RAMI-MONITOR_ALARM_OFF";
		if (m_AlarmEnabled)
			key = "#RAMI-MONITOR_ALARM_ON";
		m_Alarm.SetText(WidgetManager.Translate(key));
	}

	protected string StateKey(ACE_Medical_EVitalStateID state)
	{
		switch (state)
		{
			case ACE_Medical_EVitalStateID.STABLE: return "#RAMI-VITAL_STABLE";
			case ACE_Medical_EVitalStateID.UNSTABLE: return "#RAMI-VITAL_UNSTABLE";
			case ACE_Medical_EVitalStateID.CRITICAL: return "#RAMI-VITAL_CRITICAL";
			case ACE_Medical_EVitalStateID.CARDIAC_ARREST: return "#RAMI-VITAL_CARDIAC_ARREST";
			case ACE_Medical_EVitalStateID.RESUSCITATION: return "#RAMI-VITAL_RESUSCITATION";
		}
		return "#RAMI-UNKNOWN";
	}
}
