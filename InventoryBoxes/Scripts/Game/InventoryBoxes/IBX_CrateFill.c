//! One fill number for a crate, used by every place that shows how full a crate is: the volume bar
//! on the crate's own inventory slot, the bar in the opened crate's storage header, and the Game
//! Master inventory editor.
//!
//! Vanilla computes those bars from occupied volume alone, and each call site resolves the storage
//! component on its own - on a crate that carries both the disabled vanilla storage and the addon's
//! own one, two call sites could end up reading two different components and disagreeing. Here the
//! storage always comes from IBX_GMInventoryEditorComponent, the same one the addon mutates.
//!
//! A crate refuses an item once its cumulative volume or its weight limit is reached
//! (SCR_UniversalInventoryStorageComponent.CanStoreItem), so the number reported is the fuller of
//! the two rather than vanilla's volume alone.
class IBX_CrateFill
{
	//! The crate storage of an entity, or null when the entity is not an addon crate.
	static BaseInventoryStorageComponent GetStorage(IEntity entity)
	{
		IBX_GMInventoryEditorComponent crate = IBX_GMInventoryEditorComponent.Get(entity);
		if (!crate)
			return null;

		return crate.GetStorage();
	}

	//! 0-100 for a storage sitting on a crate, -1 for anything else. Always measures the crate's own
	//! storage, even when the caller handed over another storage component of the same crate. Can
	//! exceed 100 the same way the vanilla volume percentage does, so callers keep their own
	//! over-capacity handling.
	static float GetPercentage(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return -1;

		storage = GetStorage(storage.GetOwner());
		if (!storage)
			return -1;

		float highest;

		float volumeCapacity = storage.GetMaxVolumeCapacity();
		if (volumeCapacity > 0)
			highest = storage.GetOccupiedSpace() * 100 / volumeCapacity;

		float maxWeight = GetMaxWeight(storage);
		if (maxWeight > 0)
		{
			float weightPercentage = GetContentWeight(storage) * 100 / maxWeight;
			if (weightPercentage > highest)
				highest = weightPercentage;
		}

		// Deliberately no free-slot term: UniversalInventoryStorage scales its slots dynamically, so
		// GetSlotsCount() cannot be trusted to mean the configured 100 rather than the slots in use.
		return highest;
	}

	//! Single line for the Game Master editor, e.g. "63% FULL - 419/1000 KG - 143000/228000 VOLUME".
	static string GetSummary(BaseInventoryStorageComponent storage)
	{
		float percentage = GetPercentage(storage);
		if (percentage < 0)
			return string.Empty;

		storage = GetStorage(storage.GetOwner());
		int roundedPercentage = Math.Round(percentage);
		int weight = Math.Round(GetContentWeight(storage));
		int maxWeight = Math.Round(GetMaxWeight(storage));
		int volume = Math.Round(storage.GetOccupiedSpace());
		int maxVolume = Math.Round(storage.GetMaxVolumeCapacity());

		// The percent sign is concatenated, not part of the format string: string.Format eats a "%"
		// that follows a placeholder, so "%1%" printed the bare number.
		string percentageText = roundedPercentage.ToString() + "%";

		return string.Format(
			"%1 FULL - %2/%3 KG - %4/%5 VOLUME",
			percentageText,
			weight,
			maxWeight,
			volume,
			maxVolume);
	}

	//! Weight of the stored items, without the crate's own weight - the limit the storage enforces
	//! in IsAdditionalWeightOk is on the contents.
	protected static float GetContentWeight(notnull BaseInventoryStorageComponent storage)
	{
		float ownWeight;
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(storage.GetAttributes());
		if (attributes)
			ownWeight = attributes.GetWeight();

		return storage.GetTotalWeight() - ownWeight;
	}

	protected static float GetMaxWeight(notnull BaseInventoryStorageComponent storage)
	{
		SCR_UniversalInventoryStorageComponent universal = SCR_UniversalInventoryStorageComponent.Cast(storage);
		if (!universal)
			return 0;

		return universal.GetMaxLoad();
	}
}

//------------------------------------------------------------------------------------------------

//! The volume bar drawn on a crate's own inventory slot, in the vicinity or inside a vehicle.
//!
//! Vanilla's own value here is right, it is just volume-only, so this feeds the bar the combined
//! number the storage header shows instead. Hooked on Refresh and SetSlotVisible rather than on
//! UpdateVolumeBarValue, with the widget looked up here rather than taken from m_ProgressBar:
//! vanilla caches that widget only when the slot's own `BaseInventoryStorageComponent.Cast(m_pItem)`
//! resolves, and an unset bar keeps its layout default, which `ProgressBarWidget` draws full - red,
//! under the flipped palette.
modded class SCR_InventorySlotUI
{
	override void Refresh()
	{
		super.Refresh();
		IBX_UpdateCrateFillBar();
	}

	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);

		if (bVisible)
			IBX_UpdateCrateFillBar();
	}

	protected void IBX_UpdateCrateFillBar()
	{
		if (!m_widget || !m_pItem)
			return;

		BaseInventoryStorageComponent crateStorage = IBX_CrateFill.GetStorage(m_pItem.GetOwner());
		if (!crateStorage)
			return;

		Widget barWidget = m_widget.FindAnyWidget("ProgressBar");
		if (!barWidget)
			return;

		SCR_InventoryProgressBar bar = SCR_InventoryProgressBar.Cast(barWidget.FindHandler(SCR_InventoryProgressBar));
		if (!bar)
			return;

		float percentage = IBX_CrateFill.GetPercentage(crateStorage);
		barWidget.SetVisible(true);

		// Same 0-100 range the opened crate's header bar uses, so both show the same fill.
		bar.SetProgressRange(0, 100);
		bar.SetCurrentProgress(percentage);
		bar.SetCurrentProgressPreview(percentage);

		// Red over-capacity overlay, the same rule the storage header applies.
		Widget over = m_widget.FindAnyWidget("ProgressVolumeOver");
		if (over)
			over.SetVisible(percentage > 100);
	}
}
