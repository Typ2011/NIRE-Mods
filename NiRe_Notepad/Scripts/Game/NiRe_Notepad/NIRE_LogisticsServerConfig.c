[BaseContainerProps(configRoot: true)]
class NIRE_LogisticsServerConfig : ScriptAndConfig
{
	protected static const ResourceName CONFIG = "{24B93F54830447C0}Configs/Server/ServerConfig.conf";

	[Attribute(desc: "Allowed logistics crates and their maximum requested contents. Empty keeps the unrestricted default.")]
	ref array<ref NIRE_LogisticsCrateRule> m_aCrates = {};

	static NIRE_LogisticsServerConfig Load()
	{
		Resource holder = BaseContainerTools.LoadContainer(CONFIG);
		if (!holder || !holder.IsValid())
			return null;

		return NIRE_LogisticsServerConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
	}

	bool HasCrateRules()
	{
		return !m_aCrates.IsEmpty();
	}

	bool AllowsCrateContents(ResourceName cratePrefab, notnull array<ResourceName> itemPrefabs, notnull array<int> itemCounts)
	{
		if (!HasCrateRules())
			return true;

		foreach (NIRE_LogisticsCrateRule rule : m_aCrates)
		{
			if (rule && rule.m_sCratePrefab == cratePrefab)
				return rule.IsValid() && rule.AllowsContents(itemPrefabs, itemCounts);
		}

		return false;
	}

	bool AllowsAnyCrateContents(notnull array<ResourceName> itemPrefabs, notnull array<int> itemCounts)
	{
		if (!HasCrateRules())
			return true;

		foreach (NIRE_LogisticsCrateRule rule : m_aCrates)
		{
			if (rule && rule.IsValid() && rule.AllowsContents(itemPrefabs, itemCounts))
				return true;
		}

		return false;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sCratePrefab")]
class NIRE_LogisticsCrateRule
{
	[Attribute("", UIWidgets.EditBox, "InventoryBoxes crate prefab")]
	ResourceName m_sCratePrefab;

	[Attribute("", UIWidgets.EditBox, "Allowed contents: MaxCount=ItemPrefab;MaxCount=ItemPrefab. Empty allows all requested arsenal items.")]
	string m_sAllowedItems;

	bool IsValid()
	{
		Resource resource = Resource.Load(m_sCratePrefab);
		return resource && resource.IsValid() && SCR_BaseContainerTools.FindComponentSource(resource, IBX_GMInventoryEditorComponent);
	}

	bool AllowsContents(notnull array<ResourceName> itemPrefabs, notnull array<int> itemCounts)
	{
		string allowedItems = m_sAllowedItems.Trim();
		if (allowedItems.IsEmpty())
			return true;

		map<ResourceName, int> maximumCounts = new map<ResourceName, int>();
		array<string> entries = {};
		allowedItems.Split(";", entries, true);
		foreach (string entry : entries)
		{
			array<string> fields = {};
			entry.Split("=", fields, false);
			if (fields.Count() != 2)
				return false;

			int maximum = fields[0].ToInt();
			ResourceName prefab = fields[1].Trim();
			if (maximum < 1 || prefab.IsEmpty() || maximumCounts.Contains(prefab))
				return false;

			maximumCounts.Insert(prefab, maximum);
		}

		if (itemPrefabs.Count() != itemCounts.Count())
			return false;
		foreach (int index, ResourceName prefab : itemPrefabs)
		{
			int maximum;
			if (!maximumCounts.Find(prefab, maximum) || itemCounts[index] > maximum)
				return false;
		}

		return true;
	}
}
