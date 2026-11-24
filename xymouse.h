

class MS_XY_NewBrush : MouseCommand {
public:
	MS_XY_NewBrush() {};
	~MS_XY_NewBrush() {};
	bool MouseDown(MouseState_t& state);
	bool MouseUp(MouseState_t& state);
	bool MouseMove(MouseState_t& state);
	bool MouseOver(MouseState_t& state) {}
	const char *GetName() { return "NEWBRUSH"; };
};

