/*  Soubor: Generator asembleru
 *
 *  Autor:  Josef Oškera, VUT FIT (xosker03)
 *          Jan Juda, VUT FIT (xjudaj00)
 *          Jan Dolezel, VUT FIT (xdolez81)
 *
 *  Datum:  25.11.2018
 *
 */

#include "matematika.h"
#include "ramfile.h"
#include "gassembler.h"

#include "vestavene_fce.h"

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


#include "vestavene_fce.h"

int zapisovany_radek = 0;
int labelN = 0;
htab_t tab_fce;
htab_t tab_lokmain;


//funkce vráti pocet tokenu na rádku
int delkaRadku(int lines, int cislo_radku, token **pole){
   if(cislo_radku >= lines)
       return -1;
   int i = 0;
   while(pole[cislo_radku][i].id != KONEC_RADKU)
       ++i;
   return i + 1;
}

//zjisti zda sme na konci souboru
bool jetuEOF(sa_pos p, int lines, token **pole){
   int delka = delkaRadku(lines, lines-1, pole);
   if(p.odR >= lines-1 && p.odS >= delka)
       return true;
   return false;
}

//zjisti jestli nejsme mimo pole
static bool jsemMimo(sa_pos p, int lines, token **pole){
   if(p.odR >= lines)
       return true;
   if (p.odS >= delkaRadku(lines, p.odR, pole))
       return true;
   return false;
}


//posune cteci hlavu o požadovany posun dopredu
//pokud narazi za konec souboru, tak se nastavi na posledni endl
//pokud má jit pred soubor, tak se nastavi na prvni
sa_pos posunHlavu(sa_pos p, int posun, int lines, token **pole){
    for(int i = 0; i < posun; ++i){
        if(pole[p.odR][p.odS].id == KONEC_RADKU){
            p.odR++;
            p.odS = 0;
            if(p.odR >= lines){
                p.odR = lines - 1;
                p.odS = delkaRadku(lines, lines-1, pole);
                break;
            }
        }else{
            p.odS++;
        }
    }
    for(int i = 0; i > posun; --i){
        p.odS--;
        if(p.odS < 0){
            p.odR--;
            if(p.odR < 0){
                p.odR = 0;
                p.odS = 0;
            }else{
                p.odS = delkaRadku(lines, p.odR, pole);
            }
        }
    }
    return p;
}

//posune zarážku cteci hlavy o požadovany posun dopredu
//pokud narazi za konec souboru, tak se nastavi na posledni endl
//pokud má jit pred soubor, tak se nastavi na prvni
static sa_pos posunZarazku(sa_pos p, int posun, int lines, token **pole){
    for(int i = 0; i < posun; ++i){
        if(pole[p.doR][p.doS].id == KONEC_RADKU){
            p.doR++;
            p.doS = 0;
            if(p.doR >= lines){
                p.doR = lines - 1;
                p.doS = delkaRadku(lines, lines-1, pole);
                break;
            }
        }else{
            p.doS++;
        }
    }
    for(int i = 0; i > posun; --i){
        p.doS--;
        if(p.doS < 0){
            p.doR--;
            if(p.doR < 0){
                p.doR = 0;
                p.doS = 0;
            }else{
                p.doS = delkaRadku(lines, p.odR, pole);
            }
        }
    }
    return p;
}

//vraci true pokud je cteci hlava na zarážce
static bool semNaZaraz(sa_pos p){
    if(p.odR < p.doR)
        return false;
    if(p.odR > p.doR)
        return true;
    if(p.odS < p.doS)
        return false;
    return true;
}

//posune zarážku na chtěny end (token)
//na stejné úrovni zanoreni
static sa_pos zarazkaEND(sa_pos p, int lines, token **pole){
    int hloubka = 0;
    /*if(pole[p.doR][p.doS].id == END){
        return p;
    }
    if(pole[p.doR][p.doS].id == IF){
        hloubka++;
    }*/
    do{
        //p = posunZarazku(p, 1, lines, pole);
        if(pole[p.doR][p.doS].id == END){
            if(hloubka){
                hloubka--;
                assert(hloubka >= 0);
                if(hloubka < 0){
                    fprintf(stderr, "CHYBA:GA:zarazkaEnd: priliš mnoho END\n");
                    p.chyba = ERR_SYS;
                    return p;
                }
            } else {
                break;
            }
        }
        if(pole[p.doR][p.doS].id == IF){
            hloubka++;
        }
        if(pole[p.doR][p.doS].id == WHILE){
            hloubka++;
        }
        if(pole[p.doR][p.doS].id == DEF){
            hloubka++;
        }
        if(jetuEOF(p, lines, pole)){
            fprintf(stderr, "CHYBA:GA:zarazkaEnd: ocekavan END\n");
            p.chyba = ERR_SYS;
            return p;
        }
        p = posunZarazku(p, 1, lines, pole);
    }while(1);
    return p;
}

//posune zarážku na chtěny else (token)
//na stejné úrovni zanoreni
static sa_pos zarazkaELS(sa_pos p, int lines, token **pole, TOKTYPE tok){
    int hloubka = 0;
    assert(tok == ELSE || tok == ELSEIF);
    if(pole[p.doR][p.doS].id == tok){
        return p;
    }
    do{
        //p = posunZarazku(p, 1, lines, pole);
        if(pole[p.doR][p.doS].id == END){
                hloubka--;
                if(hloubka < 0){
                    p.chyba = true;
                    return p;
                }
        }
        if(pole[p.doR][p.doS].id == IF){
            hloubka++;
        }
        if(pole[p.doR][p.doS].id == WHILE){
            hloubka++;
        }
        if(pole[p.doR][p.doS].id == tok){
            if(hloubka == 0)
                break;
        }
        if(jetuEOF(p, lines, pole)){
            p.chyba = true;
            return p;
        }
        /*if(semNaZaraz(p)){
            p.chyba = true;
            return p;
        }*/
        p = posunZarazku(p, 1, lines, pole);
    }while(1);
    return p;
}



//posune zarážku na then (token)
//na jednom rádku
static sa_pos zarazkaTOK(sa_pos p, int lines, token **pole, TOKTYPE net){
    p.doR = p.odR;
    p.doS = p.odS;
    while(pole[p.doR][p.doS].id != net){
        p.doS++;
        if(pole[p.doR][p.doS].id == KONEC_RADKU){
            fprintf(stderr, "PARSER:GA:zarazkaTok: LLSA se pokazilo\n");
            p.chyba = ERR_SYS;
            return p;
        }
    }
    (void) lines;
    return p;
}

//tisne do ramfile nově definované proměnné
void DefVar(int *zapisovany_radek, char *key, symbol *data){
    pisNa((*zapisovany_radek)++, strlen(key), "DEFVAR LF@%s",key);
    if(data){
        if(data->formal){
            pisNa((*zapisovany_radek)++, strlen(key), "POPS LF@%s",key);
        } else {
            pisNa((*zapisovany_radek)++, strlen(key), "MOVE LF@%s nil@nil",key);
        }
    }
    else{
        pisNa((*zapisovany_radek)++, strlen(key), "MOVE LF@%s nil@nil",key);
    }
    (void) data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Generáto assembleru
ridi pruchod kodem a generuje ridici struktury jako podminky a cykly
na správnych mistech spoušti zpracováni vyrazu -- MA()
ridi se podle pravidel
funguje obdobně jako rekurzivni sestup

Pravidla:
        SPROG->SLISTV

        SVYRAZ-> MA()

        SLISTV-> nic
        SLISTV-> SVYRAZ \n SLISTV
        SLISTV-> if MA() then \n SBLOKV end \n SLISTV
        SLISTV-> while MA() do \n SLISTV end \n SLISTV
        SLISTV-> def SLISTV end \n SLISTV

        SBLOKV-> SLISTV elsif MA() then \n SBLOKV
        SBLOKV-> SLISTV else \n SLISTV
        SBLOKV-> SLISTV

GA vždy vehledá odkuď kam má spustit sama sebe nebo MA()
a pak se znovu spusti pro zbytek do konce souboru
a pritom generuje ridici struktury ifjcode18
*/
sa_pos GA(sa_pos p,int lines, int hloubka, token **tokpole, stavy_t net){
    if(jetuEOF(p, lines, tokpole)){
        return p;
    }
    if(semNaZaraz(p)){
        return p;
    }
    if(jsemMimo(p, lines, tokpole)){
        return p;
    }

    //hlavni switch
    switch(net){
        case SPROG: {
            p = GA(p, lines, hloubka+1, tokpole, SLISTV);
            if(p.chyba)
                return p;
            break;
        }
        case SVYRAZ: {
            // SVYRAZ -> MA()
            if(tokpole[p.odR][p.odS].id == IDENTIFIKATOR){
                p.odS++;
                if(tokpole[p.odR][p.odS].id == PRIRAD){
                    p.odS++;
                    if(htab_find(tab_fce, tokpole[p.odR][0].attr)){
                        fprintf(stderr, "CHYBA:GA:Seman error: Prirazeni do fce.\n");
                        p.chyba = ERR_SEM_DEF;
                        return p;
                    }

                    //vyhledáni indentifikátoru
                    if(!htab_find(tab_lokmain, tokpole[p.odR][0].attr)){
                        fprintf(stderr, "\n\nPP musel selhat! Nenašel tuto proměnnou %s\n\n", tokpole[p.odR][0].attr);
                        p.chyba = ERR_SYS; //TODO error 99
                        return p;
                    }
                    //zavoláni matematiky
                    p = MA(p, tokpole, tab_lokmain);
                    if (p.chyba)
                        return p;
                    pisNa(zapisovany_radek++, strlen(tokpole[p.odR][0].attr), "POPS LF@%s", tokpole[p.odR][0].attr);
                    pisNa(zapisovany_radek++, strlen(tokpole[p.odR][0].attr), "MOVE GF@$neporadek LF@%s", tokpole[p.odR][0].attr);
                    return p;
                }
            }
            p.odS = 0;
            if(tokpole[p.odR][p.odS].id == PRIRAD){
                fprintf(stderr, "Chyby promena do ktere prirazuji (ze prej semanticka chyba...) na radku %d\n", p.odR);
                p.chyba = ERR_SEM_JINAK;
                return p;
            }

            p = MA(p, tokpole, tab_lokmain);
            pisNa(zapisovany_radek++, 0, "POPS GF@$neporadek");
            return p;
            break;
        }

        case SLISTV: {
                switch(tokpole[p.odR][p.odS].id){
                    case DEF:{
                        p.vnoreni++;
                        ramFileSwap(&zapisovany_radek, 1);
                        sa_pos zaloha_zalohy = p;
                        p = posunHlavu(p, 1, lines, tokpole);
                        //název

                        pisNa(zapisovany_radek++, strlen(tokpole[p.odR][p.odS].attr), "LABEL FUNKCE-%s", tokpole[p.odR][p.odS].attr);
                        pisNa(zapisovany_radek++, 0, "CREATEFRAME");
                        pisNa(zapisovany_radek++, 0, "PUSHFRAME");
                        pisNa(zapisovany_radek++, 0, "MOVE GF@$neporadek nil@nil");

                        symbol *funkce = htab_data(tab_fce, tokpole[p.odR][p.odS].attr);
                        htab_t zaloha_lok = tab_lokmain;
                        if (!(funkce)){
                            fprintf(stderr, "CHYBA:GA %d:DEF: Neco se pokazilo asi v PP - funkce %s neni v tabulce fci!\n",hloubka, tokpole[p.odR][p.odS].attr);
                            p.chyba = ERR_SYS;
                            return p;
                        }
                        tab_lokmain = funkce->ukNaTab;

                        htab_foreach_spec(tab_lokmain, &zapisovany_radek, DefVar);

                        //3 --- poseun na '(' ')' konec radku
                        //2 * funkce->pocet_param - 1 --- pocet parametru + cárek
                        //preskoceni nepotrebnych tokenu
                        if(funkce->pocet_param)
                            p = posunHlavu(p, 3 + 2 * funkce->pocet_param - 1, lines, tokpole);
                        else
                            p = posunHlavu(p, 3, lines, tokpole);

                        //konec radku

                        //Vnitrek funkce
                        sa_pos zaloha = p;
                        p.doR = p.odR;
                        p.doS = p.odS;
                        p = zarazkaEND(p, lines, tokpole);
                        p = GA(p, lines, hloubka + 1, tokpole, SLISTV);
                        if(p.chyba){
                            return p;
                        }
                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;
                        //aktivni tok END

                        pisNa(zapisovany_radek++, 0, "POPFRAME");
                        pisNa(zapisovany_radek++, 0, "PUSHS GF@$neporadek");
                        pisNa(zapisovany_radek++, 0, "RETURN");
                        ramFileSwap(&zapisovany_radek, 0);
                        p.vnoreni--;
                        tab_lokmain = zaloha_lok;
                        p = posunHlavu(p, 2, lines, tokpole);
                        p.doR = zaloha_zalohy.doR;
                        p.doS = zaloha_zalohy.doS;
                        p = GA(p, lines, hloubka +1, tokpole, SLISTV);
                        if(p.chyba)
                            return p;

                        break;
                    }
                    case WHILE:{/////////////////////////////////////////////////////// WHILE
                      p.vnoreni++;
                      sa_pos zaloha = p;
                      p = zarazkaTOK(p, lines, tokpole, DO);
                      if(p.chyba)
                          return p;

                      p = posunHlavu(p, 1, lines, tokpole);
                     //aktivni token IDENTIFIKATOR

                      int first_label = labelN++;
                      pisNa(zapisovany_radek++, 0, "LABEL WHILE-start$%d", first_label);

                      p = MA(p, tokpole, tab_lokmain);
                      if(p.chyba)
                          return p;

                      p.doR = zaloha.doR;
                      p.doS = zaloha.doS;
                      //aktiv==DO
                      p = posunHlavu(p, 1, lines, tokpole);
                      //aktivni token KONEC_RADKU
                      p = posunHlavu(p, 1, lines, tokpole);


                      int second_label = labelN++;
                      generateIf(&zapisovany_radek, second_label);

                      //vnitrek While
                      p.doR = p.odR;
                      p.doS = p.odS;
                      p = zarazkaEND(p, lines, tokpole);
                      p = GA(p, lines, hloubka + 1, tokpole, SLISTV);
                      if(p.chyba)
                          return p;

                      pisNa(zapisovany_radek++, 0, "JUMP WHILE-start$%d", first_label);
                      generateElse(&zapisovany_radek, second_label);
                      pisNa(zapisovany_radek++, 0, "MOVE GF@$neporadek nil@nil");

                      //ZA WHILE //aktiv==END
                      p.doR = zaloha.doR;
                      p.doS = zaloha.doS;

                      p.vnoreni--;
                      p = posunHlavu(p, 2, lines, tokpole);

                      p = GA(p, lines, hloubka +1, tokpole, SLISTV);
                      if(p.chyba)
                          return p;

                      break;
                    }
                    case ELSEIF:{ ///////////////////////////////////////////////////////////////////////////////////////// ELSEIF
                        p.vnoreni++;
                        sa_pos zaloha = p;
                        p = posunHlavu(p, 1, lines, tokpole);
                        //nastaveni zarazky na then
                        p = zarazkaTOK(p, lines, tokpole, THEN);
                        if(p.chyba)
                            return p;

                        p = MA(p, tokpole, tab_lokmain);
                        if(p.chyba)
                            return p;

                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;
                        //aktiv==THNEN
                        p = posunHlavu(p, 1, lines, tokpole);
                        //aktivni token KONEC_RADKU
                        p = posunHlavu(p, 1, lines, tokpole);

                        //Vnitrek IFu
                        p.lokal_label = labelN++;
                        generateIf(&zapisovany_radek, p.lokal_label);
                        pisNa(zapisovany_radek++, 0, "MOVE GF@$neporadek nil@nil");

                        p.doR = p.odR;
                        p.doS = p.odS;
                        //nastaveni zarazky na prislusny end
                        p = zarazkaEND(p, lines, tokpole);
                        p = GA(p, lines, hloubka + 1, tokpole, SBLOKV);
                        if(p.chyba)
                            return p;

                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;

                        break;
                    }
                    case IF:{ ///////////////////////////////////////////////////////////////////////////////////////////////// IF
                        p.vnoreni++;
                        sa_pos zaloha = p;
                        p = posunHlavu(p, 1, lines, tokpole);
                        p = zarazkaTOK(p, lines, tokpole, THEN);
                        if(p.chyba)
                            return p;

                        p = MA(p, tokpole, tab_lokmain);
                        if(p.chyba)
                            return p;
                        if(tokpole[p.odR][p.odS].id != THEN){
                           fprintf(stderr, "CHYBA:GA %d: MATIKA nedošla k then\n", hloubka);
                           p.chyba = ERR_SYS;
                           return p;
                        }

                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;
                        //aktiv==THNEN
                        p = posunHlavu(p, 1, lines, tokpole);
                        //aktivni token KONEC_RADKU
                        p = posunHlavu(p, 1, lines, tokpole);
                        //Vnitrek IFu
                        p.lokal_label = labelN++;
                        generateIf(&zapisovany_radek, p.lokal_label);
                        pisNa(zapisovany_radek++, 0, "MOVE GF@$neporadek nil@nil");

                        p.doR = p.odR;
                        p.doS = p.odS;
                        p = zarazkaEND(p, lines, tokpole);
                        p = GA(p, lines, hloubka + 1, tokpole, SBLOKV);
                        if(p.chyba)
                            return p;

                        //ZA IF //aktiv==END
                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;
                        //aktivni token END
                        p = posunHlavu(p, 2, lines, tokpole);

                        p = GA(p, lines, hloubka +1, tokpole, SLISTV);
                        if(p.chyba)
                            return p;

                        break;
                    }
                    default: { /////////////////////////////////////////////////////////////////////////////////// default
                        // LV -> V \n LV
                        sa_pos zaloha = p;

                        ////////////////////////
                        if(semNaZaraz(p)){
                            return p;
                        }
                        ////////////////////////
                        p.doS = delkaRadku(lines, p.odR, tokpole) - 1;
                        p.doR = p.odR;

                        p = GA(p, lines, hloubka+1, tokpole, SVYRAZ);
                        if (p.chyba)
                            return p;
                        if(tokpole[p.odR][p.odS].id != KONEC_RADKU){
                           fprintf(stderr, "CHYBA:GA %d: MATIKA nedosla na konec radku\n", hloubka);
                           p.chyba = ERR_SYS;
                           return p;
                        }

                        p.doR = zaloha.doR;
                        p.doS = zaloha.doS;
                        p = posunHlavu(p, 1, lines, tokpole);
                        p = GA(p, lines, hloubka+1, tokpole, SLISTV);
                        if(p.chyba)
                            return p;

                        break;
                    }
                }
            break;
        }
        case SBLOKV: { ////////////////////////////////////////////////////////////////////////// SBLOKV

            sa_pos zaloha = p;
            p.doR = p.odR;
            p.doS = p.odS;
            sa_pos zaloha2 = p;
            TOKTYPE status; //0 - nic //1 - else //2 - ELSEIF

            //hledáni elsif
            p = zarazkaELS(p, lines, tokpole, ELSEIF);
            if(p.chyba){
                p = zaloha2;
                //hledáni else
                p = zarazkaELS(p, lines, tokpole, ELSE);
                if(p.chyba){
                    status = NIL;
                } else {
                    status = ELSE;
                }
            } else {
                status = ELSEIF;
            }
            ///////////////////////////
            if(status == NIL){
                //SBLOKV-> SLISTV
                p = zaloha;
                p = GA(p, lines, hloubka+1, tokpole, SLISTV);
                if(p.chyba){
                    fprintf(stderr, "CHYBA:GA %d:SBLOKV:NIL: chybny vnitrek IF\n", hloubka);
                    return p;
                }
                p.lokal_label = zaloha.lokal_label;
                generateElse(&zapisovany_radek, p.lokal_label);
                p.vnoreni--;
                p.doR = zaloha.doR;
                p.doS = zaloha.doS;
            } else if(status == ELSE){ ///////////////////////////////////////////////////////////
                //SBLOKV-> SLISTV else \n SLISTV
                p = GA(p, lines, hloubka+1, tokpole, SLISTV);

                if(p.chyba){
                    fprintf(stderr, "CHYBA:GA %d:SBLOKV:ELSE-1: chybny vnitrek IF\n", hloubka);
                    return p;
                }
                int second_label = labelN++;
                pisNa(zapisovany_radek++, 0, "JUMP IF-end$%d", second_label);

                p.lokal_label = zaloha.lokal_label;
                generateElse(&zapisovany_radek, p.lokal_label);
                pisNa(zapisovany_radek++, 0, "MOVE GF@$neporadek nil@nil");

                //druha cast; aktiv == ELSE

                p = posunHlavu(p, 2, lines, tokpole);
                p.doR = zaloha.doR;
                p.doS = zaloha.doS;
                p = GA(p, lines, hloubka+1, tokpole, SLISTV);
                if(p.chyba){
                    fprintf(stderr, "CHYBA:GA %d:SBLOKV:ELSE-2: chybny vnitrek IF\n", hloubka);
                    return p;
                }
                pisNa(zapisovany_radek++, 0, "LABEL IF-end$%d", second_label);
                p.vnoreni--;

            } else if(status == ELSEIF){ //////////////////////////////////////////////////////////
                //SBLOKV-> SLISTV elseif MA() thnen \n SBLOKV
                p = GA(p, lines, hloubka+1, tokpole, SLISTV);
                if(p.chyba){
                    fprintf(stderr, "CHYBA:GA %d:SBLOKV:ELSEIF: chybny vnitrek IF\n", hloubka);
                    return p;
                }
                int second_label = labelN++;
                pisNa(zapisovany_radek++, 0, "JUMP IF-end$%d", second_label);

                p.lokal_label = zaloha.lokal_label;
                generateElse(&zapisovany_radek, p.lokal_label);

                //druha cast; aktiv == ELSEIF
                assert(tokpole[p.odR][p.odS].id == ELSEIF);
                p.doR = zaloha.doR;
                p.doS = zaloha.doS;
                p = GA(p, lines, hloubka+1, tokpole, SLISTV);
                if(p.chyba){
                    fprintf(stderr, "CHYBA:GA %d:SBLOKV:ELSEIF-2: chybny vnitrek IF\n", hloubka);
                    return p;
                }
                pisNa(zapisovany_radek++, 0, "LABEL IF-end$%d", second_label);
                p.vnoreni--;
            }

            break;
        }
        default : {
            break;
        }
    }

    return p;
}
