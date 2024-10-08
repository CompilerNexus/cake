/*
 *  This file is part of cake compiler
 *  https://github.com/thradams/cake
*/

#pragma once
#include "ownership.h"
/*
* tag allow more than one type of object be inserted at the same map
*/
enum tag
{
    TAG_TYPE_NONE,
    
    TAG_TYPE_ENUN_SPECIFIER,
    TAG_TYPE_STRUCT_OR_UNION_SPECIFIER,

    TAG_TYPE_ENUMERATOR,
    TAG_TYPE_ONLY_DECLARATOR,
    TAG_TYPE_INIT_DECLARATOR,
};


struct map_entry {
    struct map_entry* _Owner _Opt next;
    unsigned int hash;
    char* _Owner key;
    void* p;
    enum tag type; /*type of the object pointed by p*/
};

struct hash_map {
    struct map_entry *_Owner * _Owner _Opt table;
    int capacity;
    int  size;
};

void hashmap_remove_all(struct hash_map* map);
void hashmap_destroy( struct hash_map* _Obj_owner map);
struct map_entry* _Opt hashmap_find(struct hash_map* map, const char* key);
void* _Opt hashmap_remove(struct hash_map* map, const char* key, enum tag* p_type_opt);
int hashmap_set(struct hash_map* map, const char* key, const void* p, enum tag type);


struct owner_map_entry {
    struct owner_map_entry* _Owner _Opt next;
    unsigned int hash;
    char* _Owner key;
    void* _Owner p;
    enum tag type; /*type of the object pointed by p*/
};

struct owner_hash_map {
    struct owner_map_entry *_Owner _Opt * _Owner _Opt table;
    int capacity;
    int  size;
};

void owner_hashmap_remove_all(struct owner_hash_map* map, void (*pf)(void* _Owner ));
void owner_hashmap_destroy( struct owner_hash_map* _Obj_owner map, void (*pf)(void*));
struct owner_map_entry* _Opt owner_hashmap_find(struct owner_hash_map* map, const char* key);
void*  _Owner _Opt owner_hashmap_remove(struct owner_hash_map* map, const char* key, enum tag* p_type_opt);
void* _Owner _Opt owner_hashmap_set(struct owner_hash_map* map, const char* key, const void* _Owner p, enum tag type);

