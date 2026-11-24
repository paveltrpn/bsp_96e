#include "global.h"
#include <share.h>

//char token[MAXTOKEN];
//bool unget;
//unsigned char *script_p;
//int scriptline;

char com_token[MAXCOMTOKEN];
bool com_eof;

//=============================================
//debug: helps find the last critical code that ran before a crash, or shows problem loops. function is
//defined to an empty macro in global.h when not needed so no overhead is created by its presence.
#ifndef LoopProblem
void LoopProblem(char *s)
{
    frame->SetCaption(s);

    /*	MSG msg;
    	if (PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)) {
       	if ((int)msg.wParam == VK_F12) {
            	sysfatal("Loop didn't end in %s",s);
          };
       };
      */
}
#endif


//=============================================
bool isPowerof2(int num)
{
    return num > 0 && (num & (num-1))==0;
}

int nextPowerof2(int num)
{
    int retval = 1;
    while(retval < num) retval <<= 1;
    return retval;
}


//=============================================
//stristr! find a substring with case insensitive search
char *stristr(char *src, char *find)
{
    if(!find || !src || !*src)
        return 0;
    if(!*find)
        return src;
    int srclen = (int) strlen(src);
    int findlen = (int) strlen(find);
    if(findlen > srclen)
        return 0;
    int len = srclen - findlen;
    for(int i = 0; i <= len; i++)
    {
        if(toupper((unsigned char)src[i]) == toupper((unsigned char)find[0]))
        {
            if(!_strnicmp(&src[i], find, findlen))
            {
                return &src[i];
            }
        }
    }
    return 0;
}
//=============================================
char *RemoveTrailingSlash(char *dir)
{
    int len = strlen(dir);
    while (len > 2 && (dir[len - 1] == '\\' || dir[len - 1] == '/'))
        dir[--len] = 0;
    return dir;
}
//=============================================
int VerifyPak(Path pakname)
{
    FILE *f;
    pakheader_t pakheader;
    const char err_title[] = "BSP Loader";
    char outstr[256];

    if (!pakname.Exists())
    {
        sprintf(outstr,"Couldn't find pak file [%s]...",(char*)pakname);
        MessageBox(0,outstr,err_title,MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    f = fopen(pakname,"rb");
    if (!f)
    {
        sprintf(outstr,"Unable to open pak file\n%s",(char*)pakname);
        MessageBox(0,outstr,err_title,MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    fread(&pakheader,sizeof(pakheader_t),1,f);
    fclose(f);

    if (!strnicmp((char *)pakheader.magic,"PACK",4))
        return 1;

    sprintf(outstr,"Invalid pak file [%s]...",(char*)pakname);
    ::MessageBox(0,outstr,err_title,MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

void StripExtension (char *path)
{
    int length = (int) strlen(path)-1;
    while (length > 0 && path[length] != '.')
    {
        length--;
        if (path[length] == '\\')
            return;		// no extension
    }
    if (length)
        path[length] = 0;
}

//expand_path - in path and out path must be MAX_PATH sized
DWORD expand_path(char *path, char *out, int outsize)
{
    //make relative paths work from main bsp dir
    SetCurrentDirectory(set.main_dir);
    return GetFullPathName(path,outsize,out,0);
}
bool file_exists(char *fname)
{
    char buf[MAX_PATH];
    expand_path(fname,buf);
    int ret = _access(buf, 0);
    return ret != -1;
}

int rint(float i)
{
    if (i > 0.0f)
        return (int)floor(i + 0.5f);
    else if (i < 0.0f)
        return (int)ceil(i - 0.5f);
    return 0;
}
//==================================================================
bool Bsp_ShellExecute(HWND hwnd,char *op,Path file, char*param, char *start_dir, int show)
{
    bool retval = false;
    DWORD seret = 0;
    seret = (DWORD_PTR) ShellExecute(hwnd, op, file, param, start_dir, show);
    if(seret > 32)
        retval = true;
    else
    {
        char *errmsg = const_cast<char *> ("unknown error");
        switch(seret)
        {
        case 0:
        case SE_ERR_OOM:
            errmsg = const_cast<char *> ("Out of memory");
            break;
        case ERROR_FILE_NOT_FOUND:
            errmsg = const_cast<char *> ("File not found");
            break;
        case ERROR_PATH_NOT_FOUND:
            errmsg = const_cast<char *> ("Path not found");
            break;
        case SE_ERR_DLLNOTFOUND:
            errmsg = const_cast<char *> ("DLL not found");
            break;
        case ERROR_BAD_FORMAT:
            errmsg = const_cast<char *> ("Invalid executable file");
            break;
        case SE_ERR_ACCESSDENIED:
        case SE_ERR_SHARE:
            errmsg = const_cast<char *> ("Access Denied / Sharing violation");
            break;
        }
        syserror(const_cast<char *> ("ShellExecute failed: %s"), errmsg);
    }
    return retval;
}
/*
==================================================================
new parsing functions
==================================================================
*/
//advance ptr to next line
void Parse_NextLine(char **text)
{
    if(!text || !*text) return;
    while(**text && **text != '\n') (*text)++;
    if(**text == '\n') (*text)++;
}
//right trim
void Parse_RTrim(char **text)
{
    if(!text || !*text) return;
    int len = (int)strlen(*text) - 1;
    while(len >= 0 && isspace((unsigned char)(*text)[len]))
    {
        (*text)[len] = 0;
        len--;
    }
}
//left trim
void Parse_LTrim(char **text)
{
    if(!text || !*text) return;
    char *s = *text;			//copy pointer (src)
    char *d = s;				//2nd copy (dst)
    while(isspace((unsigned char)*s)) s++;		//eat space
    if(s != d)  				//pointers are different
    {
        while (*s) *d++ = *s++;	//for make glorious copy
        *d = 0;					//terminate
    }
}
//trim
void Parse_Trim(char **text)
{
    Parse_LTrim(text);
    Parse_RTrim(text);
}

void Parse_EatWS(char **text)
{
    while(isspace((unsigned char)**text)) (*text)++;
}
//read quoted string withOUT escapes.
int Parse_Quoted(char **text, char *out, int outlen)
{
    if(**text != '"')
        return 0;
    char *outstr = out;
    char **save = text;
    (*text)++;
    while(**text != '"')
    {
        if(!**text || **text == '\r' || **text == '\n')  		// not allowed...
        {
            *out = 0;				// clear
            text = save;			// reset
            return 0;				// gbye
        }
        if(outlen > 1)  			// is space available
        {
            *outstr++ = **text;		// output
            outlen--;				// track num chars used...
        }
        (*text)++;					// next..
    }
    *outstr = 0;					// terminate outstr
    (*text)++;						// advance past last quote
    return 1;						// good
}

/*
==================================================================
new map parsing
==================================================================

parse text into color. handle formats: %d(from palette) and %d %d %d
  assumes no leading ws
  returns -1 on failure. valid color always has 0 in high byte

  Valid formats:
		ABC012		- hex
		#789ABC		- hex
		123 234 12	- decimal rgb
		255			- palette
*/
COLORREF Parse_Color(char *text)
{
    if(!text || !*text) return -1;

    //
    if(*text == '#') text++;

    //hex num
    for(int i=0; i<=6; i++)
    {
        if(i < 6 && !isxdigit((unsigned char)text[i]))
            break;
        else if(i==6 && !isalnum((unsigned char)text[i]))
        {
            int x;
            if(1 == sscanf(text,"%x",&x))
                return (COLORREF) ((x & 0xFF)<<16) | ((x>>16) & 0xFF) | (x & 0xFF00) ;	//change endian
        }
    }

    //decimal
    int r, g, b;
    int num = sscanf(text,"%d %d %d",&r,&g,&b);
    if(num == 3)
        return RGB(r, g, b);
    //palette index
    if(num == 1)
    {
        unsigned char *rgb = &set.pal[min(max(r,0),255) * PALSTEP];
        return (COLORREF) (((DWORD)rgb[0])) | (((DWORD)rgb[1])<<8) | (((DWORD)rgb[2])<<16) ;
    }
    return -1;
}

//check line for format "//BSPGROUPS0001" and return number portion. -1 if line does not match
int Parse_BSPGroups(char *text)
{
    if(strncmp(text,"//BSPGROUPS",11))	//must match exactly...
        return -1;
    text += 11;
    char buf[8];
    strncpy(buf,text,4);			//need 4 digits
    buf[4] = 0;
    return atoi(buf);				//ok, atoi will return 0 on error
}

//check line for format "//BSPGROUPINFO"name" "rgb color" "visible""
//returns non-zero on success

int Parse_BSPGroupInfo(char *text, groupinfo_t *gi)
{
    if(strncmp(text,"//BSPGROUPINFO",14))	//must match exactly...
        return 0;
    text += 14;
    // group name
    Parse_EatWS(&text);
    if(!Parse_Quoted(&text, gi->name, sizeof(gi->name)))
        return 0;
    // group color
    Parse_EatWS(&text);
    char buf[32];
    if(!Parse_Quoted(&text, buf, sizeof(buf)))
        return 0;
    gi->color = Parse_Color(buf);
    if(-1 == gi->color)
        gi->color = set.color_foreground;	//recover
    // vis
    Parse_EatWS(&text);
    if(!Parse_Quoted(&text, buf, sizeof(buf)))
        return 0;
    if(1 != sscanf(buf,"%d",&gi->visible))
        gi->visible = 1;				//recover

    return 1;
}

//check line for format "//BSPFAVORITES0000" and return number portion. -1 if line does not match
int Parse_BSPFavorites(char *text)
{
    char buf[8];
    if(strncmp(text,"//BSPFAVORITES",14))	//must match exactly...
        return -1;
    text += 14;
    strncpy(buf,text,4);			//need 4 digits
    buf[4] = 0;
    return atoi(buf);				//ok, atoi will return 0 on error
}
//get next texture name in ;delim list. return 0 when no texname copied
int Parse_BSPNextFav(char **text, char *fav, int favlen)
{
    char *fp = fav;
    char **tp = text;

    favlen--;							//save byte for null
    while(favlen && **text!=';' && **text>0x13)
    {
        *fp++ = **text;					//copy tex
        (*text)++;						//
        favlen--;						//count bufsize
    }
    *fp = 0;							//terminate copy
    while(**text == ';') (*text)++;		//eat the "next" semicolon
    Parse_Trim(&fav);					//trim..
    return *fav ? 1 : 0;				//return 1 if a string was copied
}
//cameras

int Parse_BSPCameras(char *text)
{
    if(strncmp(text,"//BSPCAMERAS",12))	//must match exactly...
        return -1;
    text += 12;
    char buf[8];
    strncpy(buf,text,4);			//need 4 digits
    buf[4] = 0;
    return atoi(buf);				//ok, atoi will return 0 on error
}
int Parse_BSPCamInfo(char *text, camerainfo_t *ci)
{
    if(text[0] != '/' && text[1] != '/' && text[2] != '"') return 0;
    text+=3;
    if(6 != sscanf(text, "%f %f %f %i %i %i", &ci->x,&ci->y,&ci->z, &ci->roll,&ci->pitch,&ci->yaw))
        return 0;
    return 1;
}
int Parse_BSPMapType(char *text)
{
    char buf[8];
    if(strncmp(text,"//BSPMAPTYPE",12))	//must match exactly...
        return -1;
    text += 12;
    strncpy(buf,text,4);			//need 4 digits
    buf[4] = 0;
    return atoi(buf);				//ok, atoi will return 0 on error
}


/*
==============
COM_Parse
Parse a token out of a string
==============
*/
char *COM_Parse(char *data)
{
    int		c;

    com_eof = false;

    int len = 0;
    com_token[0] = 0;

    if (!data)
        return NULL;

    // skip whitespace
skipwhite:
    while ( (c = *data) <= ' ')
    {
        if (!c)
        {
            com_eof = true;
            return NULL;			// end of file;
        }
        data++;
    }

// skip // comments
    if (c=='/' && data[1] == '/')
    {
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }


// handle quoted strings specially
    if (c == '"')
    {
        data++;
        for ( ; ; len++)
        {
            c = *data++;
            if (c=='"')
            {
                com_token[len] = '\0';
                return data;
            }
            com_token[len] = (char)c;
        }
    }

// parse single characters
    if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
    {
        com_token[len] = (char)c;
        len++;
        com_token[len] = '\0';
        return data+1;
    }

// parse a regular word
    do
    {
        com_token[len] = (char)c;
        data++;
        len++;
        c = *data;
        if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
            break;
    }
    while (c>32);

    com_token[len] = '\0';
    return data;
}

#define READBUF 2000	//keep small because it goes on the stack

//get contents of a file, return 0 on error or pointer that must be delete[]'d
unsigned char *file_get_contents(Path filename, int *outlen)
{
    if(!filename.valid)
        return 0;	//bad filename
    int f = _sopen(filename,_O_RDONLY | _O_BINARY, _SH_DENYNO, 0);
    if(f == -1)
        return 0;	//bad file

    int len = _filelength(f);

    if(len > 0)  	//must have contents
    {
        unsigned char *retval = new unsigned char[len+1];
        unsigned char *out = retval;
        int bytesread;
        unsigned char buffer[READBUF];
        while(bytesread = _read(f,buffer,READBUF))
        {
            if(bytesread < 0)
            {
                //	printf("error:%d f:%d\n",errno,f);
                delete [] retval;
                _close(f);
                return 0;
            }
            memcpy(out, buffer, bytesread);
            out += bytesread;
            *out = 0;
        }
        if(outlen) *outlen = len;	//return length if requested
        _close(f);
        return retval;
    }
    _close(f);
    return 0;
}


// BSP Specific Parsing of Group Number for a particular brush in the .map
//
//CHANGES: this function should return true/false. true if it successfully reads
//         a group number, false otherwise. when returning false, the caller should
//         treat the current line as a comment and skip the line.

bool GroupParse(Tokenizer *script)
{
    const int maxDigits = 8;
    int i;      // Counter for number of digits read...

    // Assume unlocked and in default group, in case of trouble.

    char *script_p = script->script_p;
    // Set globals...
    script->curGroup = 0;
    script->curLock  = false;

    // Group is up to maxDigits long and surrounded by " marks
    //validate...
    if (script_p[0] != '"' || !isdigit((unsigned char)script_p[1]))
        return false;
    for(i=2; i<=maxDigits; i++)
    {
        if(script_p[i] == '"')
        {
            break;
        }
        else if(!isdigit((unsigned char)script_p[i]))
        {
            return false;
        }
    }
    if(script_p[i] != '"')	// in case maxDigits was hit and next char is not quote
        return false;

    script->curGroup = atoi((char*)&script_p[1]);

    // Now get lock status
    // It's the number of chars in the group name, plus:
    //  a quote
    //  a space
    //  another quote
    //
    script->curLock = false;
    //CHANGED: if this pattern is not met, exit and assume not locked
    if(script_p[i+1] == ' ' && script_p[i+2] == '"' && isdigit((unsigned char)script_p[i+3]) && script_p[i+4] == '"')
        script->curLock = (script_p[i+3] == '1');
    return true;
}

/**
	compare two ascii strings of digit characters. stops cmp at first nondigit

	-positive numbers [0-9]* only.
	-strings "001" and "1" are equal.
	-string order example: "1", "16", "099"

	RETURN:
		-2 unknown error / strings not numeric
		-1 if string1 < string2
		 0 if string1 = string2
		 1 if string1 > string2
	*/
int compare_numeric_strings(char *string1, char *string2)
{
    if(!string1 || !string2 || !isdigit((unsigned char)*string1) || !isdigit((unsigned char)*string2))
        return -2;
    //ignore leading zeroes
    while(*string1 == '0') string1++;
    while(*string2 == '0') string2++;
    //get lengths of digits strings, if one is longer it must be a bigger number
    int str1cnt = 0, str2cnt = 0;
    for(int i=0; isdigit((unsigned char)string1[i]); i++) str1cnt++;
    for(int i=0; isdigit((unsigned char)string2[i]); i++) str2cnt++;
    if(str1cnt < str2cnt)
        return -1;
    if(str2cnt < str1cnt)
        return 1;
    //strings are same length, so find if one has a greater leading digit
    for(int i=0; i < str1cnt; i++)
    {
        if(string1[i] < string2[i])
        {
            return -1;
        }
        else if(string2[i] < string1[i])
        {
            return 1;
        }
    }
    return 0;	//strings are equal
}
/**
	compare two strings, compare repeated numerical portions as a lexical unit

	-strings "test001" and "test1" are equal
	-strings "a1a001a1a01" and "a01a01a1a001" are equal
	-string order example: "", "1", "2", "test", "test1", "test16", "test099", "testalpha"

	RETURN:
		-2 unknown error
		-1 if string1 < string2
		 0 if string1 = string2
		 1 if string1 > string2
*/

int compare_strings_int_parts(char *string1, char *string2)
{
    while(*string1 && *string2)
    {
        if(isdigit((unsigned char)*string1) && isdigit((unsigned char)*string2))
        {
            int n = compare_numeric_strings(string1, string2);
            if(n == -2)  	//shouldnt happen...
            {
                return -2;
            }
            else if(!n)  	//numeric strings match... continue
            {
                while(isdigit((unsigned char)*string1)) string1++;
                while(isdigit((unsigned char)*string2)) string2++;
            }
            else  		//strings are different, return numeric cmp result
            {
                return n;
            }
        }
        else
        {
            char s1 = toupper(*string1);
            char s2 = toupper(*string2);
            if(s1 < s2)
                return -1;
            else if(s2 < s1)
                return 1;
            string1++;
            string2++;
        }
    }

    if(!*string1 && *string2)	//string1 < string2
        return -1;
    if(*string1 && !*string2)	//string2 < string1
        return 1;
    if(!*string1 && !*string2)	//strings match
        return 0;

    return 0;
}

#ifndef __MINGW32__
int snprintf(char *buffer, size_t count, const char *format, ...)
{
    va_list		argptr;
    va_start(argptr,format);
    int retval = _vsnprintf(buffer,count,format,argptr);
    va_end (argptr);
    buffer[count-1] = 0;	//force null term
    return retval;
}
#endif

void Msgbox(char *msg, ...)
{
    va_list		argptr;
    static char		str[1024];

    va_start (argptr,msg);
    _vsnprintf (str,sizeof(str),msg,argptr);
    str[sizeof(str) - 1] = 0;
    va_end (argptr);
    size_t oldread = set.Map_Read;
    set.Map_Read = 0;
    if(client)
        MessageBox(client->hwnd,str,"BSP",64);
    else
        MessageBox(0,str,"BSP",64);
    set.Map_Read = oldread;
}
void Caption(char *msg, ...)
{
    va_list		argptr;
    static char		str[512];

    va_start (argptr,msg);
    _vsnprintf (str,sizeof(str),msg,argptr);
    str[sizeof(str)-1] = 0;
    va_end (argptr);
    frame->SetCaption(str);
}

void LogFileReset(Path logfile)
{
    FILE *fp = fopen(logfile,"w+b");
    if(fp) fclose(fp);
}
void LogFileWriteV(Path logfile, const char *txt, va_list args)
{
    int len = _vscprintf(txt, args) + 1;
    if(len <= 1) return;

    FILE *fp = fopen(logfile,"a+b");
    if (!fp) return;

    char *str = new char[len];
    _vsnprintf (str,len,txt,args);
    fprintf(fp,"%s",str);
    fclose(fp);
    delete [] str;
}
void LogFileWrite(Path logfile, const char *txt)
{
    FILE *fp = fopen(logfile,"a+b");
    if (!fp) return;

    fprintf(fp,"%s",txt);
    fclose(fp);
}

void sysfatal (char *error, ...)
{
    va_list		argptr;
    char		str[1024];

    delete splash;
    splash = NULL;

    static bool in_error = false;
    if (in_error)
        exit(0);
    in_error = true;

    va_start (argptr,error);
    _vsnprintf (str,sizeof(str),error,argptr);
    str[sizeof(str) - 1] = 0;

    LogFileWrite (const_cast<char *> (BSP_LOG), "*** ");
    LogFileWriteV(const_cast<char *> (BSP_LOG), error, argptr);
    LogFileWrite (const_cast<char *> (BSP_LOG), "\r\n");

    va_end (argptr);

    // FIXME, try to save .map file HERE!

    MessageBox(0,str,"BSP Error...",MB_OK);
    exit(0);
}
//write to console, or log file if console isnt initialized
void sysprintv(char *text, va_list vargs)
{
    if(!wconsole)
        LogFileWriteV(const_cast<char *> (BSP_LOG), text, vargs);
    else
        wconsole->WriteV(text, vargs);
}
//write to console, or log file if console isnt initialized
void sysprintf(char *text, ...)
{
    va_list vargs;
    va_start(vargs, text);
    sysprintv(text, vargs);
    va_end(vargs);
}
//force write to log file, write to console if initialized
void syserror(char *text, ...)
{
    va_list vargs;
    va_start(vargs, text);
    LogFileWrite (const_cast<char *> (BSP_LOG), "*** ");
    LogFileWriteV(const_cast<char *> (BSP_LOG), text, vargs);
    LogFileWrite (const_cast<char *> (BSP_LOG), "\r\n");
    if(wconsole)
    {
        wconsole->Write(const_cast<char *> ("*** "));
        wconsole->WriteV(text, vargs);
        wconsole->Write(const_cast<char *> ("\r\n"));
    }
    va_end(vargs);
}

// ExtractFileBase - turn c:\dir\bsp.exe into "bsp" or "bsp.exe"
void ExtractFileBase (char *path, char *dest, int includeExtension)
{
    if (strlen(path))
    {
        char *src = path + strlen(path) - 1;

        // back up until a \ or the start
        for( ; (src != path) && (*(src-1) != '\\') && (*(src-1) != '/') ; src--);


        if (!includeExtension)
        {
            while (*src)
                *dest++ = *src++;
        }
        else
        {
            while (*src && *src != '.')
                *dest++ = *src++;
        }
    }
    *dest = 0;
}

void FixOverflow(float& number, float range)
{
    while (number >= range)
        number = number  - range;
    while (number < 0)
        number = number + range;
}


//
// change the focus to the window under the mouse, for mousewheel
//

bool SetFocusUnderMouse(HWND hwnd)
{
    if(client)
    {
        POINT pt;
        GetCursorPos(&pt);
        HWND th = WindowFromPoint(pt);
        //need to make sure the handle we change focus to belongs to BSP
        if(hwnd != th && (GetParent(th) == client->hwnd || GetParent(GetParent(th)) == client->hwnd))
        {
            SetFocus(th);
            return true;
        }
    }
    return false;
}


void CenterWindow(HWND hwnd)
{
    RECT rc;
    GetWindowRect(hwnd,&rc);

    int w = rc.right-rc.left;
    int h = rc.bottom-rc.top;

    rc.left =  GetSystemMetrics(SM_CXSCREEN)/2 - w/2;	// center width
    rc.top  =  GetSystemMetrics(SM_CYSCREEN)/2 - h/2;	// center height
    rc.right = rc.left+w;
    rc.bottom = rc.top+h;

    MoveWindow(hwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,true);
}

//chop trailing zeroes past tenths place
char *FixFloatString(char *fstr)
{
    if(!fstr)
        return 0;

    int decimal = 0;
    for(int i=0; !!fstr[i]; i++)
    {
        if(0 == decimal && fstr[i]=='.')  	// find the decimal
        {
            decimal = i + 1;
            break;
        }
    }
    if(decimal > 0)
    {
        for(int i=(int)strlen(fstr)-1; i > decimal; i--)
        {
            if(fstr[i] != '0')
                break;
            fstr[i] = 0;
        }
    }
    return fstr;
}

//LoadImage replacement
HBITMAP load_bitmap(Path filename, int flags)
{
    HBITMAP sbmp = (HBITMAP) LoadImage(hInstance,filename,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    if(!sbmp)
        return 0;
    HDC sdc = CreateCompatibleDC(0);
    DeleteObject(SelectObject(sdc,sbmp));

    HDC rdc = CreateCompatibleDC(0);

    BITMAP bm;
    GetObject(sbmp, sizeof(bm), &bm);

    BITMAPINFO bi;
    memset(&bi,0,sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;

    UINT * pixels;
    HBITMAP rbmp = CreateDIBSection(rdc, &bi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
    HGDIOBJ oldbmp = SelectObject(rdc, rbmp);

    BitBlt(rdc,0,0, bm.bmWidth,bm.bmHeight, sdc,0,0,SRCCOPY);

    if(flags & LB_3DCOLORS)
    {
        BYTE *p = (BYTE*)pixels;
        int length = bm.bmWidth * bm.bmHeight;
        COLORREF ltgray = GetSysColor(COLOR_3DLIGHT);
        COLORREF gray = GetSysColor(COLOR_3DFACE);
        COLORREF dkgray = GetSysColor(COLOR_3DSHADOW);
        for( ; length > 0; length--, p += 3)
        {
            if(p[0]==192 && p[1]==192 && p[2]==192)
            {
                p[2] = GetRValue(gray);
                p[1] = GetGValue(gray);
                p[0] = GetBValue(gray);
            }
            else if(p[0]==128 && p[1]==128 && p[2]==128)
            {
                p[2] = GetRValue(dkgray);
                p[1] = GetGValue(dkgray);
                p[0] = GetBValue(dkgray);
            }
            else if(p[0]==223 && p[1]==223 && p[2]==223)
            {
                p[2] = GetRValue(ltgray);
                p[1] = GetGValue(ltgray);
                p[0] = GetBValue(ltgray);
            }
        }
    }

    SelectObject(rdc,oldbmp);
    DeleteDC(rdc);
    DeleteDC(sdc);
    DeleteObject(sbmp);
    return rbmp;
}










