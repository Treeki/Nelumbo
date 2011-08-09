#ifndef RUBY_NELUMBO_WORLD_PLAYER
#define RUBY_NELUMBO_WORLD_PLAYER

extern VALUE cNelumboWorldPlayer;

void Init_nelumbo_world_player();

typedef struct _player {
	unsigned int uid;
	VALUE shortname, name, colourCode;
	// X is stored halved in this struct
	int x, y;
	char visible;
	VALUE afkStartTime;

	/* transient info that may or may not be up to date */
	int entryCode, shape, heldObject, cookies;
} Player;

#endif


