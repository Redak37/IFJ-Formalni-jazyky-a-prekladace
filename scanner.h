/*
 *  Scanner - lexikalni analyzator
 *
 *  IFJ + IAL Projekt Kompilator IFJ18
 *
 *  Autor:  Jan Juda, VUT FIT (xjudaj00)
 *
 *  Datum:  20.11.2018
 *
 */

#ifndef SCANNER_H_
#define SCANNER_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "preprocesor.h"

//Vycet typu tokenu. Tez slouzi jako mnozina moznych terminalu. Token TOKTYPE_COUNT urcuje pocet platnych tokenu
typedef enum TOKTYPE {
    KONEC_RADKU = 0, PLUS, MINUS, KRAT, DELENO, //arit operace
    PRIRAD, ZAV_ZAC, ZAV_END,//přiřazení, zavorky
    VETSI, MENSI, MENSIROVNO, VETSIROVNO, ROVNASE, NEROVNASE,  //porovnavani
    KONSTANTA_I, KONSTANTA_D, KONSTANTA_S, IDENTIFIKATOR,    //věci do tabulky a konstanty
    DEF, END, NIL,   //definice funkce, nil
    IF, THEN, ELSE, ELSEIF, //if
    WHILE, DO, //cykly
    BTRUE, BFALSE, NOT, AND, OR, AND_Z, OR_Z, NOT_Z, //BOOLOP - boolean věci, AND_Z a OR_Z jsou znakový verze - && a ||, stějně tak NOT_Z
    TER_ZAC, TER_END, CARKA, //ternární op
    //INPUTS, INPUTI, INPUTF, PRINT, LENGTH, SUBSTR, ORD, CHR//vestavěné fce
    FILEND,
    TOKTYPE_COUNT, //pocet typu tokenu, VŽDY NA KONCI
    CHYBA = -1
} TOKTYPE;

/**
 *  Struktura tokenu
 *
 * @id = urcuje typ tokenu
 * @attr = u identifikatoru specifikuje jmeno, u retezcoveho literalu obsah literalu
 * @attr_i = obsah celociselneho literalu
 * @attr_d = obsah literalu s plavouci radovou carkou
 */
typedef struct token {
    TOKTYPE id;
    char *attr;
    int64_t attr_i;
    double attr_d;
} token;

//Vycet stavu konecneho automatu scanneru.
typedef enum STAV {
    ERROR, START, //start a error
    VYK_X, ROV_X, ZOB_M_X, ZOB_V_X, //operatory
    STR_OTZ, STR_SLASH, STR_X, STR_XH,                                   //string lit
    FLOAT_X, FLOAT_E, FLOAT_SIGN, FLOAT_BEZ_E, FLOAT_END,                                    //float
    INT, NULA_OTZ, NULA_X, NULA_B, INT_BIN, INT_HEX, INT_OCT,      //int a nuly
    IDENTIF,   //id
    AND_X, OR_X, //&& ||
    ERROR_SYSTEM = -1
} STAV;

//Snad spravny pocet nami podporovanych klicovych slov.
#define POCET_KEYWORDU 14

token ** scanner(int *radek);

#endif
