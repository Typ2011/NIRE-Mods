class TSW_HudConfig
{
	string slowLabel;
	string normalLabel;
	string fastLabel;

	void TSW_HudConfig()
	{
		slowLabel = "Slow";
		normalLabel = "Standard";
		fastLabel = "Fast";
	}

	static TSW_HudConfig CreateDefault()
	{
		return new TSW_HudConfig();
	}

	bool SaveToFile(string filePath)
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("slowLabel", slowLabel);
		saveContext.WriteValue("normalLabel", normalLabel);
		saveContext.WriteValue("fastLabel", fastLabel);
		return saveContext.SaveToFile(filePath);
	}

	string ExportToJson()
	{
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("slowLabel", slowLabel);
		saveContext.WriteValue("normalLabel", normalLabel);
		saveContext.WriteValue("fastLabel", fastLabel);
		return saveContext.ExportToString();
	}

	static TSW_HudConfig LoadFromFile(string filePath)
	{
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.LoadFromFile(filePath))
			return null;

		return ReadFromLoadContext(loadContext);
	}

	static TSW_HudConfig LoadFromString(string jsonData)
	{
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.ImportFromString(jsonData))
			return null;

		return ReadFromLoadContext(loadContext);
	}

	protected static TSW_HudConfig ReadFromLoadContext(SCR_JsonLoadContext loadContext)
	{
		TSW_HudConfig config = CreateDefault();
		loadContext.ReadValue("slowLabel", config.slowLabel);
		loadContext.ReadValue("normalLabel", config.normalLabel);
		loadContext.ReadValue("fastLabel", config.fastLabel);
		return config;
	}
}
