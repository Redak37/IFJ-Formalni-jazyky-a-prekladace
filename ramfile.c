/*  Knihovna RamFile
 *
 *  Autor:  Josef Oskera, VUT FIT (xosker03)
 *          Jan Juda, VUT FIT (xjudaj00)
 *
 *  Datum: 14.11.2018
 *
 */


#include "ramfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>


//maximalni tisknutelna velikost, pokud ji uzivatel nezvetsi
#define MAX_VEL_RADEK 256

//ramfile je linearne vazany seznam struktur obsahujici radek
//a ukazatel na navazujici radek
typedef struct radek {
    struct radek *down;
    char data[];
} rf_row;



static rf_row *first_row;   //ukazatel na prvni radek
static rf_row *last_row;    //ukazatel na posledni radek
static int pocet_radku;     //pocet radku

//vraci ukazatel na ukazatel na strukturu radku
//pokud je cislo moc velke tak vraci ukazatel na rf_row.down posledniho prvku
//do poarametru poloha zapise pozici kde lezi radek
static rf_row **ukazRadku(int x, int *poloha){
    rf_row **act = &first_row;

    //pokud je ramfile prazdny
    if(first_row == NULL){
        if(poloha) *poloha = 0;
        return &first_row;
    }
    //pokud hledame nulty radek
    if(x == 0){
        if(poloha) *poloha = 0;
        return &first_row;
    }

    int i;
    //pruchod seznamem nez narazime na pozadovane cislo nebo konec seznamu
    for(i = 0; i < x; ++i){
        if((*act)->down == NULL){
            if(poloha) *poloha = i+1;
            return &(*act)->down;
        }
        act = &(*act)->down;
    }
    if(poloha) *poloha = i;
    return act;
}


///////////////////////////////////////////////////////////////////////////
//vraci pocet aktualne zapsanych radku
int ramFilePocetRadku(void){
    return pocet_radku;
}

//prohodi vnitrni ramfile za druhy
void ramFileSwap(int *zapisovany_radek, int novy_stav){
    //uchovani potrebnych hodnot
    static rf_row *first_row_0 = NULL;
    static rf_row *last_row_0 = NULL;
    static int pocet_radku_0 = 0;
    static int zap_radek_0 = 0;;

    static rf_row *first_row_1 = NULL;
    static rf_row *last_row_1 = NULL;
    static int pocet_radku_1 = 0;
    static int zap_radek_1 = 0;;

    static int aktualni_stav = 0;

    //zalohovani dulezitych hodnot
    switch(aktualni_stav){
        case 0: {
            first_row_0 = first_row;
            last_row_0 = last_row;
            pocet_radku_0 = pocet_radku;
            if(zapisovany_radek) zap_radek_0 = *zapisovany_radek;
            break;
        }
        case 1: {
            first_row_1 = first_row;
            last_row_1 = last_row;
            pocet_radku_1 = pocet_radku;
            if(zapisovany_radek)  zap_radek_1 = *zapisovany_radek;
            break;
        }
        default : {
            return;
        }
    }

    //nacteni dulezitych hodnot
    switch(novy_stav){
        case 0: {
            first_row = first_row_0;
            last_row = last_row_0;
            pocet_radku = pocet_radku_0;
            if(zapisovany_radek)  *zapisovany_radek = zap_radek_0;
            break;
        }
        case 1: {
            first_row = first_row_1;
            last_row = last_row_1;
            pocet_radku = pocet_radku_1;
            if(zapisovany_radek)  *zapisovany_radek = zap_radek_1;
            break;
        }
    }
    aktualni_stav = novy_stav;
}

//vytiskne zpravu na posledni radek do ramfile
//parametry:    0 --- zvetseni maximalni tisknutelne delky,
//              1 az n ---stejne parametry jako printf...
void pisKonec(int max_delka_plus, const char *fmt, ...){
    char *tmp = calloc((MAX_VEL_RADEK + max_delka_plus), sizeof(char));
    va_list param;
    va_start(param, fmt);
    vsnprintf(tmp, (MAX_VEL_RADEK + max_delka_plus), fmt, param);
    va_end(param);

    //vytvoreni orezane kopie o znaky za '\0' vytisknuteho radku
    int delka = strlen(tmp);
    rf_row *r = malloc(sizeof(rf_row) + (delka + 1) * sizeof(char));
    r->down = NULL;
    memcpy(r->data, tmp, delka+1);
    free(tmp);

    if(!first_row) first_row = r;
    if(last_row) last_row->down = r;
    last_row = r;

    pocet_radku++;
}

//vytiskne zpravu na pozici, nebo na konec pokud je ramfile kratsi
//vraci polohu zapsaneho radku
//parametry:    0 --- cislo radku,
//              1 --- zvetseni maximalni tisknutelne delky,
//              2 az n ---stejne parametry jako printf...
int pisNa(int cislo_radku, int max_delka_plus, const char *fmt, ...){
    char *tmp = calloc((MAX_VEL_RADEK + max_delka_plus), sizeof(char));
    va_list param;
    va_start(param, fmt);
    vsnprintf(tmp, (MAX_VEL_RADEK + max_delka_plus), fmt, param);
    va_end(param);

    //vytvoreni orezane kopie o znaky za '\0' vytisknuteho radku
    int delka = strlen(tmp);
    rf_row *r = malloc(sizeof(rf_row) + (delka + 1) * sizeof(char));
    r->down = NULL;
    memcpy(r->data, tmp, delka+1);
    free(tmp);

    int ret;
    rf_row **t =ukazRadku(cislo_radku, &ret);

    //osetreni prazdneho ramfilu
    if(first_row == NULL){
        last_row = r;
    }

    if(last_row == NULL){
        last_row = r;
    }

    //vkladame na posledni radek
    if(t == (void*) last_row){
        last_row = r;
    }

    pocet_radku++;
    r->down = *t;
    *t = r;
    return ret;
}

//vrati ukazatel na radek
//pokud je cislo moc velke tak vraci null
char *vypisNa(int cislo){
    if(cislo >= pocet_radku)
        return NULL;
    rf_row **t =ukazRadku(cislo, NULL);
    if(*t == NULL)
        return NULL;

    return (**t).data;
}

//vymaze radek podle cisla (na chtene pozici)
//pokud je cislo moc velke tak vymaze posledni
//vraci zbyvajici pocet radku
int vymazNa(int cislo){
    if(cislo >= pocet_radku)
        cislo = pocet_radku - 1;

    rf_row **t = ukazRadku(cislo, NULL);
    if(*t == NULL)
        return pocet_radku;

    rf_row *tmp = *t;
    if(*t){
        *t = (*t)->down;
        free(tmp);
        pocet_radku--;
    }

    //osetreni toho kdyz mazany je posledni nebo prvni
    if(*t == NULL){
        if(first_row != NULL)
            last_row = (void*) t;
        else
            last_row = NULL;
    }
    return pocet_radku;
}


//funkce vracejici unikatni cislo pro kazde zavolani
int dejUID(void){
  static int cislo = 0;
  return cislo++;
}
