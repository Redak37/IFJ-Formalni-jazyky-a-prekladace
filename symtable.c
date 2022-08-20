/*  Knihovna Symtable
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *          Josef Oškera, VUT FIT (xosker03)
 *
 *  Datum:  10.11.2018
 *
 */

#include"symtable.h"


//struktura tabulky
//ktera se vnitrne v modulech pretypovava z htab_t a naopak
struct htab_s {
    unsigned size;
    unsigned arr_size;
    struct htab_listitem *strukp[];
};

struct htab_listitem {
    char *key;
    void *data;
    struct htab_listitem *next;
};


typedef struct htab_listitem litem_t;

//hash funkce ze zadani
static unsigned int htab_hash_function(const char *str);

//stara verze
static struct htab_listitem *htab_lookup_add_old(htab_t t, char *key);


//Odstrani vsechny polozky v tabulce
void htab_clear(htab_t t)
{
    struct htab_listitem *item;
    struct htab_listitem *item2;

    if(!t)
        return;

    for (unsigned i = 0; i < t->arr_size; ++i) {
        item = t->strukp[i];
        t->strukp[i] = NULL;

        while (item) {
            item2 = item->next;
            free(item->key);
            free(item->data);
            free(item);
            item = item2;
        }
    }

    t->size = 0;
}

//Vraci ukazatel na polozku v tabulce pokud existuje polozka s odpovidajicim klicem, jinak 'NULL'
struct htab_listitem *htab_find(htab_t t, char *key)
{
    struct htab_listitem *item = t->strukp[htab_hash_function(key) % t->arr_size];

    while (item) {
        if (!strcmp(item->key, key))
            return item;

        item = item->next;
    }

    return NULL;
}

//Najde a vrati ukazatel na polozku seznamu daneho klice, pokud neexistuje prida ji a vrati, pri chybe vrati 'NULL'
void **htab_lookup_add(htab_t t, char *key)
{
    unsigned i = htab_hash_function(key) % t->arr_size;
    struct htab_listitem *item = htab_find(t, key);

    if (item) {
        free(key);
        //item->data++;
        return (void**) &item->data;
    }

    item = malloc(sizeof(struct htab_listitem));

    if (!item)
        return NULL;

    item->next = t->strukp[i];
    t->strukp[i] = item;
    t->size++;

    item->data = NULL;
    item->key = key;

    return (void**) &item->data;
}

//Odstrani polozku daneho klice z tabulky, pokud existuje a vrati true, jinak vrati false
bool htab_remove(htab_t t, char *key)
{
    unsigned i = htab_hash_function(key) % t->arr_size;
    struct htab_listitem *item = t->strukp[i];

    if (item) {
        if (!strcmp(item->key, key)) {
            struct htab_listitem *item2 = item->next;
            t->size--;
            free(item->key);
            free(t->strukp[i]);
            t->strukp[i] = item2;
            return true;
        }
    }

    while (item->next) {
        if (!strcmp(item->next->key, key)) {
            struct htab_listitem *item2 = item->next->next;
            t->size--;
            free(item->next->key);
            free(item->next);
            item->next = item2;
            return true;
        }

        item = item->next;
    }

    return false;
}

//vrati velikost arr_size
unsigned htab_bucket_count(htab_t t) {
    return t->arr_size;
}

void *htab_data(htab_t t, char *key) {
    unsigned hash = htab_hash_function(key) % t->arr_size;
    struct htab_listitem *aktual = t->strukp[hash];

    while (aktual) {
       if (strcmp(aktual->key, key) == 0) {
           return (aktual->data);
       }
       aktual = aktual->next;
    }

    return NULL;
}

//pro každý zaznam vykona funkci
void htab_foreach(htab_t t, void (*func)(char *, void *)) {
    struct htab_listitem *aktual;
    for(unsigned i = 0; i < t->arr_size; ++i) {
        aktual = t->strukp[i];
        while(aktual) {
            func(aktual->key, aktual->data);
            aktual = aktual->next;
        }
    }
}

//free tabulka
void htab_free(htab_t t){
    htab_clear(t);
    free(t);
}

//inicializuje tabulku
htab_t htab_init(unsigned size) {
    if (!size)
        return NULL;

    htab_t t = malloc(sizeof(struct htab_s) + size * sizeof(litem_t*));

    if (!t)
        return NULL; //chybaPam();

    t->size = 0;
    t->arr_size = size;
    for (unsigned i = 0; i < size; ++i)
        t->strukp[i] = NULL;

    return t;
}

//presune t do nove inicializovane tabulky
//konkretne vytvari kopie zaznamu a pak stere smaže
htab_t htab_move(unsigned newsize, htab_t t){
    htab_t t2 = htab_init(newsize);
    if (!t2)
        return NULL;
    //znovupridani zaznamu do nove tabulky
    for (unsigned i = 0; i < t->arr_size; ++i) {
        struct htab_listitem *aktual = t->strukp[i];
        while (aktual) {
            struct htab_listitem *novy = htab_lookup_add_old( t2, aktual->key);
            if (!novy) {  //pokud se nezdaril malloc funkce konci
                free(t2);
                return NULL;
                //break;
            }
            novy->data = aktual->data;
            aktual = aktual->next;
        }
    }
    t->size = 0;
    //void_foreach(0,(void*)t, free);
    htab_clear(t);

    for (unsigned i = 0; i < t->arr_size; ++i)
        t->strukp[i] = NULL;

    return (void*) t2;
}

//vrati pocet prvku tabulky == pocet zaznamu
unsigned htab_size(htab_t t){
    return t->size;
}


//zkopiruje string/literar do noveho stringu
char *gS(char *data){
    if (!data || !data[0])
        return NULL;
    int delka = strlen(data) + 1;
    char *ret = malloc(sizeof(char) * delka);
    if(!ret){
        fprintf(stderr, "SYMTABLE:gS: neni ram\n");
        return NULL;
    }
    memcpy(ret, data, delka);
    return ret;
}

void htab_foreach_spec(htab_t t,void *parametr, void (*func)(int*, char*, symbol*)){
    for(unsigned i = 0; i < t->arr_size; ++i){
        struct htab_listitem *aktual = t->strukp[i];
        while(aktual){
            func(parametr, aktual->key, (symbol*)aktual->data);
            aktual = aktual->next;
        }
    }
}

void htab_foreach_super_spec(htab_t t, int *parametr, char *parametr2, void (*func)(int*, char*, char*, symbol*)){
    for(unsigned i = 0; i < t->arr_size; ++i){
        struct htab_listitem *aktual = t->strukp[i];
        while(aktual){
            func(parametr, parametr2, aktual->key, (symbol*)aktual->data);
            aktual = aktual->next;
        }
    }
}




//sdbm hash function //http://www.cse.yorku.ca/~oz/hash.html
static unsigned int htab_hash_function(const char *str) {
    unsigned long hash = 0;
    int c;

    while ((c = *str++) != '\0')
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

//Najde a vrati ukazatel na polozku seznamu daneho klice, pokud neexistuje prida ji a vrati, pri chybe vrati 'NULL'
static struct htab_listitem *htab_lookup_add_old(htab_t t, char *key)
{
    unsigned i = htab_hash_function(key) % t->arr_size;
    struct htab_listitem *item = htab_find(t, key);

    if (item) {
        free(key);
        //item->data++;
        return item;
    }

    item = malloc(sizeof(struct htab_listitem));

    if (!item)
        return NULL;

    item->next = t->strukp[i];
    t->strukp[i] = item;
    t->size++;

    item->next = NULL;
    item->data = NULL;
    item->key = key;

    return item;
}
