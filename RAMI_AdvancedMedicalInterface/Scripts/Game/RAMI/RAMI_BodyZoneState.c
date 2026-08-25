class RAMI_BodyZoneState
{
	protected ref map<ECharacterHitZoneGroup, float> m_BleedingRates = new map<ECharacterHitZoneGroup, float>();
	protected ref map<ECharacterHitZoneGroup, float> m_InjurySeverities = new map<ECharacterHitZoneGroup, float>();
	protected ref map<ECharacterHitZoneGroup, bool> m_Fractures = new map<ECharacterHitZoneGroup, bool>();
	protected ref map<ECharacterHitZoneGroup, bool> m_Tourniquets = new map<ECharacterHitZoneGroup, bool>();
	protected ref map<ECharacterHitZoneGroup, int> m_TourniquetAppliedAt = new map<ECharacterHitZoneGroup, int>();

	void Capture(SCR_CharacterDamageManagerComponent damageManager, SCR_CharacterBloodHitZone blood)
	{
		array<ECharacterHitZoneGroup> groups = {
			ECharacterHitZoneGroup.HEAD,
			ECharacterHitZoneGroup.UPPERTORSO,
			ECharacterHitZoneGroup.LOWERTORSO,
			ECharacterHitZoneGroup.LEFTARM,
			ECharacterHitZoneGroup.RIGHTARM,
			ECharacterHitZoneGroup.LEFTLEG,
			ECharacterHitZoneGroup.RIGHTLEG
		};

		foreach (ECharacterHitZoneGroup group : groups)
		{
			m_BleedingRates.Set(group, GetBleedingRate(group, damageManager, blood));
			m_InjurySeverities.Set(group, GetInjurySeverity(group, damageManager));
			bool tourniquetApplied = damageManager.GetGroupTourniquetted(group);
			if (tourniquetApplied && !m_Tourniquets.Get(group))
				m_TourniquetAppliedAt.Set(group, System.GetTickCount());
			else if (!tourniquetApplied)
				m_TourniquetAppliedAt.Remove(group);

			m_Tourniquets.Set(group, tourniquetApplied);
		}

		m_Fractures.Set(ECharacterHitZoneGroup.LEFTARM, IsFractured(ECharacterHitZoneGroup.LEFTARM, damageManager));
		m_Fractures.Set(ECharacterHitZoneGroup.RIGHTARM, IsFractured(ECharacterHitZoneGroup.RIGHTARM, damageManager));
		m_Fractures.Set(ECharacterHitZoneGroup.LEFTLEG, IsFractured(ECharacterHitZoneGroup.LEFTLEG, damageManager));
		m_Fractures.Set(ECharacterHitZoneGroup.RIGHTLEG, IsFractured(ECharacterHitZoneGroup.RIGHTLEG, damageManager));
	}

	float GetBleedingRate(ECharacterHitZoneGroup group)
	{
		return m_BleedingRates.Get(group);
	}

	float GetInjurySeverity(ECharacterHitZoneGroup group)
	{
		return m_InjurySeverities.Get(group);
	}

	bool HasFracture(ECharacterHitZoneGroup group)
	{
		return m_Fractures.Get(group);
	}

	bool HasTourniquet(ECharacterHitZoneGroup group)
	{
		return m_Tourniquets.Get(group);
	}

	int GetTourniquetElapsedSeconds(ECharacterHitZoneGroup group)
	{
		if (!HasTourniquet(group))
			return 0;

		return System.GetTickCount(m_TourniquetAppliedAt.Get(group)) / 1000;
	}

	protected float GetBleedingRate(ECharacterHitZoneGroup group, SCR_CharacterDamageManagerComponent damageManager, SCR_CharacterBloodHitZone blood)
	{
		if (blood.GetMaxHealth() <= 0)
			return 0;

		float rate;
		array<HitZone> bleedingHitZones = damageManager.GetBleedingHitZones();
		if (!bleedingHitZones)
			return 0;

		foreach (HitZone hitZone : bleedingHitZones)
		{
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (characterHitZone && characterHitZone.GetHitZoneGroup() == group)
				rate += characterHitZone.ACE_Medical_CalculateBleedingRate();
		}

		if (damageManager.GetGroupTourniquetted(group))
			rate *= damageManager.GetTourniquetStrengthMultiplier();

		return rate / blood.GetMaxHealth() * RAMI_MedicalMenuUI.REFERENCE_BLOOD_VOLUME_ML;
	}

	protected float GetInjurySeverity(ECharacterHitZoneGroup group, SCR_CharacterDamageManagerComponent damageManager)
	{
		float severity;
		array<HitZone> hitZones = {};
		damageManager.GetAllHitZones(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!characterHitZone || characterHitZone.GetHitZoneGroup() != group || characterHitZone.GetMaxHealth() <= 0)
				continue;

			severity = Math.Max(severity, 1 - characterHitZone.GetHealthScaled());
		}

		return severity;
	}

	protected bool IsFractured(ECharacterHitZoneGroup group, SCR_CharacterDamageManagerComponent damageManager)
	{
		bool isArm = group == ECharacterHitZoneGroup.LEFTARM || group == ECharacterHitZoneGroup.RIGHTARM;
		bool isLeg = group == ECharacterHitZoneGroup.LEFTLEG || group == ECharacterHitZoneGroup.RIGHTLEG;
		if (!isArm && !isLeg)
			return false;

		if (isArm && damageManager.GetAimingDamage() <= 0)
			return false;

		if (isLeg && damageManager.GetMovementDamage() <= 0)
			return false;

		array<HitZone> hitZones = {};
		damageManager.GetAllHitZones(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (
				characterHitZone
				&& characterHitZone.GetHitZoneGroup() == group
				&& characterHitZone.GetHealth() <= characterHitZone.GetCriticalHealthThreshold()
			)
				return true;
		}

		return false;
	}
}
