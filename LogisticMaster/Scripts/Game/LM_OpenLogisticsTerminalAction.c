class LM_OpenLogisticsTerminalAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		LM_LogisticsTerminalComponent terminal = LM_LogisticsTerminalComponent.Cast(pOwnerEntity.FindComponent(LM_LogisticsTerminalComponent));
		LM_LogisticsTerminalUI.Open(terminal);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return GetOwner() && GetOwner().FindComponent(LM_LogisticsTerminalComponent);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Logistikterminal öffnen";
		return true;
	}
}
