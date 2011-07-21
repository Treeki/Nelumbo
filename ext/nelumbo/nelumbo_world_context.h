#ifndef RUBY_NELUMBO_WORLD_CONTEXT
#define RUBY_NELUMBO_WORLD_CONTEXT

extern VALUE cNelumboWorldContext;

void Init_nelumbo_world_context();

#define MAX_MAP_WIDTH 300
#define MAX_MAP_HEIGHT 600
#define MAX_ITEM 3400
#define MAX_FLOOR 1000

typedef struct _dsline {
	unsigned short category, type;
	unsigned short params[8];
} DSLine;


typedef struct _worldcontext {
	/* Map Data */
	int mapWidth, mapHeight;
	unsigned short items[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
	unsigned short floors[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
	unsigned char walls[MAX_MAP_WIDTH*2][MAX_MAP_HEIGHT];

	/* Patch Info */
	bool itemWalkable[MAX_ITEM];
	bool floorWalkable[MAX_FLOOR];

	/* DS Engine */
	DSLine ds[12000];
	int dsLineCount;
	unsigned short variables[1000];

	DSLine *currentArea;
	DSLine *filters[50];
	int filterCount;

	/* Trigger Info */
	int i_movedFromX, i_movedFromY;
	int i_movedToX, i_movedToY;
	bool i_didPlayerMove;
	unsigned int i_randomSeed;
	int i_numberSaid;
	int i_facingDirection;
	int i_entryCode;
	int i_heldObject;
	int i_playersInDream;
	unsigned int i_userID;
	VALUE i_user;
	int i_dsButtonPressed;
	int i_dreamCookies;
	int i_playerCookies;
	int i_special[1024];

	/* Interop
	 * NOTE: We don't mark this for the GC as the Bot is supposed to hold
	 * a reference to WorldContext, not the other way round. */
	VALUE bot;
} WorldContext;

extern ID id_find_player_at_position;


void wc_load_map(WorldContext *wc, char *buf, int width, int height);

void wc_mark(WorldContext *wc);

bool wc_position_is_walkable(WorldContext *wc, int x, int y, VALUE player);

#endif

