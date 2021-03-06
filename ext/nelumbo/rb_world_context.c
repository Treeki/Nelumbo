#include <nelumbo.h>

VALUE cNelumboWorldContext;

#define GET_WC \
	WorldContext *wc; \
	Data_Get_Struct(self, WorldContext, wc)

static void fail_if_out_of_bounds(WorldContext *wc, int x, int y) {
	if (x < 0 || y < 0 || x >= wc->mapWidth || y >= wc->mapHeight)
		rb_raise(rb_eArgError, "specified position out of map boundaries");
}


/*******************************************************************************
 * Player Management
 ******************************************************************************/
static VALUE create_and_add_player(VALUE self, VALUE uid, VALUE name) {
	// uid might not be a fixnum so we don't check it
	// NUM2INT takes care of raising an exception for us
	Check_Type(name, T_STRING);

	GET_WC;

	VALUE initArgs[2];
	initArgs[0] = uid;
	initArgs[1] = name;

	VALUE player = rb_class_new_instance(2, initArgs, cNelumboWorldPlayer);
	Player *sPlayer;
	Data_Get_Struct(player, Player, sPlayer);

	rb_ary_push(wc->playerList, player);
	rb_hash_aset(wc->playersByShortname, sPlayer->shortname, player);
	rb_hash_aset(wc->playersByUserID, INT2NUM(sPlayer->uid), player);

	if (sPlayer->uid == wc->lastDeletedPlayerUID) {
		wc->lastDeletedPlayer = player;
	}

	return player;
}

static VALUE delete_and_remove_player(VALUE self, VALUE player) {
	GET_WC;

	Player *sPlayer;
	Data_Get_Struct(player, Player, sPlayer);

	rb_ary_delete(wc->playerList, player);
	rb_hash_delete(wc->playersByShortname, sPlayer->shortname);
	rb_hash_delete(wc->playersByUserID, INT2NUM(sPlayer->uid));
	rb_hash_delete(wc->playersByPosition, PLAYER_KEY(sPlayer->x, sPlayer->y));

	wc->lastDeletedPlayer = player;
	wc->lastDeletedPlayerUID = sPlayer->uid;

	return player;
}

static VALUE find_player_by_name(VALUE self, VALUE name) {
	Check_Type(name, T_STRING);

	GET_WC;

	VALUE shortname = rb_funcall(name, rb_intern("to_shortname"), 0);
	return rb_hash_aref(wc->playersByShortname, shortname);
}

static VALUE find_player_by_uid(VALUE self, VALUE uid) {
	GET_WC;

	return rb_hash_aref(wc->playersByUserID, uid);
}

static VALUE find_player_at_position(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	return rb_hash_aref(wc->playersByPosition, PLAYER_KEY(FIX2INT(x) / 2, FIX2INT(y)));
}

static VALUE each_player(VALUE self) {
	if (rb_block_given_p()) {
		GET_WC;

		int i, length = RARRAY_LEN(wc->playerList);
		for (i = 0; i < length; i++) {
			rb_yield(RARRAY_PTR(wc->playerList)[i]);
		}

		return Qnil;
	} else {
		return rb_funcall(self, rb_intern("to_enum"), 1, ID2SYM(rb_intern("each_player")));
	}
}

static VALUE move_tracked_player(VALUE self, VALUE player, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	Player *sPlayer;
	Data_Get_Struct(player, Player, sPlayer);

	int rX = FIX2INT(x) / 2, rY = FIX2INT(y);

	if (sPlayer->x == rX && sPlayer->y == rY)
		return Qnil;

	int oldX = sPlayer->x, oldY = sPlayer->y;
	if (oldX != -1) {
		rb_hash_delete(wc->playersByPosition, PLAYER_KEY(oldX, oldY));
	}

	sPlayer->x = rX;
	sPlayer->y = rY;

	rb_hash_aset(wc->playersByPosition, PLAYER_KEY(rX, rY), player);

	if (oldX == -1) {
		oldX = 0;
		oldY = 0;
	}

	VALUE info = rb_hash_new();
	rb_hash_aset(info, ID2SYM(rb_intern("player")), player);
	rb_hash_aset(info, ID2SYM(rb_intern("from_x")), INT2FIX(oldX*2));
	rb_hash_aset(info, ID2SYM(rb_intern("from_y")), INT2FIX(oldY));
	rb_hash_aset(info, ID2SYM(rb_intern("to_x")), INT2FIX(rX*2));
	rb_hash_aset(info, ID2SYM(rb_intern("to_y")), INT2FIX(rY));

	rb_funcall(wc->bot, rb_intern("dispatch_event"), 2, ID2SYM(rb_intern("player_move")), info);

	return Qnil;
}

/*******************************************************************************
 * Map Data Access
 ******************************************************************************/
static VALUE get_width(VALUE self) {
	GET_WC;
	return INT2FIX(wc->mapWidth);
}

static VALUE get_height(VALUE self) {
	GET_WC;
	return INT2FIX(wc->mapHeight);
}

static VALUE get_item(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->items[realX][realY]);
}

static VALUE set_item(VALUE self, VALUE x, VALUE y, VALUE item) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(item, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realItem = FIX2INT(item);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->items[realX][realY] != realItem) {
		wc->items[realX][realY] = realItem;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->itemChangeBuffer, realX, realY, realItem);
	}

	return item;
}

static VALUE get_floor(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->floors[realX][realY]);
}

static VALUE set_floor(VALUE self, VALUE x, VALUE y, VALUE floor) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(floor, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realFloor = FIX2INT(floor);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->floors[realX][realY] != realFloor) {
		wc->floors[realX][realY] = realFloor;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->floorChangeBuffer, realX, realY, realFloor);
	}

	return floor;
}

static VALUE get_wall(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x), realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX / 2, realY);

	return INT2FIX(wc->walls[realX][realY]);
}

static VALUE set_wall(VALUE self, VALUE x, VALUE y, VALUE wall) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(wall, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x), realY = FIX2INT(y), realWall = FIX2INT(wall);
	fail_if_out_of_bounds(wc, realX / 2, realY);

	if (wc->walls[realX][realY] != realWall) {
		wc->walls[realX][realY] = realWall;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->wallChangeBuffer, realX, realY, realWall);
	}

	return wall;
}

static VALUE get_region(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->regions[realX][realY]);
}

static VALUE set_region(VALUE self, VALUE x, VALUE y, VALUE region) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(region, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realregion = FIX2INT(region);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->regions[realX][realY] != realregion) {
		wc->regions[realX][realY] = realregion;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->regionChangeBuffer, realX, realY, realregion);
	}

	return region;
}

static VALUE get_effect(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->effects[realX][realY]);
}

static VALUE set_effect(VALUE self, VALUE x, VALUE y, VALUE effect) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(effect, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realeffect = FIX2INT(effect);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->effects[realX][realY] != realeffect) {
		wc->effects[realX][realY] = realeffect;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->effectChangeBuffer, realX, realY, realeffect);
	}

	return effect;
}

static VALUE get_lighting(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->lighting[realX][realY]);
}

static VALUE set_lighting(VALUE self, VALUE x, VALUE y, VALUE lighting) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(lighting, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realLighting = FIX2INT(lighting);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->lighting[realX][realY] != realLighting) {
		wc->lighting[realX][realY] = realLighting;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->lightingChangeBuffer, realX, realY, realLighting);
	}

	return lighting;
}

static VALUE get_ambience(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y);
	fail_if_out_of_bounds(wc, realX, realY);

	return INT2FIX(wc->ambience[realX][realY]);
}

static VALUE set_ambience(VALUE self, VALUE x, VALUE y, VALUE ambience) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(ambience, T_FIXNUM);

	GET_WC;

	int realX = FIX2INT(x) / 2, realY = FIX2INT(y), realAmbience = FIX2INT(ambience);
	fail_if_out_of_bounds(wc, realX, realY);

	if (wc->ambience[realX][realY] != realAmbience) {
		wc->ambience[realX][realY] = realAmbience;

		if (wc->isLoggingMapChanges)
			wc_append_to_change_buffer(wc, &wc->ambienceChangeBuffer, realX, realY, realAmbience);
	}

	return ambience;
}

/*******************************************************************************
 * DS Trigger Parameter Access
 ******************************************************************************/
static VALUE get_ds_debug(VALUE self) { GET_WC; return (wc->dsDebug) ? Qtrue : Qfalse; }
static VALUE set_ds_debug(VALUE self, VALUE newval) {
	GET_WC;

	wc->dsDebug = (newval == Qtrue) ? 1 : 0;
	return newval;
}

static VALUE get_moved_from_x(VALUE self) { GET_WC; return INT2FIX(wc->i_movedFromX*2); }
static VALUE get_moved_from_y(VALUE self) { GET_WC; return INT2FIX(wc->i_movedFromY); }
static VALUE get_moved_to_x(VALUE self) { GET_WC; return INT2FIX(wc->i_movedToX*2); }
static VALUE get_moved_to_y(VALUE self) { GET_WC; return INT2FIX(wc->i_movedToY); }
static VALUE get_trigger_x(VALUE self) { GET_WC; return INT2FIX(wc->i_triggerX*2); }
static VALUE get_trigger_y(VALUE self) { GET_WC; return INT2FIX(wc->i_triggerY); }
static VALUE get_did_player_move(VALUE self) { GET_WC; return ((wc->i_didPlayerMove != 0) ? Qtrue : Qfalse); }
static VALUE get_number_said(VALUE self) { GET_WC; return INT2FIX(wc->i_numberSaid); }
static VALUE get_facing_direction(VALUE self) { GET_WC; return INT2FIX(wc->i_facingDirection); }
static VALUE get_players_in_dream(VALUE self) { GET_WC; return INT2FIX(wc->i_playersInDream); }
static VALUE get_player(VALUE self) { GET_WC; return wc->i_playerValue; }
static VALUE get_button_pressed(VALUE self) { GET_WC; return INT2FIX(wc->i_dsButtonPressed); }
static VALUE get_dream_cookies(VALUE self) { GET_WC; return INT2FIX(wc->i_dreamCookies); }
static VALUE get_player_cookies(VALUE self) { GET_WC; return INT2FIX(wc->i_playerCookies); }

static VALUE get_variable(VALUE self, VALUE index) {
	Check_Type(index, T_FIXNUM);

	GET_WC;

	int rIndex = FIX2INT(index);
	if (rIndex < 0 || rIndex >= MAX_VARIABLE)
		return INT2FIX(0);

	return INT2FIX(wc->variables[rIndex]);
}

static VALUE set_variable(VALUE self, VALUE index, VALUE value) {
	Check_Type(index, T_FIXNUM); Check_Type(value, T_FIXNUM);

	GET_WC;

	int rIndex = FIX2INT(index);
	if (rIndex < 0 || rIndex >= MAX_VARIABLE)
		return Qnil;

	wc->variables[rIndex] = FIX2INT(value);
	return value;
}

/*******************************************************************************
 * Interfaces to C Methods
 ******************************************************************************/
static VALUE load_map(VALUE self, VALUE buf, VALUE width, VALUE height, VALUE hasDataV29, VALUE hasDataV30) {
	Check_Type(buf, T_STRING);
	Check_Type(width, T_FIXNUM); Check_Type(height, T_FIXNUM);

	GET_WC;

	int rWidth = FIX2INT(width), rHeight = FIX2INT(height);
	VALUE rBuf = StringValue(buf);

	if (rWidth < 0 || rWidth >= MAX_MAP_WIDTH || rHeight < 0 || rHeight >= MAX_MAP_HEIGHT) {
		rb_raise(rb_eArgError, "map size is out of bounds");
	}

	int planeAmount = 3;
	if (RTEST(hasDataV29) || RTEST(hasDataV30))
		planeAmount += 2;
	if (RTEST(hasDataV30))
		planeAmount += 2;
	int expectedSize = rWidth * rHeight * 2 * planeAmount;
	if (RSTRING_LEN(rBuf) < expectedSize) {
		rb_raise(rb_eTypeError, "buffer is too small to hold the map data");
	}

	wc_load_map(wc, RSTRING_PTR(rBuf), rWidth, rHeight, RTEST(hasDataV29), RTEST(hasDataV30));

	return Qtrue;
}

static VALUE save_map(VALUE self, VALUE hasDataV29, VALUE hasDataV30) {
	GET_WC;

	int planeAmount = 3;
	if (RTEST(hasDataV29) || RTEST(hasDataV30))
		planeAmount += 2;
	if (RTEST(hasDataV30))
		planeAmount += 2;
	VALUE string = rb_str_new(0, wc->mapWidth * wc->mapHeight * 2 * planeAmount);
	wc_save_map(wc, RSTRING_PTR(string), RTEST(hasDataV29), RTEST(hasDataV30));

	return string;
}

static VALUE has_data_v29(VALUE self) {
	GET_WC;

	char ret = wc_has_data_v29(wc);

	return ret ? Qtrue : Qfalse;
}

static VALUE has_data_v30(VALUE self) {
	GET_WC;

	char ret = wc_has_data_v30(wc);

	return ret ? Qtrue : Qfalse;
}

static VALUE process_line(VALUE self, VALUE line) {
	Check_Type(line, T_STRING);

	GET_WC;

	wc_process_line(wc, RSTRING_PTR(line), RSTRING_LEN(line));

	return Qtrue;
}

static VALUE add_ds_line(VALUE self, VALUE category, VALUE type, VALUE params, VALUE annotation) {
	Check_Type(category, T_FIXNUM); Check_Type(type, T_FIXNUM);
	Check_Type(params, T_ARRAY);
	if (!NIL_P(annotation)) {
		Check_Type(annotation, T_HASH);
	}

	GET_WC;

	DSLine *line = &wc->ds[wc->dsLineCount];
	line->category = FIX2INT(category);
	line->type = FIX2INT(type);

	int i;
	for (i = 0; i < RARRAY_LEN(params); i++) {
		line->params[i] = FIX2INT(RARRAY_PTR(params)[i]);
	}

	line->annotation = annotation;

	wc->dsLineCount++;

	return Qnil;
}

static VALUE begin_map_change_logging(VALUE self) {
	GET_WC;

	wc->isLoggingMapChanges++;

	if (wc->isLoggingMapChanges == 1) {
		wc_setup_change_buffer(wc, &wc->itemChangeBuffer, '>');
		wc_setup_change_buffer(wc, &wc->floorChangeBuffer, '1');
		wc_setup_change_buffer(wc, &wc->wallChangeBuffer, '2');
		wc_setup_change_buffer(wc, &wc->regionChangeBuffer, '4');
		wc_setup_change_buffer(wc, &wc->effectChangeBuffer, '5');
		wc_setup_change_buffer(wc, &wc->lightingChangeBuffer, 'E');
		wc_setup_change_buffer(wc, &wc->ambienceChangeBuffer, 'F');
	}

	return Qtrue;
}

static VALUE end_map_change_logging(VALUE self) {
	GET_WC;

	if (!wc->isLoggingMapChanges)
		rb_raise(rb_eRuntimeError, "map change logging is already off");

	wc->isLoggingMapChanges--;

	if (!wc->isLoggingMapChanges) {
		wc_flush_change_buffer(wc, &wc->itemChangeBuffer);
		wc_flush_change_buffer(wc, &wc->floorChangeBuffer);
		wc_flush_change_buffer(wc, &wc->wallChangeBuffer);
		wc_flush_change_buffer(wc, &wc->regionChangeBuffer);
		wc_flush_change_buffer(wc, &wc->effectChangeBuffer);
		wc_flush_change_buffer(wc, &wc->lightingChangeBuffer);
		wc_flush_change_buffer(wc, &wc->ambienceChangeBuffer);
	}

	return Qtrue;
}

/*******************************************************************************
 * Ruby Housekeeping
 ******************************************************************************/
static VALUE get_bot(VALUE self) {
	GET_WC;

	return wc->bot;
}

static VALUE set_callback(VALUE self, VALUE type) {
	GET_WC;

	rb_need_block();

	ID typeID = SYM2ID(type);

	if (typeID == rb_intern("item_changed")) {
		wc->cb_itemChanged = rb_block_proc();
	} else if (typeID == rb_intern("floor_changed")) {
		wc->cb_floorChanged = rb_block_proc();
	} else if (typeID == rb_intern("wall_changed")) {
		wc->cb_wallChanged = rb_block_proc();
	} else if (typeID == rb_intern("region_changed")) {
		wc->cb_regionChanged = rb_block_proc();
	} else if (typeID == rb_intern("effect_changed")) {
		wc->cb_effectChanged = rb_block_proc();
	} else if (typeID == rb_intern("lighting_changed")) {
		wc->cb_lightingChanged = rb_block_proc();
	} else if (typeID == rb_intern("ambience_changed")) {
		wc->cb_ambienceChanged = rb_block_proc();
	} else if (typeID == rb_intern("held_object_changed")) {
		wc->cb_heldObjectChanged = rb_block_proc();
	} else {
		return Qfalse;
	}

	return type;
}

static VALUE initialize(VALUE self, VALUE bot) {
	GET_WC;

	if (NIL_P(bot))
		rb_raise(rb_eArgError, "a bot must be passed to Context#new");

	wc->bot = bot;

	wc->playerList = rb_ary_new();

	wc->playersByShortname = rb_hash_new();
	wc->playersByUserID = rb_hash_new();
	wc->playersByPosition = rb_hash_new();

	wc->lastDeletedPlayer = Qnil;

	wc->cb_itemChanged = Qnil;
	wc->cb_floorChanged = Qnil;
	wc->cb_wallChanged = Qnil;
	wc->cb_regionChanged = Qnil;
	wc->cb_effectChanged = Qnil;
	wc->cb_lightingChanged = Qnil;
	wc->cb_ambienceChanged = Qnil;
	wc->cb_heldObjectChanged = Qnil;

	return self;
}

static void mark(WorldContext *wc) {
	rb_gc_mark(wc->playerList);
	rb_gc_mark(wc->playersByShortname);
	rb_gc_mark(wc->playersByUserID);
	rb_gc_mark(wc->playersByPosition);
	rb_gc_mark(wc->lastDeletedPlayer);

	int i;
	for (i = 0; i < wc->dsLineCount; i++) {
		rb_gc_mark(wc->ds[i].annotation);
	}

	rb_gc_mark(wc->cb_itemChanged);
	rb_gc_mark(wc->cb_floorChanged);
	rb_gc_mark(wc->cb_wallChanged);
	rb_gc_mark(wc->cb_regionChanged);
	rb_gc_mark(wc->cb_effectChanged);
	rb_gc_mark(wc->cb_lightingChanged);
	rb_gc_mark(wc->cb_ambienceChanged);
	rb_gc_mark(wc->cb_heldObjectChanged);
}

static VALUE allocate(VALUE klass) {
	WorldContext *wc = malloc(sizeof(WorldContext));
	memset(wc, 0, sizeof(WorldContext));
	return Data_Wrap_Struct(klass, mark, free, wc);
}

void Init_nelumbo_world_context() {
	cNelumboWorldContext = rb_define_class_under(mNelumboWorld, "Context", rb_cObject);

	rb_define_alloc_func(cNelumboWorldContext, allocate);

	rb_define_method(cNelumboWorldContext, "initialize", initialize, 1);

	rb_define_method(cNelumboWorldContext, "width", get_width, 0);
	rb_define_method(cNelumboWorldContext, "height", get_height, 0);

	rb_define_method(cNelumboWorldContext, "load_map", load_map, 5);
	rb_define_method(cNelumboWorldContext, "save_map", save_map, 2);
	rb_define_method(cNelumboWorldContext, "has_data_v29", has_data_v29, 0);
	rb_define_method(cNelumboWorldContext, "has_data_v30", has_data_v30, 0);

	rb_define_method(cNelumboWorldContext, "bot", get_bot, 0);

	rb_define_method(cNelumboWorldContext, "item", get_item, 2);
	rb_define_method(cNelumboWorldContext, "set_item", set_item, 3);
	rb_define_method(cNelumboWorldContext, "floor", get_floor, 2);
	rb_define_method(cNelumboWorldContext, "set_floor", set_floor, 3);
	rb_define_method(cNelumboWorldContext, "wall", get_wall, 2);
	rb_define_method(cNelumboWorldContext, "set_wall", set_wall, 3);
	rb_define_method(cNelumboWorldContext, "region", get_region, 2);
	rb_define_method(cNelumboWorldContext, "set_region", set_region, 3);
	rb_define_method(cNelumboWorldContext, "effect", get_effect, 2);
	rb_define_method(cNelumboWorldContext, "set_effect", set_effect, 3);
	rb_define_method(cNelumboWorldContext, "lighting", get_lighting, 2);
	rb_define_method(cNelumboWorldContext, "set_lighting", set_lighting, 3);
	rb_define_method(cNelumboWorldContext, "ambience", get_ambience, 2);
	rb_define_method(cNelumboWorldContext, "set_ambience", set_ambience, 3);

	rb_define_method(cNelumboWorldContext, "create_and_add_player", create_and_add_player, 2);
	rb_define_method(cNelumboWorldContext, "delete_and_remove_player", delete_and_remove_player, 1);

	rb_define_method(cNelumboWorldContext, "find_player_by_name", find_player_by_name, 1);
	rb_define_method(cNelumboWorldContext, "find_player_by_uid", find_player_by_uid, 1);
	rb_define_method(cNelumboWorldContext, "find_player_at_position", find_player_at_position, 2);

	rb_define_method(cNelumboWorldContext, "each_player", each_player, 0);

	rb_define_method(cNelumboWorldContext, "process_line", process_line, 1);
	rb_define_method(cNelumboWorldContext, "move_tracked_player", move_tracked_player, 3);

	rb_define_method(cNelumboWorldContext, "add_ds_line", add_ds_line, 4);

	rb_define_method(cNelumboWorldContext, "ds_debug", get_ds_debug, 0);
	rb_define_method(cNelumboWorldContext, "ds_debug=", set_ds_debug, 1);

	rb_define_method(cNelumboWorldContext, "moved_from_x", get_moved_from_x, 0);
	rb_define_method(cNelumboWorldContext, "moved_from_y", get_moved_from_y, 0);
	rb_define_method(cNelumboWorldContext, "moved_to_x", get_moved_to_x, 0);
	rb_define_method(cNelumboWorldContext, "moved_to_y", get_moved_to_y, 0);
	rb_define_method(cNelumboWorldContext, "trigger_x", get_trigger_x, 0);
	rb_define_method(cNelumboWorldContext, "trigger_y", get_trigger_y, 0);
	rb_define_method(cNelumboWorldContext, "player_moved?", get_did_player_move, 0);
	rb_define_method(cNelumboWorldContext, "number_said", get_number_said, 0);
	rb_define_method(cNelumboWorldContext, "facing_direction", get_facing_direction, 0);
	rb_define_method(cNelumboWorldContext, "players_in_dream", get_players_in_dream, 0);
	rb_define_method(cNelumboWorldContext, "player", get_player, 0);
	rb_define_method(cNelumboWorldContext, "button_pressed", get_button_pressed, 0);
	rb_define_method(cNelumboWorldContext, "dream_cookies", get_dream_cookies, 0);
	rb_define_method(cNelumboWorldContext, "player_cookies", get_player_cookies, 0);

	rb_define_method(cNelumboWorldContext, "variable", get_variable, 1);
	rb_define_method(cNelumboWorldContext, "set_variable", set_variable, 2);

	rb_define_method(cNelumboWorldContext, "begin_map_change_logging", begin_map_change_logging, 0);
	rb_define_method(cNelumboWorldContext, "end_map_change_logging", end_map_change_logging, 0);

	rb_define_method(cNelumboWorldContext, "set_callback", set_callback, 1);
}
