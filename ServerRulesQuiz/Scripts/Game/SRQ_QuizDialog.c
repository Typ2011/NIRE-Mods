// SRQ_QuizDialog.c - Configurable dialog-based quiz UI

class SRQ_QuizDialog : SCR_ConfigurableDialogUi
{
	protected const ResourceName PRESET_PATH = "Config/SRQ_QuizDialogPresets.conf";
	protected const string PRESET_TAG = "srq_quiz";
	protected const int FEEDBACK_DELAY_MS = 1400;
	protected const int COLOR_DEFAULT = 0xFF2A2E35;
	protected const int COLOR_FOCUSED = 0xFF5a6445;
	protected const int COLOR_TEXT = 0xFFFFFFFF;
	protected const int COLOR_BTN_CORRECT = 0xFF1B5E20;
	protected const int COLOR_BTN_WRONG = 0xFFB71C1C;
	protected const int COLOR_BANNER_CORRECT = 0xCC1B5E20;
	protected const int COLOR_BANNER_WRONG = 0xCCB71C1C;
	
	protected TextWidget m_wSRQTitle;
	protected TextWidget m_wSRQProgress;
	protected RichTextWidget m_wSRQRulesText;
	protected RichTextWidget m_wSRQContentText;
	protected ImageWidget m_wSRQFeedbackBanner;
	protected TextWidget m_wSRQFeedbackText;
	protected TextWidget m_wSRQScore;
	protected ButtonWidget m_wSRQConfirmButton;
	protected ref array<ButtonWidget> m_aSRQAnswerButtons;
	
	protected SCR_PlayerController m_SRQPlayerController;
	protected ref SRQ_QuizConfig m_SRQConfig;
	
	protected int m_iSRQCurrentQuestion;
	protected int m_iSRQCorrectAnswers;
	protected int m_iSRQTotalAnswered;
	protected bool m_bSRQShowingRules;
	protected bool m_bSRQQuizPassed;
	protected bool m_bSRQAwaitingFeedback;
	protected bool m_bSRQResultSubmitted;
	protected bool m_bSRQInitialized;
	protected int m_iSRQFocusedAnswer;
	
	void SRQ_QuizDialog()
	{
		m_aSRQAnswerButtons = new array<ButtonWidget>();
		m_iSRQCurrentQuestion = 0;
		m_iSRQCorrectAnswers = 0;
		m_iSRQTotalAnswered = 0;
		m_bSRQShowingRules = true;
		m_bSRQQuizPassed = false;
		m_bSRQAwaitingFeedback = false;
		m_bSRQResultSubmitted = false;
		m_bSRQInitialized = false;
		m_iSRQFocusedAnswer = -1;
	}
	
	void ~SRQ_QuizDialog()
	{
		ArmaReforgerScripted game = ArmaReforgerScripted.Cast(GetGame());
		if (game)
			game.GetCallqueue().RemoveByName(this, "AdvanceQuestion");
	}
	
	void ShowDialog(SCR_PlayerController playerController, SRQ_QuizConfig config)
	{
		m_SRQPlayerController = playerController;
		m_SRQConfig = config;
		
		if (!m_SRQConfig)
		{
			Print("[SRQ] No config available for quiz dialog", LogLevel.ERROR);
			return;
		}
		
		if (!SCR_ConfigurableDialogUi.IsPresetValid(PRESET_PATH, PRESET_TAG))
		{
			Print("[SRQ] Quiz dialog preset is invalid: " + PRESET_PATH, LogLevel.ERROR);
			return;
		}
		
		SCR_ConfigurableDialogUi.CreateFromPreset(PRESET_PATH, PRESET_TAG, this);
	}
	
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		SRQ_InitializeWidgets();
		SRQ_ShowRules();
		Print("[SRQ] Configurable quiz dialog opened", LogLevel.NORMAL);
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		SRQ_UpdateFocusedButtonState();
	}
	
	override void OnMenuClose()
	{
		super.OnMenuClose();
		if (m_SRQPlayerController)
			m_SRQPlayerController.SRQ_OnQuizClosed();
	}
	
	protected void SRQ_InitializeWidgets()
	{
		if (m_bSRQInitialized)
			return;
		
		Widget root = GetRootWidget();
		if (!root)
			return;
		
		m_wSRQTitle = TextWidget.Cast(root.FindAnyWidget("TitleText"));
		m_wSRQProgress = TextWidget.Cast(root.FindAnyWidget("ProgressText"));
		m_wSRQRulesText = RichTextWidget.Cast(root.FindAnyWidget("RulesText"));
		m_wSRQContentText = RichTextWidget.Cast(root.FindAnyWidget("ContentText"));
		m_wSRQFeedbackBanner = ImageWidget.Cast(root.FindAnyWidget("FeedbackBanner"));
		m_wSRQFeedbackText = TextWidget.Cast(root.FindAnyWidget("FeedbackText"));
		m_wSRQScore = TextWidget.Cast(root.FindAnyWidget("ScoreText"));
		m_wSRQConfirmButton = ButtonWidget.Cast(root.FindAnyWidget("ConfirmButton"));
		
		for (int i = 0; i < 4; i++)
		{
			ButtonWidget btn = ButtonWidget.Cast(root.FindAnyWidget("Answer" + i));
			if (btn)
				m_aSRQAnswerButtons.Insert(btn);
		}
		
		SRQ_ApplyTheme();
		m_bSRQInitialized = true;
	}
	
	protected void SRQ_ApplyTheme()
	{
		if (m_wSRQTitle)
			m_wSRQTitle.SetColorInt(COLOR_TEXT);
		if (m_wSRQProgress)
			m_wSRQProgress.SetColorInt(COLOR_TEXT);
		if (m_wSRQRulesText)
			m_wSRQRulesText.SetColorInt(COLOR_TEXT);
		if (m_wSRQContentText)
			m_wSRQContentText.SetColorInt(COLOR_TEXT);
		if (m_wSRQFeedbackText)
			m_wSRQFeedbackText.SetColorInt(COLOR_TEXT);
		if (m_wSRQScore)
			m_wSRQScore.SetColorInt(COLOR_TEXT);
		
		foreach (ButtonWidget btn : m_aSRQAnswerButtons)
		{
			btn.SetColorInt(COLOR_DEFAULT);
			SRQ_SetButtonText(btn, "");
		}
		
		if (m_wSRQConfirmButton)
		{
			m_wSRQConfirmButton.SetColorInt(COLOR_DEFAULT);
			SRQ_SetButtonText(m_wSRQConfirmButton, "Continue");
		}
	}
	
	protected void SRQ_ShowRules()
	{
		m_bSRQShowingRules = true;
		m_bSRQAwaitingFeedback = false;
		m_bSRQResultSubmitted = false;
		m_iSRQFocusedAnswer = -1;
		
		if (m_wSRQTitle)
			m_wSRQTitle.SetText("Server Rules");
		if (m_wSRQProgress)
			m_wSRQProgress.SetText("Read carefully before starting the quiz");
		if (m_wSRQRulesText)
		{
			m_wSRQRulesText.SetText(m_SRQConfig.GetRulesText());
			m_wSRQRulesText.SetVisible(true);
		}
		if (m_wSRQContentText)
			m_wSRQContentText.SetVisible(false);
		if (m_wSRQFeedbackBanner)
			m_wSRQFeedbackBanner.SetVisible(false);
		if (m_wSRQFeedbackText)
			m_wSRQFeedbackText.SetVisible(false);
		
		foreach (ButtonWidget btn : m_aSRQAnswerButtons)
			btn.SetVisible(false);
		
		if (m_wSRQConfirmButton)
		{
			m_wSRQConfirmButton.SetVisible(true);
			m_wSRQConfirmButton.SetColorInt(COLOR_DEFAULT);
			SRQ_SetButtonText(m_wSRQConfirmButton, "I've Read the Rules - Start Quiz");
			SRQ_SetFocusedWidget(m_wSRQConfirmButton);
		}
		
		if (m_wSRQScore)
			m_wSRQScore.SetText("Read all rules before starting");
	}
	
	protected void SRQ_ShowQuestion(int index)
	{
		m_bSRQShowingRules = false;
		m_bSRQAwaitingFeedback = false;
		m_iSRQCurrentQuestion = index;
		m_iSRQFocusedAnswer = -1;
		
		SRQ_QuizQuestion question = m_SRQConfig.GetQuestion(index);
		if (!question)
		{
			SRQ_OnQuizComplete();
			return;
		}
		
		if (m_wSRQTitle)
			m_wSRQTitle.SetText("Quiz");
		if (m_wSRQProgress)
			m_wSRQProgress.SetText(string.Format("Question %1 / %2", index + 1, m_SRQConfig.GetQuestionCount()));
		if (m_wSRQRulesText)
			m_wSRQRulesText.SetVisible(false);
		if (m_wSRQContentText)
		{
			m_wSRQContentText.SetVisible(true);
			m_wSRQContentText.SetText(question.question);
		}
		if (m_wSRQFeedbackBanner)
			m_wSRQFeedbackBanner.SetVisible(false);
		if (m_wSRQFeedbackText)
			m_wSRQFeedbackText.SetVisible(false);
		if (m_wSRQConfirmButton)
			m_wSRQConfirmButton.SetVisible(false);
		
		for (int i = 0; i < m_aSRQAnswerButtons.Count(); i++)
		{
			ButtonWidget btn = m_aSRQAnswerButtons[i];
			if (i < question.answers.Count())
			{
				btn.SetVisible(true);
				btn.SetEnabled(true);
				btn.SetColorInt(COLOR_DEFAULT);
				SRQ_SetButtonText(btn, question.answers[i]);
			}
			else
			{
				btn.SetVisible(false);
			}
		}
		
		SRQ_SetFocusedWidget(SRQ_GetFirstVisibleAnswerButton());
		SRQ_UpdateFocusedButtonState();
		SRQ_UpdateScoreDisplay();
	}
	
	protected void SRQ_OnQuizComplete()
	{
		bool passed = (m_iSRQCorrectAnswers >= m_SRQConfig.passingScore);
		m_bSRQQuizPassed = passed;
		
		foreach (ButtonWidget btn : m_aSRQAnswerButtons)
			btn.SetVisible(false);
		if (m_wSRQFeedbackBanner)
			m_wSRQFeedbackBanner.SetVisible(false);
		if (m_wSRQFeedbackText)
			m_wSRQFeedbackText.SetVisible(false);
		if (m_wSRQRulesText)
			m_wSRQRulesText.SetVisible(false);
		
		if (!m_bSRQResultSubmitted && m_SRQPlayerController)
		{
			m_bSRQResultSubmitted = true;
			if (passed)
				m_SRQPlayerController.SRQ_OnQuizPassed(m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount());
			else
				m_SRQPlayerController.SRQ_OnQuizFailed(m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount());
		}
		
		if (passed)
		{
			if (m_wSRQTitle)
				m_wSRQTitle.SetText("Quiz Passed!");
			if (m_wSRQProgress)
				m_wSRQProgress.SetText(string.Format("%1 / %2 correct", m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount()));
			if (m_wSRQContentText)
			{
				m_wSRQContentText.SetVisible(true);
				m_wSRQContentText.SetText(string.Format("Congratulations! You answered %1 out of %2 questions correctly.\n\nYou may now spawn and play. Welcome to the server!", m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount()));
			}
			if (m_wSRQScore)
				m_wSRQScore.SetText("Welcome!");
			if (m_wSRQConfirmButton)
			{
				m_wSRQConfirmButton.SetVisible(true);
				m_wSRQConfirmButton.SetColorInt(COLOR_DEFAULT);
				SRQ_SetButtonText(m_wSRQConfirmButton, "Continue to Game");
				SRQ_SetFocusedWidget(m_wSRQConfirmButton);
			}
		}
		else
		{
			if (m_wSRQTitle)
				m_wSRQTitle.SetText("Quiz Failed");
			if (m_wSRQProgress)
				m_wSRQProgress.SetText(string.Format("%1 / %2 correct", m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount()));
			if (m_wSRQContentText)
			{
				m_wSRQContentText.SetVisible(true);
				m_wSRQContentText.SetText(string.Format("You answered %1 out of %2 questions correctly.\nYou need at least %3 correct answers to pass.\n\nPlease re-read the rules carefully and try again.", m_iSRQCorrectAnswers, m_SRQConfig.GetQuestionCount(), m_SRQConfig.passingScore));
			}
			if (m_wSRQScore)
				m_wSRQScore.SetText("Read the rules and try again.");
			if (m_wSRQConfirmButton)
			{
				m_wSRQConfirmButton.SetVisible(true);
				m_wSRQConfirmButton.SetColorInt(COLOR_DEFAULT);
				SRQ_SetButtonText(m_wSRQConfirmButton, "Try Again");
				SRQ_SetFocusedWidget(m_wSRQConfirmButton);
			}
			m_iSRQCorrectAnswers = 0;
			m_iSRQTotalAnswered = 0;
			m_iSRQCurrentQuestion = 0;
		}
	}
	
	protected void SRQ_UpdateScoreDisplay()
	{
		if (m_wSRQScore)
			m_wSRQScore.SetText(string.Format("Score: %1 / %2   (need %3 to pass)", m_iSRQCorrectAnswers, m_iSRQTotalAnswered, m_SRQConfig.passingScore));
	}
	
	protected void SRQ_SetButtonText(ButtonWidget btn, string text)
	{
		if (!btn)
			return;
		
		TextWidget tw = TextWidget.Cast(btn.FindAnyWidget("Text"));
		if (!tw && btn == m_wSRQConfirmButton)
			tw = TextWidget.Cast(GetRootWidget().FindAnyWidget("Text"));
		
		if (tw)
		{
			tw.SetColorInt(COLOR_TEXT);
			tw.SetVisible(true);
			tw.SetText(text);
		}
	}
	
	protected void SRQ_SetFocusedWidget(Widget widget)
	{
		if (!widget)
			return;
		
		WorkspaceWidget workspace = widget.GetWorkspace();
		if (workspace)
			workspace.SetFocusedWidget(widget);
	}
	
	protected ButtonWidget SRQ_GetFirstVisibleAnswerButton()
	{
		foreach (ButtonWidget btn : m_aSRQAnswerButtons)
		{
			if (btn && btn.IsVisible())
				return btn;
		}
		
		return null;
	}
	
	protected void SRQ_UpdateFocusedButtonState()
	{
		if (m_bSRQAwaitingFeedback)
			return;
		
		if (m_bSRQShowingRules || m_bSRQResultSubmitted)
		{
			if (m_wSRQConfirmButton && m_wSRQConfirmButton.IsVisible())
			{
				WorkspaceWidget workspace = m_wSRQConfirmButton.GetWorkspace();
				Widget focused = null;
				if (workspace)
					focused = workspace.GetFocusedWidget();
				
				if (focused == m_wSRQConfirmButton)
					m_wSRQConfirmButton.SetColorInt(COLOR_FOCUSED);
				else
					m_wSRQConfirmButton.SetColorInt(COLOR_DEFAULT);
			}
			
			return;
		}
		
		Widget focusedWidget = null;
		Widget root = GetRootWidget();
		if (root)
		{
			WorkspaceWidget workspace = root.GetWorkspace();
			if (workspace)
				focusedWidget = workspace.GetFocusedWidget();
		}
		
		int focusedAnswer = -1;
		for (int i = 0; i < m_aSRQAnswerButtons.Count(); i++)
		{
			ButtonWidget btn = m_aSRQAnswerButtons[i];
			if (!btn || !btn.IsVisible())
				continue;
			
			if (focusedWidget == btn)
				focusedAnswer = i;
		}
		
		m_iSRQFocusedAnswer = focusedAnswer;
		
		for (int j = 0; j < m_aSRQAnswerButtons.Count(); j++)
		{
			ButtonWidget answerBtn = m_aSRQAnswerButtons[j];
			if (!answerBtn || !answerBtn.IsVisible())
				continue;
			
			if (j == m_iSRQFocusedAnswer)
				answerBtn.SetColorInt(COLOR_FOCUSED);
			else
				answerBtn.SetColorInt(COLOR_DEFAULT);
		}
	}
	
	protected void SRQ_OnAnswerSelected(int answerIndex)
	{
		if (m_bSRQAwaitingFeedback)
			return;
		
		SRQ_QuizQuestion question = m_SRQConfig.GetQuestion(m_iSRQCurrentQuestion);
		if (!question)
			return;
		
		m_bSRQAwaitingFeedback = true;
		m_iSRQFocusedAnswer = answerIndex;
		m_iSRQTotalAnswered++;
		
		bool correct = question.IsCorrect(answerIndex);
		if (correct)
			m_iSRQCorrectAnswers++;
		
		foreach (ButtonWidget btn : m_aSRQAnswerButtons)
			btn.SetEnabled(false);
		
		if (answerIndex < m_aSRQAnswerButtons.Count())
		{
			int btnColor = COLOR_BTN_WRONG;
			if (correct)
				btnColor = COLOR_BTN_CORRECT;
			m_aSRQAnswerButtons[answerIndex].SetColorInt(btnColor);
		}
		
		if (!correct && question.correctIndex < m_aSRQAnswerButtons.Count())
			m_aSRQAnswerButtons[question.correctIndex].SetColorInt(COLOR_BTN_CORRECT);
		
		if (m_wSRQFeedbackBanner)
		{
			m_wSRQFeedbackBanner.SetVisible(true);
			int bannerColor = COLOR_BANNER_WRONG;
			if (correct)
				bannerColor = COLOR_BANNER_CORRECT;
			m_wSRQFeedbackBanner.SetColorInt(bannerColor);
		}
		
		if (m_wSRQFeedbackText)
		{
			m_wSRQFeedbackText.SetVisible(true);
			if (correct)
				m_wSRQFeedbackText.SetText("Correct!");
			else
				m_wSRQFeedbackText.SetText("Wrong - the correct answer was: " + question.answers[question.correctIndex]);
		}
		
		SRQ_UpdateScoreDisplay();
		
		ArmaReforgerScripted game = ArmaReforgerScripted.Cast(GetGame());
		if (game)
			game.GetCallqueue().CallLaterByName(this, "AdvanceQuestion", FEEDBACK_DELAY_MS, false);
	}
	
	void SRQ_PrepareForPause()
	{
		m_bSRQAwaitingFeedback = false;
		
		ArmaReforgerScripted game = ArmaReforgerScripted.Cast(GetGame());
		if (game)
			game.GetCallqueue().RemoveByName(this, "AdvanceQuestion");
	}
	
	int SRQ_GetCurrentQuestionIndex()
	{
		return m_iSRQCurrentQuestion;
	}
	
	int SRQ_GetCorrectAnswers()
	{
		return m_iSRQCorrectAnswers;
	}
	
	int SRQ_GetTotalAnswered()
	{
		return m_iSRQTotalAnswered;
	}
	
	bool SRQ_IsShowingRules()
	{
		return m_bSRQShowingRules;
	}
	
	bool SRQ_HasPassedQuiz()
	{
		return m_bSRQQuizPassed;
	}
	
	bool SRQ_HasSubmittedResult()
	{
		return m_bSRQResultSubmitted;
	}
	
	void SRQ_RestoreState(int currentQuestion, int correctAnswers, int totalAnswered, bool showingRules, bool quizPassed, bool resultSubmitted)
	{
		m_iSRQCurrentQuestion = currentQuestion;
		m_iSRQCorrectAnswers = correctAnswers;
		m_iSRQTotalAnswered = totalAnswered;
		m_bSRQShowingRules = showingRules;
		m_bSRQQuizPassed = quizPassed;
		m_bSRQResultSubmitted = resultSubmitted;
		m_bSRQAwaitingFeedback = false;
		
		if (m_bSRQResultSubmitted)
		{
			SRQ_OnQuizComplete();
			return;
		}
		
		if (m_bSRQShowingRules)
			SRQ_ShowRules();
		else
			SRQ_ShowQuestion(m_iSRQCurrentQuestion);
	}
	
	void AdvanceQuestion()
	{
		m_bSRQAwaitingFeedback = false;
		m_iSRQCurrentQuestion++;
		
		if (m_iSRQCurrentQuestion >= m_SRQConfig.GetQuestionCount())
			SRQ_OnQuizComplete();
		else
			SRQ_ShowQuestion(m_iSRQCurrentQuestion);
	}
	
	protected bool SRQ_HandleActivatedWidget(Widget w)
	{
		Widget current = w;
		while (current)
		{
			if (current == m_wSRQConfirmButton)
			{
				if (m_bSRQShowingRules)
				{
					SRQ_ShowQuestion(0);
				}
				else if (m_bSRQQuizPassed)
				{
					Close();
				}
				else
				{
					SRQ_ShowRules();
				}
				
				return true;
			}
			
			for (int i = 0; i < m_aSRQAnswerButtons.Count(); i++)
			{
				if (current == m_aSRQAnswerButtons[i])
				{
					SRQ_OnAnswerSelected(i);
					return true;
				}
			}
			
			current = current.GetParent();
		}
		
		return false;
	}
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (SRQ_HandleActivatedWidget(w))
			return true;
		
		return super.OnClick(w, x, y, button);
	}
	
	override bool OnController(Widget w, ControlID control, int value)
	{
		if (value && (control == ControlID.BACK || control == ControlID.MENU))
		{
			if (m_SRQPlayerController)
				m_SRQPlayerController.SRQ_OpenPauseMenuFromQuiz();
			else
				ArmaReforgerScripted.OpenPauseMenu(true, false);
			
			return true;
		}
		
		return super.OnController(w, control, value);
	}
}
