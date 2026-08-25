modded class SCR_AvailableActionsDisplay
{
	protected ref RAMI_AvailableActionContext m_RAMI_AvailableAction;

	override void DisplayInit(IEntity owner)
	{
		if (!m_RAMI_AvailableAction)
		{
			m_RAMI_AvailableAction = new RAMI_AvailableActionContext();
			m_aActions.Insert(m_RAMI_AvailableAction);
		}

		super.DisplayInit(owner);
	}
}
