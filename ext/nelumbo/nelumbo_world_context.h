#ifndef RUBY_NELUMBO_WORLD_CONTEXT
#define RUBY_NELUMBO_WORLD_CONTEXT

extern VALUE cNelumboWorldContext;

void Init_nelumbo_world_context();

#define MAX_MAP_WIDTH 300
#define MAX_MAP_HEIGHT 600
#define MAX_ITEM 3400
#define MAX_FLOOR 1000
#define MAX_VARIABLE 1000
#define MAX_DS 12000
#define MAX_FILTERS 50

#define CHANGE_BUFFER_COUNT 100
#define CHANGE_BUFFER_ELEMENT_SIZE 6
#define CHANGE_BUFFER_SIZE (CHANGE_BUFFER_COUNT*CHANGE_BUFFER_ELEMENT_SIZE)

#define ITEM_VALID(number) (((number) >= 0) && ((number) < MAX_ITEM))
#define FLOOR_VALID(number) (((number) >= 0) && ((number) < MAX_FLOOR))

// Accepts a halved X value
static inline VALUE PLAYER_KEY(int x, int y) {
	return INT2FIX((x << 12) | y);
}

typedef struct _dsline {
	unsigned short category, type;
	unsigned short params[8];
	VALUE annotation;
} DSLine;

typedef struct _dsrandom {
	uint32_t array[64];
	uint32_t *pointer;
	int counter;
} DSRandom;

void dsr_seed(DSRandom *dsr, uint32_t seed);
uint32_t dsr_generate(DSRandom *dsr, uint32_t max);

typedef struct _changebuffer {
	char buffer[CHANGE_BUFFER_SIZE];
	char insnID;
	int end;
} ChangeBuffer;


typedef struct _worldcontext {
	/* Player Tracking */
	VALUE playerList;
	VALUE playersByShortname;
	VALUE playersByUserID;
	VALUE playersByPosition;

	/* a bit of a hack because Furc fires 0:10 after the player is deleted */
	VALUE lastDeletedPlayer;
	unsigned int lastDeletedPlayerUID;

	char hasDream;

	/* Map Data */
	int mapWidth, mapHeight;
	unsigned short items[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
	unsigned short floors[MAX_MAP_WIDTH][MAX_MAP_HEIGHT];
	unsigned char walls[MAX_MAP_WIDTH*2][MAX_MAP_HEIGHT];

	/* Patch Info */
	char itemWalkable[MAX_ITEM];
	char floorWalkable[MAX_FLOOR];

	/* DS Engine */
	DSLine ds[MAX_DS];
	int dsLineCount;
	short variables[MAX_VARIABLE];

	DSLine currentArea;
	DSLine filters[MAX_FILTERS];
	int filterCount;

	/* Trigger Info */
	int i_movedFromX, i_movedFromY;
	int i_movedToX, i_movedToY;
	int i_triggerX, i_triggerY;
	char i_didPlayerMove;
	unsigned int i_randomSeed;
	int i_numberSaid;
	int i_facingDirection;
	int i_entryCode;
	int i_heldObject;
	int i_playersInDream;
	unsigned int i_userID;
	Player *i_player;
	VALUE i_playerValue;
	int i_dsButtonPressed;
	int i_dreamCookies;
	int i_playerCookies;
	int i_special[1024];
	int i_specialIndex;

	DSRandom i_randomGenerator;

	/* Callbacks */
	VALUE cb_itemChanged;
	VALUE cb_floorChanged;
	VALUE cb_wallChanged;

	VALUE cb_heldObjectChanged;

	/* Map Change Queue */
	int isLoggingMapChanges;

	ChangeBuffer itemChangeBuffer;
	ChangeBuffer floorChangeBuffer;
	ChangeBuffer wallChangeBuffer;

	/* Interop
	 * NOTE: We don't mark this for the GC as the Bot is supposed to hold
	 * a reference to WorldContext, not the other way round. Reference cycles
	 * are no fun. */
	VALUE bot;
} WorldContext;


void wc_load_map(WorldContext *wc, char *buf, int width, int height);
void wc_save_map(WorldContext *wc, char *buf);

void wc_process_line(WorldContext *wc, char *buf, int length);

void wc_setup_change_buffer(WorldContext *wc, ChangeBuffer *cb, char insnID);
void wc_append_to_change_buffer(WorldContext *wc, ChangeBuffer *cb, int x, int y, int number);
void wc_flush_change_buffer(WorldContext *wc, ChangeBuffer *cb);

char wc_position_is_walkable(WorldContext *wc, int x, int y, Player *player);
char wc_position_is_valid(WorldContext *wc, int x, int y);
void wc_get_visibility_bounds(WorldContext *wc, int x, int y, int *x1, int *y1, int *x2, int *y2);

char intersect_line_nwse(int x1, int y1, int x2, int y2);
char intersect_line_nesw(int x1, int y1, int x2, int y2);

void wc_clamp_position(WorldContext *wc, short *x, short *y);
void wc_clamp_position_to_borders(WorldContext *wc, short *x, short *y);
void wc_move_position_ne(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_se(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_sw(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_nw(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_ne_clamped(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_se_clamped(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_sw_clamped(WorldContext *wc, short *x, short *y, int distance);
void wc_move_position_nw_clamped(WorldContext *wc, short *x, short *y, int distance);

short wc_ds_value(WorldContext *wc, int value);
short wc_ds_value_y(WorldContext *wc, int value);

int wc_read_special(WorldContext *wc);

void wc_execute_trigger(WorldContext *wc, int number, int x, int y, char isSelf);
// Internal
void wc_handle_annotation(WorldContext *wc, DSLine *line);
void wc_set_area(WorldContext *wc, DSLine *line);
void wc_execute_on_area(WorldContext *wc, DSLine *line);
void wc_add_filter(WorldContext *wc, DSLine *line);
void wc_execute_on_area_position(WorldContext *wc, DSLine *line, int x, int y);
void wc_execute_on_wall(WorldContext *wc, DSLine *line, int x, int y);
void wc_execute_effect(WorldContext *wc, DSLine *line);

uint32_t wc_random_number(WorldContext *wc, uint32_t max);

#define WC_VAR_SAFE(id) wc->variables[(id) % 1000]

#endif

