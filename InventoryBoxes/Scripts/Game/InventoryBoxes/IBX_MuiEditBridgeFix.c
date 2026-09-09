//! Downstream fix for the Mikes UI text field caret, kept here instead of in Mikes UI so this addon
//! and NiRe Notepad both get it without forking a published dependency. NiRe Notepad depends on this
//! addon, so one copy covers both.
//!
//! The bug: MUI_TextField paints its own value text and then a caret placed after the measured width
//! of the whole string, so the caret always sits at the end of the text no matter where the cursor
//! really is. Arrow keys do move the real cursor - it lives in this bridge's native EditBoxWidget -
//! but nothing on screen follows it. It cannot be drawn correctly either, because EditBoxWidget is
//! sealed and exposes neither a cursor position nor a selection to script.
//!
//! The fix, in two halves:
//!   - this file shows the native EditBox in place instead of hiding it at 2% opacity, so the engine
//!     renders the text, the caret and the selection itself, all correct by construction;
//!   - the MUI_TextField override below repaints over the value and caret MUI has already drawn, on
//!     MUI's own render surface, so the two cannot both be on screen at once.
modded class MUI_EditBridge
{
	protected static const ResourceName IBX_EDIT_LAYOUT = "{A1E6470C8DBF4300}UI/layouts/InventoryBoxes/IBX_MuiEditBox.layout";
	//! Above both MUI_RenderSurface layers - MUI_Runtime.Mount creates the root at sort 0 and the
	//! chrome at sort 5. This is the same sort order the stock bridge passes to CreateWidget;
	//! CreateWidgets takes none, hence the explicit SetZOrder.
	protected static const int IBX_EDIT_Z_ORDER = 20;
	protected static const float IBX_HIDDEN_OPACITY = 0.02;

	//! The node whose value the native box is currently drawing, if any. Read by the paint override,
	//! which cannot reach the bridge instance - MUI_Runtime keeps it private.
	protected static MUI_Node s_IBX_PaintedNode;

	protected bool m_IBX_Owned;

	//------------------------------------------------------------------------------------------------
	static bool IBX_IsNativePainting(MUI_Node node)
	{
		if (!node)
			return false;

		return s_IBX_PaintedNode == node;
	}

	//------------------------------------------------------------------------------------------------
	override bool Create(notnull WorkspaceWidget workspace, notnull Widget parent)
	{
		m_Workspace = workspace;
		m_wHost = parent;
		m_wEdit = EditBoxWidget.Cast(workspace.CreateWidgets(IBX_EDIT_LAYOUT, parent));
		if (!m_wEdit)
		{
			// Layout missing or malformed: leave the stock bridge in charge, caret bug and all,
			// rather than a text field nothing can type into.
			m_IBX_Owned = false;
			return super.Create(workspace, parent);
		}

		m_IBX_Owned = true;
		m_wEdit.SetZOrder(IBX_EDIT_Z_ORDER);
		m_wEdit.AddHandler(this);
		m_wEdit.SetOpacity(IBX_HIDDEN_OPACITY);
		m_wEdit.SetText(" ");
		m_wEdit.SetVisible(false);
		return true;
	}

	//! Positions the box over the field's input box rather than the whole node, so the native text
	//! lands on the painted text instead of over the caption above it. The geometry mirrors what
	//! MUI_TextField.PaintForeground lays out: a caption strip of 22 above the box, 24 off the node
	//! height, and the value inset 14 from each side.
	//------------------------------------------------------------------------------------------------
	override void SyncLayout()
	{
		if (!m_IBX_Owned)
		{
			super.SyncLayout();
			return;
		}

		if (!m_Node || !m_wEdit)
			return;

		MUI_Rect r = m_Node.GetWorldRect();
		float x = r.m_fX;
		float y = r.m_fY + m_Node.GetSlideY();
		float w = r.m_fW;
		float h = r.m_fH;
		if (MUI_TextField.Cast(m_Node))
		{
			float boxH = h - MUI_TextField.BOX_SHRINK;
			if (boxH < MUI_TextField.BOX_MIN_HEIGHT)
				boxH = MUI_TextField.BOX_MIN_HEIGHT;

			x = x + MUI_TextField.TEXT_INSET;
			y = y + MUI_TextField.BOX_TOP;
			w = w - MUI_TextField.TEXT_WIDTH_SHRINK;
			h = boxH;
		}

		FrameSlot.SetAnchorMin(m_wEdit, 0, 0);
		FrameSlot.SetAnchorMax(m_wEdit, 0, 0);
		FrameSlot.SetPos(m_wEdit, x, y);
		FrameSlot.SetSize(m_wEdit, w, h);
	}

	//------------------------------------------------------------------------------------------------
	override void EnsureWriteMode()
	{
		super.EnsureWriteMode();
		if (!m_IBX_Owned || !m_wEdit)
			return;

		MUI_ThemeData theme;
		if (m_Node)
			theme = m_Node.GetTheme();
		if (theme)
			m_wEdit.SetColor(theme.Text);

		m_wEdit.SetOpacity(1);
		s_IBX_PaintedNode = m_Node;

		// The placeholder space is only there so an empty box still enters write mode. Now that it
		// has, drop it: it would otherwise be the first character of the value, and a visible box
		// puts a space the player never typed in front of everything they do.
		if (m_bEmptyPlaceholder)
		{
			m_bEmptyPlaceholder = false;
			m_wEdit.SetText("");
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Detach()
	{
		super.Detach();
		s_IBX_PaintedNode = null;
		if (m_IBX_Owned && m_wEdit)
			m_wEdit.SetOpacity(IBX_HIDDEN_OPACITY);
	}
}

//! Covers the value text and the fake caret MUI_TextField paints, on MUI's own render surface and
//! immediately after it draws them, so neither can show through. Painting over rather than
//! suppressing keeps MUI_TextField.PaintForeground upstream's to change - the alternative was copying
//! its body into this addon, where it would drift silently.
modded class MUI_TextField
{
	//! Mirrors the geometry PaintForeground lays its input box out with.
	static const float BOX_TOP = 22;
	static const float BOX_SHRINK = 24;
	static const float BOX_MIN_HEIGHT = 28;
	static const float TEXT_INSET = 14;
	static const float TEXT_WIDTH_SHRINK = 24;

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		super.PaintForeground(surface);
		if (!MUI_EditBridge.IBX_IsNativePainting(this))
			return;

		MUI_ThemeData theme = GetTheme();
		if (!theme)
			return;

		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		float boxH = m_World.m_fH - BOX_SHRINK;
		if (boxH < BOX_MIN_HEIGHT)
			boxH = BOX_MIN_HEIGHT;

		surface.FillRect(DrawX() + TEXT_INSET, DrawY() + BOX_TOP, m_World.m_fW - TEXT_WIDTH_SHRINK, boxH, MUI_ColorUtil.Fade(theme.Field, op), 0);
	}
}
