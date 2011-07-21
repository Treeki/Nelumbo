#include <nelumbo.h>

VALUE cNelumboWorldContext;

ID id_find_player_at_position;

/*******************************************************************************
 * Map Data Access
 ******************************************************************************/
static VALUE get_width(VALUE self) {
	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	return INT2FIX(wc->mapWidth);
}

static VALUE get_height(VALUE self) {
	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	return INT2FIX(wc->mapHeight);
}

static VALUE get_item(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	return INT2FIX(wc->items[FIX2INT(x)][FIX2INT(y)]);
}

static VALUE set_item(VALUE self, VALUE x, VALUE y, VALUE item) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(item, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	wc->items[FIX2INT(x)][FIX2INT(y)] = FIX2INT(item);
	return item;
}

static VALUE get_floor(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	return INT2FIX(wc->floors[FIX2INT(x)][FIX2INT(y)]);
}

static VALUE set_floor(VALUE self, VALUE x, VALUE y, VALUE floor) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(floor, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	wc->floors[FIX2INT(x)][FIX2INT(y)] = FIX2INT(floor);
	return floor;
}

static VALUE get_wall(VALUE self, VALUE x, VALUE y) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	return INT2FIX(wc->walls[FIX2INT(x)][FIX2INT(y)]);
}

static VALUE set_wall(VALUE self, VALUE x, VALUE y, VALUE wall) {
	Check_Type(x, T_FIXNUM); Check_Type(y, T_FIXNUM);
	Check_Type(wall, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	wc->walls[FIX2INT(x)][FIX2INT(y)] = FIX2INT(wall);
	return wall;
}

/*******************************************************************************
 * Interfaces to C Methods
 ******************************************************************************/
static VALUE load_map(VALUE self, VALUE buf, VALUE width, VALUE height) {
	Check_Type(buf, T_STRING);
	Check_Type(width, T_FIXNUM); Check_Type(height, T_FIXNUM);

	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	int rWidth = FIX2INT(width), rHeight = FIX2INT(height);
	VALUE rBuf = StringValue(buf);

	if (rWidth < 0 || rWidth >= MAX_MAP_WIDTH || rHeight < 0 || rHeight >= MAX_MAP_HEIGHT) {
		rb_raise(rb_eTypeError, "map size is out of bounds");
	}

	int expectedSize = rWidth * rHeight * 2 * 4;
	if (RSTRING_LEN(rBuf) < expectedSize) {
		rb_raise(rb_eTypeError, "buffer is too small to hold the map data");
	}

	wc_load_map(RSTRING_PTR(rBuf), rWidth, rHeight);
}

/*******************************************************************************
 * Ruby Housekeeping
 ******************************************************************************/
static VALUE initialize(VALUE self, VALUE bot) {
	WorldContext *wc;
	Data_Get_Struct(self, WorldContext, wc);

	wc->bot = bot;
}

static VALUE allocate(VALUE klass) {
	WorldContext *wc = malloc(sizeof(WorldContext));
	memset(wc, 0, sizeof(WorldContext));
	return Data_Wrap_Struct(klass, NULL, free, wc);
}

void Init_nelumbo_world_context() {
	id_find_player_at_position = rb_intern("find_player_at_position");


	cNelumboWorldContext = rb_define_class_under(mNelumboWorld, "Context", rb_cObject);

	rb_define_alloc_func(cNelumboWorldContext, allocate);

	rb_define_method(cNelumboWorldContext, "initialize", initialize, 1);

	rb_define_method(cNelumboWorldContext, "width", get_width, 0);
	rb_define_method(cNelumboWorldContext, "height", get_height, 0);

	rb_define_method(cNelumboWorldContext, "load_map", load_map, 3);

	rb_define_method(cNelumboWorldContext, "item", get_item, 2);
	rb_define_method(cNelumboWorldContext, "set_item", set_item, 3);
	rb_define_method(cNelumboWorldContext, "floor", get_floor, 2);
	rb_define_method(cNelumboWorldContext, "set_floor", set_floor, 3);
	rb_define_method(cNelumboWorldContext, "wall", get_wall, 2);
	rb_define_method(cNelumboWorldContext, "set_wall", set_wall, 3);
}

