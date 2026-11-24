#pragma once
/* shitty code yuck */
#include <string.h>
#include "global.h"

//mini-list
template <typename T1>
struct listitem
{
    T1 data;
    listitem *next;
};
template <typename T2>
class list
{
public:
    inline list(bool delete_data = true) : items(0), tail(0), count(0), delete_data(delete_data) {}
    inline ~list()
    {
        reset<T2>();
    }
    template <typename T3>
    inline void reset()
    {
        listitem<T3> *item = items, *tmp;
        for(; item; item = tmp)
        {
            if(delete_data)
                delete item->data;
            tmp = item->next;
            delete item;
        }
        this->count = 0;
        this->tail = 0;
        this->items = 0;
    }
    template <typename T4>
    inline void add(T4 data)
    {
        listitem<T4> *item = new listitem<T4>;
        item->data = data;
        item->next = 0;
        if(!items)
        {
            items = tail = item;
        }
        else
        {
            tail->next = item;
            tail = item;
        }
        ++count;
    }
//	addsorted specialized for char*
//  fixme need to turn into template
    typedef listitem< char* > licharptr;
    inline void addsorted(char* data)
    {
        licharptr *item = new licharptr;
        item->data = data;
        item->next = 0;

        if(!items)
        {
            items = tail = item;
        }
        else
        {
            licharptr *ATLS_iter = items;

            /* add item to list with one or more items */
            licharptr **ATLS_previns = &items;	    /* insert point, start at list head */

            for(/**/; ATLS_iter; ATLS_iter = ATLS_iter->next)
            {
                int compare_strings_int_parts(char *, char *);
                if(compare_strings_int_parts(item->data, ATLS_iter->data) <= 0)
                {
                    item->next = ATLS_iter;	        /* next item is current iter item */
                    (*ATLS_previns) = item;		    /* previous item points to new item */
                    break;
                }

                if(!ATLS_iter->next)  	            /* before next... */
                {
                    ATLS_iter->next = item;
                    break;
                }
                ATLS_previns = &ATLS_iter->next;	/* remember insert point */
            }


            if(!item->next)
            {
                tail = item;
            }
        }
        ++count;
    }

    //var
    listitem<T2> *items, *tail;
    int count;
    bool delete_data;
};

/* g++ seems to miss these ? */
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

class Str
{
public:
    char *str;
    size_t length;
    //create a new string, length
    Str(char *fmt,...)
    {
        va_list vargs;
        va_start(vargs, fmt);
        length = _vscprintf(fmt, vargs);
        str = new char[length + 1];
        vsprintf(str,fmt,vargs);
        va_end(vargs);
    }
    Str(size_t minsize, char *fmt,...)
    {
        va_list vargs;
        va_start(vargs, fmt);
        length = max(minsize, (size_t) _vscprintf(fmt, vargs));
        str = new char[length + 1];
        vsprintf(str,fmt,vargs);
        va_end(vargs);
    }
    char *detach()
    {
        char *retval=str;
        str=0;
        return retval;
    }
    ~Str()
    {
        delete[] str;
    }
    operator char *()
    {
        return str;
    }
};
