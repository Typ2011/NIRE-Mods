//! Crates are exempt from the vehicle depot's per-player placing cooldown.
//! Requesting a vehicle starts a long cooldown on the provider, and vanilla's
//! server check blocks every prefab at that provider while it runs, so a player
//! who took a car could not take a crate for minutes afterwards.
modded class SCR_CampaignBuildingPlacingEditorComponent
{
	override protected bool CanPlaceEntityServer(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool updatePreview, bool showNotification, int prefabID = -1, int playerID = -1, SCR_EditorPreviewParams params = null)
	{
		//--- playerID is read by the vanilla body for the cooldown gate and nothing else -
		//--- SCR_PlacingEditorComponent.CanPlaceEntityServer ignores it - so hiding it is
		//--- the whole exemption, and supplies, rank and label checks still run.
		if (IBX_IsCrate(editableEntitySource))
			playerID = -1;

		return super.CanPlaceEntityServer(editableEntitySource, blockingBudget, updatePreview, showNotification, prefabID, playerID, params);
	}

	protected bool IBX_IsCrate(IEntityComponentSource editableEntitySource)
	{
		if (!editableEntitySource)
			return false;

		SCR_EditableEntityUIInfo info = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
		return info && info.HasEntityLabel(EEditableEntityLabel.IBX_INVENTORY_BOXES);
	}
}
