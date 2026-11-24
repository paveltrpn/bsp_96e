#ifndef _ENT_H_
#define _ENT_H_

class map;

typedef struct epair_s
{
    struct epair_s	*next;
    char	key[MAX_KEY];
    char	value[MAX_VALUE];
} epair_t;

// an Entity is a list of brush objects, with additional key / value info

class map;
class EntityClass;

class Entity
{
public:

    Entity *p_prev, *p_next;

    bool invalid;
    int group;
    bool modifiable;
    EntityClass *ec;

    int count;
    SetBrush objects;
    map *owner;

    vec3_t origin;

    epair_t	*epairs;

    Entity *undoCameFrom;

    //CACHE - for common key types
    char *classname;
    char *targetname;
    char *target;
    char *angle;

// METHODS
    Entity();
    ~Entity();

    Entity *undoCopy();
    Entity *undoCopyNoBrushes();

    void freeObjects(int killtoo);
    void addObject(SetBrush *);
    void removeObject(SetBrush *);

    bool lookForBrush(SetBrush *b);
    void appendList(Entity *e);

    Entity *copy();
    void empty();

    int numPairs();
    void freeAll();
    void initClass(char *classname);
    int initFromTokens(map *parent, Tokenizer &script);
    void setModifiable(bool m);
    char *get_targetname();
    void writeToFILE(FILE *f,int,int, map *);
    char *valueForQKey(char *k);
    void getVector(vec3_t v, char *k);
    void setKey(char *k,char *v);
    void removeKeyPair(char *key);
    void setOriginKey();
    SetBrush *createFixedBrush(vec3_t org);
};

#endif	//_ENT_H_

