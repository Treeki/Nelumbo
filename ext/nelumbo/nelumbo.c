#include <nelumbo.h>

VALUE mNelumbo;
VALUE mNelumboWorld;

void Init_nelumbo() {
	mNelumbo = rb_define_module("Nelumbo");
	mNelumboWorld = rb_define_module_under(mNelumbo, "World");

	Init_nelumbo_world_player();
	Init_nelumbo_world_context();
}

int decode_b95(const char *buffer, int length) {
	int i, value = 0;
	unsigned char *buf = (unsigned char *)buffer;

	for (i = 0; i < length; i++) {
		value = (value * 95) + (buf[i] - 32);
	}

	return value;
}

int decode_b220(const char *buffer, int length) {
	int i, mult = 1, value = 0;
	unsigned char *buf = (unsigned char *)buffer;

	for (i = 0; i < length; i++) {
		value += ((buf[i] - 35) * mult);
		mult *= 220;
	}

	return value;
}


