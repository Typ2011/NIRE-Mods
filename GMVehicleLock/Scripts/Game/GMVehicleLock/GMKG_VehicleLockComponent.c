//! Vanilla SCR_BaseLockComponent.SetLocked() only writes a local member, so a lock applied on the
//! server never reaches the clients and their Get In actions stay enabled. Mirror the state in a
//! replicated property and re-apply the vanilla flag on every machine that receives it.
modded class SCR_BaseLockComponent
{
	[RplProp(onRplName: "GMKG_OnLockedReplicated")]
	protected bool m_bGMKG_Locked;

	//------------------------------------------------------------------------------------------------
	//! Server-authoritative lock, streamed to all clients and to players joining later.
	//! \param[in] locked True to block player interaction with the vehicle
	void GMKG_SetLocked(bool locked)
	{
		if (!Replication.IsServer())
			return;

		if (m_bGMKG_Locked == locked)
			return;

		m_bGMKG_Locked = locked;
		SetLocked(locked);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return True when the vehicle is locked by the Game Master
	bool GMKG_IsLocked()
	{
		return m_bGMKG_Locked;
	}

	//------------------------------------------------------------------------------------------------
	protected void GMKG_OnLockedReplicated()
	{
		SetLocked(m_bGMKG_Locked);
	}

	//------------------------------------------------------------------------------------------------
	//! Without this the blocked Get In action is greyed out with no explanation, the vanilla reason is
	//! only filled in by the spawn protection component.
	override LocalizedString GetCannotPerformReason(IEntity user)
	{
		if (m_bGMKG_Locked)
			return "#AR-Campaign_Action_CannotEnterVehicle-UC";

		return super.GetCannotPerformReason(user);
	}
}
