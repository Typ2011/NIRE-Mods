modded class ACE_Medical_NetworkComponent
{
	override protected void RpcDo_UpdatePatientDataOwner(RplId patientID, ACE_Medical_PatientData data)
	{
		super.RpcDo_UpdatePatientDataOwner(patientID, data);
		// Only an ACE response may mark pinned readings as fresh.
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (playerController && data)
			playerController.RAMI_ReceivePinnedVitals(patientID);
	}
}
