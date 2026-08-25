modded class SCR_HealSupportStationAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		if (!Replication.IsServer() || !pOwnerEntity)
			return;

		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(pOwnerEntity.FindComponent(ACE_Medical_MedicationComponent));
		if (!medication)
			return;

		string authorName = GetGame().GetPlayerManager().GetPlayerName(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity));
		if (authorName.IsEmpty())
			authorName = "N/A";
		medication.RAMI_AddActivity(string.Format("TREATMENT // MEDICAL KIT // %1 // %2", typename.EnumToString(ECharacterHitZoneGroup, GetHitZoneGroup()), authorName));
	}
}
