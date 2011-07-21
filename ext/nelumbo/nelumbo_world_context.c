#include <nelumbo.h>


void wc_load_map(WorldContext *wc, char *buf, int width, int height) {
	unsigned char *input = (unsigned char *)buf;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			unsigned char firstByte = *(input++);
			wc->floors[x][y] = firstByte | (*(input++) << 8);
		}
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			unsigned char firstByte = *(input++);
			wc->items[x][y] = firstByte | (*(input++) << 8);
		}
	}

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			wc->walls[x*2][y] = *(input++);
		}
		for (int y = 0; y < height; y++) {
			wc->walls[x*2+1][y] = *(input++);
		}
	}
}


bool wc_position_is_walkable(WorldContext *wc, int x, int y, VALUE player) {
	if (x <= 3 || y <= 8 || x >= (wc->mapWidth - 6) || y >= (wc->mapHeight - 8))
		return false;

	int item = wc->items[x][y];
	int floor = wc->floors[x][y];

	if (item < MAX_ITEM && !wc->itemWalkable[item])
		return false;
	if (floor < MAX_FLOOR && !wc->floorWalkable[floor])
		return false;

	VALUE sPlayer = rb_funcall(wc->bot, rb_intern("find_player_at_position"), 2, x, y);
	if (!NIL_P(sPlayer)) {
		if (player != sPlayer)
			return false;
	}

	return true;
}


void wc_execute_on_area_position(WorldContext *wc, DSLine *line, int x, int y) {
	/* Note: Filter params are "baked in" when assigned */
	for (int filterID = 0; filterID < wc->filterCount; filterID++) {
		DSLine *filter = wc->filters[filterID];

		switch (filter->type) {
			case 1:
				if (wc->floors[x][y] != filter->params[0])
					return false;
				break;
			case 2:
				if (wc->floors[x][y] == filter->params[0])
					return false;
				break;
			case 3:
				if (wc->items[x][y] != filter->params[0])
					return false;
				break;
			case 4:
				if (wc->items[x][y] == filter->params[0])
					return false;
				break;
			case 5:
				if (NIL_P(rb_funcall(wc->bot, id_find_player_at_position, 2, x, y)))
					return false;
				break;
			case 6:
				if (!NIL_P(rb_funcall(wc->bot, id_find_player_at_position, 2, x, y)))
					return false;
				break;
			case 7:
				if (wc->items[x][y] == 0)
					return false;
				break;
			case 8:
				if (wc->items[x][y] != 0)
					return false;
				break;
			case 9:
				if (!wc_position_is_walkable(wc, x, y, Qnil))
					return false;
				break;
			case 10:
				if (wc_position_is_walkable(wc, x, y, Qnil))
					return false;
				break;
			case 12:
				int top, bottom, left, right;
				get_visibility_bounds(&top, &bottom, &left, &right,
						FIX2INT(rb_ivar_get(wc->i_user, id_iv_x)), FIX2INT(rb_ivar_get(wc->i_user, id_iv_y)));

	}
}


void wc_execute_on_area(WorldContext *wc, DSLine *line) {
	int x, y;

	/* Note: Area params are "baked in" when assigned */
	switch (wc->currentArea->type) {
		case 1:
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;
	}
}


void wc_execute_effect(WorldContext *wc, DSLine *line) {
	switch (line->type) {
		/* Those commented out are not relevant to us. */
		case 1: case 2: case 3: case 4: case 5: case 6:
		/*case 9:*/
		case 16: case 17: case 19: case 20: case 21: case 22: case 23: case 24:
		case 25: case 26: case 27: case 28: case 29:
		/*case 32:*/
		case 42: case 43: case 44: case 45: case 46: case 47:
		/*case 52, 53, 54, 55:*/
		case 60: case 61: case 62: case 66: case 67: case 68:
		/*case 73, 74, 75:*/
		case 77: case 80: case 81: case 82: case 83: case 84: case 85: case 86:
		case 87:
		/*case 94, 95, 96, 97, 98, 99:*/
		/*case 103:*/
		/*case 201:*/
		case 400: case 401: case 402: case 410: case 411: case 412:
		/*case 421, 423:*/
		/*case 701, 703:*/
		case 716: case 717: case 719: case 780: case 781: case 782: case 783:
		case 784: case 785: case 786: case 787:
			wc_execute_on_area(wc, line);
			break;

		case 14:
			// FUCK EVERYTHING ABOUT THIS
			int targetX = wc_ds_value(wc, line->params[0]) / 2;
			int targetY = wc_ds_value(wc, line->params[1]);
			if (wc_position_is_walkable(wc, targetX, targetY, wc->i_user)) {
				rb_funcall(wc->bot, rb_intern("move_player"), 3, wc->i_user, targetX, targetY);
			}
	}
}

