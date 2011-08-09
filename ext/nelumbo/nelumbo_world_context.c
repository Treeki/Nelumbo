#include <nelumbo.h>


void dsr_seed(DSRandom *dsr, uint32_t seed) {
	int32_t i, soFar = seed | 1;

	dsr->counter = 0;
	dsr->array[0] = soFar;

	for (i = 1; i < 62; i++) {
		soFar *= 69069;
		dsr->array[i] = soFar;
	}
}

uint32_t dsr_generate(DSRandom *dsr, uint32_t max) {
	uint32_t seed;

	dsr->counter -= 1;
	if (dsr->counter >= 0) {
		seed = *(dsr->pointer++);
	} else {
		uint32_t *dest = &dsr->array[0];
		uint32_t *src = &dsr->array[2];

		if (dsr->counter < -1) {
			dsr_seed(dsr, 4357);
		}

		dsr->counter = 61;
		dsr->pointer = &dsr->array[1];

		uint32_t seedp1 = dsr->array[0], seedp2 = dsr->array[1];

		int i;

		for (i = 0; i < 25; i++) {
			*dest++ = dsr->array[i+37] ^ ((seedp2 & 1) != 0 ? 0x9908B0DF : 0) ^ ((seedp1 ^ (seedp1 ^ seedp2) & 0x7FFFFFFE) >> 1);
			seedp1 = seedp2;
			seedp2 = *src++;
		}

		for (i = 0; i < 36; i++) {
			*dest++ = dsr->array[i] ^ ((seedp2 & 1) != 0 ? 0x9908B0DF : 0) ^ ((seedp1 ^ (seedp1 ^ seedp2) & 0x7FFFFFFE) >> 1);
			seedp1 = seedp2;
			seedp2 = *src++;
		}

		seed = dsr->array[0];
	}

	uint32_t value = ((seed >> 11) ^ seed);
	value ^= ((value & 0xFF3A58AD) << 7);
	value ^= ((value & 0xFFFFDF8C) << 15);
	value ^= (value >> 18);
	return value % max;
}


void wc_process_line(WorldContext *wc, char *buf, int length) {
	int offset, number, i, x, y, count, value;
	int fromX, fromY, toX, toY;
	short varValue;

	switch (buf[0]) {
		case '0':
			if (!wc->hasDream)
				return;

			int varID = decode_b95(&buf[1], 2);
			offset = 3;
			while (offset < length) {
				varValue = (short)(decode_b95(&buf[offset], 3) & 0xFFFF);
				offset += 3;

				if (varValue != 16384) {
					wc->variables[varID] = varValue;
					varID++;
				} else {
					count = decode_b95(&buf[offset], 3);
					varValue = (short)(decode_b95(&buf[offset+3], 3) & 0xFFFF);
					offset += 6;

					for (i = 0; i < count; i++) {
						wc->variables[varID] = varValue;
						varID++;
					}
				}
			}

			break;

		case '>': case '1': case '2':
			if (!wc->hasDream)
				return;

			offset = 1;
			while (offset < length) {
				x = decode_b220(&buf[offset], 2);
				y = decode_b220(&buf[offset+2], 2);
				number = decode_b220(&buf[offset+4], 2);

				count = (y / 1000) + ((x / 1000) * 48) + 1;
				x %= 1000;
				y %= 1000;

				for (i = 0; i < count; i++) {
					if (buf[0] == '>')
						wc->items[x][y] = number;
					else if (buf[0] == '1')
						wc->floors[x][y] = number;
					else if (buf[0] == '2')
						wc->walls[x][y] = number;
					y++;
				}

				offset += 6;
			}

			break;

		case '3':
			if (!wc->hasDream)
				return;

			offset = 1;
			i = 0;
			while (offset < length) {
				number = decode_b95(&buf[offset], 3);
				wc->i_special[i++] = number;
				offset += 3;
			}

			wc->i_specialIndex = 0;

			break;

		case '6': case '7':
			if (!wc->hasDream)
				return;

			fromX = decode_b95(&buf[1], 2);
			fromY = decode_b95(&buf[3], 2);
			toX = decode_b95(&buf[5], 2);
			toY = decode_b95(&buf[7], 2);
			offset = 9;

			while (offset < length) {
				number = decode_b95(&buf[offset], 2);
				x = decode_b95(&buf[offset+2], 2);
				y = decode_b95(&buf[offset+4], 2);
				offset += 6;

				if (number > 8000) {
					// oh crap, we have to deal with the thousands value
					// NOTE: Furcadia does not handle this correctly!
					// If a trigger occurs on line 9000, then the server sends sends: number=8000 thousands=1
					// The client uses "if number > 8000", not "if number >= 8000".
					// TODO: test what happens with line 8000
					int thousands = x;
					x = y;
					y = decode_b95(&buf[offset], 2);
					offset += 2;
					number += ((thousands - 8) * 1000);
				}

				wc->i_movedFromX = fromX;
				wc->i_movedFromY = fromY;
				wc->i_movedToX = toX;
				wc->i_movedToY = toY;

				wc_execute_trigger(wc, number - 1, x, y, (buf[0] == '6'));
			}

			break;

		case '8':
			if (!wc->hasDream)
				return;

			wc->i_didPlayerMove = decode_b95(&buf[1], 1);
			wc->i_randomSeed = decode_b95(&buf[2], 5);
			wc->i_numberSaid = decode_b95(&buf[7], 3);
			wc->i_facingDirection = decode_b95(&buf[10], 1);
			wc->i_entryCode = decode_b95(&buf[11], 3);
			wc->i_heldObject = decode_b95(&buf[14], 3);
			wc->i_playersInDream = decode_b95(&buf[17], 2);
			wc->i_userID = decode_b95(&buf[19], 6);
			wc->i_dsButtonPressed = decode_b95(&buf[25], 2);
			wc->i_dreamCookies = decode_b95(&buf[27], 3);
			wc->i_playerCookies = decode_b95(&buf[30], 2);

			dsr_seed(&wc->i_randomGenerator, wc->i_randomSeed);

			if (wc->i_userID == 0) {
				wc->i_playerValue = Qnil;
				wc->i_player = 0;

				printf("Nobody is executing some DS!\n");
			} else {
				wc->i_playerValue = rb_hash_aref(wc->playersByUserID, INT2NUM(wc->i_userID));

				if (wc->i_playerValue == Qnil) {
					// this happens when someone leaves the dream and triggers DS:
					// a non-zero user ID is sent, but Nelumbo has already deleted the
					// Player class and removed it from the records
					//
					// must fix somehow .. caching the last removed user and ID, maybe?
					wc->i_player = 0;

					printf("[%d]Unknown is executing some DS!\n", wc->i_userID);
				} else {
					Data_Get_Struct(wc->i_playerValue, Player, wc->i_player);

					printf("[%d]%s is executing some DS!\n", wc->i_userID, RSTRING_PTR(wc->i_player->name));
				}
			}
			
			break;
	}
}


void wc_load_map(WorldContext *wc, char *buf, int width, int height) {
	wc->hasDream = 1;
	wc->mapWidth = width;
	wc->mapHeight = height;

	unsigned char *input = (unsigned char *)buf;

	int x, y;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			unsigned char firstByte = *(input++);
			wc->floors[x][y] = firstByte | (*(input++) << 8);
		}
	}

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			unsigned char firstByte = *(input++);
			wc->items[x][y] = firstByte | (*(input++) << 8);
		}
	}

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			wc->walls[x*2][y] = *(input++);
		}
		for (y = 0; y < height; y++) {
			wc->walls[x*2+1][y] = *(input++);
		}
	}

	// TODO: Make this really work properly
	int i;
	for (i = 0; i < MAX_ITEM; i++)
		wc->itemWalkable[i] = 1;
	for (i = 0; i < MAX_FLOOR; i++)
		wc->floorWalkable[i] = 1;
}


void wc_save_map(WorldContext *wc, char *buf) {
	unsigned char *output = (unsigned char *)buf;

	int x, y;

	for (x = 0; x < wc->mapWidth; x++) {
		for (y = 0; y < wc->mapHeight; y++) {
			*(output++) = wc->floors[x][y] & 0xFF;
			*(output++) = wc->floors[x][y] >> 8;
		}
	}

	for (x = 0; x < wc->mapWidth; x++) {
		for (y = 0; y < wc->mapHeight; y++) {
			*(output++) = wc->items[x][y] & 0xFF;
			*(output++) = wc->items[x][y] >> 8;
		}
	}

	for (x = 0; x < wc->mapWidth; x++) {
		for (y = 0; y < wc->mapHeight; y++) {
			*(output++) = wc->walls[x*2][y];
		}
		for (y = 0; y < wc->mapHeight; y++) {
			*(output++) = wc->walls[x*2+1][y];
		}
	}
}


char wc_position_is_walkable(WorldContext *wc, int x, int y, Player *player) {
	if (x <= 3 || y <= 8 || x >= (wc->mapWidth - 6) || y >= (wc->mapHeight - 8))
		return 0;

	int item = wc->items[x][y];
	int floor = wc->floors[x][y];

	if (item < MAX_ITEM && !wc->itemWalkable[item])
		return 0;
	if (floor < MAX_FLOOR && !wc->floorWalkable[floor])
		return 0;

	VALUE sPlayer = rb_hash_lookup(wc->playersByPosition, PLAYER_KEY(x, y));
	if (!NIL_P(sPlayer)) {
		Player *sPlayerStruct;
		Data_Get_Struct(sPlayer, Player, sPlayerStruct);
		if (player != sPlayerStruct)
			return 0;
	}

	return 1;
}


char wc_position_is_valid(WorldContext *wc, int x, int y) {
	if (x < 0 || y < 0 || x >= wc->mapWidth || y >= wc->mapHeight)
		return 0;
	return 1;
}

void wc_get_visibility_bounds(WorldContext *wc, int x, int y, int *x1, int *y1, int *x2, int *y2) {
	// I don't understand this entirely
	*x1 = x - 3;
	if (y & 1)
		(*x1)--;

	*y1 = y - 8;

	if (*x1 < 0) {
		*x1 = 0;
	} else if (*x1 > (wc->mapWidth - 10)) {
		*x1 = wc->mapWidth - 10;
	}
	
	if (*y1 < 0) {
		*y1 = 0;
	} else if (*y1 > (wc->mapHeight - 17)) {
		*y1 = wc->mapHeight - 17;
	}

	*x2 = *x1 + 7;
	*y2 = *y1 + 17;
}

static char _intersect_line(int x1, int y1, int x2, int y2, int multiplier) {
	// don't ask
	int xDiff = x2 - x1;
	int yDiff = (y2 - y1) * multiplier;

	printf("Line: %d,%d %d,%d Diff %d,%d M %d A %d,%d\n", x1,y1,x2,y2,xDiff,yDiff,multiplier,(y1&1),(y2&2));

	if ((y1 & 1) == 0) {
		if ((y2 & 1) == 0)
			return ((xDiff * 2) == yDiff);
		else
			return (((xDiff - 1) * 2 + 1) == yDiff);
	} else {
		if ((y2 & 1) == 0)
			return ((xDiff * 2 + 1) == yDiff);
		else
			return ((xDiff * 2) == yDiff);
	}

	return 0;
}

char intersect_line_nwse(int x1, int y1, int x2, int y2) {
	return _intersect_line(x1, y1, x2, y2, 1);
}

char intersect_line_nesw(int x1, int y1, int x2, int y2) {
	return _intersect_line(x1, y1, x2, y2, -1);
}

void wc_clamp_position(WorldContext *wc, short *x, short *y) {
	if (*x < 0)
		*x = 0;
	if (*x > (wc->mapWidth - 1))
		*x = (wc->mapWidth - 1);
	if (*y < 0)
		*y = 0;
	if (*y > (wc->mapHeight - 1))
		*y = (wc->mapHeight - 1);
}

void wc_clamp_position_to_borders(WorldContext *wc, short *x, short *y) {
	if (*x < 4)
		*x = 4;
	if (*x > (wc->mapWidth - 5))
		*x = (wc->mapWidth - 5);
	if (*y < 9)
		*y = 9;
	if (*y > (wc->mapHeight - 9))
		*y = (wc->mapHeight - 9);
}


void wc_move_position_ne(WorldContext *wc, short *x, short *y, int distance) {
	if (distance < 0) {
		wc_move_position_sw(wc, x, y, 0 - distance);
	} else {
		int i;
		for (i = 0; i < distance; i++) {
			(*y)--;
			if ((*y & 1) != 0)
				(*x)++;
		}
	}
}

void wc_move_position_se(WorldContext *wc, short *x, short *y, int distance) {
	if (distance < 0) {
		wc_move_position_nw(wc, x, y, 0 - distance);
	} else {
		int i;
		for (i = 0; i < distance; i++) {
			(*y)++;
			if ((*y & 1) != 0)
				(*x)++;
		}
	}
}

void wc_move_position_sw(WorldContext *wc, short *x, short *y, int distance) {
	if (distance < 0) {
		wc_move_position_ne(wc, x, y, 0 - distance);
	} else {
		int i;
		for (i = 0; i < distance; i++) {
			(*y)++;
			if ((*y & 1) == 0)
				(*x)--;
		}
	}
}

void wc_move_position_nw(WorldContext *wc, short *x, short *y, int distance) {
	if (distance < 0) {
		wc_move_position_se(wc, x, y, 0 - distance);
	} else {
		int i;
		for (i = 0; i < distance; i++) {
			(*y)--;
			if ((*y & 1) == 0)
				(*x)--;
		}
	}
}

void wc_move_position_ne_clamped(WorldContext *wc, short *x, short *y, int distance) {
	wc_move_position_ne(wc, x, y, distance);
	wc_clamp_position_to_borders(wc, x, y);
}

void wc_move_position_se_clamped(WorldContext *wc, short *x, short *y, int distance) {
	wc_move_position_se(wc, x, y, distance);
	wc_clamp_position_to_borders(wc, x, y);
}
void wc_move_position_sw_clamped(WorldContext *wc, short *x, short *y, int distance) {
	wc_move_position_sw(wc, x, y, distance);
	wc_clamp_position_to_borders(wc, x, y);
}
void wc_move_position_nw_clamped(WorldContext *wc, short *x, short *y, int distance) {
	wc_move_position_nw(wc, x, y, distance);
	wc_clamp_position_to_borders(wc, x, y);
}


short wc_ds_value(WorldContext *wc, int value) {
	if (value >= 50000 && value <= 50999)
		return WC_VAR_SAFE(value - 50000);

	return (short)(value & 0xFFFF);
}

// For lines which accept either one number *or* a variable that can have Y.
// Like 5:311 (set array entry).
short wc_ds_value_y(WorldContext *wc, int value) {
	if (value >= 50000 && value <= 50999)
		return WC_VAR_SAFE(value - 50000 + 1);

	return 0;
}


uint32_t wc_random_number(WorldContext *wc, uint32_t max) {
	return dsr_generate(&wc->i_randomGenerator, max);
}


void wc_execute_trigger(WorldContext *wc, int number, int x, int y, char isSelf) {
	//printf("Executing %d at %d,%d\n", number, x, y);

	wc->i_triggerX = x;
	wc->i_triggerY = y;

	// Before we start, grab as much info from the trigger as we can
	if (wc->i_player) {
		rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, wc->i_playerValue, INT2FIX(x*2), INT2FIX(y));

		wc->i_player->entryCode = wc->i_entryCode;
		wc->i_player->heldObject = wc->i_heldObject;
		wc->i_player->cookies = wc->i_playerCookies;
	}

	// Reset the DS engine
	wc->currentArea.category = 3;
	wc->currentArea.type = 1;
	wc->filterCount = 0;

	DSLine *currentLine = &wc->ds[number];

#define LOOKING_FOR_EFFECT_BLOCK 0
#define INSIDE_EFFECT_BLOCK 1
	int stage = LOOKING_FOR_EFFECT_BLOCK;

	while (currentLine < &wc->ds[MAX_DS]) {
		if (stage == LOOKING_FOR_EFFECT_BLOCK) {
			if (currentLine->category == 0 || currentLine->category == 1) {
				// OK, do nothing
				currentLine++;
				continue;

			} else if (currentLine->category == 3 || currentLine->category == 4 || currentLine->category == 5) {
				// Found the effect block, let's fall through to that bit!
				stage = INSIDE_EFFECT_BLOCK;

			} else {
				// What?
				return;
			}
		}

		if (currentLine->category == 3) {
			// Set the current area
			wc_set_area(wc, currentLine);

		} else if (currentLine->category == 4) {
			// Clear or add filters
			if (currentLine->type == 1) {
				wc->filterCount = 0;
			} else {
				wc_add_filter(wc, currentLine);
			}

		} else if (currentLine->category == 5) {
			// Execute an effect
			wc_execute_effect(wc, currentLine);

		} else {
			// Something that's not an effect, leave!
			break;
		}

		currentLine++;
	}
}


// NOTE: WC_VAR_SAFE is defined in the .h file
#define PARAM(id) (line->params[(id)])
#define PARAM_VALUE(id) (wc_ds_value(wc, line->params[(id)]))
#define PARAM_VALUE_Y(id) (wc_ds_value_y(wc, line->params[(id)]))
#define PARAM_VAR(id) WC_VAR_SAFE(line->params[(id)])
#define PARAM_VAR_Y(id) WC_VAR_SAFE(line->params[(id)]+1)

#define GET_AND_CLAMP_X(var, id) \
	var = PARAM_VALUE(id) / 2; \
	if (var < 0) var = 0; \
	if (var >= wc->mapWidth) var = wc->mapWidth - 1;

#define GET_AND_CLAMP_Y(var, id) \
	var = PARAM_VALUE(id); \
	if (var < 0) var = 0; \
	if (var >= wc->mapHeight) var = wc->mapHeight - 1;


void wc_set_area(WorldContext *wc, DSLine *line) {
	DSLine *bakedLine = &wc->currentArea;

	bakedLine->category = line->category;
	bakedLine->type = line->type;
	bakedLine->annotation = line->annotation;

	int paramCount = 0;

	switch (bakedLine->type) {
		case 11: case 13: case 21: case 23:
		case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
		case 511: case 512: case 521: case 522: case 531: case 532: case 541: case 542:
			paramCount = 1;
			break;

		case 2: case 9:
		case 60: case 61: case 62: case 63: case 64: case 65: case 66: case 67:
			paramCount = 2;
			break;

		case 14: case 15: case 16: case 17:
			paramCount = 3;
			break;

		case 3: case 4: case 500:
			paramCount = 4;
			break;

		case 510: case 520: case 530: case 540:
			paramCount = 5;
			break;
	}

	int i;
	for (i = 0; i < paramCount; i++)
		bakedLine->params[i] = PARAM_VALUE(i);
}


void wc_execute_on_area(WorldContext *wc, DSLine *line) {
	int x, y;
	short startX, startY, endX, endY, leftX, leftY, rightX, rightY;
	int top, bottom;

	/* Note: Area params are "baked in" when assigned */
	switch (wc->currentArea.type) {
		case 1:
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 2:
			wc_execute_on_area_position(wc, line, wc->currentArea.params[0] / 2, wc->currentArea.params[1]);
			break;

		case 3:
			// Yes, this code is horribly messy.
			// No, I don't care. It works and that's what matters.
			// I've had more than enough of messing with diamonds.

			printf("Trying diamond %d,%d to %d,%d\n", wc->currentArea.params[0], wc->currentArea.params[1], wc->currentArea.params[2], wc->currentArea.params[3]);

			startX = wc->currentArea.params[0] / 2;
			startY = wc->currentArea.params[1];
			endX = wc->currentArea.params[2] / 2;
			endY = wc->currentArea.params[3];

			leftX = startX;
			leftY = startY;
			do {
				wc_move_position_sw(wc, &leftX, &leftY, 1);
				printf("Moved to %d, %d\n", leftX, leftY);
			} while (!intersect_line_nwse(leftX, leftY, endX, endY));
			printf("Left: %d, %d\n", leftX, leftY);

			rightX = startX;
			rightY = startY;
			do {
				wc_move_position_se(wc, &rightX, &rightY, 1);
				printf("Moved to %d, %d\n", rightX, rightY);
			} while (!intersect_line_nesw(rightX, rightY, endX, endY));
			printf("Right: %d, %d\n", rightX, rightY);

			x = leftX;
			top = leftY;
			bottom = leftY;

			if (leftY & 1) {
				top--;
				bottom++;
			}

			while (x <= rightX) {
				for (y = top; y <= bottom; y++) {
					wc_execute_on_area_position(wc, line, x, y);
					printf("Doing: %d, %d\n", x, y);
				}

				x += 1;

				if (x <= startX) {
					top -= 2;
					if (top < startY)
						top = startY;
				} else {
					if (top & 1)
						top += 2;
					else
						top += 1;
				}

				if (x <= endX) {
					bottom += 2;
					if (bottom > endY)
						bottom = endY;
				} else {
					if (bottom & 1)
						bottom -= 2;
					else
						bottom -= 1;
				}
			}

			break;
			
		case 4:
			for (x = wc->currentArea.params[0]; x <= wc->currentArea.params[2]; x++) {
				for (y = wc->currentArea.params[1]; y <= wc->currentArea.params[3]; y++) {
					wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 5:
			wc_execute_on_area_position(wc, line, wc->i_movedFromX, wc->i_movedFromY);
			break;
	}
}


void wc_add_filter(WorldContext *wc, DSLine *line) {
	DSLine *bakedLine = &wc->filters[wc->filterCount];
	wc->filterCount++;

	bakedLine->category = line->category;
	bakedLine->type = line->type;
	bakedLine->annotation = line->annotation;

	switch (bakedLine->type) {
		case 1: case 2: case 3: case 4:
			bakedLine->params[0] = PARAM_VALUE(0);
			break;
		case 14: case 15:
			bakedLine->params[0] = PARAM_VALUE(0);
			bakedLine->params[1] = PARAM_VALUE(1);
			break;
	}
}


void wc_execute_on_wall(WorldContext *wc, DSLine *line, int x, int y) {
	// Note: This function accepts doubled X params, as used by walls!
	int one, two, shape, texture, targetX, targetY, swap;

	switch (line->type) {
		case 42:
			shape = PARAM_VALUE(0);
			texture = wc->walls[x][y] / 12;
			goto write_wall;
		case 43:
			shape = wc->walls[x][y] % 12;
			texture = PARAM_VALUE(0);
			goto write_wall;
		case 44:
			shape = PARAM_VALUE(0);
			texture = PARAM_VALUE(1);
write_wall:
			wc->walls[x][y] = (texture * 12) + shape;
			break;

		case 45:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			shape = wc->walls[x][y] % 12;

			if (shape == one)
				wc->walls[x][y] = (wc->walls[x][y] - one) + two;
			else if (shape == two)
				wc->walls[x][y] = (wc->walls[x][y] - two) + one;
			break;

		case 46:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			texture = wc->walls[x][y] / 12;

			if (texture == one)
				wc->walls[x][y] = (wc->walls[x][y] - (one * 12)) + (two * 12);
			else if (texture == two)
				wc->walls[x][y] = (wc->walls[x][y] - (two * 12)) + (one * 12);
			break;

		case 47:
			one = (PARAM_VALUE(1) * 12) + PARAM_VALUE(0);
			two = (PARAM_VALUE(3) * 12) + PARAM_VALUE(2);

			if (wc->walls[x][y] == one)
				wc->walls[x][y] = two;
			else if (wc->walls[x][y] == two)
				wc->walls[x][y] = one;
			break;

		case 60:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);

			if ((wc->walls[x][y] % 12) == one)
				wc->walls[x][y] = (wc->walls[x][y] - one) + two;
			break;

		case 61:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			texture = wc->walls[x][y] / 12;

			if (texture == one)
				wc->walls[x][y] = (wc->walls[x][y] - (one * 12)) + (two * 12);
			break;

		case 62:
			one = (PARAM_VALUE(1) * 12) + PARAM_VALUE(0);
			two = (PARAM_VALUE(3) * 12) + PARAM_VALUE(2);

			if (wc->walls[x][y] == one)
				wc->walls[x][y] = two;
			break;

		case 66:
			wc->walls[PARAM_VALUE(0)][PARAM_VALUE(1)] = wc->walls[x][y];
			wc->walls[x][y] = 0;
			break;
		case 67:
			wc->walls[PARAM_VALUE(0)][PARAM_VALUE(1)] = wc->walls[x][y];
			break;
		case 68:
			targetX = PARAM_VALUE(0);
			targetY = PARAM_VALUE(0);
			swap = wc->walls[x][y];
			wc->walls[x][y] = wc->walls[targetX][targetY];
			wc->walls[targetX][targetY] = swap;
			break;

	}
}


void wc_execute_on_area_position(WorldContext *wc, DSLine *line, int x, int y) {
	/* Note: Filter params are "baked in" when assigned */
	int filterID;
	for (filterID = 0; filterID < wc->filterCount; filterID++) {
		DSLine *filter = &wc->filters[filterID];

		switch (filter->type) {
			case 1:
				if (wc->floors[x][y] != filter->params[0])
					return;
				break;
			case 2:
				if (wc->floors[x][y] == filter->params[0])
					return;
				break;
			case 3:
				if (wc->items[x][y] != filter->params[0])
					return;
				break;
			case 4:
				if (wc->items[x][y] == filter->params[0])
					return;
				break;
			case 5:
				if (NIL_P(rb_hash_lookup(wc->playersByPosition, PLAYER_KEY(x, y))))
					return;
				break;
			case 6:
				if (!NIL_P(rb_hash_lookup(wc->playersByPosition, PLAYER_KEY(x, y))))
					return;
				break;
			case 7:
				if (wc->items[x][y] == 0)
					return;
				break;
			case 8:
				if (wc->items[x][y] != 0)
					return;
				break;
			case 9:
				if (!wc_position_is_walkable(wc, x, y, 0))
					return;
				break;
			case 10:
				if (wc_position_is_walkable(wc, x, y, 0))
					return;
				break;
			case 12:
				//int playerX = FIX2INT(rb_ivar_get(wc->i_user, id_iv_x));
				//int playerY = FIX2INT(rb_ivar_get(wc->i_user, id_iv_y));
				if (!position_is_visible_from(x, y, wc->i_player->x, wc->i_player->y))
					return;
				break;
			case 13:
				//int playerX = FIX2INT(rb_ivar_get(wc->i_user, id_iv_x));
				//int playerY = FIX2INT(rb_ivar_get(wc->i_user, id_iv_y));
				if (position_is_visible_from(x, y, wc->i_player->x, wc->i_player->y))
					return;
				break;
			case 14:
				if (!position_is_visible_from(x, y, filter->params[0], filter->params[1]))
					return;
				break;
			case 15:
				if (position_is_visible_from(x, y, filter->params[0], filter->params[1]))
					return;
				break;

		}
	}


	int one, two, cycle[5], cycleCount, i;
	int distance, targetX, targetY, swap, shape, texture;
	short moveX, moveY;
	VALUE affectedPlayer;
	Player *sAffectedPlayer;

#define GET_AFFECTED_PLAYER \
	affectedPlayer = rb_hash_aref(wc->playersByPosition, PLAYER_KEY(x, y)); \
	if (NIL_P(affectedPlayer)) return; \
	Data_Get_Struct(affectedPlayer, Player, sAffectedPlayer)


	// All filters passed, now do it!
	switch (line->type) {
		case 42: case 43: case 44: case 45: case 46: case 47:
		case 60: case 61: case 62: case 63: case 64: case 65:
		case 66: case 67: case 68:
			wc_execute_on_wall(wc, line, x*2, y);
			wc_execute_on_wall(wc, line, x*2+1, y);
			break;

		case 1:
			wc->floors[x][y] = PARAM_VALUE(0);
			break;
		case 2:
			if (wc->floors[x][y] == PARAM_VALUE(0))
				wc->floors[x][y] = PARAM_VALUE(1);
			break;
		case 3:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->floors[x][y] == one)
				wc->floors[x][y] = two;
			else if (wc->floors[x][y] == two)
				wc->floors[x][y] = one;
			break;

		case 4:
			wc->items[x][y] = PARAM_VALUE(0);
			break;
		case 5:
			if (wc->items[x][y] == PARAM_VALUE(0))
				wc->items[x][y] = PARAM_VALUE(1);
			break;
		case 6:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->items[x][y] == one)
				wc->items[x][y] = two;
			else if (wc->items[x][y] == two)
				wc->items[x][y] = one;
			break;

		case 16: case 17: case 716: case 717:
			// This code is closely paired with (5:14) and (5:15). If it is modified,
			// please apply the changes to the other version as well.
			//
			// This version may not even work. Needs testing.
			//
			GET_AFFECTED_PLAYER;

			targetX = PARAM_VALUE(0) / 2;
			targetY = PARAM_VALUE(1);
			if (wc_position_is_walkable(wc, targetX, targetY, sAffectedPlayer)) {
				// TODO: make this into a direct C call?
				rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, affectedPlayer, INT2FIX(targetX*2), INT2FIX(targetY));
			} else {
			}

			if (line->type == 17)
				rb_warn("unsupported DS line (5:17) used");
			if (line->type == 717)
				rb_warn("unsupported DS line (5:717) used");
			break;

		case 19: case 719:
			// Awesome, another line we can't implement due to missing info.
			distance = PARAM_VALUE(0);

			if (distance > 0)
				rb_warn("unsupported DS line (5:19) or (5:719) used");
			break;

		case 20:
			moveX = x;
			moveY = y;
			if (wc->i_facingDirection == 0)
				wc_move_position_sw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == 1)
				wc_move_position_se_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == 2)
				wc_move_position_nw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == 3)
				wc_move_position_ne_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));

			wc->items[moveX][moveY] = wc->items[x][y];
			wc->items[x][y] = 0;
			break;

		case 21:
			wc->items[PARAM_VALUE(0)][PARAM_VALUE(1)] = wc->items[x][y];
			wc->items[x][y] = 0;
			break;
		case 22:
			wc->items[PARAM_VALUE(0)][PARAM_VALUE(1)] = wc->items[x][y];
			break;
		case 23:
			targetX = PARAM_VALUE(0) / 2;
			targetY = PARAM_VALUE(1);
			swap = wc->items[x][y];
			wc->items[x][y] = wc->items[targetX][targetY];
			wc->items[targetX][targetY] = swap;
			break;

		case 24:
			wc->floors[PARAM_VALUE(0)][PARAM_VALUE(1)] = wc->floors[x][y];
			break;
		case 25:
			targetX = PARAM_VALUE(0) / 2;
			targetY = PARAM_VALUE(1);
			swap = wc->floors[x][y];
			wc->floors[x][y] = wc->floors[targetX][targetY];
			wc->floors[targetX][targetY] = swap;
			break;

		case 26:
			wc->items[x][y] += PARAM_VALUE(0);
			break;
		case 27:
			wc->items[x][y] -= PARAM_VALUE(0);
			break;
		case 28:
			wc->floors[x][y] += PARAM_VALUE(0);
			break;
		case 29:
			wc->floors[x][y] -= PARAM_VALUE(0);
			break;

		case 77:
			GET_AFFECTED_PLAYER;

			sAffectedPlayer->heldObject = PARAM_VALUE(0);

			if (sAffectedPlayer == wc->i_player) {
				wc->i_heldObject = sAffectedPlayer->heldObject;
			}
			break;

		case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87:
		case 780: case 781: case 782: case 783: case 784: case 785: case 786: case 787:
			GET_AFFECTED_PLAYER;

			moveX = x;
			moveY = y;
			distance = PARAM_VALUE(0);

			switch (line->type % 100) {
				case 80: wc_move_position_ne_clamped(wc, &moveX, &moveY, distance);
					break;
				case 81: wc_move_position_se_clamped(wc, &moveX, &moveY, distance);
					break;
				case 82: wc_move_position_sw_clamped(wc, &moveX, &moveY, distance);
					break;
				case 83: wc_move_position_nw_clamped(wc, &moveX, &moveY, distance);
					break;
				case 84:
					wc_move_position_ne_clamped(wc, &moveX, &moveY, distance);
					wc_move_position_nw_clamped(wc, &moveX, &moveY, distance);
					break;
				case 85:
					wc_move_position_nw_clamped(wc, &moveX, &moveY, distance);
					wc_move_position_se_clamped(wc, &moveX, &moveY, distance);
					break;
				case 86:
					wc_move_position_sw_clamped(wc, &moveX, &moveY, distance);
					wc_move_position_se_clamped(wc, &moveX, &moveY, distance);
					break;
				case 87:
					wc_move_position_sw_clamped(wc, &moveX, &moveY, distance);
					wc_move_position_nw_clamped(wc, &moveX, &moveY, distance);
					break;
			}

			rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, affectedPlayer, INT2FIX(moveX*2), INT2FIX(moveY));
			break;

		case 400: case 410:
			cycleCount = 3;
			goto do_cycle;
		case 401: case 411:
			cycleCount = 4;
			goto do_cycle;
		case 402: case 412:
			cycleCount = 5;
do_cycle:
			for (i = 0; i < cycleCount; i++)
				cycle[i] = PARAM_VALUE(i);

			if (line->type >= 410) {
				for (i = 1; i < cycleCount; i++) {
					if (wc->floors[x][y] == cycle[i - 1]) {
						wc->floors[x][y] = cycle[i];
						return;
					}
				}

				if (wc->floors[x][y] == cycle[0])
					wc->floors[x][y] = cycle[cycleCount - 1];
			} else {
				for (i = 1; i < cycleCount; i++) {
					if (wc->items[x][y] == cycle[i - 1]) {
						wc->items[x][y] = cycle[i];
						return;
					}
				}

				if (wc->items[x][y] == cycle[0])
					wc->items[x][y] = cycle[cycleCount - 1];
			}

			break;

	}
}


void wc_execute_effect(WorldContext *wc, DSLine *line) {
	// NOTE: PARAM* and GET_AND_CLAMP_[XY] are defined above

#define GET_AND_VALIDATE_X_Y(x_id, y_id) \
	x = PARAM_VALUE(x_id) / 2; \
	y = PARAM_VALUE(y_id); \
	if (wc_position_is_valid(wc, x, y) == 0) \
	return;

#define GET_AND_VALIDATE_WALL_X_Y(x_id, y_id) \
	x = PARAM_VALUE(x_id); \
	y = PARAM_VALUE(y_id); \
	if (wc_position_is_valid(wc, x / 2, y) == 0) \
	return;

	if (!NIL_P(line->annotation)) {
		ID an_type = SYM2ID(rb_hash_aref(line->annotation, ID2SYM(rb_intern("action"))));
		if (an_type == rb_intern("event")) {
			VALUE event_name = rb_hash_aref(line->annotation, ID2SYM(rb_intern("name")));
			VALUE info = rb_hash_new();

			rb_hash_aset(info, ID2SYM(rb_intern("name")), event_name);
			rb_funcall(wc->bot, rb_intern("dispatch_event"), 2, ID2SYM(rb_intern("ds_event")), info);
		}
	}

	// error: a label can only be part of a statement and a declaration is not a statement
	// gotta love C!
	int i, x, y, targetX, targetY, divisor, dividend, index, total, count, max, modifier;
	int x1, x2, y1, y2, width, height, totalArea, check, type, value, random;

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

		case 14: case 15:
			// Here's a fun little quirk: Thanks to 5:15, it's practically impossible to
			// accurately track a player around the dream using the DS and Triggers.
			//
			// The best option is to insert a bit of code at the end of the DS file which
			// sets off a trigger whenever someone speaks or moves, which should catch 99%
			// of cases and still not be too server-intensive.
			//
			// However... this doesn't work if some DS moves a player while he's not the
			// triggering furre. :/ I may just add Nelumbo-specific annotations to the DS
			// for this... unless I can think of a better method.
			//
			// The 5:15 issues actually affect the Furcadia client.
			// Here's a bit of DS: (5:15) 20,20 (3:3) 20,12 20,30 (5:1) 32 (4:9) (5:1) 40
			// Run it while a player is at 20,20 and it'll place floor 32 at 20,20, *NOT*
			// at the player's real location. Nice ghosting, right?
			//
			// This code is closely paired with (5:16) and (5:17). If it is modified,
			// please apply the changes to the other version as well.
			//
			targetX = PARAM_VALUE(0) / 2;
			targetY = PARAM_VALUE(1);
			if (wc_position_is_walkable(wc, targetX, targetY, wc->i_player)) {
				// TODO: make this into a direct C call?
				rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, wc->i_playerValue, INT2FIX(targetX*2), INT2FIX(targetY));
			} else {
			}

			if (line->type == 15)
				rb_warn("unsupported DS line (5:15) used");
			break;

		case 76:
			wc->i_player->heldObject = PARAM_VALUE(0);
			wc->i_heldObject = wc->i_player->heldObject;

		case 300:
			PARAM_VAR(0) = PARAM_VALUE(1);
			break;

		case 301:
			PARAM_VAR(1) = PARAM_VAR(0);
			if ((PARAM(1) & 1) == 0) {
				PARAM_VAR_Y(1) = PARAM_VAR_Y(0);
			}
			break;

		case 302:
			PARAM_VAR(0) += PARAM_VALUE(1);
			break;
		case 303:
			PARAM_VAR(0) += PARAM_VAR(1);
			break;

		case 304:
			PARAM_VAR(0) -= PARAM_VALUE(1);
			break;
		case 305:
			PARAM_VAR(0) -= PARAM_VAR(1);
			break;

		case 306:
			PARAM_VAR(0) *= PARAM_VALUE(1);
			break;
		case 307:
			PARAM_VAR(0) *= PARAM_VAR(1);
			break;

		case 308:
			dividend = PARAM_VAR(0);
			divisor = PARAM_VALUE(1);
			goto divide_vars;
		case 309:
			dividend = PARAM_VAR(0);
			divisor = PARAM_VAR(1);
divide_vars:
			if (divisor == 0) {
				PARAM_VAR(2) = dividend;
				PARAM_VAR(0) = 0; // need to check this
			} else {
				PARAM_VAR(2) = dividend % divisor;
				PARAM_VAR(0) = dividend / divisor;
			}
			break;

		case 310:
			index = PARAM(0) + (PARAM_VALUE(1) * 2);
			PARAM_VAR(2) = WC_VAR_SAFE(index);
			if ((PARAM(2) & 1) == 0) {
				PARAM_VAR_Y(2) = WC_VAR_SAFE(index+1);
			}
			break;

		case 311:
			index = PARAM(0) + (PARAM_VALUE(1) * 2);
			WC_VAR_SAFE(index) = PARAM_VALUE(2);
			WC_VAR_SAFE(PARAM(2)+1) = PARAM_VALUE_Y(2);
			break;

		case 312:
			modifier = PARAM_VALUE(3);
			goto do_dice_roll;
		case 313:
			modifier = 0 - PARAM_VALUE(3);
do_dice_roll:
			max = PARAM_VALUE(2);
			count = PARAM_VALUE(1);
			total = 0;
			for (i = 0; i < count; i++) {
				total += wc_random_number(wc, max) + 1;
			}
			PARAM_VAR(0) = total + modifier;
			break;

		case 314:
			PARAM_VAR(0) = wc->i_numberSaid;
			break;

		case 315:
			//TODO: I must assign this to i_player as soon as it's obtained
			PARAM_VAR(0) = wc->i_entryCode;
			break;
		case 316:
			wc->i_entryCode = PARAM_VALUE(0);
			wc->i_player->entryCode = wc->i_entryCode;
			break;

		case 317:
			PARAM_VAR(0) = wc->i_heldObject;
			break;

		case 318:
			PARAM_VAR(0) = wc->i_playersInDream;
			break;

		case 350:
			PARAM_VAR(0) = wc->i_movedFromX * 2;
			PARAM_VAR_Y(0) = wc->i_movedFromY;
			break;
		case 351:
			PARAM_VAR(0) = wc->i_movedToX * 2;
			PARAM_VAR_Y(0) = wc->i_movedToY;
			break;

		case 352:
			PARAM_VAR(0) *= 2;
			wc_move_position_ne_clamped(wc, &PARAM_VAR(0), &PARAM_VAR_Y(0), PARAM_VALUE(1));
			PARAM_VAR(0) /= 2;
			break;
		case 353:
			PARAM_VAR(0) *= 2;
			wc_move_position_se_clamped(wc, &PARAM_VAR(0), &PARAM_VAR_Y(0), PARAM_VALUE(1));
			PARAM_VAR(0) /= 2;
			break;
		case 354:
			PARAM_VAR(0) *= 2;
			wc_move_position_sw_clamped(wc, &PARAM_VAR(0), &PARAM_VAR_Y(0), PARAM_VALUE(1));
			PARAM_VAR(0) /= 2;
			break;
		case 355:
			PARAM_VAR(0) *= 2;
			wc_move_position_nw_clamped(wc, &PARAM_VAR(0), &PARAM_VAR_Y(0), PARAM_VALUE(1));
			PARAM_VAR(0) /= 2;
			break;

		case 380:
			GET_AND_VALIDATE_X_Y(1, 2);
			PARAM_VAR(0) = wc->floors[x][y];
			break;
		case 381:
			GET_AND_VALIDATE_X_Y(1, 2);
			PARAM_VAR(0) = wc->items[x][y];
			break;
		case 382:
			GET_AND_VALIDATE_WALL_X_Y(1, 2);
			PARAM_VAR(0) = wc->walls[x][y] % 12;
			break;
		case 383:
			GET_AND_VALIDATE_WALL_X_Y(1, 2);
			PARAM_VAR(0) = (wc->walls[x][y] / 12) + 1;
			break;

		case 384:
			PARAM_VAR(0) = PARAM_VALUE(0);
			PARAM_VAR_Y(0) = PARAM_VALUE(1);
			break;

		case 390:
			index = PARAM(2) + (PARAM_VALUE(0) * 2);
			count = PARAM_VALUE(1);
			value = PARAM_VALUE(3);
			for (i = 0; i < count; i++)
				WC_VAR_SAFE(index+i) = value;
			break;

		case 399:
			for (i = 0; i < MAX_VARIABLE; i++)
				wc->variables[i] = 0;
			break;

		case 500:
			GET_AND_CLAMP_X(x1, 1);
			GET_AND_CLAMP_Y(y1, 2);
			GET_AND_CLAMP_X(x2, 3);
			GET_AND_CLAMP_Y(y2, 4);
			goto generate_random_position_unchecked;
		case 501:
		case 511: case 521: case 531: case 541:
			x = (wc->i_didPlayerMove ? wc->i_movedToX : wc->i_triggerX);
			y = (wc->i_didPlayerMove ? wc->i_movedToY : wc->i_triggerY);
			wc_get_visibility_bounds(wc, x, y, &x1, &y1, &x2, &y2);

			if (line->type == 501)
				goto generate_random_position_unchecked;
			else
				goto generate_random_position_checked;
		case 502:
		case 512: case 522: case 532: case 542:
			// "somewhere in the dream" is really "somewhere not inside the
			// walking border"
			x1 = 4;
			y1 = 9;
			x2 = wc->mapWidth - 7;
			y2 = wc->mapHeight - 9;

			if (line->type == 502)
				goto generate_random_position_unchecked;
			else
				goto generate_random_position_checked;

generate_random_position_unchecked:
			width = (x2 - x1) + 1;
			height = (y2 - y1) + 1;
			totalArea = width * height;

			random = wc_random_number(wc, totalArea);
			PARAM_VAR(0) = (x1 + (random / height)) * 2;
			PARAM_VAR_Y(0) = y1 + (random % height);
			break;

		case 510: case 520: case 530: case 540:
			GET_AND_CLAMP_X(x1, 2);
			GET_AND_CLAMP_Y(y1, 3);
			GET_AND_CLAMP_X(x2, 4);
			GET_AND_CLAMP_Y(y2, 5);
			goto generate_random_position_checked;

generate_random_position_checked:
			type = (line->type / 10) % 10;
			check = PARAM_VALUE(1);
			totalArea = 0;

			for (x = x1; x <= x2; x++) {
				for (y = y1; y <= y2; y++) {
					if (type == 1 && wc->floors[x][y] != check) continue;
					if (type == 2 && wc->floors[x][y] == check) continue;
					if (type == 3 && wc->items[x][y] != check) continue;
					if (type == 4 && wc->items[x][y] == check) continue;
					totalArea++;
				}
			}

			if (totalArea == 0)
				return;

			random = wc_random_number(wc, totalArea);

			// this code is ugly
			index = 0;
			for (x = x1; x <= x2; x++) {
				for (y = y1; y <= y2; y++) {
					if (type == 1 && wc->floors[x][y] != check) continue;
					if (type == 2 && wc->floors[x][y] == check) continue;
					if (type == 3 && wc->items[x][y] != check) continue;
					if (type == 4 && wc->items[x][y] == check) continue;
					if (index == random) {
						PARAM_VAR(0) = x * 2;
						PARAM_VAR_Y(0) = y;
						return;
					}
					index++;
				}
			}
			break;
	}
}

