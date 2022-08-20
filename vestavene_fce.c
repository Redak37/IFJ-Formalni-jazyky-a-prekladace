/*  Modul Vestavěné funkce
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *
 *  Datum:  5.12.2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "vestavene_fce.h"
#include "scanner.h"
#include "matematika.h"

#include "ramfile.h"


void Exity(void){
    pisKonec(0, "LABEL $EXIT0");
    pisKonec(0, "EXIT int@0");
    pisKonec(0, "LABEL $EXIT4");
    pisKonec(0, "EXIT int@4");
    pisKonec(0, "LABEL $EXIT9");
    pisKonec(0, "EXIT int@9");
}

void v_print(int *radek, unsigned pocet){
    for(unsigned i = 0; i < pocet; ++i){
        pisNa((*radek)++, 0, "POPS GF@$mat_a ");
        pisNa((*radek)++, 0, "WRITE GF@$mat_a ");
    }
    pisNa((*radek)++, 0, "PUSHS nil@nil");
}


int v_length(int *radek){
        pisNa((*radek)++, 0, "POPS GF@$mat_b");
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_b");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");
        pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$mat_b");
        pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
    return 0;
}

token v_length_K(token tok){
    token c;
    if(tok.id != KONSTANTA_S){
        c.id = KONEC_RADKU;
        return c;
    }
    c.id = KONSTANTA_I;
    c.attr_i = strlen(tok.attr);
    return c;
}


void v_inputs(int *radek){
    pisNa((*radek)++, 0, "READ GF@$mat_a string");
    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
}
void v_inputi(int *radek){
    pisNa((*radek)++, 0, "READ GF@$mat_a int");
    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
}
void v_inputf(int *radek){
    pisNa((*radek)++, 0, "READ GF@$mat_a float");
    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
}

int v_ord(int *radek){
    int tmp = *radek;
    pisNa((*radek)++, 0, "POPS GF@$mat_b");    //s
    pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_b");
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

    pisNa((*radek)++, 0, "POPS GF@$type_b");   //i
    pisNa((*radek)++, 0, "TYPE GF@$type_a GF@$type_b");
    pisNa((*radek)++, 0, "JUMPIFEQ ORD_INT%d string@int GF@$type_a", tmp);
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@float GF@$type_a");
    pisNa((*radek)++, 0, "FLOAT2INT GF@$type_b GF@$type_b");
    pisNa((*radek)++, 0, "LABEL ORD_INT%d", tmp);

    pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$mat_b");
    pisNa((*radek)++, 0, "LT GF@$type_a GF@$type_b GF@$mat_a");
    pisNa((*radek)++, 0, "JUMPIFEQ ORD_NOTEND%d bool@true GF@$type_a", tmp);
    pisNa((*radek)++, 0, "PUSHS nil@nil");
    pisNa((*radek)++, 0, "JUMP ORD_END%d", tmp);

    pisNa((*radek)++, 0, "LABEL ORD_NOTEND%d", tmp);
    pisNa((*radek)++, 0, "STRI2INT GF@$mat_a GF@$mat_b GF@$type_b");
    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
    pisNa((*radek)++, 0, "LABEL ORD_END%d", tmp);
    return 0;
}

int v_ord_P_K(int *radek, token tok){
    int tmp = *radek;
    if(tok.id != KONSTANTA_I && tok.id != KONSTANTA_D){
        return ERR_SEM_OPER;
    }
    if (tok.id == KONSTANTA_D)
        tok.attr_i = (int)tok.attr_d;
    if(tok.attr_i < 0){
        pisNa((*radek)++, 0, "POPS GF@$mat_b");    //s
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_b");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "PUSHS nil@nil");
    }else{
        pisNa((*radek)++, 0, "POPS GF@$mat_b");    //s
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_b");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$mat_b");
        pisNa((*radek)++, 0, "LT GF@$type_a int@%d GF@$mat_a", tok.attr_i);
        pisNa((*radek)++, 0, "JUMPIFEQ ORD_NOTEND%d bool@true GF@$type_a", tmp);
        pisNa((*radek)++, 0, "PUSHS nil@nil");
        pisNa((*radek)++, 0, "JUMP ORD_END%d", tmp);

        pisNa((*radek)++, 0, "LABEL ORD_NOTEND%d", tmp);
        pisNa((*radek)++, 0, "STRI2INT GF@$mat_a GF@$mat_b GF@$type_b");
        pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
        pisNa((*radek)++, 0, "LABEL ORD_END%d", tmp);
    }
    return 0;
}

//string int
token v_ord_K_K(token toks, token toki){
    token c;
    if((toki.id != KONSTANTA_I && toki.id != KONSTANTA_D) || toks.id != KONSTANTA_S){
        c.id = KONEC_RADKU;
        return c;
    }
    if (toki.id == KONSTANTA_D) {
        toki.attr_i = (int)toki.attr_d;
    }

    int delka = strlen(toks.attr);
    if(toki.attr_i < delka){
        c.id = KONSTANTA_I;
        c.attr_i = toks.attr[toki.attr_i];
        return c;
    } else {
        c.id = NIL;
    }
    return c;
}



void v_chr(int *radek){
    int zaloha = *radek;
    pisNa((*radek)++, 0, "POPS GF@$mat_a");
    pisNa((*radek)++, 0, "TYPE GF@$type_a GF@$mat_a");
    pisNa((*radek)++, 0, "JUMPIFEQ VCHR_INT%d string@int GF@$type_a", zaloha);
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@float GF@$type_a");
    pisNa((*radek)++, 0, "FLOAT2INT GF@$mat_a GF@$mat_a");

    pisNa((*radek)++, 0, "LABEL VCHR_INT%d", zaloha);

    pisNa((*radek)++, 0, "GT GF@$mat_b GF@$mat_a int@255");
    pisNa((*radek)++, 0, "JUMPIFEQ $EXIT4 GF@$mat_b bool@true");

    pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a int@0");
    pisNa((*radek)++, 0, "JUMPIFEQ $EXIT4 GF@$mat_b bool@true");

    pisNa((*radek)++, 0, "INT2CHAR GF@$mat_a GF@$mat_a");
    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
}


token v_chr_K(token tok){
    token c;
    if(tok.id != KONSTANTA_I && tok.id != KONSTANTA_D){
        c.id = KONEC_RADKU;
        return c;
    }
    if (tok.id == KONSTANTA_D)
        tok.attr_i = (int)tok.attr_d;

    if(tok.attr_i < 0 || tok.attr_i > 255){
        c.id = CHYBA;
        return c;
    }
    c.id = KONSTANTA_S;
    c.attr = calloc(2, sizeof(char));
    *(c.attr) = (char)tok.attr_i;
    return c;
}


//na zásobník:  n int(delka)
//              i int(odkud)
//              s string
void v_substr(int *radek){
    /*pop s
    pop i
    pop n
    kontrola s
    kontrola i
    kontrola n
*/
    int zaloha = *radek;

    pisNa((*radek)++, 0, "POPS GF@$neporadek");    //s
    pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$neporadek");
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

    pisNa((*radek)++, 0, "POPS GF@$type_a");    //i
    pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$type_a");

    pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR_INT%d string@int GF@$mat_a", zaloha);
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@float GF@$mat_a");
    pisNa((*radek)++, 0, "FLOAT2INT GF@$type_a GF@$type_a");

    pisNa((*radek)++, 0, "LABEL SUBSTR_INT%d", zaloha);

    pisNa((*radek)++, 0, "POPS GF@$type_b");    //n
    pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$type_b");
    pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@int GF@$mat_a");

    pisNa((*radek)++, 0, "LT GF@$mat_a GF@$type_a int@0");
    pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-nonili%d bool@false GF@$mat_a", zaloha);
        pisNa((*radek)++, 0, "PUSHS nil@nil");
        pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
    pisNa((*radek)++, 0, "LABEL SUBSTR-nonili%d", zaloha);

    pisNa((*radek)++, 0, "LT GF@$mat_a GF@$type_b int@0");
    pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-noniln%d bool@false GF@$mat_a", zaloha);
        pisNa((*radek)++, 0, "PUSHS nil@nil");
        pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
    pisNa((*radek)++, 0, "LABEL SUBSTR-noniln%d", zaloha);

    pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$neporadek");
    pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a GF@$type_a");
    pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-nonils%d bool@false GF@$mat_b", zaloha);
        pisNa((*radek)++, 0, "PUSHS nil@nil");
        pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
    pisNa((*radek)++, 0, "LABEL SUBSTR-nonils%d", zaloha);


    pisNa((*radek)++, 0, "ADD GF@$type_b GF@$type_a GF@$type_b");

    pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a GF@$type_b");
    pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-max%d bool@false GF@$mat_b", zaloha);
        pisNa((*radek)++, 0, "MOVE GF@$type_b GF@$mat_a");
    pisNa((*radek)++, 0, "LABEL SUBSTR-max%d", zaloha);

    pisNa((*radek)++, 0, "MOVE GF@$mat_a string@");

    pisNa((*radek)++, 0, "LABEL SUBSTR-start%d", zaloha);
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-end%d GF@$type_a GF@$type_b", zaloha);
            pisNa((*radek)++, 0, "GETCHAR GF@$mat_b GF@$neporadek GF@$type_a");
            pisNa((*radek)++, 0, "CONCAT GF@$mat_a GF@$mat_a GF@$mat_b");
            pisNa((*radek)++, 0, "ADD GF@$type_a GF@$type_a int@1");
        pisNa((*radek)++, 0, "JUMP SUBSTR-start%d", zaloha);
    pisNa((*radek)++, 0, "LABEL SUBSTR-end%d", zaloha);


    pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
    pisNa((*radek)++, 0, "LABEL SUBSTR-afterEnd%d", zaloha);
}


int v_substr_P_P_K(int *radek, token tokn){
    if(tokn.id != KONSTANTA_I && tokn.id != KONSTANTA_D){
        return ERR_SEM_OPER;
    }
    if (tokn.id == KONSTANTA_D)
        tokn.attr_i = (int)tokn.attr_d;

    int zaloha = *radek;

    if(tokn.attr_i < 0){
        pisNa((*radek)++, 0, "POPS GF@$mat_a");    //s SUBSTR_
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_a");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "POPS GF@$mat_a");    //i
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_a");
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR_INT%d string@int GF@$mat_a", zaloha);
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@float GF@$mat_a");

        pisNa((*radek)++, 0, "LABEL SUBSTR_INT%d", zaloha);

        pisNa((*radek)++, 0, "PUSHS nil@nil");
    }else{
        pisNa((*radek)++, 0, "POPS GF@$neporadek");    //s
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$neporadek");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "POPS GF@$type_a");    //i
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$type_a");
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR_INT%d string@int GF@$mat_a", zaloha);
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@float GF@$mat_a");

        pisNa((*radek)++, 0, "FLOAT2INT GF@$type_a GF@$type_a");

        pisNa((*radek)++, 0, "LABEL SUBSTR_INT%d", zaloha);

        pisNa((*radek)++, 0, "LT GF@$mat_a GF@$type_a int@0");
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-nonili%d bool@false GF@$mat_a", zaloha);
            pisNa((*radek)++, 0, "PUSHS nil@nil");
            pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-nonili%d", zaloha);

        pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$neporadek");
        pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a GF@$type_a");
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-nonils%d bool@false GF@$mat_b", zaloha);
            pisNa((*radek)++, 0, "PUSHS nil@nil");
            pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-nonils%d", zaloha);


        pisNa((*radek)++, 0, "ADD GF@$type_b GF@$type_a int@%d", tokn.attr_i);

        pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a int@%d", tokn.attr_i);
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-max%d bool@false GF@$mat_b", zaloha);
            pisNa((*radek)++, 0, "MOVE GF@$type_b GF@$mat_a");
        pisNa((*radek)++, 0, "LABEL SUBSTR-max%d", zaloha);

        pisNa((*radek)++, 0, "MOVE GF@$mat_a string@");

        pisNa((*radek)++, 0, "LABEL SUBSTR-start%d", zaloha);
            pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-end%d GF@$type_a GF@$type_b", zaloha);
                pisNa((*radek)++, 0, "GETCHAR GF@$mat_b GF@$neporadek GF@$type_a");
                pisNa((*radek)++, 0, "CONCAT GF@$mat_a GF@$mat_a GF@$mat_b");
                pisNa((*radek)++, 0, "ADD GF@$type_a GF@$type_a int@1");
            pisNa((*radek)++, 0, "JUMP SUBSTR-start%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-end%d", zaloha);


        pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
        pisNa((*radek)++, 0, "LABEL SUBSTR-afterEnd%d", zaloha);
    }
    return 0;
}
int v_substr_P_K_K(int *radek, token toki, token tokn){
    /*pop s
    pop i
    pop n
    kontrola s
    kontrola i
    kontrola n
*/

    if((toki.id != KONSTANTA_I && toki.id != KONSTANTA_D) || (tokn.id != KONSTANTA_I && tokn.id != KONSTANTA_D)){
        return ERR_SEM_OPER;
    }
    if (toki.id == KONSTANTA_D)
        toki.attr_i = (int)toki.attr_d;
    if (tokn.id == KONSTANTA_D)
        tokn.attr_i = (int)tokn.attr_d;

    int zaloha = *radek;

    if(toki.attr_i < 0 || tokn.attr_i < 0){
        pisNa((*radek)++, 0, "POPS GF@$mat_a");    //s
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_a");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "PUSHS nil@nil");
    }else{
        pisNa((*radek)++, 0, "POPS GF@$neporadek");    //s
        pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$neporadek");
        pisNa((*radek)++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$mat_a");

        pisNa((*radek)++, 0, "STRLEN GF@$mat_a GF@$neporadek");
        pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a int@%d", toki.attr_i);
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-nonils%d bool@false GF@$mat_b", zaloha);
            pisNa((*radek)++, 0, "PUSHS nil@nil");
            pisNa((*radek)++, 0, "JUMP SUBSTR-afterEnd%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-nonils%d", zaloha);

        pisNa((*radek)++, 0, "MOVE GF@$type_a int@%d", toki.attr_i);

        pisNa((*radek)++, 0, "LT GF@$mat_b GF@$mat_a int@%d", toki.attr_i + tokn.attr_i);
        pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-max%d bool@false GF@$mat_b", zaloha);
            pisNa((*radek)++, 0, "MOVE GF@$type_b GF@$mat_a");
            pisNa((*radek)++, 0, "JUMP SUBSTR-max2%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-max%d", zaloha);
        pisNa((*radek)++, 0, "MOVE GF@$type_b int@%d", toki.attr_i + tokn.attr_i);
        pisNa((*radek)++, 0, "LABEL SUBSTR-max2%d", zaloha);

        pisNa((*radek)++, 0, "MOVE GF@$mat_a string@");

        pisNa((*radek)++, 0, "LABEL SUBSTR-start%d", zaloha);
            pisNa((*radek)++, 0, "JUMPIFEQ SUBSTR-end%d GF@$type_a GF@$type_b", zaloha);
                pisNa((*radek)++, 0, "GETCHAR GF@$mat_b GF@$neporadek GF@$type_a");
                pisNa((*radek)++, 0, "CONCAT GF@$mat_a GF@$mat_a GF@$mat_b");
                pisNa((*radek)++, 0, "ADD GF@$type_a GF@$type_a int@1");
            pisNa((*radek)++, 0, "JUMP SUBSTR-start%d", zaloha);
        pisNa((*radek)++, 0, "LABEL SUBSTR-end%d", zaloha);

        pisNa((*radek)++, 0, "PUSHS GF@$mat_a");
        pisNa((*radek)++, 0, "LABEL SUBSTR-afterEnd%d", zaloha);
    }
    return 0;
}
token v_substr_K_K_K(token toks, token toki, token tokn){
    token c;
    if(toks.id != KONSTANTA_S || (toki.id != KONSTANTA_I && toki.id != KONSTANTA_D) || (tokn.id != KONSTANTA_I && tokn.id != KONSTANTA_D)){
        c.id = KONEC_RADKU;
        return c;
    }
    if (toki.id == KONSTANTA_I)
        toki.attr_d = 0;
    else
        toki.attr_i = 0;

    if (tokn.id == KONSTANTA_I)
        tokn.attr_d = 0;
    else
        tokn.attr_i = 0;

    int delka = strlen(toks.attr);

    if (toki.attr_i + (int)toki.attr_d >= delka || tokn.attr_i + (int)tokn.attr_d < 0){
        c.id = NIL;
        return c;
    }

    c.id = KONSTANTA_S;
    if(toki.attr_i + tokn.attr_i + (int)toki.attr_d + (int)tokn.attr_d > delka){
        c.attr = calloc(strlen(&(toks.attr[toki.attr_i+(int)toki.attr_d])), sizeof(char));
        strcpy(c.attr, &(toks.attr[toki.attr_i]));
        return c;
    }
    c.attr = calloc(strlen(&(toks.attr[toki.attr_i+(int)toki.attr_d])), sizeof(char));
    toks.attr[toki.attr_i+tokn.attr_i+(int)toki.attr_d+(int)tokn.attr_d] = '\0';
    strcpy(c.attr, &(toks.attr[toki.attr_i+(int)toki.attr_d]));
    return c;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void v_not(int *zapisovany_radek){
    //test typu...
    pisNa((*zapisovany_radek)++, 0, "POPS GF@$mat_a");
    pisNa((*zapisovany_radek)++, 0, "PUSHS GF@$type_a");
    pisNa((*zapisovany_radek)++, 0, "TYPE GF@$type_a GF@$mat_a");
    pisNa((*zapisovany_radek)++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");

    pisNa((*zapisovany_radek)++, 0, "NOTS");
}

token v_not_K(token tok){
    token c;
    if(tok.id != BTRUE && tok.id != BFALSE){
        c.id = KONEC_RADKU;
        return c;
    }
    else if(tok.id == BTRUE)
        c.id = BFALSE;
    else
        c.id = BTRUE;
    return c;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void generateIf(int *radek, int lokal_label){
    pisNa((*radek)++, 0, "POPS GF@$mat_b");
    pisNa((*radek)++, 0, "TYPE GF@$mat_a GF@$mat_b");

    pisNa((*radek)++, 0, "     JUMPIFEQ ELSE%d GF@$mat_a string@nil", lokal_label);
    pisNa((*radek)++, 0, "     JUMPIFNEQ START%d GF@$mat_a string@bool", lokal_label);
    pisNa((*radek)++, 0, "     JUMPIFNEQ START%d GF@$mat_b bool@false", lokal_label);
    pisNa((*radek)++, 0, "         JUMP ELSE%d", lokal_label);
    pisNa((*radek)++, 0, "LABEL START%d", lokal_label);

}

void generateElse(int *radek, int lokal_label){
    pisNa((*radek)++, 0, "LABEL ELSE%d", lokal_label);
}

void generateEnd(int *radek, int lokal_label){
    pisNa((*radek)++, 0, "LABEL END%d", lokal_label);
}
