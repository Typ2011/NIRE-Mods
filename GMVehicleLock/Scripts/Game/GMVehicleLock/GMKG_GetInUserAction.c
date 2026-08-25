//! Turrets and other add-on seats sit on child entities of the vehicle, attached through slots such
//! as Roof and Turret in M1025_armed_M2HB.et. Only the vehicle root carries an SCR_BaseLockComponent,
//! but vanilla Init() looks for it on the compartment's own entity, so those seats never see the lock
//! and stay enterable.
//!
//! Resolving it from the root parent has to happen at check time, not in Init(). Slot children are
//! not necessarily parented to the vehicle yet while their actions initialise, so GetRootParent()
//! would still return the turret itself and the null result would be cached forever. Vanilla works
//! around the same problem in SCR_TurretGetInUserAction, which looks its damage manager up on every
//! call instead of caching it.
modded class SCR_GetInUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_pLockComp)
			GMKG_ResolveLockComponent();

		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Look the lock component up on the vehicle root. Runs until it finds one, which happens on the
	//! first call after the hierarchy is complete, then the vanilla member stays cached.
	protected void GMKG_ResolveLockComponent()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		IEntity vehicle = SCR_EntityHelper.GetMainParent(owner, true);
		if (!vehicle)
			return;

		m_pLockComp = SCR_BaseLockComponent.Cast(vehicle.FindComponent(SCR_BaseLockComponent));
	}
}
