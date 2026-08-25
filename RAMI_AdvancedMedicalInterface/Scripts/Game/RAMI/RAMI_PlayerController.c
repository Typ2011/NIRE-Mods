modded class SCR_PlayerController
{
	protected RplId m_RAMI_PainTargetId = RplId.Invalid();
	protected int m_RAMI_RemotePainPercent;
	protected int m_RAMI_LastPainRequest;
	protected RplId m_RAMI_MedicationSummaryTargetId = RplId.Invalid();
	protected string m_RAMI_MedicationSummaryLabels;
	protected string m_RAMI_MedicationSummaryValues;
	protected int m_RAMI_SalineVolumeRemainingMl;
	protected int m_RAMI_SalineTimeRemainingSeconds;
	protected int m_RAMI_LastMedicationSummaryRequest;
	protected RplId m_RAMI_InventoryTargetId = RplId.Invalid();
	protected ECharacterHitZoneGroup m_RAMI_InventoryRegion;
	protected int m_RAMI_BandageCount;
	protected int m_RAMI_TourniquetCount;
	protected bool m_RAMI_CanBandage;
	protected bool m_RAMI_CanTourniquet;
	protected ResourceName m_RAMI_BandagePrefab;
	protected ResourceName m_RAMI_TourniquetPrefab;
	protected int m_RAMI_LastInventoryRequest;
	protected ref array<int> m_RAMI_MedicationCounts = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	protected ref array<bool> m_RAMI_CanUseMedication = {false, false, false, false, false, false, false, false, false, false, false, false};
	protected RplId m_RAMI_MedicationInventoryTargetId = RplId.Invalid();
	protected ECharacterHitZoneGroup m_RAMI_MedicationInventoryRegion;
	protected int m_RAMI_LastMedicationInventoryRequest;
	protected RplId m_RAMI_PreparedTargetId = RplId.Invalid();
	protected RplId m_RAMI_PreparedItemId = RplId.Invalid();
	protected ECharacterHitZoneGroup m_RAMI_PreparedRegion;
	protected int m_RAMI_PreparedCommonType;
	protected bool m_RAMI_PreparedReady;
	protected bool m_RAMI_PreparedSucceeded;
	protected RplId m_RAMI_TriageTargetId = RplId.Invalid();
	protected RAMI_ETriageLevel m_RAMI_TriageLevel;
	protected ref array<int> m_RAMI_ActivityTimes = {};
	protected ref array<string> m_RAMI_ActivityMessages = {};
	protected int m_RAMI_LastTriageRequest;

	static ResourceName RAMI_GetMedicationPrefab(int treatmentType)
	{
		switch (treatmentType)
		{
			case RAMI_ETreatmentType.EPINEPHRINE: return "{5B2FD067D70C1E8F}Prefabs/Items/Medicine/EpinephrineInjection/ACE_Medical_EpinephrineInjection.et";
			case RAMI_ETreatmentType.MORPHINE: return "{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et";
			case RAMI_ETreatmentType.NALOXONE: return "{02DD34077F51F65E}Prefabs/Items/Medicine/NaloxoneInjection/ACE_Medical_NaloxoneInjection.et";
			case RAMI_ETreatmentType.PHENYLEPHRINE: return "{9BBA766CD869002C}Prefabs/Items/Medicine/PhenylephrineInjection/ACE_Medical_PhenylephrineInjection.et";
			case RAMI_ETreatmentType.METOPROLOL: return "{D0434D5215B54181}Prefabs/Items/Medicine/MetoprololInjection/ACE_Medical_MetoprololInjection.et";
			case RAMI_ETreatmentType.AMMONIUM_CARBONATE: return "{58CF3AB87C441295}Prefabs/Items/Medicine/AmmoniumCarbonatePackage/ACE_Medical_AmmoniumCarbonatePackage.et";
			case RAMI_ETreatmentType.SALINE_500: return "{A2CEF84144FB299B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_500.et";
			case RAMI_ETreatmentType.SALINE_1000: return "{02C5C61EB9FE39D2}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1000.et";
			case RAMI_ETreatmentType.SALINE_1500: return "{743FE3842DDFD37D}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1500.et";
			case RAMI_ETreatmentType.SALINE_750: return "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et";
			case RAMI_ETreatmentType.SALINE_250: return "{02C5C61EB9FE39D3}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_250.et";
			case RAMI_ETreatmentType.SALINE_1250: return "{D434DDDBD0DAC334}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1250.et";
		}

		return string.Empty;
	}

	static bool RAMI_ItemMatchesTreatment(IEntity item, int treatmentType)
	{
		if (!item)
			return false;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent || !itemComponent.GetAttributes())
			return false;

		if (treatmentType == RAMI_ETreatmentType.BANDAGE || treatmentType == RAMI_ETreatmentType.TOURNIQUET)
			return itemComponent.GetAttributes().GetCommonType() == treatmentType;

		ResourceName medicationPrefab = RAMI_GetMedicationPrefab(treatmentType);
		if (medicationPrefab.IsEmpty() || !item.GetPrefabData())
			return false;

		ResourceName itemPrefab = item.GetPrefabData().GetPrefabName();
		if (itemPrefab == medicationPrefab)
			return true;

		switch (treatmentType)
		{
			case RAMI_ETreatmentType.SALINE_500: return itemPrefab == "{883BA62332B2278B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_500.et";
			case RAMI_ETreatmentType.SALINE_1000: return itemPrefab == "{2830987CCFB737C2}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1000.et";
			case RAMI_ETreatmentType.SALINE_1500: return itemPrefab == "{5A9C7F500A437C3B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1500.et";
			case RAMI_ETreatmentType.SALINE_750: return itemPrefab == "{527D7C5D2E476BDC}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_01.et";
			case RAMI_ETreatmentType.SALINE_250: return itemPrefab == "{2830987CCFB737C3}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_250.et";
			case RAMI_ETreatmentType.SALINE_1250: return itemPrefab == "{FA97410FF7466C72}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1250.et";
		}

		return false;
	}

	static bool RAMI_IsMedicationItem(IEntity item)
	{
		for (int treatmentType = RAMI_ETreatmentType.EPINEPHRINE; treatmentType <= RAMI_ETreatmentType.SALINE_1250; treatmentType++)
		{
			if (RAMI_ItemMatchesTreatment(item, treatmentType))
				return true;
		}
		return false;
	}

	static bool RAMI_CanApplyMedicationToRegion(IEntity target, int treatmentType, ECharacterHitZoneGroup region)
	{
		if (!target)
			return false;
		SCR_ChimeraCharacter patient = SCR_ChimeraCharacter.Cast(target);
		if (!patient)
			return false;

		if (treatmentType == RAMI_ETreatmentType.AMMONIUM_CARBONATE)
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(patient.GetCharacterController());
			return region == ECharacterHitZoneGroup.HEAD && controller && controller.IsUnconscious();
		}

		if (treatmentType >= RAMI_ETreatmentType.SALINE_500 && treatmentType <= RAMI_ETreatmentType.SALINE_1250)
			return region == ECharacterHitZoneGroup.LEFTARM || region == ECharacterHitZoneGroup.RIGHTARM;

		if (RAMI_GetMedicationPrefab(treatmentType).IsEmpty())
			return true;

		return region == ECharacterHitZoneGroup.LEFTARM
			|| region == ECharacterHitZoneGroup.RIGHTARM
			|| region == ECharacterHitZoneGroup.LEFTLEG
			|| region == ECharacterHitZoneGroup.RIGHTLEG;
	}

	bool RAMI_IsTreatmentTargetInRange(IEntity target)
	{
		IEntity user = GetControlledEntity();
		if (!user || !target)
			return false;
		if (vector.Distance(user.GetOrigin(), target.GetOrigin()) <= 5)
			return true;

		IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(user);
		return vehicle && vehicle == CompartmentAccessComponent.GetVehicleIn(target);
	}

	void RAMI_SetTreatmentTarget(SCR_ConsumableItemComponent consumable, SCR_ChimeraCharacter target, ECharacterHitZoneGroup region)
	{
		if (!consumable || !target)
			return;

		consumable.SetTargetCharacter(target);
		if (Replication.IsServer())
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(target.GetDamageManager());
		if (!damageManager)
			return;

		Rpc(RAMI_RpcAsk_SetTreatmentTarget, Replication.FindItemId(consumable), Replication.FindItemId(damageManager), region);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_SetTreatmentTarget(RplId consumableId, RplId damageManagerId, ECharacterHitZoneGroup region)
	{
		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(Replication.FindItem(consumableId));
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(damageManagerId));
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(GetControlledEntity());
		SCR_ChimeraCharacter target;
		if (damageManager)
			target = SCR_ChimeraCharacter.Cast(damageManager.GetOwner());

		if (!consumable || !RAMI_IsTreatmentTargetInRange(target))
			return;

		IEntity item = consumable.GetOwner();
		if (!RAMI_InventoryContains(user, item) && !RAMI_InventoryContains(target, item))
			return;

		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(consumable.GetConsumableEffect());
		int failReason;
		if (effect && effect.CanApplyEffectToHZ(target, user, region, failReason))
			consumable.SetTargetCharacter(target);
	}

	protected bool RAMI_InventoryContains(SCR_ChimeraCharacter character, IEntity item)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return false;

		array<IEntity> items = {};
		storageManager.GetItems(items);
		return items.Contains(item);
	}

	void RAMI_RequestPain(SCR_CharacterDamageManagerComponent damageManager)
	{
		if (!damageManager)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		if (targetId == m_RAMI_PainTargetId && System.GetTickCount(m_RAMI_LastPainRequest) < 1000)
			return;

		m_RAMI_LastPainRequest = System.GetTickCount();
		int painPercent = Math.Round(damageManager.ACE_Medical_GetPainIntensity() * 100);
		if (Replication.IsServer())
		{
			RAMI_SetPain(targetId, painPercent);
			return;
		}

		Rpc(RAMI_RpcAsk_GetPain, targetId);
	}

	bool RAMI_GetPain(SCR_CharacterDamageManagerComponent damageManager, out int painPercent)
	{
		if (!damageManager || !m_RAMI_PainTargetId.IsValid() || Replication.FindItemId(damageManager) != m_RAMI_PainTargetId)
			return false;

		painPercent = m_RAMI_RemotePainPercent;
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_GetPain(RplId targetId)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		if (!damageManager || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()))
			return;

		int painPercent = Math.Round(damageManager.ACE_Medical_GetPainIntensity() * 100);
		Rpc(RAMI_RpcDo_SetPain, targetId, painPercent);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_SetPain(RplId targetId, int painPercent)
	{
		RAMI_SetPain(targetId, painPercent);
	}

	protected void RAMI_SetPain(RplId targetId, int painPercent)
	{
		m_RAMI_PainTargetId = targetId;
		m_RAMI_RemotePainPercent = painPercent;
	}

	bool RAMI_RequestStopSaline(SCR_CharacterDamageManagerComponent damageManager)
	{
		if (!damageManager)
			return false;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid() || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()) || !ESB_PartialSaline.CanStop(damageManager.GetOwner()))
			return false;

		if (Replication.IsServer())
		{
			RAMI_StopSaline(targetId);
			return true;
		}

		Rpc(RAMI_RpcAsk_StopSaline, targetId);
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_StopSaline(RplId targetId)
	{
		RAMI_StopSaline(targetId);
	}

	protected void RAMI_StopSaline(RplId targetId)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		if (!damageManager || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()))
			return;

		ESB_PartialSaline.Stop(damageManager.GetOwner(), GetControlledEntity());
	}

	void RAMI_RequestMedicationSummary(SCR_CharacterDamageManagerComponent damageManager)
	{
		if (!damageManager)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		if (targetId == m_RAMI_MedicationSummaryTargetId && System.GetTickCount(m_RAMI_LastMedicationSummaryRequest) < 500)
			return;

		m_RAMI_LastMedicationSummaryRequest = System.GetTickCount();
		if (Replication.IsServer())
		{
			RAMI_UpdateMedicationSummary(targetId);
			return;
		}

		Rpc(RAMI_RpcAsk_GetMedicationSummary, targetId);
	}

	bool RAMI_GetMedicationSummary(SCR_CharacterDamageManagerComponent damageManager, out string labels, out string values, out int salineVolumeRemainingMl, out int salineTimeRemainingSeconds)
	{
		if (!damageManager || Replication.FindItemId(damageManager) != m_RAMI_MedicationSummaryTargetId)
			return false;

		labels = m_RAMI_MedicationSummaryLabels;
		values = m_RAMI_MedicationSummaryValues;
		salineVolumeRemainingMl = m_RAMI_SalineVolumeRemainingMl;
		salineTimeRemainingSeconds = m_RAMI_SalineTimeRemainingSeconds;
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_GetMedicationSummary(RplId targetId)
	{
		RAMI_UpdateMedicationSummary(targetId);
	}

	protected void RAMI_UpdateMedicationSummary(RplId targetId)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		if (!damageManager || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()))
			return;

		string labels;
		string values;
		int salineVolumeRemainingMl;
		int salineTimeRemainingSeconds;
		RAMI_GetSalineBagStatus(damageManager, salineVolumeRemainingMl, salineTimeRemainingSeconds);
		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(damageManager.GetOwner().FindComponent(ACE_Medical_MedicationComponent));
		ACE_Medical_Medication_Settings settings = ACE_SettingsHelperT<ACE_Medical_Medication_Settings>.GetModSettings();
		if (medication && settings)
		{
			array<ACE_Medical_EDrugType> drugs;
			array<ref array<ref ACE_Medical_Dose>> allDoses;
			medication.GetMedications(drugs, allDoses);
			map<ACE_Medical_EDrugType, float> concentrations = new map<ACE_Medical_EDrugType, float>();
			for (int drugIndex = 0; drugIndex < drugs.Count(); drugIndex++)
			{
				ACE_Medical_PharmacokineticsConfig drugConfig = settings.GetPharmacokineticsConfig(drugs[drugIndex]);
				if (!drugConfig || drugIndex >= allDoses.Count())
					continue;

				float totalConcentration;
				foreach (ACE_Medical_Dose drugDose : allDoses[drugIndex])
				{
					ACE_Medical_Bolus drugBolus = ACE_Medical_Bolus.Cast(drugDose);
					if (drugBolus)
						totalConcentration += drugBolus.GetAdministeredConcentration() * RAMI_GetMedicationConcentrationScale(drugConfig, drugDose.GetElapsedTime());
				}
				concentrations[drugs[drugIndex]] = totalConcentration;
			}

			for (int index = 0; index < drugs.Count(); index++)
			{
				ACE_Medical_PharmacokineticsConfig config = settings.GetPharmacokineticsConfig(drugs[index]);
				if (!config || config.m_fActivationRateConstant <= 0 || config.m_fDeactivationRateConstant <= 0 || index >= allDoses.Count() || allDoses[index].IsEmpty())
					continue;

				float peakTime = RAMI_GetMedicationPeakTime(config);
				float halfTime = RAMI_GetMedicationHalfTime(config, peakTime);
				ACE_Medical_PharmacodynamicsConfig effectConfig = RAMI_GetMedicationEffectConfig(settings, drugs[index]);
				float antagonistScale = RAMI_GetMedicationAntagonistScale(effectConfig, concentrations);
				string antagonistNames = RAMI_GetMedicationAntagonistNames(effectConfig, concentrations);
				for (int doseIndex = 0; doseIndex < allDoses[index].Count(); doseIndex++)
				{
					ACE_Medical_Dose dose = allDoses[index][doseIndex];
					ACE_Medical_Bolus bolus = ACE_Medical_Bolus.Cast(dose);
					if (!bolus)
						continue;

					float elapsedTime = dose.GetElapsedTime();
					float currentConcentration = bolus.GetAdministeredConcentration() * RAMI_GetMedicationConcentrationScale(config, elapsedTime);
					float peakConcentration = bolus.GetAdministeredConcentration() * RAMI_GetMedicationConcentrationScale(config, peakTime);
					if (peakConcentration <= 0)
						continue;

					if (!labels.IsEmpty())
					{
						labels += "\n";
						values += "\n";
					}

					string medicationName = RAMI_GetMedicationName(drugs[index]);
					if (allDoses[index].Count() > 1)
						medicationName += string.Format(" #%1", doseIndex + 1);
					labels += "\n" + medicationName + "\n";

					string peakValue = "PEAK ACTIVE";
					if (elapsedTime < peakTime)
						peakValue = "PEAK IN " + RAMI_FormatMedicationTime(peakTime - elapsedTime);
					string halfValue = "HALF PASSED";
					if (elapsedTime < halfTime)
						halfValue = "HALF IN " + RAMI_FormatMedicationTime(halfTime - elapsedTime);
					string strengthValue = string.Format("STRENGTH %1 %%", Math.Round(100 * currentConcentration / peakConcentration / antagonistScale));
					if (!antagonistNames.IsEmpty())
						strengthValue += " / " + antagonistNames;
					values += string.Format("%1\n%2\n%3", peakValue, halfValue, strengthValue);
				}
			}
		}

		if (labels.IsEmpty())
		{
			labels = "NONE";
			values = "";
		}

		RAMI_SetMedicationSummary(targetId, labels, values, salineVolumeRemainingMl, salineTimeRemainingSeconds);
		Rpc(RAMI_RpcDo_SetMedicationSummary, targetId, labels, values, salineVolumeRemainingMl, salineTimeRemainingSeconds);
	}

	protected void RAMI_GetSalineBagStatus(SCR_CharacterDamageManagerComponent damageManager, out int volumeRemainingMl, out int timeRemainingSeconds)
	{
		array<ref SCR_PersistentDamageEffect> effects = {};
		damageManager.FindAllDamageEffectsOfType(SCR_SalineDamageEffect, effects);
		foreach (SCR_PersistentDamageEffect persistentEffect : effects)
		{
			SCR_SalineDamageEffect salineEffect = SCR_SalineDamageEffect.Cast(persistentEffect);
			if (!salineEffect || !salineEffect.IsActive())
				continue;

			float remainingTime = salineEffect.GetMaxDuration() - salineEffect.GetCurrentDuration();
			if (remainingTime <= 0)
				continue;

			volumeRemainingMl += Math.Round(Math.AbsFloat(salineEffect.GetDPS()) * remainingTime);
			timeRemainingSeconds = Math.Max(timeRemainingSeconds, Math.Ceil(remainingTime));
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_SetMedicationSummary(RplId targetId, string labels, string values, int salineVolumeRemainingMl, int salineTimeRemainingSeconds)
	{
		RAMI_SetMedicationSummary(targetId, labels, values, salineVolumeRemainingMl, salineTimeRemainingSeconds);
	}

	protected void RAMI_SetMedicationSummary(RplId targetId, string labels, string values, int salineVolumeRemainingMl, int salineTimeRemainingSeconds)
	{
		m_RAMI_MedicationSummaryTargetId = targetId;
		m_RAMI_MedicationSummaryLabels = labels;
		m_RAMI_MedicationSummaryValues = values;
		m_RAMI_SalineVolumeRemainingMl = salineVolumeRemainingMl;
		m_RAMI_SalineTimeRemainingSeconds = salineTimeRemainingSeconds;
	}

	protected float RAMI_GetMedicationPeakTime(ACE_Medical_PharmacokineticsConfig config)
	{
		float rateDifference = config.m_fActivationRateConstant - config.m_fDeactivationRateConstant;
		if (Math.AbsFloat(rateDifference) < 0.000001)
			return 1 / config.m_fActivationRateConstant;

		return Math.Log10(config.m_fActivationRateConstant / config.m_fDeactivationRateConstant) / Math.Log10(Math.E) / rateDifference;
	}

	protected ACE_Medical_PharmacodynamicsConfig RAMI_GetMedicationEffectConfig(ACE_Medical_Medication_Settings settings, ACE_Medical_EDrugType drug)
	{
		foreach (ACE_Medical_DrugEffectConfig effect : settings.m_aPharmacodynamicsConfigs)
		{
			if (!effect.m_aDrugConfigs)
				continue;

			foreach (ACE_Medical_PharmacodynamicsConfig config : effect.m_aDrugConfigs)
			{
				if (config.m_eType == drug && config.m_aAntagonistConfigs)
					return config;
			}
		}

		return null;
	}

	protected float RAMI_GetMedicationAntagonistScale(ACE_Medical_PharmacodynamicsConfig effectConfig, map<ACE_Medical_EDrugType, float> concentrations)
	{
		float scale = 1;
		if (!effectConfig)
			return scale;

		foreach (ACE_Medical_PharmacodynamicsConfig antagonistConfig : effectConfig.m_aAntagonistConfigs)
			scale += antagonistConfig.ComputeChi(concentrations);

		return scale;
	}

	protected string RAMI_GetMedicationAntagonistNames(ACE_Medical_PharmacodynamicsConfig effectConfig, map<ACE_Medical_EDrugType, float> concentrations)
	{
		string names;
		if (!effectConfig)
			return names;

		foreach (ACE_Medical_PharmacodynamicsConfig antagonistConfig : effectConfig.m_aAntagonistConfigs)
		{
			float concentration;
			if (!concentrations.Find(antagonistConfig.m_eType, concentration) || concentration <= 0)
				continue;

			if (!names.IsEmpty())
				names += "+";
			names += RAMI_GetMedicationName(antagonistConfig.m_eType);
		}

		return names;
	}

	protected float RAMI_GetMedicationHalfTime(ACE_Medical_PharmacokineticsConfig config, float peakTime)
	{
		float peakScale = RAMI_GetMedicationConcentrationScale(config, peakTime);
		float low = peakTime;
		float high = peakTime * 2;
		while (RAMI_GetMedicationConcentrationScale(config, high) > peakScale * 0.5)
			high *= 2;

		for (int index = 0; index < 20; index++)
		{
			float middle = (low + high) * 0.5;
			if (RAMI_GetMedicationConcentrationScale(config, middle) > peakScale * 0.5)
				low = middle;
			else
				high = middle;
		}

		return high;
	}

	protected float RAMI_GetMedicationConcentrationScale(ACE_Medical_PharmacokineticsConfig config, float time)
	{
		float rateDifference = config.m_fActivationRateConstant - config.m_fDeactivationRateConstant;
		if (Math.AbsFloat(rateDifference) < 0.000001)
			return config.m_fActivationRateConstant * time * ACE_Math.Exp(-config.m_fActivationRateConstant * time);

		return config.m_fActivationRateConstant / rateDifference
			* (ACE_Math.Exp(-config.m_fDeactivationRateConstant * time) - ACE_Math.Exp(-config.m_fActivationRateConstant * time));
	}

	protected string RAMI_GetMedicationName(ACE_Medical_EDrugType drug)
	{
		switch (drug)
		{
			case ACE_Medical_EDrugType.EPINEPHRINE: return "EPINEPHRINE";
			case ACE_Medical_EDrugType.METOPROLOL: return "METOPROLOL";
			case ACE_Medical_EDrugType.MORPHINE: return "MORPHINE";
			case ACE_Medical_EDrugType.NALOXONE: return "NALOXONE";
			case ACE_Medical_EDrugType.PHENYLEPHRINE: return "PHENYLEPHRINE";
		}

		return "MEDICATION";
	}

	protected string RAMI_FormatMedicationTime(float time)
	{
		int secondsTotal = Math.Round(time);
		string seconds = (secondsTotal % 60).ToString();
		if (seconds.Length() < 2)
			seconds = "0" + seconds;
		return string.Format("%1:%2", secondsTotal / 60, seconds);
	}

	void RAMI_RequestMedicationInventory(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region)
	{
		if (!damageManager)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		if (targetId == m_RAMI_MedicationInventoryTargetId && region == m_RAMI_MedicationInventoryRegion && System.GetTickCount(m_RAMI_LastMedicationInventoryRequest) < 500)
			return;

		m_RAMI_LastMedicationInventoryRequest = System.GetTickCount();
		if (Replication.IsServer())
		{
			RAMI_UpdateMedicationInventory(targetId, region);
			return;
		}

		Rpc(RAMI_RpcAsk_GetMedicationInventory, targetId, region);
	}

	bool RAMI_GetMedicationInventory(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region, int treatmentType, out int count, out bool canApply)
	{
		int index = treatmentType - RAMI_ETreatmentType.EPINEPHRINE;
		if (!damageManager || Replication.FindItemId(damageManager) != m_RAMI_MedicationInventoryTargetId || region != m_RAMI_MedicationInventoryRegion || index < 0 || index >= m_RAMI_MedicationCounts.Count())
			return false;

		count = m_RAMI_MedicationCounts[index];
		canApply = m_RAMI_CanUseMedication[index];
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_GetMedicationInventory(RplId targetId, ECharacterHitZoneGroup region)
	{
		RAMI_UpdateMedicationInventory(targetId, region);
	}

	protected void RAMI_UpdateMedicationInventory(RplId targetId, ECharacterHitZoneGroup region)
	{
		array<int> counts = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		array<bool> canApply = {false, false, false, false, false, false, false, false, false, false, false, false};
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(GetControlledEntity());
		SCR_ChimeraCharacter target;
		if (damageManager)
			target = SCR_ChimeraCharacter.Cast(damageManager.GetOwner());

		if (RAMI_IsTreatmentTargetInRange(target))
		{
			array<IEntity> items = {};
			SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(target.FindComponent(SCR_InventoryStorageManagerComponent));
			if (storageManager)
				storageManager.GetItems(items);
			foreach (IEntity item : items)
			{
				for (int index = 0; index < counts.Count(); index++)
				{
					int treatmentType = RAMI_ETreatmentType.EPINEPHRINE + index;
					if (!RAMI_ItemMatchesTreatment(item, treatmentType))
						continue;

					counts.Set(index, counts[index] + 1);
					SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
					SCR_ConsumableEffectHealthItems effect;
					if (consumable)
						effect = SCR_ConsumableEffectHealthItems.Cast(consumable.GetConsumableEffect());
					int failReason;
					if (RAMI_CanApplyMedicationToRegion(target, treatmentType, region) && effect && effect.CanApplyEffectToHZ(target, user, region, failReason))
						canApply[index] = true;
					break;
				}
			}
		}

		ResourceName inventory;
		for (int index = 0; index < counts.Count(); index++)
		{
			if (!inventory.IsEmpty())
				inventory += "|";
			inventory += counts[index].ToString();
			if (canApply[index])
				inventory += "|1";
			else
				inventory += "|0";
		}

		if (Replication.IsServer())
			RAMI_SetMedicationInventory(targetId, region, inventory);
		Rpc(RAMI_RpcDo_SetMedicationInventory, targetId, region, inventory);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_SetMedicationInventory(RplId targetId, ECharacterHitZoneGroup region, ResourceName inventory)
	{
		RAMI_SetMedicationInventory(targetId, region, inventory);
	}

	protected void RAMI_SetMedicationInventory(RplId targetId, ECharacterHitZoneGroup region, ResourceName inventory)
	{
		array<string> values = {};
		inventory.Split("|", values, false);
		if (values.Count() != 24)
			return;

		m_RAMI_MedicationInventoryTargetId = targetId;
		m_RAMI_MedicationInventoryRegion = region;
		for (int index = 0; index < m_RAMI_MedicationCounts.Count(); index++)
		{
			m_RAMI_MedicationCounts.Set(index, values[index * 2].ToInt());
			m_RAMI_CanUseMedication.Set(index, values[index * 2 + 1] == "1");
		}
	}

	void RAMI_RequestTreatmentInventory(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region)
	{
		if (!damageManager)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		if (targetId == m_RAMI_InventoryTargetId && region == m_RAMI_InventoryRegion && System.GetTickCount(m_RAMI_LastInventoryRequest) < 500)
			return;

		m_RAMI_LastInventoryRequest = System.GetTickCount();
		if (Replication.IsServer())
		{
			RAMI_UpdateTreatmentInventory(targetId, region);
			return;
		}

		Rpc(RAMI_RpcAsk_GetTreatmentInventory, targetId, region);
	}

	bool RAMI_GetTreatmentInventory(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region, out int bandageCount, out int tourniquetCount, out bool canBandage, out bool canTourniquet, out ResourceName bandagePrefab, out ResourceName tourniquetPrefab)
	{
		if (!damageManager || Replication.FindItemId(damageManager) != m_RAMI_InventoryTargetId || region != m_RAMI_InventoryRegion)
			return false;

		bandageCount = m_RAMI_BandageCount;
		tourniquetCount = m_RAMI_TourniquetCount;
		canBandage = m_RAMI_CanBandage;
		canTourniquet = m_RAMI_CanTourniquet;
		bandagePrefab = m_RAMI_BandagePrefab;
		tourniquetPrefab = m_RAMI_TourniquetPrefab;
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_GetTreatmentInventory(RplId targetId, ECharacterHitZoneGroup region)
	{
		RAMI_UpdateTreatmentInventory(targetId, region);
	}

	protected void RAMI_UpdateTreatmentInventory(RplId targetId, ECharacterHitZoneGroup region)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(GetControlledEntity());
		SCR_ChimeraCharacter target;
		if (damageManager)
			target = SCR_ChimeraCharacter.Cast(damageManager.GetOwner());
		if (!RAMI_IsTreatmentTargetInRange(target))
		{
			if (Replication.IsServer())
				RAMI_SetTreatmentInventory(targetId, region, 0, 0, false, false, string.Empty, string.Empty);
			Rpc(RAMI_RpcDo_SetTreatmentInventory, targetId, region, 0, 0, false, false, string.Empty, string.Empty);
			return;
		}

		int bandageCount;
		int tourniquetCount;
		bool canBandage;
		bool canTourniquet;
		ResourceName bandagePrefab;
		ResourceName tourniquetPrefab;
		array<IEntity> items = {};
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(target.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManager)
			storageManager.GetItems(items);
		foreach (IEntity item : items)
		{
			InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!itemComponent || !itemComponent.GetAttributes())
				continue;

			int commonType = itemComponent.GetAttributes().GetCommonType();
			if (commonType != 1 && commonType != 4)
				continue;
			if (commonType == 1)
			{
				bandageCount++;
				if (bandagePrefab.IsEmpty() && item.GetPrefabData())
					bandagePrefab = item.GetPrefabData().GetPrefabName();
			}
			else
			{
				tourniquetCount++;
				if (tourniquetPrefab.IsEmpty() && item.GetPrefabData())
					tourniquetPrefab = item.GetPrefabData().GetPrefabName();
			}

			SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
			SCR_ConsumableEffectHealthItems effect;
			if (consumable)
				effect = SCR_ConsumableEffectHealthItems.Cast(consumable.GetConsumableEffect());
			int failReason;
			if (!effect || !effect.CanApplyEffectToHZ(target, user, region, failReason))
				continue;
			if (commonType == 1)
				canBandage = true;
			else
				canTourniquet = true;
		}

		if (Replication.IsServer())
			RAMI_SetTreatmentInventory(targetId, region, bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab);
		Rpc(RAMI_RpcDo_SetTreatmentInventory, targetId, region, bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_SetTreatmentInventory(RplId targetId, ECharacterHitZoneGroup region, int bandageCount, int tourniquetCount, bool canBandage, bool canTourniquet, ResourceName bandagePrefab, ResourceName tourniquetPrefab)
	{
		RAMI_SetTreatmentInventory(targetId, region, bandageCount, tourniquetCount, canBandage, canTourniquet, bandagePrefab, tourniquetPrefab);
	}

	protected void RAMI_SetTreatmentInventory(RplId targetId, ECharacterHitZoneGroup region, int bandageCount, int tourniquetCount, bool canBandage, bool canTourniquet, ResourceName bandagePrefab, ResourceName tourniquetPrefab)
	{
		m_RAMI_InventoryTargetId = targetId;
		m_RAMI_InventoryRegion = region;
		m_RAMI_BandageCount = bandageCount;
		m_RAMI_TourniquetCount = tourniquetCount;
		m_RAMI_CanBandage = canBandage;
		m_RAMI_CanTourniquet = canTourniquet;
		m_RAMI_BandagePrefab = bandagePrefab;
		m_RAMI_TourniquetPrefab = tourniquetPrefab;
	}

	void RAMI_RequestForeignTreatment(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region, int commonType)
	{
		if (!damageManager || (commonType != RAMI_ETreatmentType.BANDAGE && commonType != RAMI_ETreatmentType.TOURNIQUET && RAMI_GetMedicationPrefab(commonType).IsEmpty()))
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		m_RAMI_PreparedTargetId = targetId;
		m_RAMI_PreparedRegion = region;
		m_RAMI_PreparedCommonType = commonType;
		m_RAMI_PreparedItemId = RplId.Invalid();
		m_RAMI_PreparedReady = false;
		m_RAMI_PreparedSucceeded = false;
		if (Replication.IsServer())
		{
			RAMI_PrepareForeignTreatment(targetId, region, commonType);
			return;
		}

		Rpc(RAMI_RpcAsk_PrepareForeignTreatment, targetId, region, commonType);
	}

	bool RAMI_GetPreparedForeignTreatment(SCR_CharacterDamageManagerComponent damageManager, ECharacterHitZoneGroup region, int commonType, out RplId itemId, out bool succeeded)
	{
		if (!damageManager || Replication.FindItemId(damageManager) != m_RAMI_PreparedTargetId || region != m_RAMI_PreparedRegion || commonType != m_RAMI_PreparedCommonType || !m_RAMI_PreparedReady)
			return false;

		itemId = m_RAMI_PreparedItemId;
		succeeded = m_RAMI_PreparedSucceeded;
		return true;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_PrepareForeignTreatment(RplId targetId, ECharacterHitZoneGroup region, int commonType)
	{
		RAMI_PrepareForeignTreatment(targetId, region, commonType);
	}

	protected void RAMI_PrepareForeignTreatment(RplId targetId, ECharacterHitZoneGroup region, int commonType)
	{
		RplId itemId = RplId.Invalid();
		if (commonType != RAMI_ETreatmentType.BANDAGE && commonType != RAMI_ETreatmentType.TOURNIQUET && RAMI_GetMedicationPrefab(commonType).IsEmpty())
		{
			Rpc(RAMI_RpcDo_PrepareForeignTreatment, targetId, region, commonType, itemId, false);
			return;
		}

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(GetControlledEntity());
		SCR_ChimeraCharacter target;
		if (damageManager)
			target = SCR_ChimeraCharacter.Cast(damageManager.GetOwner());
		if (RAMI_IsTreatmentTargetInRange(target))
		{
			IEntity item = RAMI_FindTreatmentItem(target, user, region, commonType);
			SCR_InventoryStorageManagerComponent userStorageManager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
			BaseInventoryStorageComponent destination;
			if (item && userStorageManager)
				destination = userStorageManager.FindStorageForItem(item, EStoragePurpose.PURPOSE_ANY);
			if (destination && userStorageManager.TryMoveItemToStorage(item, destination))
			{
				SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
				if (consumable)
					itemId = Replication.FindItemId(consumable);
			}
		}

		bool succeeded = itemId.IsValid();
		if (Replication.IsServer())
			RAMI_SetPreparedForeignTreatment(targetId, region, commonType, itemId, succeeded);
		Rpc(RAMI_RpcDo_PrepareForeignTreatment, targetId, region, commonType, itemId, succeeded);
	}

	protected IEntity RAMI_FindTreatmentItem(SCR_ChimeraCharacter target, SCR_ChimeraCharacter user, ECharacterHitZoneGroup region, int commonType)
	{
		if (!RAMI_CanApplyMedicationToRegion(target, commonType, region))
			return null;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(target.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return null;

		array<IEntity> items = {};
		storageManager.GetItems(items);
		foreach (IEntity item : items)
		{
			if (!RAMI_ItemMatchesTreatment(item, commonType))
				continue;
			SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
			SCR_ConsumableEffectHealthItems effect;
			if (consumable)
				effect = SCR_ConsumableEffectHealthItems.Cast(consumable.GetConsumableEffect());
			int failReason;
			if (effect && effect.CanApplyEffectToHZ(target, user, region, failReason))
				return item;
		}
		return null;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_PrepareForeignTreatment(RplId targetId, ECharacterHitZoneGroup region, int commonType, RplId itemId, bool succeeded)
	{
		RAMI_SetPreparedForeignTreatment(targetId, region, commonType, itemId, succeeded);
	}

	protected void RAMI_SetPreparedForeignTreatment(RplId targetId, ECharacterHitZoneGroup region, int commonType, RplId itemId, bool succeeded)
	{
		m_RAMI_PreparedTargetId = targetId;
		m_RAMI_PreparedRegion = region;
		m_RAMI_PreparedCommonType = commonType;
		m_RAMI_PreparedItemId = itemId;
		m_RAMI_PreparedSucceeded = succeeded;
		m_RAMI_PreparedReady = true;
	}

	void RAMI_RequestTriage(SCR_CharacterDamageManagerComponent damageManager)
	{
		if (!damageManager)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;
		if (targetId == m_RAMI_TriageTargetId && System.GetTickCount(m_RAMI_LastTriageRequest) < 500)
			return;

		m_RAMI_LastTriageRequest = System.GetTickCount();
		if (Replication.IsServer())
		{
			RAMI_UpdateTriage(targetId);
			return;
		}

		Rpc(RAMI_RpcAsk_GetTriage, targetId);
	}

	bool RAMI_GetTriage(SCR_CharacterDamageManagerComponent damageManager, out RAMI_ETriageLevel triageLevel, out array<int> times, out array<string> messages)
	{
		if (!damageManager || Replication.FindItemId(damageManager) != m_RAMI_TriageTargetId)
			return false;

		triageLevel = m_RAMI_TriageLevel;
		times = m_RAMI_ActivityTimes;
		messages = m_RAMI_ActivityMessages;
		return true;
	}

	void RAMI_SetPatientTriage(SCR_CharacterDamageManagerComponent damageManager, RAMI_ETriageLevel triageLevel)
	{
		if (!damageManager || triageLevel < RAMI_ETriageLevel.NONE || triageLevel > RAMI_ETriageLevel.EXPECTANT)
			return;

		RplId targetId = Replication.FindItemId(damageManager);
		if (!targetId.IsValid())
			return;

		if (Replication.IsServer())
		{
			RAMI_ApplyTriage(targetId, triageLevel);
			return;
		}

		Rpc(RAMI_RpcAsk_SetTriage, targetId, triageLevel);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_GetTriage(RplId targetId)
	{
		RAMI_UpdateTriage(targetId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RAMI_RpcAsk_SetTriage(RplId targetId, RAMI_ETriageLevel triageLevel)
	{
		RAMI_ApplyTriage(targetId, triageLevel);
	}

	protected void RAMI_ApplyTriage(RplId targetId, RAMI_ETriageLevel triageLevel)
	{
		if (triageLevel < RAMI_ETriageLevel.NONE || triageLevel > RAMI_ETriageLevel.EXPECTANT)
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		if (!damageManager || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()))
			return;

		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(damageManager.GetOwner().FindComponent(ACE_Medical_MedicationComponent));
		if (!medication)
			return;

		medication.RAMI_SetTriageLevel(triageLevel);
		RAMI_SendTriage(targetId, medication);
	}

	protected void RAMI_UpdateTriage(RplId targetId)
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(Replication.FindItem(targetId));
		if (!damageManager || !RAMI_IsTreatmentTargetInRange(damageManager.GetOwner()))
			return;

		ACE_Medical_MedicationComponent medication = ACE_Medical_MedicationComponent.Cast(damageManager.GetOwner().FindComponent(ACE_Medical_MedicationComponent));
		if (medication)
			RAMI_SendTriage(targetId, medication);
	}

	protected void RAMI_SendTriage(RplId targetId, ACE_Medical_MedicationComponent medication)
	{
		array<int> times = {};
		array<string> messages = {};
		medication.RAMI_GetActivity(times, messages);

		string packedTimes;
		string packedMessages;
		for (int index = 0; index < times.Count(); index++)
		{
			if (index > 0)
				packedTimes += "|";
			packedTimes += times[index].ToString();
			packedMessages += messages[index].Length().ToString() + ":" + messages[index];
		}

		RAMI_SetTriageSnapshot(targetId, medication.RAMI_GetTriageLevel(), packedTimes, packedMessages);
		Rpc(RAMI_RpcDo_SetTriage, targetId, medication.RAMI_GetTriageLevel(), packedTimes, packedMessages);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RAMI_RpcDo_SetTriage(RplId targetId, RAMI_ETriageLevel triageLevel, string packedTimes, string packedMessages)
	{
		RAMI_SetTriageSnapshot(targetId, triageLevel, packedTimes, packedMessages);
	}

	protected void RAMI_SetTriageSnapshot(RplId targetId, RAMI_ETriageLevel triageLevel, string packedTimes, string packedMessages)
	{
		m_RAMI_TriageTargetId = targetId;
		m_RAMI_TriageLevel = triageLevel;
		m_RAMI_ActivityTimes.Clear();
		m_RAMI_ActivityMessages.Clear();
		if (packedTimes.IsEmpty())
			return;

		array<string> times = {};
		packedTimes.Split("|", times, false);
		string remainingMessages = packedMessages;
		foreach (string unused : times)
		{
			int separator = remainingMessages.IndexOf(":");
			if (separator < 1)
				break;
			int messageLength = remainingMessages.Substring(0, separator).ToInt();
			if (messageLength < 0 || remainingMessages.Length() < separator + 1 + messageLength)
				break;
			m_RAMI_ActivityMessages.Insert(remainingMessages.Substring(separator + 1, messageLength));
			remainingMessages = remainingMessages.Substring(separator + 1 + messageLength, remainingMessages.Length() - separator - 1 - messageLength);
		}
		if (times.Count() != m_RAMI_ActivityMessages.Count())
			m_RAMI_ActivityMessages.Clear();

		foreach (string time : times)
		{
			if (!m_RAMI_ActivityMessages.IsEmpty())
				m_RAMI_ActivityTimes.Insert(time.ToInt());
		}
	}
}
