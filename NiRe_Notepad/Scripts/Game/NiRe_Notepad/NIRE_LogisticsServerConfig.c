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

	bool AllowsCrate(ResourceName cratePrefab)
	{
		if (!HasCrateRules())
			return true;

		NIRE_LogisticsCrateRule rule = FindRule(cratePrefab);
		return rule && rule.IsValid();
	}

	//! How many of one item a single crate of this prefab may carry: -1 for any amount, 0 for none.
	int GetMaximumCount(ResourceName cratePrefab, ResourceName itemPrefab)
	{
		if (!HasCrateRules())
			return -1;

		NIRE_LogisticsCrateRule rule = FindRule(cratePrefab);
		if (!rule || !rule.IsValid())
			return 0;

		return rule.GetMaximumCount(itemPrefab);
	}

	//! A request is only refused when one of its items has no crate that may carry it. Quantities no
	//! longer matter here, because a large request is split over as many crates as it needs.
	bool AllowsAnyCrateContents(notnull array<ResourceName> itemPrefabs)
	{
		if (!HasCrateRules())
			return true;

		foreach (ResourceName itemPrefab : itemPrefabs)
		{
			bool allowed = false;
			foreach (NIRE_LogisticsCrateRule rule : m_aCrates)
			{
				if (rule && rule.IsValid() && rule.GetMaximumCount(itemPrefab) != 0)
					allowed = true;
			}
			if (!allowed)
				return false;
		}

		return true;
	}

	protected NIRE_LogisticsCrateRule FindRule(ResourceName cratePrefab)
	{
		foreach (NIRE_LogisticsCrateRule rule : m_aCrates)
		{
			if (rule && rule.m_sCratePrefab == cratePrefab)
				return rule;
		}

		return null;
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

	//! -1 when the rule allows any amount of the item, 0 when it does not allow it at all.
	int GetMaximumCount(ResourceName itemPrefab)
	{
		string allowedItems = m_sAllowedItems.Trim();
		if (allowedItems.IsEmpty())
			return -1;

		array<string> entries = {};
		allowedItems.Split(";", entries, true);
		foreach (string entry : entries)
		{
			array<string> fields = {};
			entry.Split("=", fields, false);
			if (fields.Count() == 2 && fields[1].Trim() == itemPrefab)
			{
				int maximum = fields[0].ToInt();
				if (maximum < 0)
					return 0;

				return maximum;
			}
		}

		return 0;
	}
}
