modded class SCR_ConsumableEffectHealthItems
{
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		string treatmentName = "MEDICAL ITEM";
		int commonType;
		InventoryItemComponent itemComponent;
		if (item)
			itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (itemComponent)
		{
			if (itemComponent.GetUIInfo())
				treatmentName = itemComponent.GetUIInfo().GetName();
			if (itemComponent.GetAttributes())
				commonType = itemComponent.GetAttributes().GetCommonType();
		}

		super.ApplyEffect(target, user, item, animParams);
		if (!Replication.IsServer()
			|| commonType == RAMI_ETreatmentType.TOURNIQUET
			|| SCR_PlayerController.RAMI_IsMedicationItem(item))
			return;

		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(target.FindComponent(ACE_Medical_MedicationComponent));
		if (!medication)
			return;

		string authorName = GetGame().GetPlayerManager().GetPlayerName(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user));
		if (authorName.IsEmpty())
			authorName = "N/A";
		medication.RAMI_AddActivity(string.Format("TREATMENT // %1 // %2", treatmentName, authorName));
	}
}
