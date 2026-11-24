#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "global.h"

class Tokenizer
{
public:
    Tokenizer(char *data);
    ~Tokenizer();

    int avail_line();				//equiv !TokenAvail
    void skipline();			//read to end of line
    bool next (bool crossline);	//next token
    void putback();				//current token will be returned on next()
    void begin();				//move to beginning of data

    char token[MAXTOKEN];
    bool unget;
    char *script_p;	//points to byte after current token
    char *script_start;
    int line;
    bool getGroups;	//try to parse groups in comments
    int curGroup;
    bool curLock;
    typedef enum error_e {success, err_eof, err_eol, err_too_large} tok_err;
    tok_err error;
    int error_line;
    const char *errormsg();
    //todo error reset
};

#endif	//_TOKENIZER_H_
