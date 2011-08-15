#ifndef RUBY_NELUMBO
#define RUBY_NELUMBO

#include <ruby.h>

#include <nelumbo_world_player.h>
#include <nelumbo_world_context.h>

extern VALUE mNelumbo;
extern VALUE mNelumboWorld;

int decode_b95(const char *buffer, int length);
int decode_b220(const char *buffer, int length);

void encode_b95(int value, char *buffer, int length);
void encode_b220(int value, char *buffer, int length);

#endif

