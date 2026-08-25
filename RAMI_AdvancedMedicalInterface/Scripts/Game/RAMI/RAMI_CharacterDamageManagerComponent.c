modded class SCR_CharacterDamageManagerComponent : SCR_DamageManagerComponent
{
	override void SetTourniquettedGroup(ECharacterHitZoneGroup hitZoneGroup, bool setTourniquetted)
	{
		bool wasSet = GetGroupTourniquetted(hitZoneGroup);
		super.SetTourniquettedGroup(hitZoneGroup, setTourniquetted);
		if (!Replication.IsServer() || wasSet == GetGroupTourniquetted(hitZoneGroup))
			return;

		string action = "REMOVED";
		if (setTourniquetted)
			action = "APPLIED";
		RAMI_AddActivity(string.Format("TREATMENT // TOURNIQUET %1 // %2", action, typename.EnumToString(ECharacterHitZoneGroup, hitZoneGroup)));
	}

	protected void RAMI_AddActivity(string message)
	{
		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(GetOwner().FindComponent(ACE_Medical_MedicationComponent));
		if (medication)
			medication.RAMI_AddActivity(message);
	}
}
