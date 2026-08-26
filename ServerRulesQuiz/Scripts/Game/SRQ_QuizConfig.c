// SRQ_QuizConfig.c - Data structures for quiz configuration

class SRQ_QuizQuestion
{
	string question;
	ref array<string> answers;
	int correctIndex;
	
	void SRQ_QuizQuestion()
	{
		answers = new array<string>();
	}
	
	bool IsCorrect(int selectedIndex)
	{
		return selectedIndex == correctIndex;
	}
}

class SRQ_QuizConfig
{
	ref array<string> rules;
	int passingScore;
	int maxAttemptsBeforeKick;
	bool showQuizOnEveryConnect;
	ref array<ref SRQ_QuizQuestion> questions;
	
	void SRQ_QuizConfig()
	{
		rules = new array<string>();
		questions = new array<ref SRQ_QuizQuestion>();
		passingScore = 1;
		maxAttemptsBeforeKick = 0;
		showQuizOnEveryConnect = false;
	}
	
	static SRQ_QuizConfig CreateDefault()
	{
		SRQ_QuizConfig config = new SRQ_QuizConfig();
		config.rules.Insert("1. Be respectful to all players.");
		config.rules.Insert("2. No teamkilling or griefing.");
		config.rules.Insert("3. Follow orders from squad leaders.");
		config.rules.Insert("4. No cheating or exploiting bugs.");
		config.rules.Insert("5. Use appropriate language in chat.");
		config.rules.Insert("6. Do not block spawn points or vehicles.");
		config.rules.Insert("7. Report issues to admins, do not retaliate.");
		config.passingScore = 2;
		config.maxAttemptsBeforeKick = 3;
		config.showQuizOnEveryConnect = false;
		
		SRQ_QuizQuestion questionA = new SRQ_QuizQuestion();
		questionA.question = "Is teamkilling allowed on this server?";
		questionA.answers.Insert("Yes, always");
		questionA.answers.Insert("Only if they deserve it");
		questionA.answers.Insert("No, never");
		questionA.answers.Insert("Only in self-defense");
		questionA.correctIndex = 2;
		config.questions.Insert(questionA);
		
		SRQ_QuizQuestion questionB = new SRQ_QuizQuestion();
		questionB.question = "What should you do if someone breaks the rules?";
		questionB.answers.Insert("Teamkill them");
		questionB.answers.Insert("Report to admins");
		questionB.answers.Insert("Leave the server");
		questionB.answers.Insert("Ignore it");
		questionB.correctIndex = 1;
		config.questions.Insert(questionB);
		
		SRQ_QuizQuestion questionC = new SRQ_QuizQuestion();
		questionC.question = "Should you follow squad leader orders?";
		questionC.answers.Insert("No, play however you want");
		questionC.answers.Insert("Only if you agree with them");
		questionC.answers.Insert("Yes, follow orders");
		questionC.answers.Insert("Squad leaders have no authority");
		questionC.correctIndex = 2;
		config.questions.Insert(questionC);
		
		return config;
	}
	
	bool SaveToFile(string filePath)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("rules", rules);
		saveContext.WriteValue("passingScore", passingScore);
		saveContext.WriteValue("maxAttemptsBeforeKick", maxAttemptsBeforeKick);
		saveContext.WriteValue("showQuizOnEveryConnect", showQuizOnEveryConnect);
		saveContext.WriteValue("questions", questions);
		return saveContext.SaveToFile(filePath);
	}
	
	string ExportToJson()
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("rules", rules);
		saveContext.WriteValue("passingScore", passingScore);
		saveContext.WriteValue("maxAttemptsBeforeKick", maxAttemptsBeforeKick);
		saveContext.WriteValue("showQuizOnEveryConnect", showQuizOnEveryConnect);
		saveContext.WriteValue("questions", questions);
		return saveContext.ExportToString();
	}
	
	static SRQ_QuizConfig LoadFromString(string jsonData)
	{
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.ImportFromString(jsonData))
		{
			Print("[SRQ] Failed to import config from JSON string", LogLevel.ERROR);
			return null;
		}
		
		return ReadFromLoadContext(loadContext);
	}
	
	static SRQ_QuizConfig LoadFromFile(string filePath)
	{
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.LoadFromFile(filePath))
		{
			Print("[SRQ] Failed to load config from: " + filePath, LogLevel.ERROR);
			return null;
		}
		
		return ReadFromLoadContext(loadContext);
	}
	
	protected static SRQ_QuizConfig ReadFromLoadContext(SCR_JsonLoadContext loadContext)
	{
		SRQ_QuizConfig config = new SRQ_QuizConfig();
		
		if (!loadContext.ReadValue("rules", config.rules))
			Print("[SRQ] Failed to read 'rules' from config", LogLevel.WARNING);
		
		if (!loadContext.ReadValue("passingScore", config.passingScore))
			config.passingScore = 1;
		
		if (!loadContext.ReadValue("maxAttemptsBeforeKick", config.maxAttemptsBeforeKick))
			config.maxAttemptsBeforeKick = 0;
		
		if (!loadContext.ReadValue("showQuizOnEveryConnect", config.showQuizOnEveryConnect))
			config.showQuizOnEveryConnect = false;
		
		array<ref SRQ_QuizQuestion> tempQuestions = new array<ref SRQ_QuizQuestion>();
		if (loadContext.ReadValue("questions", tempQuestions))
			config.questions = tempQuestions;
		
		Print(string.Format("[SRQ] Loaded config: %1 rules, %2 questions, passing score: %3, max attempts: %4, repeat on connect: %5",
			config.rules.Count(),
			config.questions.Count(),
			config.passingScore,
			config.maxAttemptsBeforeKick,
			config.showQuizOnEveryConnect), LogLevel.NORMAL);
		
		return config;
	}
	
	int GetQuestionCount()
	{
		return questions.Count();
	}
	
	SRQ_QuizQuestion GetQuestion(int index)
	{
		if (index < 0 || index >= questions.Count())
			return null;
		return questions[index];
	}
	
	string GetRulesText()
	{
		string result = "";
		foreach (int i, string rule : rules)
		{
			if (i > 0)
				result += "\n";
			result += rule;
		}
		return result;
	}
}
