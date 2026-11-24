#if 0
//todo - maybe find a way to fix this????
#include <vector>
using namespace std;

//stuff
#define MC_NUM_BUTTONS 3
#define MC_NUM_MODKEYS 3
#define MC_MAX_NAME 128
//mouse buttons - sequential
#define MC_BTN_LEFT    1
#define MC_BTN_MIDDLE  2
#define MC_BTN_RIGHT   3
//modifier keys - bit masks
#define MC_MOD_CTRL    1
#define MC_MOD_ALT     2
#define MC_MOD_SHIFT   4
//mouse event states
#define MC_NONE        0
#define MC_MOUSEDOWN   1
#define MC_MOUSEUP     2
#define MC_MOUSEMOVE   3
#define MC_MOUSEOVER   4

class MouseCommand;
//mouse state
typedef struct MouseState_s {
	MouseCommand *cmd;			// command selected by mousedown, to be performed by other funcs
	int event;					// set before execbinding, set to 0 to end mouse 
	POINT start;
	POINT current;
	POINT delta;
	POINT relative;
} MouseState_t;


//MouseCommand - abstract - individual command
class MouseCommand {
public:
	MouseCommand() {};
	virtual ~MouseCommand() = 0;
	virtual bool MouseDown(MouseState_t& state) = 0;
	virtual bool MouseUp(MouseState_t& state) = 0;
	virtual bool MouseMove(MouseState_t& state) = 0;
	virtual bool MouseOver(MouseState_t& state) = 0;
	virtual const char *GetName() = 0;
};


//MouseCommandList - list of all available commands per config
// note: this object never deletes MouseCommand items
typedef vector<MouseCommand*> MCL_t;
typedef MCL_t::iterator MCL_iter;
class MouseCommandList {
public:
	MouseCommandList();
	~MouseCommandList();
	bool Add(MouseCommand *item);
	void Clear();
	int Count();
	MouseCommand *GetItem(int index);
	MouseCommand *GetItem(const char *name);
protected:
	MCL_t *items;
	int count;
};


typedef struct mouseconf_s {
	char name[MC_MAX_NAME];
	MouseCommandList *action[MC_NUM_BUTTONS][1<<MC_NUM_MODKEYS];
} mouseconf_t;

//MouseBinding - subset of commands for one mouse button
class MouseConfig {
public:
	MouseConfig(const char* cfg_file, MouseCommandList *mcl);
	~MouseConfig();
	
	bool ExecBinding(MouseState_t& state, DWORD mbutton, DWORD modkeys);
	
	
protected:
	bool parse_config(const char *cfg_file);
	void clear_data();
	void init_data(int num_cfg);
	
	MouseCommandList *command_list;
	mouseconf_t *setup;
	int cfg_count;
	int cfg_current;
};



#endif