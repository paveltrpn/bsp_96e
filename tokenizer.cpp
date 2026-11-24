#include "global.h"


//
// Tokenizer class
//
Tokenizer::Tokenizer(char *data)
    : line(1), error(success), error_line(0), script_p(data), unget(false)
{
    token[0] = 0;
    getGroups = false;
    curLock = false;
    curGroup = 0;
    script_start = data;
}
Tokenizer::~Tokenizer() {}
void Tokenizer::begin()
{
    token[0] = 0;
    getGroups = false;
    curLock = false;
    curGroup = 0;
    script_p = script_start;
    error = success;
    line = 0;
    error_line = 0;
    unget = false;
}
//"Returns true if there is another token on the line"
int Tokenizer::avail_line()
{
    char *search_p = script_p;

    while ( *search_p <= 32)
    {
        if (*search_p=='\0' || *search_p == '\n')
            return 0;
        search_p++;
    }
    //check if next char is comment
    return (*search_p != ';' && *search_p != '#' &&
            !(*search_p=='/' && (search_p[1]=='*' || search_p[1]=='/')));
}
void Tokenizer::skipline()
{
    if(error) return;
    while (*script_p++ != '\n')
        if (!*script_p)
        {
            error_line = line;
            error = err_eof;
            return;
        }
    line++;
}
void Tokenizer::putback()
{
    unget = true;
}

/*
TODO
this code will probably return an array of tokens. .. . but... could probably be done better.
void Tokenizer::clear_array()
{
	if(!t_array) return;
	//last pointer in array is null
	char **p = t_array;
	for(;p;p++) delete [] *p;
	delete [] t_array;
	t_array = 0;
}
//get multiple tokens. returns number of tokens read. if ret < count, then at EOF
int Tokenizer::get_array(int count, bool crossline, char ***array)
{
	if(count < 1) return 0;
	if(error) return 0;
	if(!array) return 0;
	clear_array();
	*t_array = new char*[count + 1];
	memset(t_array,0,count*sizeof(char*));
	int retval = 0;
	for( ; count > 0 ; count--) {
		if(!next(crossline))
			break;
		int len = strlen(token);
		t_array[retval] = new char[len + 1];
		strncpy(array[t_array++], token, len+1);
	}
	array = t_array;
	return retval;
}
*/
bool Tokenizer::next(bool crossline)
{
    if (unget)                           // is a token already waiting?
    {
        unget = false;
        return true;
    }
    //lets unget reset if there is an error
    if(error) return false;

//
// skip space
//
skipspace:
    while (*script_p <= 32)
    {
        if (!*script_p)
        {
            if (!crossline)
            {
                error_line = line;
                error = err_eol;
            }
            return false;
        }
        if (*script_p++ == '\n')
        {
            if (!crossline)
            {
                error_line = line;
                error = err_eol;
                return false;
            }
            line++;
        }
    }

    // ; and # are comment line markers
    if (*script_p == ';' || *script_p == '#')
    {
        if (!crossline)
        {
            error_line = line;
            error = err_eol;
            return false;
        }
        while (*script_p++ != '\n')
            if (!*script_p)
            {
                if (!crossline)
                {
                    error_line = line;
                    error = err_eol;
                }
                return false;
            }
        line++;
        goto skipspace;
    }

    // / * * / style comments... remember to increment line counting
    if (script_p[0] == '/' && script_p[1] == '*')  	// comment field
    {

        // skip the /*
        script_p+=2;

        while (!(script_p[0] == '*' && script_p[1] == '/'))
        {
            script_p++;
            if (!*script_p)
            {
                if (!crossline)
                {
                    error_line = line;
                    error = err_eol;
                }

                line++;
                return false;
            }

            if (*script_p == '\n')
                line++;
        }

        // Skip the */
        script_p += 2;
        goto skipspace;
    }

    // "//" Style comments.  In BSP, we parse the group info here...
    if (script_p[0] == '/' && script_p[1] == '/')	// comment field
    {
        script_p+=2;

        // get the GROUP number, if none, set GROUP Flag to -1
        if (getGroups)
            GroupParse(this);

        while (*script_p++ != '\n')
        {
            if (!*script_p)
            {
                if (!crossline)
                {
                    error_line = line;
                    error = err_eol;
                    return false;
                }
            }
        }
        line++;
        goto skipspace;
    }

    //
    // copy token
    //
    char *token_p = token;

    if (*script_p == '"')
    {
        // Skip start "
        script_p++;
        while ( *script_p != '"' )
        {
            if (!*script_p)
            {
                error_line = line;
                error = err_eof;
                return false;
            }
            *token_p++ = *script_p++;
            if (token_p == &token[MAXTOKEN])
            {
                error_line = line;
                error = err_too_large;
                return false;
            }
        }
        // Skip end "
        script_p++;
    }
    else while ( *script_p > 32 )
        {
            *token_p++ = *script_p++;
            if (token_p == &token[MAXTOKEN])
            {
                error_line = line;
                error = err_too_large;
                return false;
            }
        }

    *token_p = 0;

    return true;
}
const char *Tokenizer::errormsg()
{
    switch(error)
    {
    case success:
        return "Success";
    case err_eof:
        return "Unexpected EOF";
    case err_eol:
        return "Unexpected end of line";
    case err_too_large:
        return "Token too large";
    }
    return "unknown error";
}
