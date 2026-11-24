#include "global.hpp"

//
// MouseCommand
//
//need stub for virtual destructor
MouseCommand::~MouseCommand() {}


//
// MouseCommandList
//
MouseCommandList::MouseCommandList()
	: count(0)
{
	items = new vector<MouseCommand*>;
}
MouseCommandList::~MouseCommandList() {
	items->clear();
	delete items;
}
bool MouseCommandList::Add(MouseCommand *item) {
	if(!item)
		return false;
	//if new item exists ret false
	MCL_iter iter;
	for(iter = items->begin(); iter != items->end(); iter++) {
		if(*(iter)==item)
			return false;
	}
	//add
	items->push_back(item);
	count++;
	return true;
}
void MouseCommandList::Clear() {
	items->clear();
	count = 0;
}
int MouseCommandList::Count() {
	return count;
}
MouseCommand *MouseCommandList::GetItem(int index) {
	if(index < 0 || index >= count)
		return 0;
	return items->at(index);
}
MouseCommand *MouseCommandList::GetItem(const char *name) {
	if(name) {
		MouseCommand *cmd = GetItem(0);
		for(int i=0; cmd; cmd = GetItem(++i)) {
			if(!stricmp(cmd->GetName(), name))
				return cmd;
		}
	}
	return 0;
}

//
// MouseConfig
//
MouseConfig::MouseConfig(const char* cfg_file, MouseCommandList *mcl) 
	: setup(0), cfg_count(0), cfg_current(0), command_list(mcl)
{
	parse_config(cfg_file);
}
MouseConfig::~MouseConfig() 
{
	clear_data();
}
//user
bool MouseConfig::ExecBinding(MouseState_t& state, DWORD mbutton, DWORD modkeys) {
	//check event state
	if(state.event <= 0)
		return false;
	//make sure cfg number is set and valid
	if(!setup || cfg_current < 1 || cfg_current > cfg_count)
		return false;
	//validate range of mbutton/modkeys
	if(mbutton < 0 || mbutton >= 1<<MC_NUM_BUTTONS)
		return false;
	if(modkeys < 0 || modkeys >= 1<<MC_NUM_MODKEYS)
		return false;
	//update state / positions
	switch(state.event) {
	case MC_MOUSEDOWN:
		state.cmd = 0;
		state.delta.x = state.delta.y = 0;
		state.relative.x = state.relative.y = 0;
		state.start.x = state.current.x;
		state.start.y = state.current.y;
		break;
	case MC_MOUSEUP:
	case MC_MOUSEMOVE:
		{
			int tmprelx = state.current.x - state.start.x;
			int tmprely = state.current.y - state.start.y;
			state.delta.x = tmprelx - state.relative.x;
			state.delta.y = tmprely - state.relative.y;
			state.relative.x = tmprelx;
			state.relative.y = tmprely;
		}
		break;
	case MC_MOUSEOVER:
		//all positions besides current are invalid
		state.cmd = 0;
	//	state.relative.x = state.relative.y = 0;
	//	state.delta.x = state.delta.y = 0;
	//	state.start.x = state.start.y = 0;
		break;
	}
	
	//see if we have a command available
	mouseconf_t *mct = &setup[cfg_current-1];
	MouseCommandList *mcl = mct->action[mbutton][modkeys];
	if(!mcl)
		return false;
		
	//we have a command list, find a command to exec
	MouseCommand *cmd;
	switch(state.event) {
	case MC_MOUSEDOWN: 
		// loop until command is handled
		cmd = mcl->GetItem(0);
		for(int i=0; cmd != 0; cmd = mcl->GetItem(++i)) {
			if(cmd->MouseDown(state)) {
				state.cmd = cmd;
				return true;
			}
		}
		break;
	case MC_MOUSEUP:
		if(state.cmd)
			state.cmd->MouseUp(state);
		return true;
	case MC_MOUSEMOVE:
		if(state.cmd)
			state.cmd->MouseMove(state);
		return true;
	case MC_MOUSEOVER:
		// loop until command is handled. mouseover uses list from current modkeys + left button
		cmd = mcl->GetItem(0);
		for(int i=0; cmd != 0; cmd = mcl->GetItem(++i)) {
			if(cmd->MouseOver(state))
				return true;
		}
		break;
	}
	return false;
}
//internal
/*
	INPUT:
		cfg_file: filename only of config. file must be in set->set_directory
*/
bool MouseConfig::parse_config(const char *cfg_file) 
{
	char path[MAX_PATH];
	snprintf(path,sizeof(path),"%s%s",set->set_directory, cfg_file);

	int handle = open(path,O_RDONLY | O_BINARY);
	if (handle == -1) {
		Msgbox("Error opening %s: %s",path, strerror(errno));
		return false;
	}
	int length = filelength(handle);
	close(handle);

	unsigned char *dat = new unsigned char[length+16];

	UTIL_LoadFile(path, dat);
	StartTokenParsing(dat);
	
	//determine number of setups
	if (!GetToken (true)) {
		Msgbox("Parsing Mouse: Number of Setups not found");
		goto bad_exit;
	}

	//set some arbitrary limitations...
	int numEventTypes = atoi(token);
	if (numEventTypes <= 0 || numEventTypes > 16) {
		Msgbox("Parsing Mouse: Number of Setups [1 min/ 16 max] invalid");
		goto bad_exit;
	}
	
	//clear and re-init setups
	clear_data();
	init_data(numEventTypes);
	

	if (!GetToken (true)) {
		Msgbox("Parsing Mouse: Current Setup not found");
		goto bad_exit;
	}
	curEventType = atoi(token);
	if (curEventType <= 0 || curEventType > numEventTypes) {
		Msgbox("Parsing Mouse: Current Setup invalid");
		goto bad_exit;
	}

	curEventType--; // adjust down...

	for (int i = 0; i < numEventTypes; i++) {

		if (!GetToken (true)) {
			Msgbox("Parsing Mouse: '{' not found");
			goto bad_exit;
		}
		if (!GetToken (true)) {
			Msgbox("Parsing Mouse: Current Setup Name not found");
			goto bad_exit;
		}

		//store setup name
		sprintf(setup[i].name, token);

		while (1) {
			if (!GetToken (true))
      			break;  // done...

			if (strcmpi (token, "{"))
				break;

 			if (!GetToken (false))
      			break;  // done...
	      
			//literally "get number of semicolons...". always returns >= 1
			int count = CountWads(token);
      	
      		//this is kind of arbitrary...
			if (count >= 32) {
      			Msgbox("Parse Mouse:  Too many mouse options [%i]...",count);
				goto bad_exit;
      		}

			//read and store command list
			static char tmp_commands[MAXTOKEN];
			strncpy(tmp_commands, token, sizeof(tmp_commands));
			
			if (!GetToken (false))
      			break;  // done...
      			
			int mbutton;
			int modkeys = 0;
			
      		//get mouse button
			if (!strcmpi(token,"LEFT"))
      			mbutton = MC_BTN_LEFT;
			else if (!strcmpi(token,"MIDDLE"))
      			mbutton = MC_BTN_MIDDLE;
			else if (!strcmpi(token,"RIGHT"))
      			mbutton = MC_BTN_RIGHT;
			else {
				Msgbox("Mouse Parse:  Bad mouse type...");
				goto bad_exit;
			}
			
			//get modkeys
			while (1) {
				if (!GetToken (false))
   					break;  // done...
				if (!strcmpi(token,"}"))
         			break;

				if (!strcmpi(token,"SHIFT"))
         			modkeys |= MC_MOD_SHIFT;
				else if (!strcmpi(token,"CTRL"))
         			modkeys |= MC_MOD_CTRL;
				else if (!strcmpi(token,"ALT"))
         			modkeys |= MC_MOD_ALT;
			} 
			
			//make sure an MCL exists in action slot
			if(!setup[i].action[mbutton][modkeys]) {
				setup[i].action[mbutton][modkeys] = new MouseCommandList;
			}
			
			char *start = tmp_commands;
			char *end;
			char cmdname[64];
			for (int c = 0; c < count; c++) {

				for (end = start; end && *end && *end != ';' && *end != ' '; end++);
				
				int toklen = end-start;
				if(toklen > 0 && toklen < sizeof(cmdname)) {
					strncpy(cmdname, start, toklen);
					cmdname[toklen] = 0;
					MouseCommand *cmd = command_list->GetItem(cmdname);
					setup[i].action[mbutton][modkeys]->Add(cmd);
				}
				start = ++end;
			}
		}
		
	} // end for...

	delete[] dat;
	return true;

bad_exit:
	
	delete[] dat;
	return false;
}

void MouseConfig::clear_data() {
	//clear any used slots in setup[]
	if(cfg_count > 0) {
		for(int i=0; i<cfg_count; i++) {
			for(int j=0; j<MC_NUM_BUTTONS; j++) {
				for(int k=0; k<(1<<MC_NUM_MODKEYS); k++) {
					if(setup[i].action[j][k])
						delete setup[i].action[j][k];
				}
			}
		}
	}
	delete[] setup;
	setup = 0;
	cfg_count = 0;
	cfg_current = 0;
}
void MouseConfig::init_data(int num_cfg) {
	if(setup != 0)
		clear_data();
		
	setup = new mouseconf_t[num_cfg];
	cfg_count = num_cfg;
	cfg_current = 0;
	//clear slots
	if(cfg_count > 0) {
		for(int i=0; i<cfg_count; i++) {
			for(int j=0; j<MC_NUM_BUTTONS; j++) {
				for(int k=0; k<(1<<MC_NUM_MODKEYS); k++) {
					setup[i].action[j][k] = 0;
				}
			}
		}
	}
}


