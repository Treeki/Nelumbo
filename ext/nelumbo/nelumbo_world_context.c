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
					count = decode_b95(&buf[offset], 3) + 1;
					varValue = (short)(decode_b95(&buf[offset+3], 3) & 0xFFFF);
					offset += 6;

					for (i = 0; i < count; i++) {
						wc->variables[varID] = varValue;
						varID++;
					}
				}
			}

			break;

		case '>': case '1': case '2': case '4': case '5': case 'E': case 'F':
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
					else if (buf[0] == '4')
						wc->regions[x][y] = number;
					else if (buf[0] == '5')
						wc->effects[x][y] = number;
					else if (buf[0] == 'E')
						wc->lighting[x][y] = number;
					else if (buf[0] == 'F')
						wc->ambience[x][y] = number;
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

			// Gather the info we can
			if (wc->i_player) {
				wc->i_player->entryCode = wc->i_entryCode;
				wc->i_player->heldObject = wc->i_heldObject;
				wc->i_player->cookies = wc->i_playerCookies;
			}

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

			wc->i_didPlayerMove = wc->i_origDidPlayerMove = decode_b95(&buf[1], 1);
			wc->i_randomSeed = decode_b95(&buf[2], 5);
			wc->i_numberSaid = decode_b95(&buf[7], 3);
			wc->i_facingDirection = decode_b95(&buf[10], 1);
			wc->i_entryCode = wc->i_origEntryCode = decode_b95(&buf[11], 3);
			wc->i_heldObject = wc->i_origHeldObject = decode_b95(&buf[14], 3);
			wc->i_playersInDream = decode_b95(&buf[17], 2);
			wc->i_userID = wc->i_originalUserID = decode_b95(&buf[19], 6);
			wc->i_dsButtonPressed = decode_b95(&buf[25], 2);
			wc->i_dreamCookies = decode_b95(&buf[27], 3);
			wc->i_playerCookies = decode_b95(&buf[30], 2);
			wc->i_lastPortalX = decode_b95(&buf[32], 2);
			wc->i_lastPortalY = decode_b95(&buf[34], 2);
			wc->i_second = buf[36];
			wc->i_minute = buf[37];
			wc->i_hour = buf[38];
			wc->i_day = buf[39];
			wc->i_month = buf[40];
			wc->i_year = decode_b95(&buf[41], 2);
			wc->i_lastDeadPortalX = decode_b95(&buf[43], 2);
			wc->i_lastDeadPortalY = decode_b95(&buf[45], 2);

			dsr_seed(&wc->i_randomGenerator, wc->i_randomSeed);

			if (wc->i_userID == 0) {
				wc->i_playerValue = wc->i_originalPlayerValue = Qnil;
				wc->i_player = wc->i_originalPlayer = 0;

				if (wc->dsDebug)
					printf("\nDS: Nobody              ");
			} else {
				if (wc->i_userID == wc->lastDeletedPlayerUID)
					wc->i_playerValue = wc->i_originalPlayerValue = wc->lastDeletedPlayer;
				else
					wc->i_playerValue = wc->i_originalPlayerValue =
						rb_hash_aref(wc->playersByUserID, INT2NUM(wc->i_userID));

				if (wc->i_playerValue == Qnil) {
					wc->i_player = wc->i_originalPlayer = 0;

					if (wc->dsDebug)
						printf("\nDS: Unknown %08d    ", wc->i_userID);
				} else {
					Data_Get_Struct(wc->i_playerValue, Player, wc->i_player);
					wc->i_originalPlayer = wc->i_player;

					if (wc->dsDebug)
						printf("\nDS: %20s", RSTRING_PTR(wc->i_player->name));
				}
			}

			break;
	}
}


void wc_load_map(WorldContext *wc, char *buf, int width, int height, char hasDataV29, char hasDataV30) {
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

	if (hasDataV29 || hasDataV30) {
		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				unsigned char firstByte = *(input++);
				wc->regions[x][y] = firstByte | (*(input++) << 8);
			}
		}
		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				unsigned char firstByte = *(input++);
				wc->effects[x][y] = firstByte | (*(input++) << 8);
			}
		}
	}

	if (hasDataV30) {
		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				unsigned char firstByte = *(input++);
				wc->lighting[x][y] = firstByte | (*(input++) << 8);
			}
		}
		for (x = 0; x < width; x++) {
			for (y = 0; y < height; y++) {
				unsigned char firstByte = *(input++);
				wc->ambience[x][y] = firstByte | (*(input++) << 8);
			}
		}
	}

	// TODO: Make this really work properly
	int i;
	for (i = 0; i < MAX_ITEM; i++)
		wc->itemWalkable[i] = 1;
	for (i = 0; i < MAX_FLOOR; i++)
		wc->floorWalkable[i] = 1;
}


void wc_save_map(WorldContext *wc, char *buf, char hasDataV29, char hasDataV30) {
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

	if (hasDataV29 || hasDataV30) {
		for (x = 0; x < wc->mapWidth; x++) {
			for (y = 0; y < wc->mapHeight; y++) {
				*(output++) = wc->regions[x][y] & 0xFF;
				*(output++) = wc->regions[x][y] >> 8;
			}
		}

		for (x = 0; x < wc->mapWidth; x++) {
			for (y = 0; y < wc->mapHeight; y++) {
				*(output++) = wc->effects[x][y] & 0xFF;
				*(output++) = wc->effects[x][y] >> 8;
			}
		}
	}

	if (hasDataV30) {
		for (x = 0; x < wc->mapWidth; x++) {
			for (y = 0; y < wc->mapHeight; y++) {
				*(output++) = wc->lighting[x][y] & 0xFF;
				*(output++) = wc->lighting[x][y] >> 8;
			}
		}

		for (x = 0; x < wc->mapWidth; x++) {
			for (y = 0; y < wc->mapHeight; y++) {
				*(output++) = wc->ambience[x][y] & 0xFF;
				*(output++) = wc->ambience[x][y] >> 8;
			}
		}
	}
}


char wc_has_data_v29(WorldContext *wc) {
	int x, y;

	for (x = 0; x < wc->mapWidth; x++)
		for (y = 0; y < wc->mapHeight; y++)
			if (wc->regions[x][y] != 0)
				return 1;
			else if (wc->effects[x][y] != 0)
				return 1;

	return 0;
}
char wc_has_data_v30(WorldContext *wc) {
	int x, y;

	for (x = 0; x < wc->mapWidth; x++)
		for (y = 0; y < wc->mapHeight; y++)
			if (wc->lighting[x][y] != 0)
				return 1;
			else if (wc->ambience[x][y] != 0)
				return 1;

	return 0;
}


void wc_setup_change_buffer(WorldContext *wc, ChangeBuffer *cb, char insnID) {
	cb->insnID = insnID;
	cb->end = 0;
}

void wc_append_to_change_buffer(WorldContext *wc, ChangeBuffer *cb, int x, int y, int number) {
	if (cb->end >= CHANGE_BUFFER_SIZE) {
		wc_flush_change_buffer(wc, cb);
	}

	encode_b220(x, &cb->buffer[cb->end], 2);
	encode_b220(y, &cb->buffer[cb->end+2], 2);
	encode_b220(number, &cb->buffer[cb->end+4], 2);
	cb->end += 6;
}

void wc_flush_change_buffer(WorldContext *wc, ChangeBuffer *cb) {
	if (cb->end == 0)
		return;

	VALUE str = rb_sprintf("edit %c", cb->insnID);
	rb_str_cat(str, cb->buffer, cb->end);
	rb_funcall(wc->bot, rb_intern("write_line"), 1, str);

	cb->end = 0;
}


char wc_position_is_walkable(WorldContext *wc, int x, int y, Player *player) {
	// TODO: Make this understand region flags sometime?
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

	//printf("Line: %d,%d %d,%d Diff %d,%d M %d A %d,%d\n", x1,y1,x2,y2,xDiff,yDiff,multiplier,(y1&1),(y2&2));

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


int wc_read_special(WorldContext *wc) {
	return wc->i_special[wc->i_specialIndex++];
}


uint32_t wc_random_number(WorldContext *wc, uint32_t max) {
	return dsr_generate(&wc->i_randomGenerator, max);
}


void wc_execute_trigger(WorldContext *wc, int number, int x, int y, char isSelf) {
	if (wc->dsDebug)
		printf(" [%4d @ %3d,%3d]", number, x, y);

	wc->i_triggerX = x;
	wc->i_triggerY = y;

	// Before we start, move the player if needed
	if (wc->i_player) {
		rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, wc->i_playerValue, INT2FIX(x*2), INT2FIX(y));
	}

	// Reset the DS engine
	wc->currentArea.category = 3;
	wc->currentArea.type = 1;
	wc->filterCount = 0;

	DSLine *currentLine = &wc->ds[number];

	wc_handle_annotation(wc, currentLine);

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


void wc_handle_annotation(WorldContext *wc, DSLine *line) {
	if (!NIL_P(line->annotation)) {
		ID an_type = SYM2ID(rb_hash_aref(line->annotation, ID2SYM(rb_intern("action"))));
		if (an_type == rb_intern("event")) {
			VALUE event_name = rb_hash_aref(line->annotation, ID2SYM(rb_intern("name")));
			VALUE params_list = rb_hash_aref(line->annotation, ID2SYM(rb_intern("params")));
			VALUE info = Qnil;
			if (RARRAY_LEN(params_list) > 0) {
				info = rb_hash_new();
				rb_hash_aset(info, ID2SYM(rb_intern("annotation_params")), params_list);
			}

			printf("Going to dispatch event [ %s ]\n", RSTRING_PTR(event_name));
			VALUE event_sym = ID2SYM(rb_intern2(RSTRING_PTR(event_name), RSTRING_LEN(event_name)));

			rb_funcall(wc->bot, rb_intern("dispatch_event"), 2, event_sym, info);
		}
	}
}


static void item_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_itemChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->items[x][y])};
		rb_proc_call_with_block(wc->cb_itemChanged, 3, args, Qnil);
	}
}

static void floor_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_floorChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->floors[x][y])};
		rb_proc_call_with_block(wc->cb_floorChanged, 3, args, Qnil);
	}
}

static void wall_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_wallChanged)) {
		VALUE args[3] = {INT2FIX(x), INT2FIX(y), INT2FIX(wc->walls[x][y])};
		rb_proc_call_with_block(wc->cb_wallChanged, 3, args, Qnil);
	}
}

static void region_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_regionChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->regions[x][y])};
		rb_proc_call_with_block(wc->cb_regionChanged, 3, args, Qnil);
	}
}

static void effect_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_effectChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->effects[x][y])};
		rb_proc_call_with_block(wc->cb_effectChanged, 3, args, Qnil);
	}
}

static void lighting_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_lightingChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->lighting[x][y])};
		rb_proc_call_with_block(wc->cb_lightingChanged, 3, args, Qnil);
	}
}

static void ambience_changed(WorldContext *wc, int x, int y) {
	if (!NIL_P(wc->cb_ambienceChanged)) {
		VALUE args[3] = {INT2FIX(x*2), INT2FIX(y), INT2FIX(wc->ambience[x][y])};
		rb_proc_call_with_block(wc->cb_ambienceChanged, 3, args, Qnil);
	}
}

static void held_object_changed(WorldContext *wc, VALUE player, int oldObj, int newObj) {
	if (!NIL_P(wc->cb_heldObjectChanged)) {
		VALUE args[3] = {player, INT2FIX(oldObj), INT2FIX(newObj)};
		rb_proc_call_with_block(wc->cb_heldObjectChanged, 3, args, Qnil);
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

#define GET_AND_CLAMP_WALL_X(var, id) \
	var = PARAM_VALUE(id); \
	if (var < 0) var = 0; \
	if (var >= wc->mapWidth) var = wc->mapWidth - 1;

#define GET_AND_CLAMP_Y(var, id) \
	var = PARAM_VALUE(id); \
	if (var < 0) var = 0; \
	if (var >= wc->mapHeight) var = wc->mapHeight - 1;

#define GET_TARGET_POSITION(xID, yID) \
	GET_AND_CLAMP_X(targetX, (xID)); \
	GET_AND_CLAMP_Y(targetY, (yID));

#define GET_WALL_TARGET_POSITION(xID, yID) \
	GET_AND_CLAMP_WALL_X(targetX, (xID)); \
	GET_AND_CLAMP_Y(targetY, (yID));


static int direction_behind[4] = {
	/* DIR_SW => */ DIR_NE,
	/* DIR_SE => */ DIR_NW,
	/* DIR_NW => */ DIR_SE,
	/* DIR_NE => */ DIR_SW,
};

static int direction_left[4] = {
	/* DIR_SW => */ DIR_SE,
	/* DIR_SE => */ DIR_NE,
	/* DIR_NW => */ DIR_SW,
	/* DIR_NE => */ DIR_NW,
};

static int direction_right[4] = {
	/* DIR_SW => */ DIR_NW,
	/* DIR_SE => */ DIR_SW,
	/* DIR_NW => */ DIR_NE,
	/* DIR_NE => */ DIR_SE,
};

void wc_set_area(WorldContext *wc, DSLine *line) {
	DSLine *bakedLine = &wc->currentArea;

	bakedLine->category = line->category;
	bakedLine->type = line->type;
	bakedLine->annotation = line->annotation;

	int paramCount = 0;

	switch (bakedLine->type) {
		case 11: case 13: case 21: case 23: case 30: case 31:
		case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
		case 511: case 512: case 521: case 522: case 531: case 532: case 541: case 542:
		case 551: case 552: case 561: case 562: case 571: case 572:
		case 581: case 582: case 591: case 592: case 601: case 602:
			paramCount = 1;
			break;

		case 2: case 9: case 32: case 33:
		case 60: case 61: case 62: case 63: case 64: case 65: case 66: case 67:
			paramCount = 2;
			break;

		case 14: case 15: case 16: case 17:
			paramCount = 3;
			break;

		case 3: case 4: case 500:
			paramCount = 4;
			break;

		case 510: case 520: case 530: case 540: case 550:
		case 560: case 570: case 580: case 590: case 600:
			paramCount = 5;
			break;
	}

	int i;
	for (i = 0; i < paramCount; i++)
		bakedLine->params[i] = PARAM_VALUE(i);
}


void wc_execute_on_area(WorldContext *wc, DSLine *line) {
	int x, y;
	short startX, startY, endX, endY, leftX, leftY, rightX, rightY, moveX, moveY;
	int top, bottom, steps, direction, region, low, high;

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
			while (!intersect_line_nwse(leftX, leftY, endX, endY)) {
				wc_move_position_sw(wc, &leftX, &leftY, 1);
			}

			rightX = startX;
			rightY = startY;
			while (!intersect_line_nesw(rightX, rightY, endX, endY)) {
				wc_move_position_se(wc, &rightX, &rightY, 1);
			}

			x = leftX;
			top = leftY;
			bottom = leftY;

			if (leftY & 1) {
				if (top > startY)
					top--;
				if (bottom < endY)
					bottom++;
			}

			while (x <= rightX) {
				for (y = top; y <= bottom; y++) {
					wc_execute_on_area_position(wc, line, x, y);
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
			startX = wc->currentArea.params[0] / 2;
			startY = wc->currentArea.params[1];
			endX = wc->currentArea.params[2] / 2;
			endY = wc->currentArea.params[3];

			for (x = startX; x <= endX; x++) {
				for (y = startY; y <= endY; y++) {
					wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 5:
			wc_execute_on_area_position(wc, line, wc->i_movedFromX, wc->i_movedFromY);
			break;
		case 6:
			wc_execute_on_area_position(wc, line, wc->i_movedToX, wc->i_movedToY);
			break;
		case 7:
			if (wc->i_didPlayerMove)
				wc_execute_on_area_position(wc, line, wc->i_movedToX, wc->i_movedToY);
			else
				wc_execute_on_area_position(wc, line, wc->i_movedFromX, wc->i_movedFromY);
			break;

		case 10:
			steps = 1;
			direction = wc->i_facingDirection;
			goto stepsInDirectionMovedTo;
		case 11:
			direction = wc->i_facingDirection;
			goto stepsInDirectionMovedTo_Param0;

		case 12:
			steps = 1;
			direction = direction_behind[wc->i_facingDirection];
			goto stepsInDirectionMovedTo;
		case 13:
			direction = direction_behind[wc->i_facingDirection];
			goto stepsInDirectionMovedTo_Param0;

		case 18:
			wc_execute_on_area_position(wc, line, wc->i_lastPortalX, wc->i_lastPortalY);
			break;

		case 20:
			steps = 1;
			direction = direction_left[wc->i_facingDirection];
			goto stepsInDirectionMovedTo;
		case 21:
			direction = direction_left[wc->i_facingDirection];
			goto stepsInDirectionMovedTo_Param0;

		case 22:
			steps = 1;
			direction = direction_right[wc->i_facingDirection];
			goto stepsInDirectionMovedTo;
		case 23:
			direction = direction_right[wc->i_facingDirection];
			goto stepsInDirectionMovedTo_Param0;

		case 30:
			region = wc->currentArea.params[0];
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					if (wc->regions[x][y] == region)
						wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 31:
			region = wc->currentArea.params[0];
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					if (wc->regions[x][y] != region)
						wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 32:
			low = wc->currentArea.params[0];
			high = wc->currentArea.params[1];
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					if (wc->regions[x][y] >= low && wc->regions[x][y] <= high)
						wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 33:
			low = wc->currentArea.params[0];
			high = wc->currentArea.params[1];
			for (x = 0; x < wc->mapWidth; x++) {
				for (y = 0; y < wc->mapHeight; y++) {
					if (wc->regions[x][y] < low || wc->regions[x][y] > high)
						wc_execute_on_area_position(wc, line, x, y);
				}
			}
			break;

		case 50:
			direction = DIR_NE;
			goto stepsInDirectionMovedFrom_Param0;
		case 51:
			direction = DIR_SE;
			goto stepsInDirectionMovedFrom_Param0;
		case 52:
			direction = DIR_SW;
			goto stepsInDirectionMovedFrom_Param0;
		case 53:
			direction = DIR_NW;
			goto stepsInDirectionMovedFrom_Param0;

		case 54:
			direction = DIR_NE;
			goto stepsInDirectionMovedTo_Param0;
		case 55:
			direction = DIR_SE;
			goto stepsInDirectionMovedTo_Param0;
		case 56:
			direction = DIR_SW;
			goto stepsInDirectionMovedTo_Param0;
		case 57:
			direction = DIR_NW;
			goto stepsInDirectionMovedTo_Param0;

stepsInDirectionMovedFrom_Param0:
			steps = wc->currentArea.params[0];
			goto stepsInDirectionMovedFrom;
stepsInDirectionMovedTo_Param0:
			steps = wc->currentArea.params[0];
			goto stepsInDirectionMovedTo;

stepsInDirectionMovedFrom:
			moveX = wc->i_movedFromX;
			moveY = wc->i_movedFromY;
			goto stepsInDirection;
stepsInDirectionMovedTo:
			moveX = wc->i_movedToX;
			moveY = wc->i_movedToY;
stepsInDirection:
			if (direction == DIR_SW)
				wc_move_position_sw_clamped(wc, &moveX, &moveY, steps);
			else if (direction == DIR_SE)
				wc_move_position_se_clamped(wc, &moveX, &moveY, steps);
			else if (direction == DIR_NW)
				wc_move_position_nw_clamped(wc, &moveX, &moveY, steps);
			else if (direction == DIR_NE)
				wc_move_position_ne_clamped(wc, &moveX, &moveY, steps);

			wc_execute_on_area_position(wc, line, moveX, moveY);
			break;

		case 60: case 61: case 62: case 63:
			moveX = wc->i_movedFromX;
			moveY = wc->i_movedFromY;
			goto doMoveTwoDirs;
		case 64: case 65: case 66: case 67:
			moveX = wc->i_movedToX;
			moveY = wc->i_movedToY;

doMoveTwoDirs:
			switch ((wc->currentArea.type - 60) % 4) {
				case 0:
					wc_move_position_ne_clamped(wc, &moveX, &moveY, wc->currentArea.params[0]);
					wc_move_position_nw_clamped(wc, &moveX, &moveY, wc->currentArea.params[1]);
					break;
				case 1:
					wc_move_position_ne_clamped(wc, &moveX, &moveY, wc->currentArea.params[0]);
					wc_move_position_se_clamped(wc, &moveX, &moveY, wc->currentArea.params[1]);
					break;
				case 2:
					wc_move_position_sw_clamped(wc, &moveX, &moveY, wc->currentArea.params[0]);
					wc_move_position_se_clamped(wc, &moveX, &moveY, wc->currentArea.params[1]);
					break;
				case 3:
					wc_move_position_sw_clamped(wc, &moveX, &moveY, wc->currentArea.params[0]);
					wc_move_position_nw_clamped(wc, &moveX, &moveY, wc->currentArea.params[1]);
					break;
			}

			wc_execute_on_area_position(wc, line, moveX, moveY);
			break;

		case 70:
			// Pretty sure this one is broken
			break;
		case 71:
			if (wc->i_lastDeadPortalX > 0 || wc->i_lastDeadPortalY > 0)
				wc_execute_on_area_position(wc, line, wc->i_lastDeadPortalX, wc->i_lastDeadPortalY);
			break;

		default:
			//TODO: Implement all the random spot areas
			printf("unknown area: %d\n", line->type);
	}
}


void wc_add_filter(WorldContext *wc, DSLine *line) {
	DSLine *bakedLine = &wc->filters[wc->filterCount];
	wc->filterCount++;

	bakedLine->category = line->category;
	bakedLine->type = line->type;
	bakedLine->annotation = line->annotation;

	switch (bakedLine->type) {
		case 1: case 2: case 3: case 4: case 30: case 31: case 40: case 41:
		case 42: case 43: case 44: case 45:
			bakedLine->params[0] = PARAM_VALUE(0);
			break;
		case 14: case 15: case 32: case 33:
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
			wall_changed(wc, x, y);
			break;

		case 45:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			shape = wc->walls[x][y] % 12;

			if (shape == one) {
				wc->walls[x][y] = (wc->walls[x][y] - one) + two;
				wall_changed(wc, x, y);
			} else if (shape == two) {
				wc->walls[x][y] = (wc->walls[x][y] - two) + one;
				wall_changed(wc, x, y);
			}
			break;

		case 46:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			texture = wc->walls[x][y] / 12;

			if (texture == one) {
				wc->walls[x][y] = (wc->walls[x][y] - (one * 12)) + (two * 12);
				wall_changed(wc, x, y);
			} else if (texture == two) {
				wc->walls[x][y] = (wc->walls[x][y] - (two * 12)) + (one * 12);
				wall_changed(wc, x, y);
			}
			break;

		case 47:
			one = (PARAM_VALUE(1) * 12) + PARAM_VALUE(0);
			two = (PARAM_VALUE(3) * 12) + PARAM_VALUE(2);

			if (wc->walls[x][y] == one) {
				wc->walls[x][y] = two;
				wall_changed(wc, x, y);
			} else if (wc->walls[x][y] == two) {
				wc->walls[x][y] = one;
				wall_changed(wc, x, y);
			}
			break;

		case 60:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);

			if ((wc->walls[x][y] % 12) == one) {
				wc->walls[x][y] = (wc->walls[x][y] - one) + two;
				wall_changed(wc, x, y);
			}
			break;

		case 61:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			texture = wc->walls[x][y] / 12;

			if (texture == one) {
				wc->walls[x][y] = (wc->walls[x][y] - (one * 12)) + (two * 12);
				wall_changed(wc, x, y);
			}
			break;

		case 62:
			one = (PARAM_VALUE(1) * 12) + PARAM_VALUE(0);
			two = (PARAM_VALUE(3) * 12) + PARAM_VALUE(2);

			if (wc->walls[x][y] == one) {
				wc->walls[x][y] = two;
				wall_changed(wc, x, y);
			}
			break;

		case 66:
			GET_WALL_TARGET_POSITION(0, 1);
			wc->walls[targetX][targetY] = wc->walls[x][y];
			wc->walls[x][y] = 0;
			wall_changed(wc, targetX, targetY);
			break;
		case 67:
			GET_WALL_TARGET_POSITION(0, 1);
			wc->walls[targetX][targetY] = wc->walls[x][y];
			wall_changed(wc, targetX, targetY);
			break;
		case 68:
			GET_WALL_TARGET_POSITION(0, 1);
			swap = wc->walls[x][y];
			wc->walls[x][y] = wc->walls[targetX][targetY];
			wc->walls[targetX][targetY] = swap;
			wall_changed(wc, x, y);
			wall_changed(wc, targetX, targetY);
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
				if (!position_is_visible_from(x, y, filter->params[0]/2, filter->params[1]))
					return;
				break;
			case 15:
				if (position_is_visible_from(x, y, filter->params[0]/2, filter->params[1]))
					return;
				break;
			case 30:
				if (wc->regions[x][y] != filter->params[0])
					return;
				break;
			case 31:
				if (wc->regions[x][y] == filter->params[0])
					return;
				break;
			case 32:
				if (wc->regions[x][y] < filter->params[0] || wc->regions[x][y] > filter->params[1])
					return;
				break;
			case 33:
				if (wc->regions[x][y] >= filter->params[0] && wc->regions[x][y] <= filter->params[1])
					return;
				break;
			case 40:
				if (wc->effects[x][y] != filter->params[0])
					return;
				break;
			case 41:
				if (wc->effects[x][y] == filter->params[0])
					return;
				break;
			case 42:
				if (wc->lighting[x][y] != filter->params[0])
					return;
				break;
			case 43:
				if (wc->lighting[x][y] == filter->params[0])
					return;
				break;
			case 44:
				if (wc->ambience[x][y] != filter->params[0])
					return;
				break;
			case 45:
				if (wc->ambience[x][y] == filter->params[0])
					return;
				break;

		}
	}


	int one, two, cycle[5], cycleCount, i;
	int distance, targetX, targetY, swap, shape, texture, value, old;
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
		case 60: case 61: case 62:
		case 66: case 67: case 68:
			wc_execute_on_wall(wc, line, x*2, y);
			wc_execute_on_wall(wc, line, x*2+1, y);
			break;

		case 1:
			wc->floors[x][y] = PARAM_VALUE(0);
			floor_changed(wc, x, y);
			break;
		case 2:
			if (wc->floors[x][y] == PARAM_VALUE(0)) {
				wc->floors[x][y] = PARAM_VALUE(1);
				floor_changed(wc, x, y);
			}
			break;
		case 3:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->floors[x][y] == one) {
				wc->floors[x][y] = two;
				floor_changed(wc, x, y);
			} else if (wc->floors[x][y] == two) {
				wc->floors[x][y] = one;
				floor_changed(wc, x, y);
			}
			break;

		case 4:
			wc->items[x][y] = PARAM_VALUE(0);
			item_changed(wc, x, y);
			break;
		case 5:
			if (wc->items[x][y] == PARAM_VALUE(0)) {
				wc->items[x][y] = PARAM_VALUE(1);
				item_changed(wc, x, y);
			}
			break;
		case 6:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->items[x][y] == one) {
				wc->items[x][y] = two;
				item_changed(wc, x, y);
			} else if (wc->items[x][y] == two) {
				wc->items[x][y] = one;
				item_changed(wc, x, y);
			}
			break;

		case 7:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			wc->floors[x][y] = one + wc_random_number(wc, two - one + 1);
			floor_changed(wc, x, y);
			break;
		case 13:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			wc->items[x][y] = one + wc_random_number(wc, two - one + 1);
			item_changed(wc, x, y);
			break;
		case 230:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			wc->effects[x][y] = one + wc_random_number(wc, two - one + 1);
			effect_changed(wc, x, y);
			break;

		case 16: case 17: case 716: case 717:
			// This code is closely paired with (5:14) and (5:15). If it is modified,
			// please apply the changes to the other version as well.
			//
			// This version may not even work. Needs testing.
			//
			GET_AFFECTED_PLAYER;

			GET_TARGET_POSITION(0, 1);
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
			// Awesome, another line we can't implement properly due to missing info.
			GET_AFFECTED_PLAYER;
			distance = PARAM_VALUE(0);

			if (distance > 0) {
				if (sAffectedPlayer == wc->i_player) {
					moveX = x;
					moveY = y;
					if (wc->i_facingDirection == DIR_SW)
						wc_move_position_sw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
					else if (wc->i_facingDirection == DIR_SE)
						wc_move_position_se_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
					else if (wc->i_facingDirection == DIR_NW)
						wc_move_position_nw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
					else if (wc->i_facingDirection == DIR_NE)
						wc_move_position_ne_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));

					rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, affectedPlayer, INT2FIX(moveX*2), INT2FIX(moveY));
				} else {
					rb_warn("unsupported DS line (5:19) or (5:719) used on someone who is not the triggering furre");
				}
			}
			break;

		case 20:
		case 231:
			moveX = x;
			moveY = y;
			if (wc->i_facingDirection == DIR_SW)
				wc_move_position_sw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == DIR_SE)
				wc_move_position_se_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == DIR_NW)
				wc_move_position_nw_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));
			else if (wc->i_facingDirection == DIR_NE)
				wc_move_position_ne_clamped(wc, &moveX, &moveY, PARAM_VALUE(0));

			if (line->type == 20) {
				wc->items[moveX][moveY] = wc->items[x][y];
				wc->items[x][y] = 0;
				item_changed(wc, x, y);
				item_changed(wc, moveX, moveY);
			} else {
				wc->effects[moveX][moveY] = wc->effects[x][y];
				wc->effects[x][y] = 0;
				effect_changed(wc, x, y);
				effect_changed(wc, moveX, moveY);
			}
			break;

		case 21:
			GET_TARGET_POSITION(0, 1);
			wc->items[targetX][targetY] = wc->items[x][y];
			wc->items[x][y] = 0;
			item_changed(wc, x, y);
			item_changed(wc, targetX, targetY);
			break;
		case 22:
			GET_TARGET_POSITION(0, 1);
			wc->items[targetX][targetY] = wc->items[x][y];
			item_changed(wc, targetX, targetY);
			break;
		case 23:
			GET_TARGET_POSITION(0, 1);
			swap = wc->items[x][y];
			wc->items[x][y] = wc->items[targetX][targetY];
			wc->items[targetX][targetY] = swap;
			item_changed(wc, x, y);
			item_changed(wc, targetX, targetY);
			break;

		case 24:
			GET_TARGET_POSITION(0, 1);
			wc->floors[targetX][targetY] = wc->floors[x][y];
			floor_changed(wc, x, y);
			break;
		case 25:
			GET_TARGET_POSITION(0, 1);
			swap = wc->floors[x][y];
			wc->floors[x][y] = wc->floors[targetX][targetY];
			wc->floors[targetX][targetY] = swap;
			floor_changed(wc, x, y);
			floor_changed(wc, targetX, targetY);
			break;

		case 26:
			wc->items[x][y] += PARAM_VALUE(0);
			item_changed(wc, x, y);
			break;
		case 27:
			wc->items[x][y] -= PARAM_VALUE(0);
			item_changed(wc, x, y);
			break;
		case 28:
			wc->floors[x][y] += PARAM_VALUE(0);
			floor_changed(wc, x, y);
			break;
		case 29:
			wc->floors[x][y] -= PARAM_VALUE(0);
			floor_changed(wc, x, y);
			break;

		case 77:
			GET_AFFECTED_PLAYER;

			value = PARAM_VALUE(0);
			old = sAffectedPlayer->heldObject;

			sAffectedPlayer->heldObject = value;
			held_object_changed(wc, affectedPlayer, old, value);

			if (sAffectedPlayer == wc->i_player) {
				wc->i_heldObject = sAffectedPlayer->heldObject;
			}

			if (sAffectedPlayer == wc->i_originalPlayer) {
				wc->i_origHeldObject = sAffectedPlayer->heldObject;
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

		case 120:
			wc->regions[x][y] = PARAM_VALUE(0);
			region_changed(wc, x, y);
			break;

		case 150:
			wc->effects[x][y] = PARAM_VALUE(0);
			effect_changed(wc, x, y);
			break;
		case 153:
			if (wc->effects[x][y] == PARAM_VALUE(0)) {
				wc->effects[x][y] = PARAM_VALUE(1);
				effect_changed(wc, x, y);
			}
			break;
		case 154:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->effects[x][y] == one) {
				wc->effects[x][y] = two;
				effect_changed(wc, x, y);
			} else if (wc->effects[x][y] == two) {
				wc->effects[x][y] = one;
				effect_changed(wc, x, y);
			}
			break;

		case 155:
			wc->lighting[x][y] = PARAM_VALUE(0);
			lighting_changed(wc, x, y);
			break;
		case 158:
			if (wc->lighting[x][y] == PARAM_VALUE(0)) {
				wc->lighting[x][y] = PARAM_VALUE(1);
				lighting_changed(wc, x, y);
			}
			break;
		case 159:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->lighting[x][y] == one) {
				wc->lighting[x][y] = two;
				lighting_changed(wc, x, y);
			} else if (wc->lighting[x][y] == two) {
				wc->lighting[x][y] = one;
				lighting_changed(wc, x, y);
			}
			break;

		case 225:
			wc->ambience[x][y] = PARAM_VALUE(0);
			ambience_changed(wc, x, y);
			break;
		case 228:
			if (wc->ambience[x][y] == PARAM_VALUE(0)) {
				wc->ambience[x][y] = PARAM_VALUE(1);
				ambience_changed(wc, x, y);
			}
			break;
		case 229:
			one = PARAM_VALUE(0);
			two = PARAM_VALUE(1);
			if (wc->ambience[x][y] == one) {
				wc->ambience[x][y] = two;
				ambience_changed(wc, x, y);
			} else if (wc->ambience[x][y] == two) {
				wc->ambience[x][y] = one;
				ambience_changed(wc, x, y);
			}
			break;

		case 232:
			GET_TARGET_POSITION(0, 1);
			wc->effects[targetX][targetY] = wc->effects[x][y];
			wc->effects[x][y] = 0;
			effect_changed(wc, x, y);
			effect_changed(wc, targetX, targetY);
			break;
		case 233:
			GET_TARGET_POSITION(0, 1);
			wc->effects[targetX][targetY] = wc->effects[x][y];
			effect_changed(wc, targetX, targetY);
			break;
		case 234:
			GET_TARGET_POSITION(0, 1);
			swap = wc->effects[x][y];
			wc->effects[x][y] = wc->effects[targetX][targetY];
			wc->effects[targetX][targetY] = swap;
			effect_changed(wc, x, y);
			effect_changed(wc, targetX, targetY);
			break;

		case 400: case 405: case 410: case 415:
			cycleCount = 3;
			goto do_cycle;
		case 401: case 406: case 411: case 416:
			cycleCount = 4;
			goto do_cycle;
		case 402: case 407: case 412: case 417:
			cycleCount = 5;
do_cycle:
			for (i = 0; i < cycleCount; i++)
				cycle[i] = PARAM_VALUE(i);

			if (line->type >= 415) {
				for (i = 1; i < cycleCount; i++) {
					if (wc->ambience[x][y] == cycle[i - 1]) {
						wc->ambience[x][y] = cycle[i];
						ambience_changed(wc, x, y);
						return;
					}
				}

				if (wc->ambience[x][y] == cycle[0]) {
					wc->ambience[x][y] = cycle[cycleCount - 1];
					ambience_changed(wc, x, y);
				}
			} else if (line->type >= 410) {
				for (i = 1; i < cycleCount; i++) {
					if (wc->floors[x][y] == cycle[i - 1]) {
						wc->floors[x][y] = cycle[i];
						floor_changed(wc, x, y);
						return;
					}
				}

				if (wc->floors[x][y] == cycle[0]) {
					wc->floors[x][y] = cycle[cycleCount - 1];
					floor_changed(wc, x, y);
				}
			} else if (line->type >= 405) {
				for (i = 1; i < cycleCount; i++) {
					if (wc->lighting[x][y] == cycle[i - 1]) {
						wc->lighting[x][y] = cycle[i];
						lighting_changed(wc, x, y);
						return;
					}
				}

				if (wc->lighting[x][y] == cycle[0]) {
					wc->lighting[x][y] = cycle[cycleCount - 1];
					lighting_changed(wc, x, y);
				}
			} else {
				for (i = 1; i < cycleCount; i++) {
					if (wc->items[x][y] == cycle[i - 1]) {
						wc->items[x][y] = cycle[i];
						item_changed(wc, x, y);
						return;
					}
				}

				if (wc->items[x][y] == cycle[0]) {
					wc->items[x][y] = cycle[cycleCount - 1];
					item_changed(wc, x, y);
				}
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

#define REQUIRE_PLAYER \
	if (!wc->i_player) { \
		printf("WARNING: DS executed for %d, but they weren't around!\n", wc->i_userID); \
		return; \
	}

	wc_handle_annotation(wc, line);

	// error: a label can only be part of a statement and a declaration is not a statement
	// gotta love C!
	int i, x, y, targetX, targetY, divisor, dividend, index, total, count, max, modifier;
	int x1, x2, y1, y2, width, height, totalArea, check, type, value, random, old;
	int shape, texture;

	unsigned int uid;
	VALUE newPlayer;
	Player *sNewPlayer;

	/* UNIMPLEMENTED v30/v31 LINES:
	 * 48,722 move triggering furre to random location in region #
	 * 49,723 move any furre present to random location in region #
	 * 148,720 move triggering furre to #,# or nearby in same region
	 * 149,721 move any furre present to #,# or nearby in same region
	 * 157,227 place lighting/ambience # at triggering furre pos
	 * 460..463 move any item present # steps NE/SE/SW/NW
	 * 470..473 move any effect present # steps NE/SE/SW/NW
	 * 1251 set var # to colour of trig furre's remap #
	 *
	 * there might be more stuff from previous updates that was
	 * left out
	 */

	switch (line->type) {
		/* Those commented out are not relevant to us. */
		case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			/*case 9:*/
		case 13:
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
			/*case 115:*/
		case 120:
		case 150: case 153: case 154:
		case 155: case 158: case 159:
		case 225: case 228: case 229:
		case 230: case 231: case 232: case 233: case 234:
			/*case 201:*/
		case 400: case 401: case 402: case 405: case 406: case 407:
		case 410: case 411: case 412: case 415: case 416: case 417:
			/*case 421, 423:*/
			/*case 701, 703:*/
		case 716: case 717: case 719: case 780: case 781: case 782: case 783:
		case 784: case 785: case 786: case 787:
			wc_execute_on_area(wc, line);
			break;

			/* Stuff we don't need in our DS engine. */
			/* Sounds / Music: */
		case 8: case 9: case 10: case 11: case 12:
		case 30: case 31: case 32: case 33: case 34:
			/* Countdown Timers (Purely Server Side): */
		case 50:
			/* Dream Control (Purely Server Side): */
		case 52: case 53: case 54: case 55:
		case 78: case 102: case 103:
		case 104: case 105:
		case 106: case 107: case 108: case 109: case 110: case 111:
		case 112: case 113: case 114:
			/* Region Display Control (Client Only): */
		case 122: case 123: case 124: case 125: case 127: case 128:
		case 130: case 131: case 132: case 133:
		case 134: case 135: case 136: case 137: case 138: case 139:
		case 140: case 141: case 142: case 143:
		case 144: case 145: case 146: case 147:
		case 170: case 171: case 172: case 173:
		case 174: case 175: case 176: case 177:
		case 178: case 179:
			/* Region Behaviour Control (Ignored Right Now): */
		case 160: case 161: case 162: case 163: case 164: case 165: case 166: case 167:
		case 168: case 169: case 194: case 195: case 196: case 197: case 198: case 199:
			/* Poses (Ignored Right Now): */
		case 70: case 71: case 72: case 73: case 74: case 75:
		case 88: case 89: case 90: case 91: case 92: case 93:
		case 94: case 95: case 96: case 97: case 98: case 99:
			/* Classic Mode: */
		case 100: case 101: case 1000:
			/* DS Buttons: */
		case 180: case 181: case 182: case 183:
		case 190: case 191: case 192: case 193:
			/* Emits / Speech: */
		case 200: case 201: case 202: case 203: case 204: case 205:
		case 210: case 212: case 213: case 214: case 215:
			/* String Variables: */
		case 250: case 251: case 252: case 253: case 254: case 255:
		case 256: case 257: case 258:
		case 270: case 271: case 272: case 273: case 274:
		case 275: case 276: case 277: case 278:
			/* Map Shaking: */
		case 420: case 421: case 422: case 423:
			/* KitterSpeak: */
		case 430: case 431: case 432: case 433:
		case 434: case 435: case 436: case 437:
		case 438: case 439: case 440: case 441:
		case 442: case 443: case 444:
		case 445: case 446: case 447:
		case 448: case 449: case 450:
		case 451: case 452: case 453:
			/* PS Memorise: */
		case 600: case 601: case 602: case 603: case 604: case 605:
			/* PS Strings: */
		case 613: case 614: case 615:
			/* PS Forget and Control: */
		case 630: case 631: case 632: case 633: case 634: case 635:
		case 650: case 651: case 880:
			/* Cookies: */
		case 700: case 701: case 702: case 703:
		case 706: case 707: case 708: case 709: case 710:
			/* Dialog Boxes: */
		case 900: case 901: case 910: case 911:
			/* Localspecies: */
		case 1200: case 1201: case 1202: case 1203:
		case 1205: case 1206: case 1207:
			/* Remap Changes: */
		case 1250:
			/* Crash the Tribble: */
		case 2000:
			break;

		case 14: case 15: case 714: case 715:
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
			REQUIRE_PLAYER;
			GET_TARGET_POSITION(0, 1);
			if (wc_position_is_walkable(wc, targetX, targetY, wc->i_player)) {
				// TODO: make this into a direct C call?
				rb_funcall(wc->bot, rb_intern("move_tracked_player"), 3, wc->i_playerValue, INT2FIX(targetX*2), INT2FIX(targetY));
			} else {
			}

			if (line->type == 15)
				rb_warn("unsupported DS line (5:15) used");
			if (line->type == 715)
				rb_warn("unsupported DS line (5:715) used");
			break;

		case 40:
			GET_TARGET_POSITION(1, 2);
			wc->floors[targetX][targetY] = PARAM_VALUE(0);
			floor_changed(wc, targetX, targetY);
			break;
		case 41:
			GET_TARGET_POSITION(1, 2);
			wc->items[targetX][targetY] = PARAM_VALUE(0);
			item_changed(wc, targetX, targetY);
			break;

		case 51:
			GET_AND_VALIDATE_X_Y(0, 1);
			newPlayer = rb_hash_aref(wc->playersByPosition, PLAYER_KEY(x, y));
			goto changeTF;

		case 56:
			uid = wc_read_special(wc);
			uid += (wc_read_special(wc) * 10000);
			newPlayer = rb_hash_aref(wc->playersByUserID, INT2NUM(uid));
			goto changeTF;

changeTF:
			if (!NIL_P(newPlayer)) {
				Data_Get_Struct(newPlayer, Player, sNewPlayer);

				wc->i_userID = sNewPlayer->uid;
				wc->i_player = sNewPlayer;
				wc->i_playerValue = newPlayer;

				if (sNewPlayer->uid == wc->i_originalUserID) {
					wc->i_heldObject = wc->i_origHeldObject;
					wc->i_entryCode = wc->i_origEntryCode;
					wc->i_didPlayerMove = wc->i_origDidPlayerMove;
				} else {
					wc->i_heldObject = sNewPlayer->heldObject;
					wc->i_entryCode = sNewPlayer->entryCode;
					wc->i_didPlayerMove = 0;
				}
			}

			break;

		case 63:
			GET_WALL_TARGET_POSITION(1, 2);
			shape = PARAM_VALUE(0);
			texture = wc->walls[targetX][targetY] / 12;
			goto write_pwall;
		case 64:
			GET_WALL_TARGET_POSITION(1, 2);
			shape = wc->walls[targetX][targetY] % 12;
			texture = PARAM_VALUE(0);
			goto write_pwall;
		case 65:
			GET_WALL_TARGET_POSITION(2, 3);
			shape = PARAM_VALUE(0);
			texture = PARAM_VALUE(1);
write_pwall:
			wc->walls[targetX][targetY] = (texture * 12) + shape;
			wall_changed(wc, targetX, targetY);
			break;

		case 76:
			REQUIRE_PLAYER;
			value = PARAM_VALUE(0);
			old = wc->i_player->heldObject;
			wc->i_player->heldObject = value;
			wc->i_heldObject = wc->i_player->heldObject;
			held_object_changed(wc, wc->i_playerValue, old, value);

			if (wc->i_player = wc->i_originalPlayer) {
				wc->i_origHeldObject = wc->i_heldObject;
			}

			break;

		case 121:
			GET_TARGET_POSITION(0, 1);
			wc->regions[targetX][targetY] = PARAM_VALUE(2);
			region_changed(wc, targetX, targetY);
			break;

		case 151:
			GET_TARGET_POSITION(0, 1);
			wc->effects[targetX][targetY] = PARAM_VALUE(2);
			effect_changed(wc, targetX, targetY);
			break;

		case 156:
			GET_TARGET_POSITION(0, 1);
			wc->lighting[targetX][targetY] = PARAM_VALUE(2);
			lighting_changed(wc, targetX, targetY);
			break;

		case 226:
			GET_TARGET_POSITION(0, 1);
			wc->ambience[targetX][targetY] = PARAM_VALUE(2);
			ambience_changed(wc, targetX, targetY);
			break;

		case 184:
			// Bugged in Furc currently!
			PARAM_VAR(0) = wc->i_dsButtonPressed;
			break;

		case 280:
		case 281:
		case 282:
		case 283:
		case 284:
		case 327:
			PARAM_VAR(0) = wc_read_special(wc);
			break;

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
			PARAM_VAR(0) = wc->i_entryCode;
			break;
		case 316:
			wc->i_entryCode = PARAM_VALUE(0);
			REQUIRE_PLAYER;
			wc->i_player->entryCode = wc->i_entryCode;
			break;

		case 317:
			PARAM_VAR(0) = wc->i_heldObject;
			break;

		case 318:
			PARAM_VAR(0) = wc->i_playersInDream;
			break;

		case 319:
			PARAM_VAR(0) = wc->i_lastPortalX * 2;
			PARAM_VAR_Y(0) = wc->i_lastPortalY;
			break;

		case 320:
			GET_TARGET_POSITION(1, 2);
			PARAM_VAR(0) = wc->regions[targetX][targetY];
			break;

		case 321:
			PARAM_VAR(0) = wc->i_day;
			break;
		case 322:
			PARAM_VAR(0) = wc->i_hour;
			break;
		case 323:
			PARAM_VAR(0) = wc->i_minute;
			break;
		case 324:
			PARAM_VAR(0) = wc->i_month;
			break;
		case 325:
			PARAM_VAR(0) = wc->i_second;
			break;
		case 326:
			PARAM_VAR(0) = wc->i_year;
			break;

		case 330:
			GET_TARGET_POSITION(1, 2);
			PARAM_VAR(0) = wc->effects[targetX][targetY];
			break;
		case 331:
			GET_TARGET_POSITION(1, 2);
			PARAM_VAR(0) = wc->lighting[targetX][targetY];
			break;
		case 332:
			GET_TARGET_POSITION(1, 2);
			PARAM_VAR(0) = wc->ambience[targetX][targetY];
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
			PARAM_VAR(0) = PARAM_VALUE(1);
			PARAM_VAR_Y(0) = PARAM_VALUE(2);
			break;

		case 390:
			index = PARAM(2) + PARAM_VALUE(0);
			count = PARAM_VALUE(1);
			value = PARAM_VALUE(3);
			for (i = 0; i < count; i++)
				WC_VAR_SAFE(index+(i*2)) = value;
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

		case 610:
		case 612:
			index = PARAM(1);
			goto rememberPSValue;
		case 611:
			index = PARAM(2);
rememberPSValue:
			WC_VAR_SAFE(index) = wc_read_special(wc);
			if ((index & 1) == 0)
				WC_VAR_SAFE(index+1) = wc_read_special(wc);
			break;

		case 704:
			PARAM_VAR(0) = wc->i_dreamCookies;
			break;
		case 705:
			PARAM_VAR(0) = wc->i_playerCookies;
			break;

		default:
			printf("unknown effect: %d\n", line->type);
	}
}
