//------------------------------------------------------------------------------------------------
// Physical carry/drag: pick a crate up (slow, height/rotation adjustable) or drag it along the
// ground (backward-only, faster than carrying, no height/rotation control).
//------------------------------------------------------------------------------------------------

enum IBX_ECrateCarryMode
{
	NONE,
	CARRY,
	DRAG
}

class IBX_CrateCarryComponentClass : ScriptComponentClass
{
}

class IBX_CrateCarryComponent : ScriptComponent
{
	protected static const float CARRY_FORWARD_DISTANCE = 1.6;
	protected static const float CARRY_BASE_HEIGHT = 1.0;
	protected static const float CARRY_HEIGHT_MIN = -0.6;
	protected static const float CARRY_HEIGHT_MAX = 1.2;
	protected static const float DRAG_BACKWARD_TOLERANCE = 70;
	protected static const float DRAG_MAX_RANGE = 2.5;
	protected static const int HOLD_REBROADCAST_TICKS = 40;
	// Vertical span of the support trace, measured from the crate's current position. The
	// up-clearance lets a dragged crate climb onto a doorstep or a floor edge instead of only ever
	// falling, and keeps the trace from starting flush with the surface it is already resting on.
	// The downward reach covers a crate dropped over a railing or off a roof.
	protected static const float SUPPORT_TRACE_UP = 0.5;
	protected static const float SUPPORT_TRACE_DOWN = 20;
	// Steeper than this and the surface is a wall or a rock face rather than something to set a
	// crate on, so the crate stays upright instead of standing on its side. cos(40 degrees).
	protected static const float SUPPORT_MAX_TILT_COS = 0.766;

	protected static ref map<IEntity, IBX_CrateCarryComponent> s_HeldByCharacter = new map<IEntity, IBX_CrateCarryComponent>();

	// Authoritative on the server only - these never reach a proxy copy despite [RplProp] (see
	// GetMode/IsHeldBy). An IEntity is never sent over the network here; only the holder's RplId is,
	// resolved back to an entity on demand.
	[RplProp()]
	protected IBX_ECrateCarryMode m_eMode = IBX_ECrateCarryMode.NONE;

	protected IEntity m_HolderCharacter;

	[RplProp()]
	protected RplId m_HolderRplId;

	protected float m_fHeightOffset;
	protected float m_fYawOffset;
	protected float m_fDragForwardOffset;
	protected float m_fDragHeightOffset;
	protected bool m_bDragAttached;
	protected int m_iTicksSinceRebroadcast;

	// Physics simulation state captured while the crate is held, so the exact state it had is put
	// back on drop rather than a guessed one.
	protected bool m_bSimulationSuspended;
	protected SimulationState m_eSavedSimulationState;

	// The simulation state the crate has while it sits loose in the world, captured from the crate
	// itself so what gets put back after a storage trip is its real state and not a guess.
	protected bool m_bWorldSimulationStateKnown;
	protected SimulationState m_eWorldSimulationState;
	protected bool m_bWasParented;

	// Resolved fresh each call - a proxy's RplComponent may not be queryable at init, and a stale
	// "is authority" default would make clients mutate their non-authoritative copy.
	protected bool IsAuthority()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return true;

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl)
			return true;

		return !rpl.IsProxy();
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(WatchStorageReturn, 500, true);
	}

	// The engine suspends an item's physics when it goes into a storage and does not bring it back
	// when the item comes out again - vanilla item components restore it themselves, and these
	// crates are props with no such component. Measured directly on a crate loaded into a vehicle
	// and taken out again: sim=COLLISION while loose, sim=NONE in the vehicle, sim=NONE again after
	// unloading, with its collider layers byte-identical throughout. A body in NONE is not in the
	// collision world, which is why an unloaded crate stopped blocking characters.
	//
	// ponytail: polled rather than event-driven. InventoryItemComponent.m_OnParentSlotChangedInvoker
	// is the tidier hook, but subscribing to it silently found nothing - the component does not
	// exist yet at init - and no EntityEvent covers a parent change. Switch to the invoker if its
	// timing can be pinned down; one GetParent() per crate every 500 ms is cheap enough until then.
	protected void WatchStorageReturn()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		bool parented = owner.GetParent() != null;
		if (parented == m_bWasParented)
		{
			// Cheap after the first success - the capture guards on already knowing.
			if (!parented)
				CaptureWorldSimulationState();

			return;
		}

		m_bWasParented = parented;
		ApplyStoredVisibility(owner, parented);
		if (!parented)
			RestoreWorldSimulationState();
	}

	// A crate inside a vehicle storage must not be drawn in the world, and vanilla only half
	// does that for these prefabs:
	//
	// - SCR_UniversalInventoryStorageComponent.OnAddedToSlot deliberately calls ShowOwner()
	//   again for any item whose volume reaches MIN_VOLUME_TO_SHOW_ITEM_IN_SLOT (200,000 cm3),
	//   so the four crates above that line - V3 (221,500), V3 covered (302,700), V4 (283,800)
	//   and V4 covered (415,000) - stayed fully visible inside the truck.
	// - The engine's own hide covers only the item entity. The covered stacks keep their cover
	//   in a child entity (vanilla EquipmentBoxStack_*_covered is a Hierarchy: stack root plus
	//   cover child), so the cover kept floating in the middle of the truck once the root went
	//   away.
	//
	// Clearing the flag recursively covers both, and SetVisible() takes the crate's Game Master
	// icon down with it - GM has no business showing a world icon for a crate that is cargo.
	//
	// ponytail: rides the same 500 ms poll as the physics restore, so a big crate can stay drawn
	// for up to one tick after it is loaded. Move both onto InventoryItemComponent's parent-slot
	// invoker if its init timing ever gets pinned down.
	protected void ApplyStoredVisibility(notnull IEntity owner, bool stored)
	{
		if (stored)
			owner.ClearFlags(EntityFlags.VISIBLE, true);
		else
			owner.SetFlags(EntityFlags.VISIBLE, true);

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(owner.FindComponent(SCR_EditableEntityComponent));
		if (editable)
			editable.SetVisible(!stored);
	}

	protected void CaptureWorldSimulationState()
	{
		// Never while carried: the hold deliberately parks the crate in NONE, which is not the
		// state to put back later.
		if (m_bWorldSimulationStateKnown || m_bSimulationSuspended)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		Physics physics = owner.GetPhysics();
		if (!physics)
			return;

		m_eWorldSimulationState = physics.GetSimulationState();
		m_bWorldSimulationStateKnown = true;
	}

	protected void RestoreWorldSimulationState()
	{
		if (!m_bWorldSimulationStateKnown || m_bSimulationSuspended)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		Physics physics = owner.GetPhysics();
		if (!physics || physics.GetSimulationState() == m_eWorldSimulationState)
			return;

		SCR_PhysicsHelper.ChangeSimulationState(owner, m_eWorldSimulationState, true);
	}

	// Runs on every machine, not just the server: characters are simulated locally, so a client
	// that still has collision on its copy of a crate someone else is carrying gets shoved and
	// lifted by it.
	protected void SetCrateCollisionEnabled(bool enable)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		Physics physics = owner.GetPhysics();

		if (!enable)
		{
			// Already suspended - re-saving now would capture NONE and make the restore a no-op.
			if (m_bSimulationSuspended || !physics)
				return;

			m_eSavedSimulationState = physics.GetSimulationState();
			m_bSimulationSuspended = true;

			// SimulationState.NONE takes the body out of the collision world entirely. Clearing
			// interaction layer masks instead was not enough: on the stacked-crate prefabs the
			// engine ignored the mask writes outright (they read back unchanged), and the crate
			// kept blocking and lifting characters. This is the same pair vanilla uses when an
			// item is picked up and dropped again - see SCR_HeadgearInventoryItemComponent.
			// Recursive, because a few crate appearances carry their colliders on child entities.
			SCR_PhysicsHelper.ChangeSimulationState(owner, SimulationState.NONE, true);
			return;
		}

		if (!m_bSimulationSuspended)
			return;

		// Cleared even when the body is gone. A crate carried straight into a vehicle storage has
		// no physics left to restore by the time the hold ends, and leaving the flag set made the
		// next carry of that crate skip suspending collision altogether.
		m_bSimulationSuspended = false;

		if (physics)
			SCR_PhysicsHelper.ChangeSimulationState(owner, m_eSavedSimulationState, true);
	}

	// A hold only ends through StopServer, which until now was reachable only from a player
	// action - so a holder who died, disconnected, boarded a vehicle, or stuffed the crate into
	// a vehicle's storage left it locked in mode CARRY/DRAG forever, hidden from every action.
	protected bool IsHoldStillValid()
	{
		IEntity owner = GetOwner();
		// Parented means the crate now lives inside a storage (loaded into a vehicle from the
		// inventory screen) - there is nothing left to carry.
		if (!owner || owner.GetParent())
			return false;

		if (!m_HolderCharacter)
			return false;

		// Holders are always player characters, so anything that stops looking like a live one -
		// failed cast, missing controller - means the entity is gone or going, not that the hold
		// is still good. The permissive version of this returned true in those cases and was why
		// a crate stayed latched to a dead player.
		ChimeraCharacter character = ChimeraCharacter.Cast(m_HolderCharacter);
		if (!character || character.IsInVehicle())
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		// Unconscious drops the crate the same way death does - a ragdolled holder cannot keep
		// a crate floating in front of them, and they cannot reach the drop action either.
		return !controller.IsDead() && !controller.IsUnconscious();
	}

	// Both mode timers run only while this crate is held, so they are the one place every hold
	// passes through - the validity guard lives here instead of in each caller. Returns false
	// once the hold has ended and the caller must not touch the crate any more.
	protected bool TickHoldCommon()
	{
		if (!IsHoldStillValid())
		{
			StopServer();
			return false;
		}

		m_iTicksSinceRebroadcast++;
		if (m_iTicksSinceRebroadcast >= HOLD_REBROADCAST_TICKS)
		{
			m_iTicksSinceRebroadcast = 0;
			// A client that connected mid-hold missed the original broadcast and still has full
			// collision on its copy. Re-sending is idempotent on every receiver.
			BroadcastHoldChanged(m_eMode, m_HolderCharacter);
		}

		return true;
	}

	// s_HeldByCharacter is keyed by the holder entity. Once that entity is deleted (disconnect)
	// m_HolderCharacter is already null and the key is dangling, so fall back to finding this
	// hold by value - otherwise the entry is never removed.
	protected void ReleaseHolderRegistration(IEntity holder)
	{
		if (holder)
		{
			s_HeldByCharacter.Remove(holder);
			return;
		}

		IEntity staleKey;
		foreach (IEntity character, IBX_CrateCarryComponent held : s_HeldByCharacter)
		{
			if (held == this)
			{
				staleKey = character;
				break;
			}
		}

		if (staleKey)
			s_HeldByCharacter.Remove(staleKey);
	}

	// This crate has no transform-sync component; a server-side SetWorldTransform alone never
	// reaches clients - every move must be applied locally AND explicitly broadcast.
	protected void SyncCrateTransform(vector mat[4])
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		owner.SetWorldTransform(mat);
		owner.Update();

		Rpc(RpcDo_SetCrateTransform, mat[0], mat[1], mat[2], mat[3]);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetCrateTransform(vector right, vector up, vector fwd, vector pos)
	{
		// The server already applied this locally in SyncCrateTransform; only clients need it
		// from here.
		if (IsAuthority())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		vector world[4];
		world[0] = right;
		world[1] = up;
		world[2] = fwd;
		world[3] = pos;
		owner.SetWorldTransform(world);
		owner.Update();
	}

	static IBX_CrateCarryComponent Get(IEntity owner)
	{
		if (!owner)
			return null;

		return IBX_CrateCarryComponent.Cast(owner.FindComponent(IBX_CrateCarryComponent));
	}

	// [RplProp] on this component never reaches proxies - remote clients read the IBX_GMCarryClient
	// cache driven by RpcDo_HoldChanged instead.
	IBX_ECrateCarryMode GetMode()
	{
		if (IsAuthority())
			return m_eMode;

		return IBX_GMCarryClient.GetModeFor(GetOwner());
	}

	// Entity reference and replicated id are set together so they can never drift apart.
	protected void SetHolder(IEntity user)
	{
		m_HolderCharacter = user;

		if (!user)
		{
			m_HolderRplId = RplId.Invalid();
			return;
		}

		RplComponent userRpl = RplComponent.Cast(user.FindComponent(RplComponent));
		if (userRpl)
			m_HolderRplId = userRpl.Id();
		else
			m_HolderRplId = RplId.Invalid();
	}

	// Compares replicated ids, not m_HolderCharacter - that field only ever holds a real value on
	// the server.
	protected bool IsUser(IEntity user)
	{
		if (!user)
			return false;

		RplComponent userRpl = RplComponent.Cast(user.FindComponent(RplComponent));
		return userRpl && userRpl.Id() == m_HolderRplId;
	}

	// Shared by the drop/raise/lower/rotate world actions below - all require this crate to be held
	// by the specific user looking at it. [RplProp] on this component never reaches proxies - remote
	// clients read the IBX_GMCarryClient cache driven by RpcDo_HoldChanged instead.
	bool IsHeldBy(IEntity user)
	{
		if (!IsAuthority())
			return IBX_GMCarryClient.IsHolding(GetOwner());

		if (m_eMode == IBX_ECrateCarryMode.NONE)
			return false;

		return IsUser(user);
	}

	// Purely "can this user START a carry": hidden on every crate, this one included, while already
	// holding something - Drop covers letting go. Stopping an active carry is authorized by
	// ToggleCarryServer itself, not here, so both branches can agree on hiding.
	// [RplProp] on this component never reaches proxies - remote clients read the IBX_GMCarryClient
	// cache driven by RpcDo_HoldChanged instead. Not holding anything locally does not prove nobody
	// else holds THIS crate, so it stays optimistically available and the toggle round-trips to the
	// server, which decides either way.
	bool CanToggleCarry(IEntity user)
	{
		if (!user)
			return false;

		if (!IsAuthority())
			return !IBX_GMCarryClient.IsHoldingAnything();

		if (m_eMode != IBX_ECrateCarryMode.NONE)
			return false;

		return !s_HeldByCharacter.Contains(user);
	}

	// Unlike CanToggleCarry, this is polled continuously for the entire duration of a held drag
	// (Duration -1/PerformPerFrame 1) - returning false for the crate being dragged cancels the hold
	// immediately, so it must stay true for THIS crate and only hide on every other one.
	// [RplProp] on this component never reaches proxies - remote clients read the IBX_GMCarryClient
	// cache driven by RpcDo_HoldChanged instead.
	bool CanStartDrag(IEntity user)
	{
		if (!user)
			return false;

		if (!IsAuthority())
		{
			if (IBX_GMCarryClient.IsHolding(GetOwner()))
				return IBX_GMCarryClient.GetModeFor(GetOwner()) == IBX_ECrateCarryMode.DRAG;

			return !IBX_GMCarryClient.IsHoldingAnything();
		}

		if (m_eMode == IBX_ECrateCarryMode.DRAG && IsUser(user))
			return true;

		return m_eMode == IBX_ECrateCarryMode.NONE && !s_HeldByCharacter.Contains(user);
	}

	// Shared by IBX_DragCrateAction.CanBePerformedScript (so the framework's own continuous
	// re-check of that during a held action naturally cancels it - and updates the client's UI
	// correctly - once out of range) and TickDragServer's own fallback stop.
	bool IsWithinDragRange(notnull IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		vector delta = owner.GetOrigin() - user.GetOrigin();
		return delta.LengthSq() <= DRAG_MAX_RANGE * DRAG_MAX_RANGE;
	}

	// The stop path is authorized here rather than through CanToggleCarry, which now answers only
	// "can this user start a carry" so the action can be hidden while holding. A client whose UI is
	// stale can still legitimately ask to toggle off, so keep honouring that from the actual holder.
	void ToggleCarryServer(IEntity user)
	{
		if (m_eMode != IBX_ECrateCarryMode.NONE)
		{
			if (IsUser(user))
				StopServer();

			return;
		}

		if (CanToggleCarry(user))
			StartCarryServer(user);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ToggleCarry(RplId userId)
	{
		RplComponent userRpl = RplComponent.Cast(Replication.FindItem(userId));
		if (userRpl)
			ToggleCarryServer(userRpl.GetEntity());
	}

	// PerformAction/OnActionStart/OnActionCanceled also run on the performing client - gate on
	// authority, RPC otherwise.
	void RequestToggleCarry(notnull IEntity user)
	{
		if (IsAuthority())
		{
			ToggleCarryServer(user);
			return;
		}

		RplComponent userRpl = RplComponent.Cast(user.FindComponent(RplComponent));
		if (userRpl)
			Rpc(RpcAsk_ToggleCarry, userRpl.Id());
	}

	protected void StartCarryServer(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		m_fHeightOffset = 0;
		m_fYawOffset = 0;
		m_iTicksSinceRebroadcast = 0;

		m_eMode = IBX_ECrateCarryMode.CARRY;
		SetHolder(user);
		s_HeldByCharacter.Set(user, this);

		// Ahead of the first placement, so collision is already off on every machine by the time
		// the crate teleports in front of the character.
		BroadcastHoldChanged(IBX_ECrateCarryMode.CARRY, user);

		ApplyCarryOffset();

		// AddChild reparenting does not replicate parent/child to clients - the crate's absolute
		// world position is re-applied and broadcast every tick instead.
		GetGame().GetCallqueue().Remove(TickCarryServer);
		GetGame().GetCallqueue().CallLater(TickCarryServer, 50, true);
	}

	protected void TickCarryServer()
	{
		if (m_eMode != IBX_ECrateCarryMode.CARRY)
		{
			GetGame().GetCallqueue().Remove(TickCarryServer);
			return;
		}

		if (!TickHoldCommon())
			return;

		ApplyCarryOffset();
	}

	// Tells every machine to match this crate's collision to the hold, and tells the holder's own
	// machine to engage IBX_GMCarryClient's speed cap. A Broadcast RPC does not loop back to its
	// sender - apply locally too (matters for a listen-server host). An unresolvable holder (one
	// who disconnected) is still worth broadcasting: collision has to come back regardless.
	protected void BroadcastHoldChanged(IBX_ECrateCarryMode mode, IEntity holder)
	{
		RplId holderId = RplId.Invalid();
		if (holder)
		{
			RplComponent holderRpl = RplComponent.Cast(holder.FindComponent(RplComponent));
			if (holderRpl)
				holderId = holderRpl.Id();
		}

		Rpc(RpcDo_HoldChanged, mode, holderId);
		ApplyHoldChangedLocally(mode, holderId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_HoldChanged(IBX_ECrateCarryMode mode, RplId holderId)
	{
		ApplyHoldChangedLocally(mode, holderId);
	}

	// Shared by BroadcastHoldChanged (this machine) and RpcDo_HoldChanged (remote peers). Collision
	// applies everywhere; the speed cap only on the holder's own machine. Both halves are
	// idempotent, so a repeat (the JIP catch-up re-broadcast) changes nothing.
	protected void ApplyHoldChangedLocally(IBX_ECrateCarryMode mode, RplId holderId)
	{
		SetCrateCollisionEnabled(mode == IBX_ECrateCarryMode.NONE);

		// Ending a hold must not depend on resolving the holder back to this machine's local
		// player. A player who died while carrying is a corpse or already a freshly respawned
		// entity by the time this arrives, so that comparison fails and the client is left
		// believing forever that it still holds a crate - which hides Carry on every crate.
		// OnHoldEnded already ignores crates this client was not tracking, and that is the only
		// check the stop path actually needs.
		if (mode == IBX_ECrateCarryMode.NONE)
		{
			IBX_GMCarryClient.OnHoldEnded(GetOwner());
			return;
		}

		RplComponent holderRpl = RplComponent.Cast(Replication.FindItem(holderId));
		if (!holderRpl)
			return;

		if (holderRpl.GetEntity() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		IBX_GMCarryClient.OnHoldStarted(GetOwner(), mode);
	}

	void AdjustCarryServer(float heightDelta, float yawDelta)
	{
		if (m_eMode != IBX_ECrateCarryMode.CARRY)
			return;

		m_fHeightOffset = Math.Clamp(m_fHeightOffset + heightDelta, CARRY_HEIGHT_MIN, CARRY_HEIGHT_MAX);
		m_fYawOffset = m_fYawOffset + yawDelta;
		while (m_fYawOffset >= 360)
			m_fYawOffset -= 360;
		while (m_fYawOffset < 0)
			m_fYawOffset += 360;

		ApplyCarryOffset();
	}

	// Raise/Lower/Rotate drive a server-side per-tick rate from OnActionStart/OnActionCanceled -
	// PerformAction is not confirmed to repeat reliably for a held action, those two hooks are.
	protected float m_fAdjustHeightRate;
	protected float m_fAdjustYawRate;

	protected void TickAdjustServer()
	{
		if (m_eMode != IBX_ECrateCarryMode.CARRY || (m_fAdjustHeightRate == 0 && m_fAdjustYawRate == 0))
		{
			GetGame().GetCallqueue().Remove(TickAdjustServer);
			return;
		}

		AdjustCarryServer(m_fAdjustHeightRate, m_fAdjustYawRate);
	}

	void StartAdjustServer(float heightRate, float yawRate)
	{
		if (m_eMode != IBX_ECrateCarryMode.CARRY)
			return;

		m_fAdjustHeightRate = heightRate;
		m_fAdjustYawRate = yawRate;

		GetGame().GetCallqueue().Remove(TickAdjustServer);
		GetGame().GetCallqueue().CallLater(TickAdjustServer, 50, true);
	}

	// Only clears the rate this specific action started, so releasing one adjust key doesn't stop
	// another that's still held (raise+rotate at once, if ever bound to be pressable together).
	void StopAdjustServer(float heightRate, float yawRate)
	{
		if (m_fAdjustHeightRate == heightRate)
			m_fAdjustHeightRate = 0;
		if (m_fAdjustYawRate == yawRate)
			m_fAdjustYawRate = 0;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_StartAdjust(float heightRate, float yawRate)
	{
		StartAdjustServer(heightRate, yawRate);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_StopAdjust(float heightRate, float yawRate)
	{
		StopAdjustServer(heightRate, yawRate);
	}

	// OnActionStart/OnActionCanceled also run on the performing client - gate on authority, RPC
	// otherwise.
	void RequestAdjustStart(float heightRate, float yawRate)
	{
		if (IsAuthority())
			StartAdjustServer(heightRate, yawRate);
		else
			Rpc(RpcAsk_StartAdjust, heightRate, yawRate);
	}

	void RequestAdjustStop(float heightRate, float yawRate)
	{
		if (IsAuthority())
			StopAdjustServer(heightRate, yawRate);
		else
			Rpc(RpcAsk_StopAdjust, heightRate, yawRate);
	}

	// Dummy param required - a zero-argument RPC never arrived server-side (engine quirk).
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_Drop(bool dummy)
	{
		StopServer();
	}

	void RequestDrop()
	{
		if (IsAuthority())
			StopServer();
		else
			Rpc(RpcAsk_Drop, true);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_StartDrag(RplId userId)
	{
		RplComponent userRpl = RplComponent.Cast(Replication.FindItem(userId));
		if (userRpl)
			StartDragServer(userRpl.GetEntity());
	}

	void RequestStartDrag(notnull IEntity user)
	{
		if (IsAuthority())
		{
			StartDragServer(user);
			return;
		}

		RplComponent userRpl = RplComponent.Cast(user.FindComponent(RplComponent));
		if (userRpl)
			Rpc(RpcAsk_StartDrag, userRpl.Id());
	}

	// The placement offset (forward/height) always uses the character's own facing, unaffected by
	// m_fYawOffset; only the crate's visual rotation includes the yaw offset, so rotating it spins
	// it in place instead of swinging its position around the character.
	protected void ApplyCarryOffset()
	{
		IEntity owner = GetOwner();
		if (!owner || !m_HolderCharacter)
			return;

		float userYaw = m_HolderCharacter.GetYawPitchRoll()[0];

		vector facingRot[3];
		Math3D.AnglesToMatrix(Vector(userYaw, 0, 0), facingRot);
		vector forward = facingRot[2];

		vector crateRot[3];
		Math3D.AnglesToMatrix(Vector(userYaw + m_fYawOffset, 0, 0), crateRot);

		vector world[4];
		world[0] = crateRot[0];
		world[1] = crateRot[1];
		world[2] = crateRot[2];
		world[3] = m_HolderCharacter.GetOrigin() + forward * CARRY_FORWARD_DISTANCE + Vector(0, CARRY_BASE_HEIGHT + m_fHeightOffset, 0);

		SyncCrateTransform(world);
	}

	// Drag does not use the user-action system's own continuous/loop-hold mechanism - a plain
	// toggle action starts drag here, and a normal server-side repeating timer drives the
	// movement, same as every other timed thing in this file.
	void StartDragServer(IEntity user)
	{
		if (!CanStartDrag(user))
			return;

		// Already dragging (same holder re-triggering the action): don't stack a second timer.
		if (m_eMode == IBX_ECrateCarryMode.DRAG && m_HolderCharacter == user)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		m_eMode = IBX_ECrateCarryMode.DRAG;
		SetHolder(user);
		s_HeldByCharacter.Set(user, this);
		m_iTicksSinceRebroadcast = 0;

		// Collision is disabled for the whole hold, not just while actually attached/following -
		// otherwise the character can step up onto the crate (Reforger characters auto-step onto
		// small obstacles) while it's briefly detached, e.g. not yet in front or momentarily
		// paused, which then reads as "too close to judge direction" and blocks dragging from
		// ever engaging properly. Broadcast ahead of the first placement so it is already off on
		// every machine by the time the crate moves.
		BroadcastHoldChanged(IBX_ECrateCarryMode.DRAG, user);

		// Attach immediately, at pickup range, same as carry - not on the first backward step,
		// which could be many meters from wherever the crate actually was and "teleport" it in.
		// Only if it's actually in front of the character; if it's behind, TickDragServer picks
		// it up once the character turns to face it (still holding the drag key) and steps back.
		if (IsInFrontOfUser(owner, user))
			AttachDrag(owner, user);

		GetGame().GetCallqueue().Remove(TickDragServer);
		GetGame().GetCallqueue().CallLater(TickDragServer, 50, true);
	}

	// Fixed-range following: while walking backward with the crate in front of the character it
	// follows at a constant offset, so it can never lag behind regardless of movement speed. It
	// detaches and stays put once backward movement stops or the crate ends up behind, and
	// re-attaches when both conditions hold again. While attached, its height is re-snapped to the
	// surface under its own position every tick, or it drifts underground crossing a slope.
	protected void TickDragServer()
	{
		IEntity owner = GetOwner();
		IEntity user = m_HolderCharacter;
		if (m_eMode != IBX_ECrateCarryMode.DRAG || !owner || !user)
		{
			GetGame().GetCallqueue().Remove(TickDragServer);
			return;
		}

		if (!TickHoldCommon())
			return;

		// Fallback only: IBX_DragCrateAction.CanBePerformedScript checks the same range every
		// frame the framework re-validates a held action, which is what actually cancels the
		// hold and updates the client's UI. This just guarantees the crate stops following even
		// if that re-check assumption turns out not to hold.
		if (!IsWithinDragRange(user))
		{
			StopServer();
			return;
		}

		bool backward = IsMovingBackward(user);
		bool attached = m_bDragAttached;
		bool inFront = IsInFrontOfUser(owner, user);

		if (backward && !attached && inFront)
			AttachDrag(owner, user);
		else if ((!backward || !inFront) && attached)
			DetachDrag(owner, user);
		else if (attached)
		{
			// Re-placed from the holder's live transform every tick - no parenting means nothing
			// follows automatically.
			ApplyDragOffset(owner, user);
			ResnapDragHeight(owner);
		}
	}

	protected bool IsMovingBackward(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		CharacterCommandHandlerComponent handler;
		if (character)
			handler = character.GetCommandHandler();

		CharacterCommandMove move;
		if (handler)
			move = handler.GetCommandMove();

		float inputAngle;
		if (!move || !move.GetCurrentInputAngle(inputAngle))
			return false;

		// AbsFloat makes this symmetric: a diagonal backward-left (negative angle) and
		// backward-right (positive angle) input are both accepted equally within the tolerance
		// cone around straight-back (180/-180), not just a dead-straight backward step.
		return Math.AbsFloat(inputAngle) >= (180 - DRAG_BACKWARD_TOLERANCE);
	}

	protected bool IsInFrontOfUser(notnull IEntity owner, notnull IEntity user)
	{
		vector toBox = owner.GetOrigin() - user.GetOrigin();
		toBox[1] = 0;
		// Too close to reliably tell front from behind - collision is off for the whole drag
		// hold, so the character can walk right up to/inside the crate's footprint, and passing
		// through its center flips which side is "in front" from one tick to the next. Radius
		// wide enough to cover standing anywhere in/on a crate, not just its exact origin point.
		if (toBox.LengthSq() < 2.25)
			return false;

		toBox = toBox.Normalized();

		vector rot[3];
		Math3D.AnglesToMatrix(Vector(user.GetYawPitchRoll()[0], 0, 0), rot);
		vector forward = rot[2];

		return (toBox[0] * forward[0] + toBox[1] * forward[1] + toBox[2] * forward[2]) > 0;
	}

	// Height of the first solid surface under a position - a building floor, a bridge deck, a
	// container roof - falling back to the terrain height only when the trace finds nothing at all.
	//
	// SCR_TerrainHelper only ever answers with the heightmap, so every crate dropped inside or on
	// top of a building was teleported down to the ground underneath it. Same trace setup vanilla
	// item placement uses (SCR_ItemPlacementComponent): world plus entities, projectile layer mask.
	protected static float GetSupportY(vector pos, notnull IEntity owner, IEntity ignore = null, out vector surfaceNormal = vector.Up)
	{
		surfaceNormal = vector.Up;

		BaseWorld world = owner.GetWorld();
		if (!world)
			return pos[1];

		TraceParam trace = new TraceParam();
		trace.Start = pos + Vector(0, SUPPORT_TRACE_UP, 0);
		trace.End = pos - Vector(0, SUPPORT_TRACE_DOWN, 0);
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerPresets.Projectile;

		// The trace starts inside the crate's own collider, so it has to ignore the crate or every
		// snap lands it on itself. Filtering the whole hierarchy rather than just the root matters:
		// the covered stacks keep their tarp in a child entity with its own collider.
		trace.Exclude = ignore;
		SCR_Global.g_TraceFilterEnt = owner;
		float traveled = world.TraceMove(trace, SCR_Global.FilterCallback_IgnoreEntityWithChildren);
		SCR_Global.g_TraceFilterEnt = null;
		if (traveled < 1)
		{
			surfaceNormal = trace.TraceNorm;
			return trace.Start[1] + (trace.End[1] - trace.Start[1]) * traveled;
		}

		return SCR_TerrainHelper.GetTerrainY(pos, world);
	}

	// Tilts a transform so its up axis matches the surface it is resting on, keeping the yaw it
	// already has. SCR_EntityHelper.OrientUpToVector is the vanilla helper for this, but it rebuilds
	// the whole basis out of the normal alone and throws the yaw away, which would spin every
	// dropped crate to some arbitrary new facing.
	protected static void OrientToSurface(inout vector mat[4], vector surfaceNormal)
	{
		// Normalized before anything reads it: an unscaled basis is what keeps SetWorldTransform
		// from resizing the crate, and the tilt limit below is a cosine.
		vector up = surfaceNormal;
		if (up.LengthSq() < 0.0001)
			return;

		up.Normalize();
		if (up[1] < SUPPORT_MAX_TILT_COS)
			return;

		// Degenerate only if the crate is somehow already facing straight along the normal, which
		// the tilt limit above all but rules out - the guard is here so a zero-length basis can
		// never reach SetWorldTransform.
		vector right = up * mat[2];
		if (right.LengthSq() < 0.0001)
			return;

		right.Normalize();

		vector forward = right * up;
		forward.Normalize();

		mat[0] = right;
		mat[1] = up;
		mat[2] = forward;
	}

	protected void ResnapDragHeight(notnull IEntity owner)
	{
		vector mat[4];
		owner.GetWorldTransform(mat);
		float supportY = GetSupportY(mat[3], owner, m_HolderCharacter);
		if (Math.AbsFloat(mat[3][1] - supportY) < 0.02)
			return;

		mat[3][1] = supportY;
		SyncCrateTransform(mat);
	}

	// Re-centers the crate onto the character's forward axis (zero sideways offset) while keeping
	// its current forward distance and height - otherwise sidestepping while detached leaves it
	// dragged from off to one side. Captured as forward-distance + height, re-applied by
	// ApplyDragOffset from the holder's live transform every tick while attached.
	protected void AttachDrag(notnull IEntity owner, notnull IEntity user)
	{
		vector delta = owner.GetOrigin() - user.GetOrigin();

		vector rot[3];
		Math3D.AnglesToMatrix(Vector(user.GetYawPitchRoll()[0], 0, 0), rot);
		vector forward = rot[2];

		m_fDragForwardOffset = delta[0] * forward[0] + delta[1] * forward[1] + delta[2] * forward[2];
		m_fDragHeightOffset = delta[1];
		m_bDragAttached = true;

		ApplyDragOffset(owner, user);
	}

	// Rotation matches the character's own live orientation exactly (not a fixed capture, not an
	// identity+parent trick) so the crate visibly pivots as the character turns while backing up.
	protected void ApplyDragOffset(notnull IEntity owner, notnull IEntity user)
	{
		vector rot[3];
		Math3D.AnglesToMatrix(Vector(user.GetYawPitchRoll()[0], 0, 0), rot);
		vector forward = rot[2];

		vector world[4];
		world[0] = rot[0];
		world[1] = rot[1];
		world[2] = rot[2];
		world[3] = user.GetOrigin() + forward * m_fDragForwardOffset + Vector(0, m_fDragHeightOffset, 0);

		SyncCrateTransform(world);
	}

	protected void DetachDrag(notnull IEntity owner, notnull IEntity user)
	{
		m_bDragAttached = false;

		vector mat[4];
		owner.GetWorldTransform(mat);
		vector surfaceNormal;
		float supportY = GetSupportY(mat[3], owner, user, surfaceNormal);
		mat[3][1] = supportY;
		OrientToSurface(mat, surfaceNormal);
		SyncCrateTransform(mat);
		ResyncPhysics(owner);
	}

	void StopServer()
	{
		IEntity owner = GetOwner();
		if (!owner || m_eMode == IBX_ECrateCarryMode.NONE)
			return;

		m_bDragAttached = false;

		GetGame().GetCallqueue().Remove(TickCarryServer);
		GetGame().GetCallqueue().Remove(TickDragServer);

		IEntity holder = m_HolderCharacter;

		// The hold is released before anything that touches the world below. A dead or deleted
		// holder makes the reposition the most likely thing here to fail, and if it does the
		// crate must still come free - otherwise it stays latched to a corpse forever, invisible
		// to every action because its mode never left CARRY.
		m_eMode = IBX_ECrateCarryMode.NONE;
		SetHolder(null);
		m_fHeightOffset = 0;
		m_fYawOffset = 0;

		ReleaseHolderRegistration(holder);

		// Restores collision on every machine (see ApplyHoldChangedLocally) and clears the
		// holder's speed penalty. Collision is disabled for the whole hold in both modes,
		// independent of whether the crate happens to be attached right now.
		BroadcastHoldChanged(IBX_ECrateCarryMode.NONE, holder);

		// A crate that ended up inside a storage (loaded into a vehicle from the inventory screen)
		// has no world position to be dropped at - snapping it to a surface would yank it straight
		// back out of the vehicle.
		if (!owner.GetParent())
		{
			vector mat[4];
			owner.GetWorldTransform(mat);
			vector surfaceNormal;
			float supportY = GetSupportY(mat[3], owner, holder, surfaceNormal);
			mat[3][1] = supportY;
			OrientToSurface(mat, surfaceNormal);
			SyncCrateTransform(mat);
			ResyncPhysics(owner);
		}
	}

	// A moved static-collision entity needs its cached bounds/culling and physics state
	// refreshed explicitly, or it can render/cull incorrectly (visible from some angles only)
	// and carry stale velocity. Same pattern a working shipped crate-move mod uses.
	protected static void ResyncPhysics(notnull IEntity owner)
	{
		Physics physics = owner.GetPhysics();
		if (physics)
			physics.SetVelocity(vector.Zero);

		owner.Update();
	}
}
