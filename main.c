/*  Soubor: main.c
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *          Josef Oškera, VUT FIT (xosker03)
 *          Jan Juda, VUT FIT (xjudaj00)
 *          Jan Dolezel, VUT FIT (xdolez81)
 *
 *  Datum:  15.11.2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "scanner.h"
#include "scanner_fce.h"
#include "parser.h"
#include "ramfile.h"
#include "vestavene_fce.h"
#include "prvni_pruchod.h"
#include "matematika.h"

#include "zasobniky.h"
#include "optimalizace.h"

#include "opti2.h"


#define ERR_LEX 1
#define ERR_SYNTAX 2
#define ERR_SYS 99


extern htab_t tab_fce;
extern htab_t tab_lokmain;



////////////////////////////////////////////////////////////////////////////////
static void DelVar(int *zapisovany_radek, char *key, symbol *data){
    if(data)
        htab_free(data->ukNaTab);
    (void) zapisovany_radek;
    (void) key;
}
////////////////////////////////////////////////////////////////////////////////

int main(){
    int lines = 0;

    tab_lokmain = htab_init(128);
    tab_fce = htab_init(64);

    //Pushovani predef fcí do symtable fcí
    //print
    symbol **novy = (symbol **) htab_lookup_add(tab_fce, gS("print"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = PROMENY_POC_PARAM;
    (*novy)->ukNaTab = NULL;
    //length
    novy = (symbol **) htab_lookup_add(tab_fce, gS("length"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 1;
    (*novy)->ukNaTab = NULL;
    //not
    novy = (symbol **) htab_lookup_add(tab_fce, gS("not"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 1;
    (*novy)->ukNaTab = NULL;
    //inputi
    novy = (symbol **) htab_lookup_add(tab_fce, gS("inputi"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 0;
    (*novy)->ukNaTab = NULL;
    //inputs
    novy = (symbol **) htab_lookup_add(tab_fce, gS("inputs"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 0;
    (*novy)->ukNaTab = NULL;
    //inputf
    novy = (symbol **) htab_lookup_add(tab_fce, gS("inputf"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 0;
    (*novy)->ukNaTab = NULL;
    //substr
    novy = (symbol **) htab_lookup_add(tab_fce, gS("substr"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 3;
    (*novy)->ukNaTab = NULL;
    //chr
    novy = (symbol **) htab_lookup_add(tab_fce, gS("chr"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 1;
    (*novy)->ukNaTab = NULL;
    //ord
    novy = (symbol **) htab_lookup_add(tab_fce, gS("ord"));
    (*novy) = calloc(1,sizeof(symbol));
    if(!(*novy)){
        fprintf(stderr, "Nedostatek prostředků (MAIN)\n");
        htab_free(tab_fce);
        htab_free(tab_lokmain);
        return ERR_SYS;
    }
    (*novy)->pocet_param = 2;
    (*novy)->ukNaTab = NULL;

    token **tok = scanner(&lines);

    if (tok == (token **)-ERR_SYS) {
        return ERR_SYS;
    }
    if (tok == (token **)-ERR_SYNTAX) {
        return ERR_SYNTAX;
    }
    if (tok == (token **)-ERR_LEX) {
        return ERR_LEX;
    }

    if(tok == NULL){
        fprintf(stderr, "SCANNER: Lexikalni chyba!!!\n");
        return ERR_LEX; //toto je return lex chyby
    }

    if(lines == 0){

        ArStack SStack;
        ASInit(&SStack, 64);

        for(int i=0; i <= lines; i++){
            for(int j = 0; 1 ; ++j){
                if(tok[i][j].id == KONEC_RADKU)
                    break;
                if(tok[i][j].attr){
                    if(!ASSearch(&SStack, tok[i][j].attr))
                        ASPush(&SStack, tok[i][j].attr);
                }
            }
            ASPush(&SStack, tok[i]);
        }
        ASPush(&SStack, tok);
        ASDestroy(&SStack, 2);

        htab_free(tab_fce);
        htab_free(tab_lokmain);
        fprintf(stdout, ".IFJcode18");
        fprintf(stdout, "EXIT int@0");
        return 0;
    }



////////////////////////////// PARSER
    sa_pos pos = {  0,
                    0,
                    lines-1,
                    delkaRadku(lines, lines-1, tok) - 1,
                    false,
                    0,
                    0
    };


    pos.chyba = LLSA(tok, PROG);
    //pos.chyba = 0;
    if(!pos.chyba){
        pos.chyba = PP(lines, tok);
        if(!pos.chyba){
            pos = GA(pos, lines, 0, tok, SPROG);
        }
    }

    if(!pos.chyba){/////JEN POKUD NEBYLA CHYBA V SA!
    /////////////////////////////////////////
    //// Hlavička výstupu
        pisNa(0, 0, ".IFJcode18");
        pisNa(1, 0, "CREATEFRAME");
        pisNa(2, 0, "PUSHFRAME");
        pisNa(3, 0, "DEFVAR GF@$mat_a");
        pisNa(4, 0, "DEFVAR GF@$mat_b");
        pisNa(5, 0, "DEFVAR GF@$type_a");
        pisNa(6, 0, "DEFVAR GF@$type_b");
        pisNa(7, 0, "DEFVAR GF@$neporadek");
        int zap_radek = 8;
        htab_foreach_spec(tab_lokmain, &zap_radek, DefVar);

        Exity();
    /////////////////////////////////////////
        optimalizuj();
        ramFileSwap(NULL, 1);
        optimalizuj();
        ramFileSwap(NULL, 0);


        opt2(0, ramFilePocetRadku());
        ramFileSwap(NULL, 1);
        opt2(0, ramFilePocetRadku());
        ramFileSwap(NULL, 0);

    ////////////////////////////////////////


        for(int i = 0; 1; i++){
            char *t = vypisNa(i);
            if(!t)
                break;
            else
                fprintf(stdout, "%s\n", t);
        }
        ramFileSwap(NULL, 1);
        for(int i = 0; 1; i++){
            char *t = vypisNa(i);
            if(!t)
                break;
            fprintf(stdout, "%s\n", t);
        }
        ramFileSwap(NULL, 0);

    }/////JEN POKUD NEBYLA CHYBA V SA - KONEC!

///////////////////////////////////////// DESTROY FUNKCE //////////////////////////////
    ArStack SStack;
    ASInit(&SStack, 64);

    for(int i=0; i <= lines; i++){
        for(int j = 0; 1 ; ++j){
            if(tok[i][j].id == KONEC_RADKU)
                break;
            if(tok[i][j].attr){
                if(!ASSearch(&SStack, tok[i][j].attr))
                    ASPush(&SStack, tok[i][j].attr);
            }
        }
        ASPush(&SStack, tok[i]);
    }
    ASPush(&SStack, tok);
    ASDestroy(&SStack, 2);

    htab_foreach_spec(tab_fce, NULL, DelVar);
    if(!pos.chyba){
        htab_free(tab_lokmain);
        htab_free(tab_fce);
    }
    while(vymazNa(0) != 0){

    }

    ramFileSwap(NULL, 1);
    while(vymazNa(0) != 0) {

    }



    return pos.chyba;
}
