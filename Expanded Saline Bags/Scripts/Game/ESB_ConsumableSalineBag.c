class ESB_PartialSaline
{
	static bool CanStop(IEntity target)
	{
		IEntity bag;
		int volumeMl;
		bool isUS;
		return FindBag(target, bag, volumeMl, isUS) && FindEffect(target) != null;
	}

	static void Stop(IEntity target, IEntity user)
	{
		if (!target || !user)
			return;

		RplComponent targetReplication = RplComponent.Cast(target.FindComponent(RplComponent));
		if (targetReplication && targetReplication.IsProxy())
			return;

		IEntity bag;
		int volumeMl;
		bool isUS;
		if (!FindBag(target, bag, volumeMl, isUS))
			return;

		SCR_SalineDamageEffect effect = FindEffect(target);
		if (!effect)
			return;

		float duration = effect.GetMaxDuration();
		if (duration <= 0)
			return;

		int remainingMl = Math.Floor(volumeMl * Math.Max(0, duration - effect.GetCurrentDuration()) / duration / 250) * 250;

		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageManager || !damageManager.TerminateDamageEffect(effect))
			return;

		RplComponent.DeleteRplEntity(bag, false);
		SpawnRemaining(user, GetPrefab(remainingMl, isUS));
	}

	protected static bool FindBag(IEntity target, out IEntity bag, out int volumeMl, out bool isUS)
	{
		SCR_SalineStorageComponent storage = SCR_SalineStorageComponent.Cast(target.FindComponent(SCR_SalineStorageComponent));
		if (!storage)
			return false;

		for (int i, count = storage.GetSlotsCount(); i < count; i++)
		{
			SCR_SalineBagStorageSlot slot = SCR_SalineBagStorageSlot.Cast(storage.GetSlot(i));
			if (!slot)
				continue;

			IEntity candidate = slot.GetAttachedEntity();
			if (candidate && GetBagData(candidate, volumeMl, isUS))
			{
				bag = candidate;
				return true;
			}
		}

		return false;
	}

	protected static SCR_SalineDamageEffect FindEffect(IEntity target)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		if (!character)
			return null;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageManager)
			return null;

		return SCR_SalineDamageEffect.Cast(damageManager.FindDamageEffectOfType(SCR_SalineDamageEffect));
	}

	protected static bool GetBagData(IEntity bag, out int volumeMl, out bool isUS)
	{
		if (!bag.GetPrefabData())
			return false;

		string path = bag.GetPrefabData().GetPrefabName().GetPath();
		isUS = path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_250.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_500.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1000.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1250.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1500.et";

		bool isUSSR = path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_01.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_250.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_500.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1000.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1250.et"
			|| path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1500.et";

		if (!isUS && !isUSSR)
			return false;

		if (path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_250.et" || path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_250.et")
			volumeMl = 250;
		else if (path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_500.et" || path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_500.et")
			volumeMl = 500;
		else if (path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et" || path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_01.et")
			volumeMl = 750;
		else if (path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1000.et" || path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1000.et")
			volumeMl = 1000;
		else if (path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1250.et" || path == "Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1250.et")
			volumeMl = 1250;
		else
			volumeMl = 1500;

		return true;
	}

	protected static ResourceName GetPrefab(int volumeMl, bool isUS)
	{
		if (isUS)
		{
			switch (volumeMl)
			{
				case 250: return "{02C5C61EB9FE39D3}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_250.et";
				case 500: return "{A2CEF84144FB299B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_500.et";
				case 750: return "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et";
				case 1000: return "{02C5C61EB9FE39D2}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1000.et";
				case 1250: return "{D434DDDBD0DAC334}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1250.et";
				case 1500: return "{743FE3842DDFD37D}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_1500.et";
			}
		}
		else
		{
			switch (volumeMl)
			{
				case 250: return "{2830987CCFB737C3}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_250.et";
				case 500: return "{883BA62332B2278B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_500.et";
				case 750: return "{527D7C5D2E476BDC}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_01.et";
				case 1000: return "{2830987CCFB737C2}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1000.et";
				case 1250: return "{FA97410FF7466C72}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1250.et";
				case 1500: return "{5A9C7F500A437C3B}Prefabs/Items/Medicine/SalineBag_01/SalineBag_USSR_1500.et";
			}
		}

		return ResourceName.Empty;
	}

	protected static void SpawnRemaining(IEntity user, ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return;

		Resource resource = Resource.Load(prefab);
		if (!resource)
		{
			PrintFormat("Expanded Saline Bags: Cannot load remaining prefab %1", prefab, level: LogLevel.ERROR);
			return;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		user.GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;

		IEntity remainingBag = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!remainingBag)
		{
			PrintFormat("Expanded Saline Bags: Cannot spawn remaining prefab %1", prefab, level: LogLevel.ERROR);
			return;
		}

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventoryManager)
			inventoryManager.TryInsertItem(remainingBag, EStoragePurpose.PURPOSE_DEPOSIT);
	}
}

modded class SCR_SalineBagUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		if (!ESB_PartialSaline.CanStop(GetOwner()))
			return super.GetActionNameScript(outName);

		string language;
		WidgetManager.GetLanguage(language);
		if (language == "de_de")
			outName = "Saline stoppen";
		else
			outName = "Stop saline";

		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (ESB_PartialSaline.CanStop(GetOwner()))
			return true;

		return super.CanBeShownScript(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (ESB_PartialSaline.CanStop(GetOwner()))
			return true;

		return super.CanBePerformedScript(user);
	}

	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (ESB_PartialSaline.CanStop(GetOwner()))
			return super.GetActionProgressScript(fProgress, timeSlice);

		return GetActionDuration();
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (ESB_PartialSaline.CanStop(pOwnerEntity))
		{
			ESB_PartialSaline.Stop(pOwnerEntity, pUserEntity);
			return;
		}

		super.PerformAction(pOwnerEntity, pUserEntity);
	}
}
