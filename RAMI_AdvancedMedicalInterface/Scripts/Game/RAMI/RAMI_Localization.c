class RAMI_Localization
{
	// RPC payloads retain keys until display, so the server language never determines the client's text.
	static string TranslateTokens(string text)
	{
		string result;
		int offset;
		while (offset < text.Length())
		{
			int start = text.IndexOfFrom(offset, "#");
			if (start < 0)
				return result + text.Substring(offset, text.Length() - offset);

			result += text.Substring(offset, start - offset);
			int end = start + 1;
			while (end < text.Length() && "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-".Contains(text.Substring(end, 1)))
				end++;

			string token = text.Substring(start, end - start);
			if (token.Length() > 1 && "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz".Contains(token.Substring(1, 1)))
				result += WidgetManager.Translate(token);
			else
				result += token;
			offset = end;
		}
		return result;
	}

	static string TranslateActivity(string message)
	{
		array<string> fields = {};
		message.Split(" // ", fields, false);
		if (fields.Count() < 2)
			return message;

		bool hasAuthor = fields[0] != "TRIAGE SET" && !fields[1].StartsWith("TOURNIQUET ");
		int authorIndex = 2;
		if (fields[0] == "TREATMENT" && fields[1] == "MEDICAL KIT")
			authorIndex = 3;
		string result;
		foreach (int index, string field : fields)
		{
			if (index > 0)
				result += " // ";
			// Author names are opaque user text, even when they resemble a localization key.
			if (hasAuthor && index >= authorIndex)
				result += field;
			else
				result += TranslateActivityField(field);
		}
		return result;
	}

	protected static string TranslateActivityField(string field)
	{
		switch (field)
		{
			case "TRIAGE SET": return WidgetManager.Translate("#RAMI-TRIAGE_SET");
			case "TREATMENT": return WidgetManager.Translate("#RAMI-TREATMENT");
			case "MEDICATION": return WidgetManager.Translate("#RAMI-MEDICATION");
			case "MEDICAL KIT": return WidgetManager.Translate("#RAMI-MEDICAL_KIT");
			case "MEDICAL ITEM": return WidgetManager.Translate("#RAMI-MEDICAL_ITEM");
			case "TOURNIQUET APPLIED": return WidgetManager.Translate("#RAMI-TOURNIQUET") + " " + WidgetManager.Translate("#RAMI-APPLIED");
			case "TOURNIQUET REMOVED": return WidgetManager.Translate("#RAMI-TOURNIQUET_REMOVED");
			case "HEAD": return WidgetManager.Translate("#RAMI-HEAD");
			case "UPPERTORSO": return WidgetManager.Translate("#RAMI-CHEST");
			case "LOWERTORSO": return WidgetManager.Translate("#RAMI-ABDOMEN");
			case "LEFTARM": return WidgetManager.Translate("#RAMI-LEFT_ARM");
			case "RIGHTARM": return WidgetManager.Translate("#RAMI-RIGHT_ARM");
			case "LEFTLEG": return WidgetManager.Translate("#RAMI-LEFT_LEG");
			case "RIGHTLEG": return WidgetManager.Translate("#RAMI-RIGHT_LEG");
			case "NONE": return WidgetManager.Translate("#RAMI-NONE");
			case "MINIMAL": return WidgetManager.Translate("#RAMI-MINIMAL");
			case "DELAYED": return WidgetManager.Translate("#RAMI-DELAYED");
			case "IMMEDIATE": return WidgetManager.Translate("#RAMI-IMMEDIATE");
			case "EXPECTANT": return WidgetManager.Translate("#RAMI-DECEASED");
		}
		return TranslateTokens(field);
	}
}
