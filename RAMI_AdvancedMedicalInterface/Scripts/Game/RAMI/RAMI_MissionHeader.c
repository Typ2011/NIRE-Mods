modded class SCR_MissionHeader
{
	// Workbench derives the checkbox caption from this serialized field name.
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allow to open AMI Menu while unconscious.\nEnabled: unconscious players may open AMI to observe their treatment; treatment permissions remain unchanged.\nDisabled (default): AMI cannot be opened while unconscious and closes immediately when the player loses consciousness.", category: "AMI")]
	bool m_bAllowToOpenAMIMenuWhileUnconscious;
}
