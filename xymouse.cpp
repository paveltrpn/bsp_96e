#include "global.hpp"

/****************************************
MS_XY_NewBrush

NEWBRUSH
****************************************/
bool MS_XY_NewBrush::MouseDown(MouseState_t& state) {
	map *m = map_i[set->curmap];
	if (m->numSelected() == 0 && m->current->modifiable) {
		if(newBrushDragFrom(state.start)) {
			snapPoint(&state.start);
			return true;
		}
	}
	return false;
}
bool MS_XY_NewBrush::MouseUp(MouseState_t& state) {
	map *m = map_i[set->curmap];
	
	newbrush->removeUnusedFaces();
	if (newbrush->IsInvalid()) {
		m->current->removeObject(newbrush);
		delete newbrush;
		newbrush = NULL;
	}

}
bool MS_XY_NewBrush::MouseMove(MouseState_t& state) {
}