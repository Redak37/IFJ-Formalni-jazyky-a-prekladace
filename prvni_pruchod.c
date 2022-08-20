/*  Prvni pruchod
 *
 *  Autor:  Josef Oškera, VUT FIT (xosker03)
 *          Jan Juda, VUT FIT (xjudaj00)
 *          Jan Dolezel, VUT FIT (xdolez81)
 *
 *  Datum:  15.11.2018
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include "prvni_pruchod.h"
#include "scanner.h"
#include "matematika.h"
#include "symtable.h"


extern htab_t tab_fce;
extern htab_t tab_lokmain;

//zjisti zda funkce uz je volana z aktivni funkce
bool jeTam(symbol *fce, char *hledan){
    for(int i = 0; i < fce->pocet_volani; ++i){
        if(!strcmp(fce->volam[i], hledan)){
            return true;
        }
    }
    return false;
}

typedef struct kaskada_volani{
    char *fce;
    struct kaskada_volani *next;
}kas_vol;

//zjisti zda funkce byla jiz volana v aktualni lince volani
bool bylVolan(const kas_vol *const first, char *hledan){
    kas_vol *akt = (kas_vol*) first;
    do{
        if(!strcmp(hledan, akt->fce)){
            return true;
        }
        akt = akt->next;
    }while(akt->next);
    return false;
}

//rekurzivni funkce pro vyhledani volani nedefinovane funkce
int nejsemDefined(int hloubka, const int radek_volani, const char *hledany, const kas_vol *const first, kas_vol *akt){
    akt->fce = (char*) hledany;
    hloubka += strlen(hledany) + 5;
    if(hledany == NULL){
        return 99;
    }
    symbol *fce = (symbol*) htab_data(tab_fce, (char*) hledany);
    if(fce == NULL){
        return ERR_SEM_DEF;
    }
    if(fce->radek > radek_volani){
        return ERR_SEM_DEF;
    }
    //pro kazdou funkci co vola aktualni funkce
    for (int i = 0; i < fce->pocet_volani; i++) {
        akt->next = calloc(1, sizeof(kas_vol));
        if(!bylVolan(first, fce->volam[i])){
            int ret = nejsemDefined(hloubka + 1,radek_volani, fce->volam[i], first, akt->next);
            if(ret){
                return ret;
            }
        }
        free(akt->next);
    }
    return 0;
}


void jeTamVolanKdekoliv(int *ret, char *hledan, char *struct_name, symbol *fce){
    (void) struct_name;
    (*ret) |= jeTam(fce, hledan);
}

/*
Semanticky analyzator (tzv. Prvni pruchod, zkracene PP) linearnim pruchodem prochazi pole tokenu,
pri kterem pridava do prislusne tabulky symbolu funkce a promenne u kterych si zaznamena kde jsou definovany.
Dale kontroluje zda promenna neni volana jako funkce. Take kontroluje zda je v miste volani funkce definovana.
Pri pruchodu funkcemi si do dat funkce v symtable pridava informace o volanich, ktera tato funkce provadi.
Pote co PP narazi na volani funkce z tela programu, se rekurzivne overuje zda volana funkce
je v tomto miste definovana a to i pro vsechna jeji volani. Pri teto cinnosti vytvari linearni seznam volani
pro zastaveni kontroly pripadne rekurze jakehokoliv druhu.
*/
int PP (int lines, token **pole){
    //inicializace struktury pro rizeni pozice
    sa_pos p = {    0,
                    0,
                    lines-1,
                    delkaRadku(lines, lines-1, pole) - 1,
                    false,
                    0,
                    0
    };

    bool akt_fce = false;
    char *akt_fce_name = NULL;
    int vnoreni = 0;
    htab_t aktivni_lok = tab_lokmain;
    //pole[p.odR][p.odS].id
    do {
        //zpracovani funkce
        if (pole[p.odR][p.odS].id == DEF){
            if(akt_fce || vnoreni){
                fprintf(stderr, "CHYBA:PP:DEF: Definice funkce zasahuje do prikazu! (jina def, if nebo cyklus)\n");
                return ERR_SEM_JINAK;
            }
            p = posunHlavu(p, 1, lines, pole);

            //vytvoreni noveho zaznamu v symtable pro novou funkci
            symbol **funkce = (symbol**) htab_lookup_add(tab_fce, gS(pole[p.odR][p.odS].attr));
            (*funkce) = calloc(1, sizeof(symbol) + 6 * sizeof(char*));
            if(!(*funkce)){
                fprintf(stderr, "CHYBA:PP:DEF calloc\n");
                return ERR_SYS;
            }
            //vytvoreni tabulky promennych konkretni funkce
            (*funkce)->ukNaTab = htab_init(64);
            (*funkce)->radek = p.odR;
            (*funkce)->pocet_volani_max = 6;
            aktivni_lok = (*funkce)->ukNaTab;
            akt_fce = true;
            akt_fce_name = pole[p.odR][p.odS].attr;

            p = posunHlavu(p, 1, lines, pole);
            //zivorka (

            //zpracovani parametru
            int pocet_param = 0;
            int pocet_carek = 0;
            while(1){
                p = posunHlavu(p, 1, lines, pole);
                if(pole[p.odR][p.odS].id == ZAV_END){
                    break;
                }
                if(pole[p.odR][p.odS].id == IDENTIFIKATOR){
                    symbol *fce = htab_data(tab_fce, pole[p.odR][p.odS].attr);
                    if (fce) {
                        fprintf(stderr, "Redefinice funkce jako formalni parametr\n");
                        return ERR_SEM_DEF;
                    }
                    //pridavani argumentu funkce do tabulky funkce
                    symbol **promena = (symbol**) htab_lookup_add(aktivni_lok, gS(pole[p.odR][p.odS].attr));
                    (*promena) = calloc(1, sizeof(symbol));
                    (*promena)->vyskyt = 1;
                    (*promena)->radek = p.odR;
                    (*promena)->formal = true;
                    pocet_param++;
                } else if(pole[p.odR][p.odS].id == CARKA){
                    pocet_carek++;
                } else {
                    fprintf(stderr, "CHYBA:PP:DEF: identifikator\n");
                    return  ERR_SYS;
                }
            }
            (*funkce)->pocet_param = pocet_param;


            p = posunHlavu(p, 1, lines, pole);
            //konec radku

        }
        ////////////////////////////////////////////////////////////////////////////
        else if (pole[p.odR][p.odS].id == END){
            if(vnoreni){
                vnoreni--;
            }
            else if(akt_fce){
                aktivni_lok = tab_lokmain;
                akt_fce = false;
                akt_fce_name = NULL;
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        else if (pole[p.odR][p.odS].id == IF || pole[p.odR][p.odS].id == WHILE){

            vnoreni++;
        }
        ////////////////////////////////////////////////////////////////////////////
        else if (pole[p.odR][p.odS].id == IDENTIFIKATOR){
            //vyhledani zda identifikator neni funkce
            symbol *fce = htab_data(tab_fce, pole[p.odR][p.odS].attr);
            char *hledan = pole[p.odR][p.odS].attr;
            if (fce) {
                fce->vyskyt++;
                p = posunHlavu(p, 1, lines, pole);
                if (pole[p.odR][p.odS].id == PRIRAD){
                    return ERR_SEM_DEF;
                }
                p = posunHlavu(p, -1, lines, pole);
                if(akt_fce){
                    //pridani zaznamu o volani do dat aktivni funkce
                    symbol **nad_fce = (symbol**) htab_lookup_add(tab_fce, gS(akt_fce_name));
                    if(!jeTam(*nad_fce, hledan)){
                        if((*nad_fce)->pocet_volani == (*nad_fce)->pocet_volani_max){
                            (*nad_fce)->pocet_volani_max *= 2;
                            symbol *tmp = realloc(fce, sizeof(symbol) + (*nad_fce)->pocet_volani_max * sizeof(char*));
                            if(!tmp){
                                //TODO
                                //ukoncit s 99
                            }
                            (*nad_fce) = tmp;
                        }
                        (*nad_fce)->volam[(*nad_fce)->pocet_volani] = hledan;
                        (*nad_fce)->pocet_volani++;
                    }
                } else {
                    //kontrola zda je funkce v miste volani definovana
                    kas_vol start;
                    start.fce = NULL;
                    start.next = NULL;
                    int ret_tmp = nejsemDefined(0,p.odR, hledan, &start, &start);
                    if(ret_tmp){
                        return ret_tmp;
                    }
                }
            } else {
                //vyhledani zda identifikator neni funkce
                symbol *prom = htab_data(aktivni_lok, pole[p.odR][p.odS].attr);
                if(prom){
                    prom->vyskyt++;
                } else {
                    p = posunHlavu(p, 1, lines, pole);
                    //pokud se do identifikatoru neprirazuje tak je to funkce
                    if (pole[p.odR][p.odS].id != PRIRAD){
                        p = posunHlavu(p, -1, lines, pole);
                        if(akt_fce){
                            //pridani zaznamu o volani do dat aktivni funkce
                            symbol **nad_fce = (symbol**) htab_lookup_add(tab_fce, gS(akt_fce_name));
                            if(!jeTam(*nad_fce, hledan)){
                                if((*nad_fce)->pocet_volani == (*nad_fce)->pocet_volani_max){
                                    (*nad_fce)->pocet_volani_max *= 2;
                                    symbol *tmp = realloc(fce, sizeof(symbol) + (*nad_fce)->pocet_volani_max * sizeof(char*));
                                    if(!tmp){
                                        //TODO
                                        //ukoncit s 99
                                    }
                                    (*nad_fce) = tmp;
                                }
                                (*nad_fce)->volam[(*nad_fce)->pocet_volani] = hledan;
                                (*nad_fce)->pocet_volani++;
                            }
                        } else {
                            fprintf(stderr, "volani nedefinovane funkce\n");
                            return ERR_SEM_DEF;
                        }
                    } else {
                        //pridani zaznamu o nove promenne
                        p = posunHlavu(p, -1, lines, pole);
                        int delka_attr = strlen(pole[p.odR][p.odS].attr);
                        if(pole[p.odR][p.odS].attr[delka_attr - 1] == '?' || pole[p.odR][p.odS].attr[delka_attr - 1] == '!'){
                            fprintf(stderr, "Promenna ma syntakticky spatny identifikator ('?' nebo '!' na konci)\n");
                            return ERR_SEM_DEF;
                        }

                        int returnik = 0;
                        htab_foreach_super_spec(tab_fce, &returnik, pole[p.odR][p.odS].attr, jeTamVolanKdekoliv);
                        if(returnik){
                            fprintf(stderr, "Promenna se jmenuje jako funkce, ktera mozna bude definovana\n");
                            return ERR_SEM_DEF;
                        }

                        symbol **pp = (symbol**) htab_lookup_add(aktivni_lok, gS(pole[p.odR][p.odS].attr));
                        (*pp) = calloc(1, sizeof(symbol));
                        (*pp)->vyskyt = 1;
                        (*pp)->radek = p.odR;
                        (*pp)->formal = false;
                    }
                }
            }
        }

        p = posunHlavu(p, 1, lines, pole);
        if(!jetuEOF(p,lines, pole)){ //Jen pokud neni konec - kvuli tok ze vyjel mimo pole, nechtel bych segfault
        }
    } while (!jetuEOF(p,lines, pole));


    return 0;
}
