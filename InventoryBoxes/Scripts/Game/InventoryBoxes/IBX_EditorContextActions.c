class IBX_EditInventoryContextAction : SCR_BaseContextAction
{
	protected ref SCR_UIInfo m_IBXInfo;

	override bool IsEnabled()
	{
		return true;
	}

	override bool IsServer()
	{
		return false;
	}

	override SCR_UIInfo GetInfo()
	{
		if (!m_IBXInfo)
			m_IBXInfo = SCR_UIInfo.CreateInfo("Edit Inventory", "Manage finite crate inventory");

		return m_IBXInfo;
	}

	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return FindEditorComponent(hoveredEntity, selectedEntities) != null;
	}

	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return FindEditorComponent(hoveredEntity, selectedEntities) != null;
	}

	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		IBX_GMInventoryEditorComponent component = FindEditorComponent(hoveredEntity, selectedEntities);
		if (component)
			IBX_GMInventoryEditorUI.Open(component);
	}

	protected IBX_GMInventoryEditorComponent FindEditorComponent(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities)
	{
		IBX_GMInventoryEditorComponent component;
		if (hoveredEntity)
		{
			component = IBX_GMInventoryEditorComponent.Cast(hoveredEntity.GetOwner().FindComponent(IBX_GMInventoryEditorComponent));
			if (component)
				return component;
		}

		foreach (SCR_EditableEntityComponent selectedEntity : selectedEntities)
		{
			component = IBX_GMInventoryEditorComponent.Cast(selectedEntity.GetOwner().FindComponent(IBX_GMInventoryEditorComponent));
			if (component)
				return component;
		}

		return null;
	}
}

class IBX_ExportInventoryPresetContextAction : IBX_EditInventoryContextAction
{
	override SCR_UIInfo GetInfo()
	{
		if (!m_IBXInfo)
			m_IBXInfo = SCR_UIInfo.CreateInfo("Export Inventory Preset", "Copy crate contents as a preset config entry");

		return m_IBXInfo;
	}

	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		IBX_GMInventoryEditorComponent component = FindEditorComponent(hoveredEntity, selectedEntities);
		if (component)
			IBX_GMInventoryEditorUI.OpenForExport(component);
	}
}

class IBX_PasteInventoryContextAction : IBX_EditInventoryContextAction
{
	override SCR_UIInfo GetInfo()
	{
		if (!m_IBXInfo)
			m_IBXInfo = SCR_UIInfo.CreateInfo("Paste Inventory", "Replace crate contents from copied inventory text");

		return m_IBXInfo;
	}

	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		IBX_GMInventoryEditorComponent component = FindEditorComponent(hoveredEntity, selectedEntities);
		if (component)
			IBX_GMInventoryEditorUI.OpenForPaste(component);
	}
}

modded class SCR_ContextActionsEditorComponent
{
	protected ref IBX_EditInventoryContextAction m_IBXInventoryAction;
	protected ref IBX_ExportInventoryPresetContextAction m_IBXExportInventoryAction;
	protected ref IBX_PasteInventoryContextAction m_IBXPasteInventoryAction;

	override void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		if (!m_IBXInventoryAction)
			m_IBXInventoryAction = new IBX_EditInventoryContextAction();
		if (!m_IBXExportInventoryAction)
			m_IBXExportInventoryAction = new IBX_ExportInventoryPresetContextAction();
		if (!m_IBXPasteInventoryAction)
			m_IBXPasteInventoryAction = new IBX_PasteInventoryContextAction();

		int actionIndex = actions.Count();
		actions.Insert(m_IBXInventoryAction);
		actions.Insert(m_IBXExportInventoryAction);
		actions.Insert(m_IBXPasteInventoryAction);
		super.EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
		actions.Remove(actionIndex + 2);
		actions.Remove(actionIndex + 1);
		actions.Remove(actionIndex);
	}
}
