#include <nelumbo.h>

VALUE mNelumbo;
VALUE mNelumboWorld;

void Init_nelumbo() {
	mNelumbo = rb_define_module("Nelumbo");
	mNelumboWorld = rb_define_module_under(mNelumbo, "World");

	Init_nelumbo_world_context();
}

