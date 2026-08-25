[BaseContainerProps(configRoot: true)]
class IBX_CratePresetConfig : ScriptAndConfig
{
	protected const ResourceName CONFIG = "{A1E6470C8DBF4260}Configs/Inventory/CratePresets.conf";

	[Attribute(desc: "Named crate inventory presets")]
	ref array<ref IBX_CratePreset> m_aPresets;

	static IBX_CratePresetConfig Load()
	{
		Resource holder = BaseContainerTools.LoadContainer(CONFIG);
		if (!holder || !holder.IsValid())
			return null;

		return IBX_CratePresetConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class IBX_CratePreset
{
	[Attribute("New Preset", UIWidgets.EditBox, "Preset name")]
	string m_sName;

	[Attribute("", UIWidgets.EditBox, "Items: Count=Prefab;Count=Prefab")]
	string m_sItems;

	ref array<ResourceName> m_Prefabs = {};
	ref array<int> m_Counts = {};
	int m_TotalItems;

	bool Parse()
	{
		m_sName = m_sName.Trim();
		if (m_sName.IsEmpty())
			return false;

		m_Prefabs.Clear();
		m_Counts.Clear();
		m_TotalItems = 0;

		array<string> entries = {};
		m_sItems.Split(";", entries, true);
		set<ResourceName> uniquePrefabs = new set<ResourceName>();
		foreach (string entry : entries)
		{
			array<string> fields = {};
			entry.Split("=", fields, false);
			if (fields.Count() != 2)
				return false;

			int count = fields[0].ToInt();
			ResourceName prefab = fields[1].Trim();
			if (count < 1 || count > IBX_GMInventoryEditorComponent.MAX_MUTATION_QUANTITY || prefab.IsEmpty() || uniquePrefabs.Contains(prefab))
				return false;

			uniquePrefabs.Insert(prefab);
			m_Prefabs.Insert(prefab);
			m_Counts.Insert(count);
			m_TotalItems += count;
		}

		if (m_Prefabs.Count() > IBX_GMInventoryEditorComponent.MAX_ITEM_TYPES || m_TotalItems > IBX_GMInventoryEditorComponent.MAX_PRESET_ITEMS)
			return false;

		return true;
	}
}
