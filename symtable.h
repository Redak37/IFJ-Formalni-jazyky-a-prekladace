#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

/*  Knihovna symtable.h
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *          Josef Oškera, VUT FIT (xosker03)
 *
 *  Datum:  10.11.2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


struct htab_listitem;

typedef struct htab_s *htab_t;

#define PROMENY_POC_PARAM -1

typedef struct promena_do_tab{
    bool formal;
    int vyskyt;
    int radek;
    //pro fce:
    int pocet_param;
    htab_t ukNaTab;

    int pocet_volani;
    int pocet_volani_max;
    char *volam[];
} symbol;



//Vytvori prazdnou tabulku s velikosti size a vrati ukazatel na ni.
htab_t htab_init(unsigned size);

//Presune tabulku do nove tabulky s velikosti newsize a vrati ukazatel na novou tabulku.
htab_t htab_move(unsigned newsize, htab_t t);

//Vraci pocet polozek v tabulce.
unsigned htab_size(htab_t t);

//Vraci velikosti tabulky.
unsigned htab_bucket_count(htab_t t);

//Pridava polozku, nebo inkrementuje jeji vyskyt a vraci ukazatel na jeji data
void **htab_lookup_add(htab_t t, char *key);

//Pokud najde polozku, vrati na ni ukazatel, jinak vrati NULL.
struct htab_listitem *htab_find(htab_t t, char *key);

//Pokud najde polozku, vrati ukazatel na jeji data, jinak vrati NULL.
void *htab_data(htab_t t, char *key);

//Pro kazdou polozku tabulky se spusti funkce.
void htab_foreach(htab_t t, void (*func)(char *, void *));

//Smaze polozku tabulky.
bool htab_remove(htab_t t, char *key);

//Smaze obsah tabulky.
void htab_clear(htab_t t);

//Smaze tabulku a jeji obsah.
void htab_free(htab_t t);

//Vytvori dynamicky alokovany string s obsahem literalu.
char *gS(char *data);

void htab_foreach_spec(htab_t t,void *parametr, void (*func)(int*, char*, symbol*));
void htab_foreach_super_spec(htab_t t, int *parametr, char *parametr2, void (*func)(int*, char*, char*, symbol*));


#endif
