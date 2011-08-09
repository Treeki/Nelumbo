#include <nelumbo.h>

VALUE cNelumboWorldPlayer;

/*******************************************************************************
 * Properties and Properties and Properties
 ******************************************************************************/
#define GET_PL \
	Player *pl; \
	Data_Get_Struct(self, Player, pl)

VALUE get_uid(VALUE self) { GET_PL; return INT2NUM(pl->uid); }
VALUE get_shortname(VALUE self) { GET_PL; return pl->shortname; }

VALUE get_name(VALUE self) { GET_PL; return pl->name; }
VALUE set_name(VALUE self, VALUE name) { GET_PL; pl->name = name; return name; }

VALUE get_color_code(VALUE self) { GET_PL; return pl->colourCode; }
VALUE set_color_code(VALUE self, VALUE code) { GET_PL; pl->colourCode = code; return code; }

VALUE get_x(VALUE self) { GET_PL; return INT2FIX(pl->x * 2); }
VALUE get_y(VALUE self) { GET_PL; return INT2FIX(pl->y); }

VALUE get_visible(VALUE self) { GET_PL; return (pl->visible != 0) ? Qtrue : Qfalse; }
VALUE set_visible(VALUE self, VALUE vis) { GET_PL; pl->visible = RTEST(vis); return vis; }

VALUE get_afk_start_time(VALUE self) { GET_PL; return pl->afkStartTime; }
VALUE set_afk_start_time(VALUE self, VALUE time) { GET_PL; pl->afkStartTime = time; return time; }

VALUE afk_check(VALUE self) { GET_PL; return NIL_P(pl->afkStartTime) ? Qfalse : Qtrue; }

VALUE afk_length(VALUE self) {
	GET_PL;
	return rb_funcall(rb_class_new_instance(0, 0, rb_cTime), rb_intern("-"), 1, pl->afkStartTime);
}

VALUE get_entry_code(VALUE self) { GET_PL; return INT2FIX(pl->entryCode); }
VALUE set_entry_code(VALUE self, VALUE num) { Check_Type(num, T_FIXNUM); GET_PL; pl->entryCode = FIX2INT(num); return num; }

VALUE get_shape(VALUE self) { GET_PL; return INT2FIX(pl->shape); }
VALUE set_shape(VALUE self, VALUE num) { Check_Type(num, T_FIXNUM); GET_PL; pl->shape = FIX2INT(num); return num; }

VALUE get_held_object(VALUE self) { GET_PL; return INT2FIX(pl->heldObject); }
VALUE set_held_object(VALUE self, VALUE num) { Check_Type(num, T_FIXNUM); GET_PL; pl->heldObject = FIX2INT(num); return num; }

VALUE get_cookies(VALUE self) { GET_PL; return INT2FIX(pl->cookies); }
VALUE set_cookies(VALUE self, VALUE num) { Check_Type(num, T_FIXNUM); GET_PL; pl->cookies = FIX2INT(num); return num; }

/*******************************************************************************
 * Ruby Housekeeping
 ******************************************************************************/
static VALUE initialize(VALUE self, VALUE uid, VALUE name) {
	GET_PL;

	pl->x = -1;
	pl->y = -1;

	pl->uid = NUM2INT(uid);
	pl->name = name;
	pl->shortname = rb_funcall(name, rb_intern("to_shortname"), 0);

	pl->afkStartTime = Qnil;

	return self;
}

static void mark(Player *pl) {
	rb_gc_mark(pl->name);
	rb_gc_mark(pl->shortname);
	rb_gc_mark(pl->afkStartTime);
	rb_gc_mark(pl->colourCode);
}

static VALUE allocate(VALUE klass) {
	Player *pl = malloc(sizeof(Player));
	memset(pl, 0, sizeof(Player));
	return Data_Wrap_Struct(klass, mark, free, pl);
}

void Init_nelumbo_world_player() {
	cNelumboWorldPlayer = rb_define_class_under(mNelumboWorld, "Player", rb_cObject);

	rb_define_alloc_func(cNelumboWorldPlayer, allocate);

	rb_define_method(cNelumboWorldPlayer, "initialize", initialize, 2);

	rb_define_method(cNelumboWorldPlayer, "uid", get_uid, 0);
	rb_define_method(cNelumboWorldPlayer, "shortname", get_shortname, 0);

	rb_define_method(cNelumboWorldPlayer, "name", get_name, 0);
	rb_define_method(cNelumboWorldPlayer, "name=", set_name, 1);

	rb_define_method(cNelumboWorldPlayer, "color_code", get_color_code, 0);
	rb_define_method(cNelumboWorldPlayer, "color_code=", set_color_code, 1);

	rb_define_method(cNelumboWorldPlayer, "x", get_x, 0);
	rb_define_method(cNelumboWorldPlayer, "y", get_y, 0);

	rb_define_method(cNelumboWorldPlayer, "visible", get_visible, 0);
	rb_define_method(cNelumboWorldPlayer, "visible=", set_visible, 1);

	rb_define_method(cNelumboWorldPlayer, "afk_start_time", get_afk_start_time, 0);
	rb_define_method(cNelumboWorldPlayer, "afk_start_time=", set_afk_start_time, 1);
	rb_define_method(cNelumboWorldPlayer, "afk?", afk_check, 0);
	rb_define_method(cNelumboWorldPlayer, "afk_length", afk_length, 0);

	rb_define_method(cNelumboWorldPlayer, "entry_code", get_entry_code, 0);
	rb_define_method(cNelumboWorldPlayer, "entry_code=", set_entry_code, 1);

	rb_define_method(cNelumboWorldPlayer, "shape", get_shape, 0);
	rb_define_method(cNelumboWorldPlayer, "shape=", set_shape, 1);

	rb_define_method(cNelumboWorldPlayer, "held_object", get_held_object, 0);
	rb_define_method(cNelumboWorldPlayer, "held_object=", set_held_object, 1);

	rb_define_method(cNelumboWorldPlayer, "cookies", get_cookies, 0);
	rb_define_method(cNelumboWorldPlayer, "cookies=", set_cookies, 1);

}

