class IBX_CarryCrateAction : SCR_ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		return comp && comp.CanToggleCarry(user);
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Carry Crate";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(pOwnerEntity);
		if (comp)
			comp.RequestToggleCarry(pUserEntity);
	}
}

class IBX_DragCrateAction : SCR_ScriptedUserAction
{
	// Checked continuously while the action is held, not just when the hold starts - going out of
	// range cancels the hold (OnActionCanceled) and updates the client's UI.
	override bool CanBePerformedScript(IEntity user)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		return comp && comp.CanStartDrag(user) && comp.IsWithinDragRange(user);
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Drag Crate";
		return true;
	}

	// OnActionStart/OnActionCanceled are the hooks that reliably fire for a held action (unlike
	// OnConfirmed for a single tap), and they run on the performing client too - hence the
	// authority check inside RequestStartDrag/RequestDrop.
	override void OnActionStart(IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		if (comp)
			comp.RequestStartDrag(pUserEntity);
	}

	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(pOwnerEntity);
		if (comp)
			comp.RequestDrop();
	}

	// Deliberately no PerformAction override: with Duration -1/PerformPerFrame 1 it fires every
	// frame the key is held, not once on release, so dropping from there undid every attach.
	// OnActionCanceled plus CanBePerformedScript's continuous re-check cover both stop conditions.
	override bool CanBroadcastScript()
	{
		return false;
	}
}

// Client-only: local movement-speed override while carrying or dragging. Not networked - each
// client only ever drives this for the crate it is itself holding. Drop/height/rotate are world
// actions, not raw key polls: an Rpc() from a bare CallLater never arrived server-side, so all
// RPCs must originate from action callbacks.
class IBX_GMCarryClient
{
	protected static const float CARRY_SPEED_FRACTION = 0.35;
	protected static const float DRAG_SPEED_FRACTION = 0.65;

	protected static IBX_ECrateCarryMode s_eMode = IBX_ECrateCarryMode.NONE;
	protected static IEntity s_CarriedCrateOwner;
	protected static IEntity s_HolderCharacter;

	static void OnHoldStarted(IEntity crateOwner, IBX_ECrateCarryMode mode)
	{
		s_eMode = mode;
		s_CarriedCrateOwner = crateOwner;
		s_HolderCharacter = SCR_PlayerController.GetLocalControlledEntity();
		SetLocalSpeedFraction(GetSpeedFractionFor(mode));
	}

	// Called from StopServer for whichever crate the local player was holding.
	static void OnHoldEnded(IEntity crateOwner)
	{
		if (s_CarriedCrateOwner != crateOwner)
			return;

		s_eMode = IBX_ECrateCarryMode.NONE;
		s_CarriedCrateOwner = null;
		s_HolderCharacter = null;
		SetLocalSpeedFraction(1.0);
	}

	protected static float GetSpeedFractionFor(IBX_ECrateCarryMode mode)
	{
		if (mode == IBX_ECrateCarryMode.CARRY)
			return CARRY_SPEED_FRACTION;

		return DRAG_SPEED_FRACTION;
	}

	// Client-side substitute for the [RplProp] fields that never reach a proxy copy of
	// IBX_CrateCarryComponent - OnHoldStarted/OnHoldEnded above are driven by RpcDo_HoldChanged,
	// which is reliable and already matches the local player before calling either.
	static bool IsHolding(IEntity crateOwner)
	{
		return crateOwner && s_CarriedCrateOwner == crateOwner;
	}

	// Used to hide Carry/Drag toggle actions on every crate (not just the held one) while this
	// client already holds something - Drop covers letting go, and the player can't pick up or
	// drag a second crate while already carrying/dragging one anyway.
	static bool IsHoldingAnything()
	{
		if (s_eMode == IBX_ECrateCarryMode.NONE)
			return false;

		// Self-heal: the character that started this hold is no longer the one being controlled,
		// so the player died and respawned. Whatever they were carrying belonged to the old body.
		// Without this, a stop broadcast that never resolved leaves the client convinced it is
		// still holding a crate, which hides Carry on every crate for the rest of the session.
		if (s_HolderCharacter != SCR_PlayerController.GetLocalControlledEntity())
		{
			OnHoldEnded(s_CarriedCrateOwner);
			return false;
		}

		return true;
	}

	static IBX_ECrateCarryMode GetModeFor(IEntity crateOwner)
	{
		if (!IsHolding(crateOwner))
			return IBX_ECrateCarryMode.NONE;

		return s_eMode;
	}

	protected static void SetLocalSpeedFraction(float fraction)
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		CharacterControllerComponent controller = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		if (controller)
			controller.OverrideMaxSpeed(fraction);
	}
}

// One-shot: drops the crate from either mode. A dedicated action rather than relying solely on
// releasing the drag key or re-toggling carry, so there's always an explicit, visible way to let
// go. CanBePerformedScript only requires this user to be the current holder - no range check
// needed, the crate is right in front of them either way.
class IBX_DropCrateAction : SCR_ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		return comp && comp.IsHeldBy(user);
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Drop Crate";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(pOwnerEntity);
		if (comp)
			comp.RequestDrop();
	}
}

// Shared base for the four carry-only held adjust actions (raise/lower/rotate left/rotate right).
// Drives a server-side ticking rate via OnActionStart/OnActionCanceled - PerformAction is not
// confirmed to repeat reliably for a held action, those two hooks are.
class IBX_CarryAdjustActionBase : SCR_ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		return comp && comp.IsHeldBy(user) && comp.GetMode() == IBX_ECrateCarryMode.CARRY;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override void OnActionStart(IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(GetOwner());
		if (comp)
			StartAdjust(comp);
	}

	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		IBX_CrateCarryComponent comp = IBX_CrateCarryComponent.Get(pOwnerEntity);
		if (comp)
			StopAdjust(comp);
	}

	protected void StartAdjust(IBX_CrateCarryComponent comp)
	{
	}

	protected void StopAdjust(IBX_CrateCarryComponent comp)
	{
	}

	override bool CanBroadcastScript()
	{
		return false;
	}
}

class IBX_RaiseCrateAction : IBX_CarryAdjustActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Raise Crate";
		return true;
	}

	override void StartAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStart(0.02, 0);
	}

	override void StopAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStop(0.02, 0);
	}
}

class IBX_LowerCrateAction : IBX_CarryAdjustActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Lower Crate";
		return true;
	}

	override void StartAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStart(-0.02, 0);
	}

	override void StopAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStop(-0.02, 0);
	}
}

class IBX_RotateCrateLeftAction : IBX_CarryAdjustActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Rotate Crate Left";
		return true;
	}

	override void StartAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStart(0, -3);
	}

	override void StopAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStop(0, -3);
	}
}

class IBX_RotateCrateRightAction : IBX_CarryAdjustActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Rotate Crate Right";
		return true;
	}

	override void StartAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStart(0, 3);
	}

	override void StopAdjust(IBX_CrateCarryComponent comp)
	{
		comp.RequestAdjustStop(0, 3);
	}
}
