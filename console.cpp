#include "global.h"
//todo #include <richedit.h>

/////////////////////////////////////////////////////////////////////////////
/// tab complete
/////////////////////////////////////////////////////////////////////////////



#define COLUMN_WIDTH 20
#define COLUMN_MAX_WIDTH 190
void BSPConsole::display_columns(list<char*>& list)
{
    char buf[COLUMN_MAX_WIDTH + COLUMN_WIDTH + 1];
    int cols = max(COLUMN_WIDTH, textwidth - COLUMN_WIDTH);
    int p = 4;
    memset(buf, 0x20, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    for(listitem<char*> *i = list.items; i; i = i->next)
    {
        int len = strlen(i->data);
        strncpy(buf + p, i->data, len);
        int tmp = p + len;
        while(len >= 0)
        {
            p += COLUMN_WIDTH;
            len -= COLUMN_WIDTH;
        }
        if(p >= cols)
        {
            strcpy(buf + tmp, "\r\n");	//terminate line
            sysprintf(buf);

            memset(buf, 0x20, COLUMN_MAX_WIDTH);
            buf[COLUMN_MAX_WIDTH] = 0;
            p = 4;
        }
    }
    if(p != 4)
    {
        strcpy(buf + p, "\r\n");
        sysprintf(buf);
    }

}

void BSPConsole::tab_complete()
{
    //get selection size
    DWORD start = 0, end = 0;
    input->SendMessage(EM_GETSEL , (WPARAM)&start, (LPARAM)&end);
    if(start != end)
    {
        return;		//start != end
    }
    //get buffer length
    int len = input->SendMessage(WM_GETTEXTLENGTH , 0,0);
    if(len < 0)
    {
        return;		//no text in buffer
    }
    //get whole buffer
    char *buffer = new char[++len];
    input->SendMessage(WM_GETTEXT, len, (LPARAM)buffer);
    //find word that will be completed
    int i;
    for(i = start - 1; i >= 0; i--)
    {
        if(!(isalnum((unsigned char)buffer[i]) || buffer[i]=='_'))
            break;
    }
    DWORD wordstart = i + 1;
    if(wordstart >= start)
    {
        delete [] buffer;
        return;		//no word found to complete
    }

    for(i = start; buffer[i]; i++)
    {
        if(!(isalnum((unsigned char)buffer[i]) || buffer[i]=='_'))
            break;
    }
    int wordend = i;
    // we now have:  [word start]abc[tab start]xyz[word end]
    int prefixlen = start - wordstart;
    //int half2len = wordend - start;
    if(prefixlen <= 0)
    {
        delete [] buffer;
        return;		//no word found before tab
    }

    char prefix[64];
    strncpy(prefix, &buffer[wordstart], min(sizeof(prefix), prefixlen));
    prefix[prefixlen] = 0;

    list<char*> set_matches(false);
    list<char*> cmd_matches(false);
    char replset[64] = "";
    Cset_Complete(prefix, replset, sizeof(replset), &set_matches);
    char replcmd[64] = "";
    Ccmd_Complete(prefix, replcmd, sizeof(replcmd), &cmd_matches);

    //default to setting.. check for override...
    char * replacement = replset;
    //setting takes precedence
    if(*replset)
    {
        if(*replcmd)  	//match in both - need to take shortest match
        {
            if(strlen(replcmd) < strlen(replset))
                replacement = replcmd;
        }
    }
    else if(*replcmd)
        replacement = replcmd;

    //display
    if(set_matches.count > 1 && cmd_matches.count > 1)
    {
        //display set of settings and commands
        sysprintf(const_cast<char *> ("Settings:\r\n"));
        display_columns(set_matches);
        sysprintf(const_cast<char *> ("\r\nCommands:\r\n"));
        display_columns(cmd_matches);
    }
    else if (set_matches.count > 1)
    {
        //display settings only
        sysprintf(const_cast<char *> ("Settings:\r\n"));
        display_columns(set_matches);
    }
    else if (cmd_matches.count > 1)
    {
        //display commands only
        sysprintf(const_cast<char *> ("Commands:\r\n"));
        display_columns(cmd_matches);
    }

    //this replaces [wordstart] to [wordend] with replacement
    if(*replacement)
    {
        input->SendMessage(EM_SETSEL, wordstart, wordend);
        input->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)replacement);
    }

    delete [] buffer;
}


/////////////////////////////////////////////////////////////////////////////
/// ConsoleHistory
/////////////////////////////////////////////////////////////////////////////

ConsoleHistory::ConsoleHistory(int size)
    : head(0), tail(0), current(0), count(0), maxsize(size), activebuf(0)
{}
ConsoleHistory::~ConsoleHistory()
{
    clear();
}

bool ConsoleHistory::add(char *line)	//add a line, set current == head
{
    if(!maxsize || !line || !*line)
        return false;

    conhist_t *c = new conhist_t;

    c->line = new char[strlen(line) + 1];
    strcpy(c->line, line);

    c->next = 0;	//none after the first
    c->prev = head;	//old head is now 2nd item
    if(head) head->next = c;
    head = c;		//the first...
    current = 0;	//current is reset

    if(!count)
    {
        tail = c;	//set tail to first in list
    }
    count++;
    trim();
    return true;
}
char *ConsoleHistory::next()			//if next!=0, move to next item, return line
{
    if(!current)
        return 0;
    if(current == head)
    {
        current = 0;
        return activebuf;
    }
    current = current->next;
    char *ret = current->line;
    return ret;
}
char *ConsoleHistory::prev(char *active)	//if prev!=0, move to prev item, return line
{
    if(active && !current)
    {
        delete [] activebuf;
        activebuf = new char[strlen(active) + 1];
        strcpy(activebuf, active);
    }
    if(!current)
    {
        current = head;
        return current ? current->line : 0;
    }
    if(!current->prev)
        return 0;
    current = current->prev;
    char *ret = current->line;
    return ret;
}
void ConsoleHistory::clear()			//remove all items and reset
{
    conhist_t *c = head, *tmp;
    for( ; c; c = tmp)
    {
        delete[] c->line;
        tmp = c->prev;
        delete c;
    }
    delete [] activebuf;
    activebuf = 0;
    count = 0;
    head = 0;
    tail = 0;
    current = 0;
}
void ConsoleHistory::trim()				//adjust list to fit within maxsize
{
    if(maxsize <= 0)
    {
        if(count > 0)
            clear();
    }
    else
    {
        while(tail && count > maxsize)
        {
            if(tail->prev)
                syserror(const_cast<char *> ("trim console history: tail is being deleted, but tail has previous item"));
            if(current == tail)
                current = tail->next;		//bound current
            conhist_t *tmp = tail->next;	//get item closer to head
            delete [] tail->line;		    //delete text
            delete tail;					//delete tail
            tmp->prev = 0;					//new tail
            tail = tmp;
            count--;
        }
    }
}
void ConsoleHistory::setsize(int size)	//set max size and trim
{
    maxsize = size;
    trim();
}

/////////////////////////////////////////////////////////////////////////////
/// WConDisplay
/////////////////////////////////////////////////////////////////////////////

WConDisplay::WConDisplay(HWND parent,int id,char *text,int cx,int cy, int width,int height)
    : WEdit(parent,id,text,cx,cy,width,height,0,true)
{
    //todo?
    //LoadLibrary("riched20.dll");
    //wc.lpszClassName = RICHEDIT_CLASS;
}
LRESULT WConDisplay::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_MOUSEWHEEL:
        if(SetFocusUnderMouse(GetParent(hwnd)))
            return 0;
        break;

        //copy selected text automatically
    case WM_LBUTTONUP:
        {
            DWORD start = 0, stop = 0;
            int len = SendMessage(EM_GETSEL, (WPARAM)&start, (LPARAM)&stop);
            if(len > 0)
            {
                SendMessage(WM_COPY, 0,0);
            }
            SendMessage(EM_SETSEL,0xFFFF,0xFFFF);
            ::SetFocus(con->input->hwnd);
        }
        break;
    }
    return WControl::WndProc(msg,wParam,lParam);
}


/////////////////////////////////////////////////////////////////////////////
/// WConInput
/////////////////////////////////////////////////////////////////////////////

WConInput::WConInput(HWND parent,int id,char *text,int cx,int cy, int width,int height)
    : WEdit(parent,id,text,cx,cy,width,height,0,false)
{}
LRESULT WConInput::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_MOUSEWHEEL:
        if(SetFocusUnderMouse(GetParent(hwnd)))
            return 0;
        if(((short) HIWORD(wParam)) > 0)
            con->Scroll(-3);
        else
            con->Scroll(3);
        break;
    case WM_SETFOCUS:
        KillAccels = true;
        break;
    case WM_KILLFOCUS:
        KillAccels = false;
        break;
    case WM_KEYDOWN:
        switch(wParam)
        {

            //tab complete names for settings and commands
        case VK_TAB:
        {
            con->tab_complete();
            break;
        }

        //key to hide - hardcoded to F8 because user-set key might be alpha or such
        case VK_F8:
            CmToggleConsole();
            break;

        case VK_UP:
        {
            char *active = 0;
            int len = SendMessage(WM_GETTEXTLENGTH, 0,0);
            if(len > 0 && con->history->head && !con->history->current)
            {
                active = new char[len+2];
                SendMessage(WM_GETTEXT, len+1, (LPARAM)active);
            }
            char *line = con->history->prev(active);
            delete [] active;
            if(line)
                this->SetText(line);
            SendMessage(EM_SETSEL,0xFFFF,0xFFFF);
            return 0;
        }
        case VK_DOWN:
        {
            char *line = con->history->next();
            if(line)
                this->SetText(line);
            SendMessage(EM_SETSEL,0xFFFF,0xFFFF);
            return 0;
        }

        case VK_PRIOR:
        {
            con->PageUp();
            break;
        }
        case VK_NEXT:
        {
            con->PageDown();
            break;
        }

        //clear input buffer. if input buffer is already empty, hide the console.
        case VK_ESCAPE:
            if(this->SendMessage(WM_GETTEXTLENGTH,0,0) == 0)
            {
                CmToggleConsole();
                break;
            }
            SetText("");
            break;

            //process buffer
        case VK_RETURN:
            con->process_buffer();
        }
    }
    return WControl::WndProc(msg,wParam,lParam);
}


/////////////////////////////////////////////////////////////////////////////
/// BSPConsole
/////////////////////////////////////////////////////////////////////////////

BSPConsole::BSPConsole(HWND parent, char *caption)
    : TCWindow(parent,caption)
{
    attr.width = 550;
    attr.height = 180;
    attr.dwStyle |= WS_BORDER;

    input = 0;
    display = 0;
    bgbrush = 0;
    textwidth = 20;
    font = GetFont(set.console_font, -set.console_font_size);

    history = new ConsoleHistory();
}

BSPConsole::~BSPConsole()
{
    if(font) DeleteObject(font);
    if(bgbrush) DeleteObject(bgbrush);
    delete display;
    delete input;
    delete history;

}

LRESULT BSPConsole::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        display = new WConDisplay(hwnd, IDC_DISPLAY, const_cast<char *> (""),4,4,100,100);
        input = new WConInput(hwnd, IDC_INPUT,const_cast<char *> (""),4,104,100,20);

        input->attr.dwExStyle = 0;
        input->attr.dwStyle = WS_BORDER | WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL;
        display->attr.dwExStyle = 0;
        display->attr.dwStyle = WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL;

        display->con = this;
        input->con = this;
        display->Create();
        input->Create();
        display->SendMessage(WM_SETFONT,(WPARAM)font,0);
        input->SendMessage(WM_SETFONT,(WPARAM)font,0);
        break;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
        SetTextColor((HDC)wParam, set.color_consolefg);
        SetBkColor((HDC)wParam, set.color_consolebg);
        if(bgbrush) DeleteObject(bgbrush);
        bgbrush = CreateSolidBrush(set.color_consolebg);
        return (intptr_t)(LRESULT) bgbrush;
    case WM_SETFOCUS:
        if(this->IsVisible())
            ::SetFocus(input->hwnd);
        break;
    case WM_MOUSEWHEEL:
        if(SetFocusUnderMouse(hwnd))
            return 0;
        return 0;
    case WM_SIZE:
    {
        if(IsIconic() || wParam == SIZE_MINIMIZED)
            break;
        int w = (short)LOWORD(lParam) - 6;
        int h = (short)HIWORD(lParam) - 6;

        RECT rc;
        GetWindowRect(input->hwnd,&rc);
        int INP_HEIGHT = rc.bottom - rc.top;

        if (w > 0 && h > 0)
        {
            int dispheight = max(0, h - INP_HEIGHT);
            MoveWindow(display->hwnd,	3,	3,				w,	dispheight,		TRUE);
            MoveWindow(input->hwnd,		3,	3+dispheight,	w,	INP_HEIGHT,		TRUE);
        }
        HDC hdc = GetDC(input->hwnd);
        HFONT oldfont = (HFONT)SelectObject(hdc, font);
        SIZE size;
        GetTextExtentPoint32(hdc, "W", 1, &size);
        SelectObject(hdc, oldfont);
        ReleaseDC(input->hwnd, hdc);

        textwidth = size.cx > 0 ? w / size.cx : 20;
    }
    //fall...
    case WM_PAINT:
    {
        if(IsIconic() || wParam == SIZE_MINIMIZED)
            break;

        InvalidateRect(hwnd,0,TRUE);
        TCWindow::WndProc(WM_PAINT,wParam,lParam);

        HDC hdc = GetDC(hwnd);
        RECT rc;
        GetClientRect(hwnd, &rc);
        InflateRect(&rc,-1,-1);
        DrawEdge(hdc,&rc, EDGE_SUNKEN, BF_RECT);
        ReleaseDC(hwnd, hdc);
        return 0;
    }

    }

    return TCWindow::WndProc(msg,wParam,lParam);
}
bool BSPConsole::CanClose()
{
    if (!KillAllWindows)
    {
        ShowWindow(SW_HIDE);
    }
    return (0 != KillAllWindows);
}

//WindowPlacement
HWND BSPConsole::WP_GetHwnd()
{
    return hwnd;
}
const char *BSPConsole::WP_WindowName()
{
    return "Console";
}
//ScrollEnd
void BSPConsole::ScrollEnd()
{
    display->SendMessage(EM_LINESCROLL,-0xFFFF,0xFFFF);	//scroll to bottom
//	display->SendMessage(EM_LINESCROLL,0,-1);	//scroll up one
    if(::IsWindowVisible(display->hwnd))
        InvalidateRect(display->hwnd,0,true);

}
//Scroll
void BSPConsole::Scroll(int lines)
{
    display->SendMessage(EM_LINESCROLL,-0xFFFF,lines);
    if(::IsWindowVisible(display->hwnd))
        InvalidateRect(display->hwnd,0,true);

}
//Write
#define DISPLAY_SIZE_THRESHOLD 25000
#define DISPLAY_SIZE_TRIM       1000
void BSPConsole::Write(char *text)
{
    //trim buffer to avoid hitting size limit (30000)
    int tlen = display->SendMessage(WM_GETTEXTLENGTH,0,0);
    if(tlen > DISPLAY_SIZE_THRESHOLD)
    {
        display->SendMessage(EM_SETSEL, 0, tlen - DISPLAY_SIZE_THRESHOLD + DISPLAY_SIZE_TRIM);
        display->SendMessage(EM_REPLACESEL,0,(LPARAM)"");
    }
    //hack [^\r]\n|\r[^\n] into \r\n
    char *text2 = new char[strlen(text)*2+1];
    char *src=text, *dest=text2;
    *dest=0;
    while(*src)
    {
        if((*src=='\r' && *(src+1)=='\n' ) || (*src=='\r' && *(src+1)!='\n'))
        {
            *dest++='\r';
            *dest++='\n';
            src+=2;
        }
        else if(*src=='\n')
        {
            *dest++='\r';
            *dest++='\n';
            src++;
        }
        else *dest++ = *src++;
    }
    *dest=0;

    //write text: set cursor to end, insert text, scroll to bottom - 1 line
    display->SendMessage(EM_SETSEL,0xFFFF,0xFFFF);
    display->SendMessage(EM_REPLACESEL,0,(LPARAM)text2);
    display->SendMessage(EM_LINESCROLL,-0xFFFF,0xFFFF);	//scroll to bottom

    delete[]text2;

    if(::IsWindowVisible(display->hwnd))
        InvalidateRect(display->hwnd,0,true);

}
//WriteV
void BSPConsole::WriteV(char *text, va_list args)
{
    int len = _vscprintf(text, args) + 1;
    char *str = 0;
    if(len > 1)
    {
        str = new char[len];
        _vsnprintf (str, len, text, args);
    }
    Write(str);
    delete [] str;
}
//Writef
void BSPConsole::Writef(char *text, ...)
{
    va_list args;
    va_start (args, text);
    int len = _vscprintf(text, args) + 1;
    char *str = 0;
    if(len > 1)
    {
        str = new char[len];
        _vsnprintf (str, len, text, args);
    }
    va_end (args);
    Write(str);
    delete [] str;
}


void BSPConsole::PageUp()
{
    ::SendMessage(display->hwnd,EM_SCROLL, SB_PAGEUP,0);
//	::InvalidateRect(display->hwnd,0,1);
}

void BSPConsole::PageDown()
{
    ::SendMessage(display->hwnd,EM_SCROLL, SB_PAGEDOWN,0);
    //InvalidateRect(con->display->hwnd,0,1);
}

void BSPConsole::process_buffer()
{
    int len = input->SendMessage(WM_GETTEXTLENGTH, 0,0);
    char *buf = new char[len+2];
    input->SendMessage(WM_GETTEXT, len+1, (LPARAM)buf);

    history->add(buf);
    Write(const_cast<char *> (">> "));

    Parse_Trim(&buf);
    if(*buf)
    {
        Write(buf);

        Tokenizer tok(buf);
        if(tok.next(false))
        {
            ccmd_t *c;
            cset_t *s;
            //help
            if(!strcmp(tok.token,"?") || !stricmp(tok.token,"help"))
            {
                ShowHelp(tok.script_p);
                //clear
            }
            else if(!stricmp(tok.token, "clear") || !stricmp(tok.token, "cls"))
            {
                ClearScreen();
                //groups
            }
            else if(!stricmp(tok.token, "group") || !stricmp(tok.token, "g"))
            {
                ShowGroups(tok.script_p);
                //setting
            }
            else if(s = Cset_FindSet(tok.token))
            {
                Write(const_cast<char *> (" (setting)"));

                char key[MAXTOKEN];
                strncpy(key,tok.token,sizeof(key));
                Parse_Trim((char**)&tok.script_p); //mangles buf
                char *val = 0;
                if(*tok.script_p == '"')
                {
                    if(tok.next(false))
                        val = tok.token;
                }
                else
                {
                    val = tok.script_p;
                }
                if(val && *val && Cset_SetValue(key, val))
                {
                    Ccmd_UpdateEnablers();
                }
                else
                {
                    Write(const_cast<char *> ("\r\n   "));
                    Cset_PrintValue(s);
                }
                //command
            }
            else if(c = Ccmd_FindCmd(tok.token))
            {
                Write(const_cast<char *> (" (command)\r\n"));
                Ccmd_Execute(c->id);
                goto script_exit;
                //debug
            }
            else if(!stricmp(tok.token, "debug"))
            {
                Parse_Trim((char**)&tok.script_p);
                DebugCommands(tok.script_p);
            }
            else
            {
                Write(const_cast<char *> ("\r\n"));
                goto script_exit;
            }
        }
        if(tok.error != Tokenizer::success)
            Writef(const_cast<char *> ("\r\nError: %s on line %d\r\n"), tok.errormsg(), tok.error_line);
    }
    else
    {
        ScrollEnd();
    }
    Write(const_cast<char *> ("\r\n"));
script_exit:
    input->SetText(const_cast<char *> (""));
    delete [] buf;
}
void BSPConsole::DebugCommands(char *cmd)
{
    Write(const_cast<char *> ("\r\n"));
    //show setinfo holes
    if(!stricmp(cmd, "setinfo"))
    {
        cset_t *c = cset_all;
        for(; c; c = c->next)
        {
            if(!c->setinfo)
                Writef(const_cast<char *> ("Missing setinfo for \"%s\"\r\n"), c->name);
        }
    }
    else if(!stricmp(cmd,"cmdimg"))
    {
        ccmd_t *c = cmd_head;
        for(; c; c = c->next)
        {
            if(c->image < 0)
                Writef(const_cast<char *> ("Missing toolbar image for \"%s\"\r\n"), c->name);
        }
    }
    else if(!stricmp(cmd,"commands"))
    {
        ccmd_group_t *g = cmd_groups;
        for(; g; g = g->next)
        {
            Writef(const_cast<char *> ("\r\n%s\r\n"), g->name);
            ccmd_t *c = g->cmd_next;
            for(; c; c = c->group_next)
            {
                Writef(const_cast<char *> ("%s\r\n"), c->name);
                if(c->info_text)
                    Writef(const_cast<char *> ("%s\r\n"), c->info_text);
                else
                    Write(const_cast<char *> ("*** MISSING INFO TEXT ***\r\n"));
                Write(const_cast<char *> ("\r\n"));
            }
        }
    }
    else if(!stricmp(cmd,"settings"))
    {
        cset_group_t *g = cset_groups;
        for( ; g; g = g->next)
        {
            Writef(const_cast<char *> ("\r\n%s\r\n"), g->name);
            cset_t *c = g->head;
            for(; c; c = c->next_in_group)
            {
                Writef(const_cast<char *> ("%s - "), c->name);
                if(c->setinfo)
                {
                    if(c->setinfo->info)
                        Writef(const_cast<char *> ("%s\r\n"), c->setinfo->info);
                    else
                        Write(const_cast<char *> ("*** MISSING SETINFO->INFO ***\r\n"));
                    if(c->setinfo->values)
                    {
                        listitem<setinfo_value*> *li = c->setinfo->values->items;
                        for( ; li; li = li->next)
                        {
                            Writef(const_cast<char *> ("    %s - %s\r\n"), li->data->value, li->data->info);
                        }
                    }
                }
                else
                {
                    Write(const_cast<char *> ("*** MISSING SETINFO DATA ***\r\n"));
                }
                Write(const_cast<char *> ("\r\n"));

            }
        }
    }
    else
    {
        Write(const_cast<char *> ("Debug commands:\r\n"));
        Write(const_cast<char *> ("  cmdimg     list all commands missing toolbar images\r\n"));
        Write(const_cast<char *> ("  commands   list all commands by group with info text.\r\n"));
        Write(const_cast<char *> ("  setinfo    list all settings missing a setinfo defintion.\r\n"));
        Write(const_cast<char *> ("  settings   list all settings by group with info text.\r\n"));
    }
}
void BSPConsole::ClearScreen()
{
    display->SetText("");
}

void BSPConsole::ShowGroups(char *args)
{
    //trim args
    if(args)
        while(isspace((uintptr_t) *args)) args++;
    if(args && *args)
    {
        //show items in group
        cset_group_t *gmatch = 0, *g = cset_groups;
        ccmd_group_t *cmatch = 0, *c = cmd_groups;
        //find if "args" matches a group available in commands or settings
        for(; g; g=g->next)
            if(!stricmp(g->name, args))
            {
                gmatch = g;
                break;
            }
        for(; c; c=c->next)
            if(!stricmp(c->name, args))
            {
                cmatch = c;
                break;
            }
        if(!gmatch && !cmatch)
        {
            Writef(const_cast<char *> ("\r\nNo group found matching: %s\r\n"), args);
        }
        else
        {
            Write(const_cast<char *> ("\r\nGroup Items:\r\n"));
            list<char*> items(false);
            if(gmatch)
            {
                cset_t *cs = gmatch->head;
                for(; cs; cs=cs->next_in_group)
                    items.addsorted(cs->name);
            }
            if(cmatch)
            {
                ccmd_t *cc = cmatch->cmd_next;
                for(; cc; cc=cc->group_next)
                    items.addsorted(cc->name);
            }
            //display items
            display_columns(items);
        }
    }
    else
    {
        //list all groups
        Write(const_cast<char *> ("\r\nGroups:\r\n"));

        list<char*> groups(false);

        //add all setting groups to list
        cset_group_t *g = cset_groups;
        for(; g; g=g->next)
        {
            groups.addsorted(g->name);
        }
        //add all command groups, but check if exists first
        ccmd_group_t *c = cmd_groups;
        for(; c; c=c->next)  						//iterate over items to add
        {
            bool skip = false;
            listitem<char*> *i = groups.items;	//iterate over items in group
            for(; i; i = i->next)					//
                if(!stricmp(i->data, c->name))  	//compare existing name and name to add
                {
                    skip = true;				//
                    break;						//skip if already exists
                }
            if(!skip)
                groups.addsorted(c->name);				//add
        }

        //display groups
        display_columns(groups);
    }
}

void BSPConsole::ShowHelpItem(char *args)
{
    ccmd_t *c;
    cset_t *s;

    //setting
    if(s = Cset_FindSet(args))
    {
        Writef(const_cast<char *> ("\r\n"));
        Writef(const_cast<char *> ("   Group: %s\r\n"), (s->group ? s->group->name : const_cast<char *> ("none")));
        //display info text
        if(s->setinfo && s->setinfo->info)
            Writef(const_cast<char *> ("   Info: %s\r\n"), s->setinfo->info);
        Writef(const_cast<char *> ("   Value: "));
        Cset_PrintValue(s);
        Writef(const_cast<char *> ("\r\n"));
        //print out description from setinfo.cfg
        if(s->setinfo)
        {
            //display range for int/float
            if(s->setinfo->hasmin && s->setinfo->hasmax)
            {
                if(s->setinfo->type == CSET_INT)
                    Writef(const_cast<char *> ("   Value range: %d - %d\r\n"), s->setinfo->imin, s->setinfo->imax);
                else if(s->setinfo->type == CSET_FLOAT)
                {
                    char fmin[32], fmax[32];
                    snprintf(fmin, 32, "%f", s->setinfo->fmin);
                    snprintf(fmax, 32, "%f", s->setinfo->fmax);
                    Writef(const_cast<char *> ("   Value range: %s - %s\r\n"), FixFloatString(fmin), FixFloatString(fmax));
                }
            }
            else if(s->setinfo->hasmin)
            {
                if(s->setinfo->type == CSET_INT)
                    Writef(const_cast<char *> ("   Minimum value: %d\r\n"), s->setinfo->imin);
                else if(s->setinfo->type == CSET_FLOAT)
                {
                    char fmin[32];
                    snprintf(fmin, 32, "%f", s->setinfo->fmin);
                    Writef(const_cast<char *> ("   Minimum value: %s\r\n"), FixFloatString(fmin));
                }
            }
            else if(s->setinfo->hasmax)
            {
                if(s->setinfo->type == CSET_INT)
                    Writef(const_cast<char *> ("   Maximum value: %d\r\n"), s->setinfo->imax);
                else if(s->setinfo->type == CSET_FLOAT)
                {
                    char fmax[32];
                    snprintf(fmax, 32, "%f", s->setinfo->fmax);
                    Writef(const_cast<char *> ("   Maximum value: %s\r\n"), FixFloatString(fmax));
                }
            }
            //display values
            if(s->setinfo->values)
            {
                Writef(const_cast<char *> ("   Allowed values:\r\n"));
                listitem<setinfo_value*> *v = s->setinfo->values->items;
                for( ; v; v = v->next)
                {
                    Writef(const_cast<char *> ("     \"%s\": %s\r\n"), v->data->value, v->data->info);
                }
            }

        }

        //command
    }
    else if(c = Ccmd_FindCmd(args))
    {

        char shortcut[256];
        kbset->ShowAccelsForCommand(c->id, shortcut, sizeof(shortcut));

        Write(const_cast<char *> ("\r\n"));
        Writef(const_cast<char *> ("   Group: %s\r\n"), (c->group ? c->group->name : const_cast<char *> ("none")));
        Writef(const_cast<char *> ("   Info: %s\r\n"), c->info_text);
        if(*shortcut)
            Writef(const_cast<char *> ("   Keys: %s\r\n"), shortcut);
        //invalid
    }
    else
    {
        Write(const_cast<char *> ("   invalid command or setting\r\n"));
    }
}

void BSPConsole::ShowHelp(char *args)
{
    Write(const_cast<char *> ("\r\n"));
    if(args)
        while(isspace((unsigned char) *args)) args++;
    if(args && *args)
    {
        ShowHelpItem(args);
    }
    else
    {
        Write(const_cast<char *> ("Console commands:\r\n"));
        Write(const_cast<char *> ("   (command)          execute command\r\n"));
        Write(const_cast<char *> ("   (setting) [value]  view or change setting\r\n"));
        Write(const_cast<char *> ("   clear, cls         clear console window\r\n"));
        Write(const_cast<char *> ("   debug              debug menu\r\n"));
        Write(const_cast<char *> ("   group, g [item]    display groups, or items in specified group\r\n"));
        Write(const_cast<char *> ("   help, ? [item]     show this screen, or show help for [item]\r\n"));
    }
}
