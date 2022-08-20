/*  Knihovna pro optimalizace 2
 *
 *  Autor:  Josef Oškera, VUT FIT (xosker03)
 *
 *  Datum:  15.11.2018
 *
 */

#include "opti2.h"
#include "ramfile.h"
#include "scanner.h"
#include "optimalizace.h"

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


#include "vestavene_fce.h"


typedef enum operace{
    BEZZMENY,
    HLOUBKAPLUS,    //opimalizaci lze provadet jen o uroven hloubeji
    ZASOP, //opimalizaci lze provadet jen o uroven hloubeji
    NELZE,
    PUSHF,
    POPF,
    CHYBKA
}operace_t;

static operace_t analyzator(int radek);
static void presun(int odkud, int kam);

//Slucuje PUSHS a POPS do MOVE
int opt2(int radek, int radky){
    int ret = 0;
    while(radek < radky){
        bool pokracovat = false;
        int pocitadlo = 1;
        int palec = 0;
        if(analyzator(radek) == PUSHF)
        for(int i = radek + 1; i < radky; ++i){
            operace_t op;
            op = analyzator(i);
            bool preskoc = false;
            switch (op){
                case BEZZMENY: {
                    break;
                }
                case HLOUBKAPLUS : {
                    pocitadlo--;
                    palec++;
                    if(pocitadlo <= 0){
                        preskoc = true;
                    }
                    break;
                }
                case ZASOP: {
                    pocitadlo -= 2;
                    palec++;
                    if(pocitadlo <= 0){
                        preskoc = true;
                    }
                    break;
                }
                case NELZE: {
                    preskoc = true;
                    break;
                }
                case PUSHF: {
                    pocitadlo++;
                    break;
                }
                case POPF: {
                    if(palec){
                        palec--;
                    } else {
                        pocitadlo--;
                        if(pocitadlo < 0){
                            preskoc = true;
                            break;
                        }
                        if(pocitadlo == 0){
                            presun(radek, i);
                            preskoc = true;
                            pokracovat = true;
                            ret++;
                        }
                    }
                    break;
                }
                case CHYBKA: {
                    break;
                }
            }
            if(preskoc){
                break;
            }
        }
        if(pokracovat){
            continue;
        }
        radek++;
    }
    if(ret){
        return optimalizuj();
    }
    return ret;
}




static void presun(int odkud, int kam){
    char *z_ceho = vypisNa(odkud);
    char *do_ceho = vypisNa(kam);

    char *tmp = calloc(1, strlen(z_ceho) + strlen(do_ceho) + 5);

    sprintf(tmp, "MOVE %s %s", do_ceho + 5,z_ceho + 6);

    pisNa(kam+1, strlen(tmp), "%s", tmp);
    vymazNa(kam);
    vymazNa(odkud);
    free(tmp);
}



//0 je to ok
//1 stack se zmensil o 1
//2 nelze
//-1 sirka +1
static operace_t analyzator(int radek){
    char op0[100] = {0};
    /*char  op1[100] = {0};
    char  op2[100] = {0};
    char  op3[100] = {0};*/
    char *data = vypisNa(radek);
    if(!data)
        return NELZE;

    sscanf(data, "%s ", op0);

    if(strcmp(op0,"CREATEFRAME") == 0) {return BEZZMENY;}
    if(strcmp(op0,"PUSHFRAME") == 0) {return BEZZMENY;}
    if(strcmp(op0,"POPFRAME") == 0) {return BEZZMENY;}
    if(strcmp(op0,"DEFVAR") == 0) {return BEZZMENY;}
    if(strcmp(op0,"CALL") == 0) {return NELZE;}
    if(strcmp(op0,"RETURN") == 0) {return NELZE;}
    if(strcmp(op0,"MOVE") == 0) {return BEZZMENY;}

    if(strcmp(op0,"PUSHS") == 0) {return PUSHF;}
    if(strcmp(op0,"POPS") == 0) {return POPF;}
    if(strcmp(op0,"CLEARS") == 0) {return NELZE;}

    if(strcmp(op0,"ADD") == 0) {return BEZZMENY;}
    if(strcmp(op0,"SUB") == 0) {return BEZZMENY;}
    if(strcmp(op0,"MUL") == 0) {return BEZZMENY;}
    if(strcmp(op0,"DIV") == 0) {return BEZZMENY;}
    if(strcmp(op0,"IDIV") == 0) {return BEZZMENY;}

    if(strcmp(op0,"ADDS") == 0) {return ZASOP;}
    if(strcmp(op0,"SUBS") == 0) {return ZASOP;}
    if(strcmp(op0,"MULS") == 0) {return ZASOP;}
    if(strcmp(op0,"DIVS") == 0) {return ZASOP;}
    if(strcmp(op0,"IDIVS") == 0) {return ZASOP;}

    if(strcmp(op0,"LT") == 0) {return BEZZMENY;}
    if(strcmp(op0,"GT") == 0) {return BEZZMENY;}
    if(strcmp(op0,"EQ") == 0) {return BEZZMENY;}

    if(strcmp(op0,"LTS") == 0) {return ZASOP;}
    if(strcmp(op0,"GTS") == 0) {return ZASOP;}
    if(strcmp(op0,"EQS") == 0) {return ZASOP;}

    if(strcmp(op0,"AND") == 0) {return BEZZMENY;}
    if(strcmp(op0,"OR") == 0) {return BEZZMENY;}
    if(strcmp(op0,"NOT") == 0) {return BEZZMENY;}

    if(strcmp(op0,"ANDS") == 0) {return ZASOP;}
    if(strcmp(op0,"ORS") == 0) {return ZASOP;}
    if(strcmp(op0,"NOTS") == 0) {return HLOUBKAPLUS;}

    if(strcmp(op0,"INT2FLOAT") == 0) {return BEZZMENY;}
    if(strcmp(op0,"FLOAT2INT") == 0) {return BEZZMENY;}
    if(strcmp(op0,"INT2CHAR") == 0) {return BEZZMENY;}
    if(strcmp(op0,"STRI2INT") == 0) {return BEZZMENY;}

    if(strcmp(op0,"INT2FLOATS") == 0) {return HLOUBKAPLUS;}
    if(strcmp(op0,"FLOAT2INTS") == 0) {return HLOUBKAPLUS;}
    if(strcmp(op0,"INT2CHARS") == 0) {return HLOUBKAPLUS;}
    if(strcmp(op0,"STRI2INTS") == 0) {return HLOUBKAPLUS;}

    if(strcmp(op0,"READ") == 0) {return BEZZMENY;}
    if(strcmp(op0,"WRITE") == 0) {return BEZZMENY;}
    if(strcmp(op0,"CONCAT") == 0) {return BEZZMENY;}
    if(strcmp(op0,"STRLEN") == 0) {return BEZZMENY;}
    if(strcmp(op0,"GETCHAR") == 0) {return BEZZMENY;}
    if(strcmp(op0,"SETCHAR") == 0) {return BEZZMENY;}
    if(strcmp(op0,"TYPE") == 0) {return BEZZMENY;}

    if(strcmp(op0,"LABEL") == 0) {return NELZE;}
    if(strcmp(op0,"JUMP") == 0) {return NELZE;}
    if(strcmp(op0,"JUMPIFEQ") == 0) {return NELZE;}
    if(strcmp(op0,"JUMPIFNEQ") == 0) {return NELZE;}
    if(strcmp(op0,"JUMPIFEQS") == 0) {return NELZE;}
    if(strcmp(op0,"JUMPIFNEQS") == 0) {return NELZE;}
    if(strcmp(op0,"EXIT") == 0) {return NELZE;}
    if(strcmp(op0,"BREAK") == 0) {return NELZE;}
    if(strcmp(op0,"DPRINT") == 0) {return NELZE;}
    return CHYBKA;
}
