/*  Soubor: Knihovna pro optimalizace
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *
 *  Datum:  15.11.2018
 *
 */

#include "parser.h"
#include "ramfile.h"
#include "matematika.h"

#include "vestavene_fce.h"

#include <stdbool.h>
#include <stdio.h>


#include "vestavene_fce.h"

//enum pro snadne chapani chovani funkce dejToken
typedef enum {
    DEJ_TOKEN,
    VRAT_TOKEN
} ridic;


static int zavyraz(int neprazdny, token **tokpole);
//Pomocna funkce, ktera dava 1 nasledujici token.
//Muze se maximalne o 1 token vratit.
static token dejToken(ridic akce, token **tok)
{
    static int radek = 0;
    static int old_radek = 0;
    static int sloupec = 0;
    static int old_sloupec = 0;
    token ret = {0};

    if (akce == DEJ_TOKEN) {
        old_radek = radek;
        old_sloupec = sloupec;
        ret = tok[radek][sloupec];
        if (ret.id == KONEC_RADKU) {
            ++radek;
            sloupec = 0;
        } else {
            if (tok[radek][sloupec].id != FILEND)
                ++sloupec;
        }
    } else if (akce == VRAT_TOKEN) {
        radek = old_radek;
        sloupec = old_sloupec;
    } else
        fprintf(stderr, "Error: Tohle neni platna akce.\n");

    return ret;
}

//Rekurzivni syntakticka analyza, vraci 0, nebo kod chyby.
int LLSA(token **tokpole, neterminaly_t net)
{
    int retval = 0;
    //Nacte token pro zpracovani
    token tok = dejToken(DEJ_TOKEN, tokpole);

    switch (net) {
        //Zpracovani hlavniho tela programu
        case PROG: {
                //Uspesne ukonceni
                if (tok.id == FILEND){
                    return 0;
                }
                //Kontrola syntaxe definice funkce
                if (tok.id == DEF){
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if (tok.id != IDENTIFIKATOR){
                        fprintf(stderr, "CHYBA:LLSA: v definici fce je ocekavan identifikator.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != ZAV_ZAC){
                        fprintf(stderr, "CHYBA:LLSA: za definici funkce je ocekavana zavorka.\n");
                        return ERR_SYNTAX;
                    }
                    retval = LLSA(tokpole, ARG);
                    if(retval){
                        return retval;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != ZAV_END){
                        fprintf(stderr, "CHYBA:LLSA: za parametry je ocekavan konec zavorky.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != KONEC_RADKU){
                        fprintf(stderr, "CHYBA:LLSA: za definici fce je ocekavan eol.\n");
                        return ERR_SYNTAX;
                    }
                    retval = LLSA(tokpole, LISTV);
                    if(retval){
                        return retval;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != END){
                        fprintf(stderr, "CHYBA:LLSA: za definici fce je ocekavan end.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != KONEC_RADKU){
                        fprintf(stderr, "CHYBA:LLSA: za end fce je ocekavan eol.\n");
                        return ERR_SYNTAX;
                    }
                    //Poracuje se ve zpracovavani hlavniho tela programu
                    return LLSA(tokpole, PROG);
                }
                //Kontrola syntaxe ifu
                if (tok.id == IF){
                    retval = zavyraz(0, tokpole);
                    if(retval){
                        return retval;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != THEN){
                        fprintf(stderr, "CHYBA:LLSA: za if ocekavan then.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != KONEC_RADKU){
                        fprintf(stderr, "CHYBA:LLSA: za then ocekavan eol.\n");
                        return ERR_SYNTAX;
                    }
                    retval = LLSA(tokpole, LISTIF);
                    if(retval){
                        return retval;
                    }
                    retval = LLSA(tokpole, BLOKV);
                    if(retval){
                        return retval;
                    }
                    //Poracuje se ve zpracovavani hlavniho tela programu
                    return LLSA(tokpole, PROG);
                }
                //Kontrola syntaxe cyklu while
                if (tok.id == WHILE){
                    retval = zavyraz(0, tokpole);
                    if(retval){
                        return retval;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != DO){
                        fprintf(stderr, "CHYBA:LLSA: za while je ocekavan do.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != KONEC_RADKU){
                        fprintf(stderr, "CHYBA:LLSA: za do je ocekavan eol.\n");
                        return ERR_SYNTAX;
                    }
                    retval = LLSA(tokpole, LISTV);
                    if(retval){
                        return retval;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != END){
                        fprintf(stderr, "CHYBA:LLSA: za cyklem je ocekavan end.\n");
                        return ERR_SYNTAX;
                    }
                    tok = dejToken(DEJ_TOKEN, tokpole);
                    if(tok.id != KONEC_RADKU){
                        fprintf(stderr, "CHYBA:LLSA: za end je ocekavan eol.\n");
                        return ERR_SYNTAX;
                    }
                    //Poracuje se ve zpracovavani hlavniho tela programu
                    return LLSA(tokpole, PROG);
                }
                //V jinem pripade se zpracovava prikaz, pro ten se vraci token,
                //aby si jej mohl precist
                dejToken(VRAT_TOKEN, tokpole);
                retval = LLSA(tokpole, PRIKAZ);
                if(retval){
                    return retval;
                }
                //Pokracovani v hlavnim tele programu
                return LLSA(tokpole, PROG);
        }
        //telo cyklu nebo funkce
        case LISTV:{
            //Kontrola syntaxe ifu
            if (tok.id == IF){
                retval = zavyraz(0, tokpole);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != THEN){
                    fprintf(stderr, "CHYBA:LLSA: za if v listv je ocekavan then.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za then v listv je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTIF);
                if(retval){
                    return retval;
                }
                retval = LLSA(tokpole, BLOKV);
                if(retval){
                    return retval;
                }
                //Poracuje se ve zpracovavani  tela funkce nebo cyklu
                return LLSA(tokpole, LISTV);
            }
            //Kontrola syntaxe cyklu while
            if (tok.id == WHILE){
                retval = zavyraz(0, tokpole);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != DO){
                    fprintf(stderr, "CHYBA:LLSA: za while je ocekavan do.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za do je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTV);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != END){
                    fprintf(stderr, "CHYBA:LLSA: za listv ve while je ocekavan end.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za end whilu je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                //Poracuje se ve zpracovavani  tela funkce nebo cyklu
                return LLSA(tokpole, LISTV);
            }
            //Ukonceni funkce nebo cyklu
            if (tok.id == END) {
                dejToken(VRAT_TOKEN, tokpole);
                return 0;
            }
            //V jinem pripade se zpracovava prikaz, pro ten se vraci token,
            //aby si jej mohl precist
            dejToken(VRAT_TOKEN, tokpole);
            retval = LLSA(tokpole, PRIKAZ);
            if(retval){
                return retval;
            }
            //Pokracovani v tele funkce nebo cyklu
            return LLSA(tokpole, LISTV);
        }
        //Zpracovani tela ifu
        case LISTIF:{
            //Kontrola syntaxe ifu
            if (tok.id == IF){
                retval = zavyraz(0, tokpole);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != THEN){
                    fprintf(stderr, "CHYBA:LLSA: za if v listif je ocekavan then.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za then v listif je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTIF);
                if(retval){
                    return retval;
                }
                retval = LLSA(tokpole, BLOKV);
                if(retval){
                    return retval;
                }
                //Poracuje se ve zpracovavani ifu
                return LLSA(tokpole, LISTIF);
            }
            //Kontrola syntaxe whilu
            if (tok.id == WHILE){
                int retval = zavyraz(0, tokpole);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != DO){
                    fprintf(stderr, "CHYBA:LLSA: za while v listif je ocekavan do.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za do v listif je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTV);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != END){
                    fprintf(stderr, "CHYBA:LLSA: za listv ve whilu v listif je ocekavan end.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za end whilu v listif je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                //Poracuje se ve zpracovavani ifu
                return LLSA(tokpole, LISTIF);
            }
            //Ukonceni tela ifu
            if (tok.id == END || tok.id == ELSEIF || tok.id == ELSE) {
                dejToken(VRAT_TOKEN, tokpole);
                return 0;
            }
            //V jinem pripade se zpracovava prikaz, pro ten se vraci token,
            //aby si jej mohl precist
            dejToken(VRAT_TOKEN, tokpole);
            retval = LLSA(tokpole, PRIKAZ);
            if(retval){
                return retval;
            }
            //Pokracovani ve zpracovavani ifu
            return LLSA(tokpole, LISTIF);
        }
        //Zpracovavani argumentu funkce
        case ARG:{
            //Ukonceni, pokud prijde konec zavorky
            if(tok.id == ZAV_END){
                dejToken(VRAT_TOKEN, tokpole);
                return 0;
            }
            //Kontrola syntaxe, pokud funkce obsahuje argumenty
            if(tok.id == IDENTIFIKATOR){
                int delka = strlen(tok.attr) - 1;
                if(tok.attr[delka] != '?' && tok.attr[delka] != '!'){
                    //Pokracuje zpracovavanim dalsich argumentu
                    return LLSA(tokpole, NARG);
                }
            }
            //Chyba, pokud argument neni identifikator
            fprintf(stderr, "CHYBA:LLSA: jako argument ocekavan identifikator.\n");
            return ERR_SYNTAX;
        }
        //Zpracovavani neprvniho argumentu funkce
        case NARG:{
            //Ukonceni, pokud prijde konec zavorky
            if(tok.id == ZAV_END){
                dejToken(VRAT_TOKEN, tokpole);
                return 0;
            }
            //Pokud nebyl konec zavorky, ani neprijde carka, jedna se o chybu
            if(tok.id != CARKA){
                fprintf(stderr, "CHYBA:LLSA: ocekavana carka nebo konec zavorky.\n");
                return ERR_SYNTAX;
            }
            //Zpracovani argumentu
            tok = dejToken(DEJ_TOKEN, tokpole);
            if(tok.id == IDENTIFIKATOR){
                int delka = strlen(tok.attr) - 1;
                if(tok.attr[delka] != '?' && tok.attr[delka] != '!'){
                    //Pokracuje zpracovavanim dalsich argumentu
                    return LLSA(tokpole, NARG);
                }
            }
            //Chyba, pokud argumen neni identifikator
            fprintf(stderr, "CHYBA:LLSA: jako nargument ocekavan identifikator.\n");
            return ERR_SYNTAX;
        }
        //Zpracovavani prikazu
        case PRIKAZ:{
            //Spravne ukonceni, pokud je radek prazdny
            if(tok.id == KONEC_RADKU){
                return 0;
            }
            //Pokud radek zacina identifikatorem, zkontroluje se, jestli muze nepatrit funkci.
            //Kdyz ano, zpracovava se ve stavu IDN1, jinak se posouva za vyraz.
            if(tok.id == IDENTIFIKATOR){
                int delka = strlen(tok.attr) - 1;
                if(tok.attr[delka] != '?' && tok.attr[delka] != '!'){
                    return LLSA(tokpole, IDN1);
                }
            }
            dejToken(VRAT_TOKEN, tokpole);
            retval = zavyraz(0, tokpole);
            if(retval){
                return retval;
            }
            tok = dejToken(DEJ_TOKEN, tokpole);
            //Chyba, pokud za vyrazem neni konec radku
            if(tok.id != KONEC_RADKU){
                fprintf(stderr, "CHYBA:LLSA: za prikazem je ocekavan eol.\n");
                return ERR_SYNTAX;
            }
            return 0;
        }
        //Zpracovani prikazu od druheho tokenu
        case IDN1: {
            //V pripade prirazeni se zkontroluje vyraz za znakem prirazeni
            if(tok.id == PRIRAD){
                retval = zavyraz(0, tokpole);
                if(retval) {
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    return ERR_SYNTAX;
                }
                return 0;
            }
            //Spravne ukoncnei, pokud byl na radku jen token identifikator
            if(tok.id == KONEC_RADKU) {
                return 0;
            }
            //Preskoci vyraz, cislem 1 da najevo, ze 1 token jiz byl zpracovan
            dejToken(VRAT_TOKEN, tokpole);
            return zavyraz(1, tokpole);
        }
        //Zpracovani ELSIF a ELSE vetvi ifu a jeho ukonceni
        case BLOKV:{
            //Kontrola syntaxe elsif vetve
            if(tok.id == ELSEIF){
                retval = zavyraz(0, tokpole);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != THEN){
                    fprintf(stderr, "CHYBA:LLSA: za elsif je ocekavan then.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za then v elsif je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTIF);
                if(retval){
                    return retval;
                }
                return LLSA(tokpole, BLOKV);
            }
            //Kontrola syntace else vetve a ukonceni cyklu
            if(tok.id == ELSE){
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za else je ocekavan eol\n");
                    return ERR_SYNTAX;
                }
                retval = LLSA(tokpole, LISTV);
                if(retval){
                    return retval;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != END){
                    fprintf(stderr, "CHYBA:LLSA: za else v blokv je ocekavan end.\n");
                    return ERR_SYNTAX;
                }
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za end za else v blokv je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                return 0;
            }
            //Ukonceni cyklu bez else vetve
            if(tok.id == END){
                tok = dejToken(DEJ_TOKEN, tokpole);
                if(tok.id != KONEC_RADKU){
                    fprintf(stderr, "CHYBA:LLSA: za end v blokv je ocekavan eol.\n");
                    return ERR_SYNTAX;
                }
                return 0;
            }
            //Chyba, pokud nenastal ani jeden z vyššich pripadu
            fprintf(stderr, "CHYBA:LLSA: v blokv prislo neco necekane.\n");
            return ERR_SYNTAX;
        }

        default:
            //Abnormalni chyba, pokud se podarilo dostat sem
            fprintf(stderr, "CHYBA:LLSA: spusteno s chybnym argumentem.\n");
            return 99;
    }
}
//Funkce slouzici k preskoceni vyrazu a pripadne jeho vyhodnoceni, jako chybny.
static int zavyraz(int neprazdny, token **tokpole){
    //Promenna pro uchovnai tokenu
    token tok;
    //Promenna pro kontrolu chybejiciho/prebyvajiciho znaku ternarniho operatoru
    int ternarni = 0;
    //Promenna pro kontrolu chybejicich/prebyvajicich zavorek
    int zavorky = 0;
    //Zpracovavani do odhaleni tokenu, ktery nenalezi vyrazu
    while (1) {
        tok = dejToken(DEJ_TOKEN, tokpole);
        switch (tok.id) {
            case PRIRAD:
                fprintf(stderr, "CHYBA:LLSA: Prirazeni ve vyrazu.\n");
                return ERR_SYNTAX;
            case FILEND:
                fprintf(stderr, "CHYBA:LLSA: Konec souboru kde nema byt.\n");
                return ERR_SYNTAX;
            case DEF:
            case END:
            case IF:
            case THEN:
            case ELSE:
            case ELSEIF:
            case WHILE:
            case DO:
            case KONEC_RADKU:
                //Ukonceni s chybou, pokud nesedi pocet znaku ternarniho operratoru, zavorek, nebo je vyraz prazdny
                if (ternarni || zavorky || !neprazdny) {
                    fprintf(stderr, "CHYBA:LLSA: neukonceny ternarni operator, zavorky, nebo prazdny vyraz.\n");
                    return ERR_SYNTAX;
                }
                dejToken(VRAT_TOKEN, tokpole);
                return 0;
            case TER_ZAC:
                if (neprazdny == 0) {
                    fprintf(stderr, "CHYBA:LLSA: Pred Zacatkem ternarniho operatoru chybi vyraz.\n");
                    return ERR_SYNTAX;
                }
                --neprazdny;
                ternarni++;
                break;
            case TER_END:
                --neprazdny;
                ternarni--;
                break;
            case ZAV_ZAC:
                --neprazdny;
                zavorky++;
                break;
            case ZAV_END:
                --neprazdny;
                zavorky--;
                break;
            default:
                break;
        }
        //Pokud se v kteremkoliv okamziku najde ukonceni driv nez zacatek, konci se s chybou
        if (zavorky < 0 || ternarni < 0) {
            fprintf(stderr, "CHYBA:LLSA: zavorky nebo ternarni operator ukonceny driv, nez zacaty.\n");
            return ERR_SYNTAX;
        }
        ++neprazdny;
    }
}
