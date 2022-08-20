/*  Knihovna matematika
 *
 *  Autor:  Josef Oskera, VUT FIT (xosker03)
 *          Radek Duchoň, VUT FIT (xducho07)
 *          Jan Juda, VUT FIT (xjudaj00)
 *  Datum: 15.11.2018
 *
 */


#include "matematika.h"
#include "ramfile.h"

#include "vestavene_fce.h"

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


#include "vestavene_fce.h"

extern int zapisovany_radek;
extern int labelN;
extern htab_t tab_fce;
extern htab_t tab_lokmain;


//////////////////////////////////////////////////////////// ZÁSOBNÍK vyuzivany MA

typedef struct tselem {
    token token;
    struct tselem *nextPtr;
} TSElem;

typedef struct tstack {
    TSElem *topPtr;
} TStack;

static void zas_init(TStack *s)
{
    s->topPtr = NULL;
}

static int zas_push(TStack *s, token tok)
{
    if (s) {
        TSElem *newElemPtr = calloc(1, sizeof(TSElem));
        if (newElemPtr) {
            newElemPtr->token = tok;
            newElemPtr->nextPtr = s->topPtr;
            s->topPtr = newElemPtr;
            return 0;
        }
    }
    return ERR_SYS;
}


static token zas_pop(TStack *s)
{
    token ret = {KONEC_RADKU};
    if (s) {
        TSElem *elemPtr;
        if (s->topPtr != NULL) {
            elemPtr = s->topPtr;
            ret = s->topPtr->token;
            s->topPtr = s->topPtr->nextPtr;
            free(elemPtr);
        }
    }
    return ret;
}

bool zas_empty(TStack *s)
{
    return !(s->topPtr);
}
//////////////////////////////////////////////////////////////////



//Hlavicky funkci MA
sa_pos ma_plus(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_minus(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_krat(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_deleno(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_and(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_or(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_vetsi(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_mensi(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_vetsirovno(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_mensirovno(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_rovno(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_nerovno(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_terzac(sa_pos p, TStack *zas, int *codezas);
sa_pos ma_terend(sa_pos p, TStack *zas, int *codezas);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////



//RADEK
typedef struct tselem2 {
    token data;
    struct tselem2 *nextPtr;
} TSElem2;

typedef struct tstack2 {
    TSElem2 *topPtr;
} TStack2;

void *Push(TStack2 *s, token d);
void Pop(TStack2 *s);
token Top(TStack2 *s);
bool SEmpty(TStack2 *s);
int priority(token p);
bool postfix(token *infix, token *postfix, int radek);
void SInit(TStack2 *s);

//Inicializuje zasobnik.
void SInit(TStack2 *s)
{
    s->topPtr = NULL;
}
//Da token do zasobniku, vrati NULL, pokud by se mu nezdarila alokace.
void *Push(TStack2 *s, token d)
{
    if (s) {
        TSElem2 *newElemPtr = calloc(1, sizeof(TSElem2));
        if (newElemPtr) {
            newElemPtr->data = d;
            newElemPtr->nextPtr = s->topPtr;
            s->topPtr = newElemPtr;
            return s;
        }
    }
    return NULL;
}
//Vyhodi horni token ze zasobniku.
void Pop(TStack2 *s)
{
    if (s) {
        TSElem2 *elemPtr;
        if (s->topPtr != NULL) {
            elemPtr = s->topPtr;
            s->topPtr = s->topPtr->nextPtr;
            free(elemPtr);
        }
    }
}
//Vraci token na vrcholu zasobniku.
token Top(TStack2 *s)
{
    return s->topPtr->data;
}
//Funkce rikajici, jestli je zasobnik prazdny
bool SEmpty(TStack2 *s)
{
    return !(s->topPtr);
}

//Pomocna funkce pro urceni priority operatoru
int priority(token c)
{
    if (c.id == KRAT || c.id == DELENO)
        return 10;
    else if (c.id == PLUS || c.id == MINUS)
        return 9;
    else if (c.id == VETSI || c.id == MENSI || c.id == MENSIROVNO || c.id == VETSIROVNO)
        return 8;
    else if (c.id == ROVNASE || c.id == NEROVNASE)
        return 7;
    else if (c.id == AND_Z)
        return 6;
    else if (c.id == OR_Z)
        return 5;
    else if (c.id == TER_ZAC || c.id == TER_END || c.id == CARKA)
        return 4;
    else if (c.id == PRIRAD)
        return 3;
    else if (c.id == OR || c.id == AND)
        return 1;

    return 0;
}
//Pomocna funkce rikajici, jestli je token nejaka konstanta
bool constant(token t) {
    if (t.id == BTRUE || t.id == BFALSE || t.id == NIL)
        return true;
    if (t.id == KONSTANTA_I || t.id == KONSTANTA_D || t.id == KONSTANTA_S)
        return true;

    return false;
}

//Funkce na prevedeni vyrazu z infixoveho do postfixoveho zapisu
//Zaroven hleda syntakticke chyby vyrazu.
bool postfix(token *infix, token *postfix, int radek)
{
    TStack2 zasobnik;
    TStack2 *s = &zasobnik;
    SInit(s);
    token tok;
    //Pomocne promenne pro indexaci postfixu a prioritu aktualniho a predesleho tokenu
    int j = 0, prio, old_prio = 0;
    //Pouze operatory maji nenulovou prioritu

    for (int i = 0; infix[i].id != KONEC_RADKU && infix[i].id != THEN && infix[i].id != DO; ++i) {
        tok = infix[i];
        prio = priority(tok);
        //Chyba, pokud byly nalezeny 2 operatory vedle sebe s vyjimkou ternarniho operatoru
        if (prio && prio != 4 && old_prio && old_prio != 4) {
            fprintf(stderr, "Error: 2 operatory vedle sebe na radku %d.\n", radek);
            return false;
        }
        if (old_prio == 4 && prio && prio != 4) {
            fprintf(stderr, "Error: ternarni operator/carka a hned operator na radku %d.\n", radek);
            return false;
        }
        if (prio == 4 && old_prio && old_prio != 4) {
            fprintf(stderr, "Error: operator a hned ternarni operator/carka na radku %d.\n", radek);
            return false;
        }
        if (old_prio == 4 && infix[i-1].id != TER_END && tok.id == ZAV_END) {
            fprintf(stderr, "Error: Carka a konec zavorky na radku %d\n", radek);
            return false;
        }
        //Chyba pokud byla carka vedle oternarniho peratoru
        if (prio && priority(infix[i+1]) && (tok.id == CARKA || infix[i+1].id == CARKA)) {
            fprintf(stderr, "Error: carka vedle ternarniho eoperatoru na radku %d.\n", radek);
            return false;
        }
        if ((tok.id == NOT || tok.id == NOT_Z) && infix[i+1].id == CARKA) {
            fprintf(stderr, "Error: carka vedle operatoru not na radku %d.\n", radek);
            return false;
        }
        //Chyba, pokud byl operator pred koncem zavorky, nebo za koncem zavorky nasleduje zacatek zavorky ci operand
        if (tok.id == ZAV_END) {
            if (old_prio || (i > 0 && (infix[i-1].id == NOT || infix[i-1].id == NOT_Z))) {
                fprintf(stderr, "Error: Operator pred koncem zavorky na radku %d.\n", radek);
                return false;
            }
            if (infix[i+1].id == ZAV_ZAC) {
                fprintf(stderr, "Error: Konec a hned zacatek zavorky na radku %d.\n", radek);
                return false;
            }
            if (constant(infix[i+1]) || infix[i+1].id == IDENTIFIKATOR) {
                fprintf(stderr, "Error: Identifikator nebo konstanta za koncem zavorky na radku %d.\n", radek);
                return false;
            }
        }
        //Chyba, pokud pted zacatkem zavorky je konstanta
        if (tok.id == ZAV_ZAC && i > 0 && !old_prio) {
            if (constant(infix[i-1])) {
                fprintf(stderr, "Error: Operand a za tim je zavorka na radku %d.\n", radek);
                return false;
            }
        }
        //Chyba, pokud jsou 2 operandy vedle sebe
        if (constant(tok) || tok.id == IDENTIFIKATOR) {
            if (constant(infix[i+1]) || infix[i+1].id == IDENTIFIKATOR) {
                fprintf(stderr, "Error: 2 operandy vedle sebe na radku %d.\n", radek);
                return false;
            }
        }
        //Chyby prazdnych argumentu
        if (tok.id == CARKA && infix[i+1].id == ZAV_END) {
            fprintf(stderr, "Error: Carka a konec zavorky na radku %d.\n", radek);
            return false;
        }
        if (tok.id == ZAV_ZAC && infix[i+1].id == CARKA) {
            fprintf(stderr, "Error: Zacatek zavorky a carka na radku %d.\n", radek);
            return false;
        }

        //Prevod doo postfixu, nemeni pozici ternarniho operatoru, carky a prirazeni
        if (prio < 5) {
            //Pri vyse zminenych se zapisuje do vyprazdneni zasobniku nebo nalezeni zavorky.
            //Nasledne zapise dany token.
            if (prio > 2) {
                while (!SEmpty(s) && Top(s).id != ZAV_ZAC) {
                    postfix[j++] = Top(s);
                    Pop(s);
                }
                postfix[j++] = tok;
                //Zapisuje tokeny ze zasobniku, dokud nemuze vlozit novy token na zasobnik.
            } else if (prio > 0) {
                while (!SEmpty(s) && prio <= priority(Top(s))) {
                    postfix[j++] = Top(s);
                    Pop(s);
                }
                Push(s, tok);
                //Vyprazdnuje zasobnik, dokud nenalezne zacatek zavorky.
            } else if (tok.id == ZAV_END) {
                while (Top(s).id != ZAV_ZAC) {
                    postfix[j++] = Top(s);
                    Pop(s);
                }
                postfix[j++] = tok;
                if (!SEmpty(s))
                    Pop(s);
                //Zapise si zacatek zavorky a pushne ji na zasobnik.
            } else if (tok.id == ZAV_ZAC) {
                Push(s, tok);
                postfix[j++] = tok;
                //Jinak si zapise token.
            } else {
                postfix[j++] = tok;
            }
            //Pokud je priorita vetsi, zapisuje a vyprazdnuje zasobnik, dokud nelze vlozit novy token
        } else {
            if (!(SEmpty(s) || Top(s).id == ZAV_ZAC)) {
                while (!SEmpty(s) && prio <= priority(Top(s))) {
                    postfix[j++] = Top(s);
                    Pop(s);
                }
            }
            Push(s, tok);
        }
        //Ulozi si predchozi prioritu pro snadnejsi syntaktickou kontrolu vyrazu.
        old_prio = prio;
    }
    //vypise zbytek zasobniku do postfixu
    while (!SEmpty(s)) {
        postfix[j++] = Top(s);
        Pop(s);
    }

    //Prepise postfixovy zapis infixovym
    memcpy(infix, postfix, j * sizeof(token));

    return true;
}

//Pro vnitrni potreby programu prohodi poradi argumentu funkci
void convertfce(token *tok, token *pole)
{
    //S pomoci uzavorkovani hleda konec funkce (do tehdy prohazuje argumenty)
    int zavorka = 1, end, prepis = 0;
    for (end = 0; zavorka; ++end) {
        if (tok[end].id == ZAV_ZAC)
            ++zavorka;
        else if (tok[end].id == ZAV_END)
            --zavorka;
    }
    --end;
    pole[end] = tok[end];//kopiruje konec zavorky
    int j = end - 1;//pred zavorkou
    //Hleda zacatek argumentu od posledniho a zapisuje je tak pozpatku
    for (int k = j; k >= 0; --k) {
        if (tok[k].id == ZAV_ZAC)
            ++zavorka;
        else if (tok[k].id == ZAV_END)
            --zavorka;
        else if (!zavorka && tok[k].id == CARKA) {
            for (int l = 1; l + k <= j ; ++l)
                pole[prepis++] = tok[l+k];

            pole[prepis++] = tok[k];//kopiruje carku
            j = k - 1;//posem bude dalsi kopirovani
        }
    }
    //Konecne nakopirovani prvniho argumentu na posledni
    for (int i = 0; i <= j; ++i)
        pole[prepis++] = tok[i];
    //Prehraje vstupni pole
    memcpy(tok, pole, sizeof(token) * end);
}
//Funkce, ktera ma na starost prevod vyrazu tak, aby funkce
//obsahovaly zavorky, argumenty byly opacne a nasledne
//prevod do postfixu a syntaktickou kontrolu vyrazu
int convert(token **tok, int radek, token *pole, int j)
{
    int zav_dopln = 0, zavorka = 0, carka = 0;
    //Zjistuje kolik zavorek bude potreba doplnit k funkcim
    for (int i = 0; i < j; ++i) {
        if (tok[radek][i].id == IDENTIFIKATOR && htab_find(tab_fce, tok[radek][i].attr) && tok[radek][i+1].id != ZAV_ZAC) {
            ++zav_dopln;
        } else if ((tok[radek][i].id == NOT || tok[radek][i].id == NOT_Z) && tok[radek][i+1].id != ZAV_ZAC)
            ++zav_dopln;
    }
    //vracim o kolik se zvetsi pole tokenu
    int vracim = zav_dopln * 2;
    symbol *data;
    if (zav_dopln) {
        void *ctrl = realloc(pole, (j + vracim)  * sizeof(token));
        if (!ctrl) {
            fprintf(stderr, "Error: selhal realloc.\n");
            free(pole);
            return -ERR_SYS;
        }
        pole = ctrl;
        ctrl = realloc(tok[radek], (j + vracim) * sizeof(token));
        if (!ctrl) {
            fprintf(stderr, "Error: selhal realloc.\n");
            free(pole);
            return -ERR_SYS;
        }
        tok[radek] = ctrl;
    }

    //Od konce doplnuji zavorky
    for (int i = j - 1; zav_dopln; --i) {
        if ((tok[radek][i].id == IDENTIFIKATOR && htab_find(tab_fce, tok[radek][i].attr)) || tok[radek][i].id == NOT || tok[radek][i].id == NOT_Z) {
            if (tok[radek][i].id == NOT || tok[radek][i].id == NOT_Z)
                data = htab_data(tab_fce, "not");
            else
                data = htab_data(tab_fce, tok[radek][i].attr);
            //Tady se musi doplnit zavorky
            if (tok[radek][i+1].id != ZAV_ZAC) {
                ++i;
                carka = 0;
                --zav_dopln;
                //Posun tokenu, aby se vytvorilo misto pro zavorku
                memmove(&tok[radek][i+1], &tok[radek][i], (j-i) * sizeof(token));
                j += 2;
                tok[radek][j-1] = tok[radek][j-2];
                tok[radek][i].id = ZAV_ZAC;
                //Pro vysokou prioritu ma operator ! vyjimku z chovani, uzavorkuje se co nejmensi vyraz za nim
                if (tok[radek][i-1].id == NOT_Z) {
                    if ((tok[radek][i+1].id == IDENTIFIKATOR && htab_find(tab_fce, tok[radek][i+1].attr)) || tok[radek][i+1].id == NOT || tok[radek][i+1].id == NOT_Z) {
                        int x = 3;//na pozici o 1 dal je teprve funkce, dal je zavorka, dal az se musi neco resit
                        zavorka = 1;
                        while (zavorka) {
                            if (tok[radek][i+x].id == ZAV_ZAC)
                                ++zavorka;
                            else if (tok[radek][i+x].id == ZAV_END)
                                --zavorka;
                            ++x;
                        }
                        memmove(&tok[radek][i+x+1], &tok[radek][i+x], (j-i-x-1) * sizeof(token));
                        tok[radek][i+x].id = ZAV_END;
                    } else {
                        memmove(&tok[radek][i+3], &tok[radek][i+2], (j-i-3) * sizeof(token));//Zkontrolovat
                        tok[radek][i+2].id = ZAV_END;
                    }
                } else {
                    zavorka = 1;
                    for (int x = 1; i + x < j - 1; ++x) {
                        if (tok[radek][i+x].id == ZAV_ZAC)
                            ++zavorka;
                        else if (tok[radek][i+x].id == ZAV_END)
                            --zavorka;
                        else if (tok[radek][i+x].id == CARKA)
                            ++carka;

                        if (!zavorka || data->pocet_param == carka) {
                            memmove(&tok[radek][i+x+1], &tok[radek][i+x], (j-i-x-2) * sizeof(token));//zkontrolovat
                            tok[radek][i+x].id = ZAV_END;
                            break;
                        }
                    }
                    if (tok[radek][j-2].id == 0) {
                        tok[radek][j-2].id = ZAV_END;
                    }
                }
            }
        }
    }

    for (int i = 0; i < j; ++i) {
        pole[i] = tok[radek][i];
        if (tok[radek][i].id == IDENTIFIKATOR && htab_find(tab_fce, tok[radek][i].attr)) {
            data = htab_data(tab_fce, tok[radek][i].attr);
            ++i;
            pole[i] = tok[radek][i];//kopirovani zavorky
            //Neni potreba otacet argumenty funkce, ktera ma 0-1 argument.
            //mene nez 0 znaci funkce s promenlivym poctm parametru.
            if (data->pocet_param > 1 || data->pocet_param < 0)
                convertfce(&tok[radek][i+1], &pole[i+1]);
            else if (data->pocet_param == 0 && tok[radek][i+1].id != ZAV_END) {
                fprintf(stderr, "Error: Funkce by nemela mit zadne parametry.\n");
                free(pole);
                return -ERR_PARAM;
            }
        }
    }
    //Vraceni pripadne chyby pri prevodu do postfixu
    if (!postfix(tok[radek], pole, radek)) {
        free(pole);
        return -ERR_SYNTAX;
    }

    free(pole);

    return vracim;
}

/**
 * MA - Matematicky analyzator
 * Matematicky analyzator provadi syntaktickou a sémantickou kontrolu vyrazu a téz pro ne primo i generuje kód.
 *
 * @param p - ridici struktura, ktera MA rika, na kterém radku (p.odR) a od jakého (p.odS) do jakého (p.doS) sloupce ma zpracovavat tokeny
 * @param tokpole - 2D pole tokenu
 * @param tab_lok - ukazatel na lokalni tabulku symbolu. Muze byt tabulkou symtbolu hlavniho tela programu nebo nekteré z definovanych funkci
 * @return MA vraci ridici strukturu, pres kterou téz vraci i kód chyby, pokud k nejaké doslo (p.chyba)
 */
sa_pos MA(sa_pos p, token **tokpole, htab_t tab_lok){
    int codezas = 0; //Pocitadlo kolik mezivysledku je na assemblerovém zasobniku
    //kontrola rozsahu, token na indexu p.doS se jiz nezpracovava
    if(p.odS + 1 > p.doS){
        fprintf(stderr, "Chybi matematicky vyraz na radku %d.\n", p.odR);
        p.chyba = ERR_SEM_JINAK;
        return p;
    }
    //kontrola tokenu nepatricich do vyrazu
    int nezavorky=0, ternarniP=0;
    for(int i = p.odS; i < p.doS; i++){
        switch (tokpole[p.odR][i].id){
            case PRIRAD:
            case DEF:
            case END:
            case IF:
            case THEN:
            case ELSE:
            case ELSEIF:
            case WHILE:
            case DO:
            case KONEC_RADKU:
                fprintf(stderr, "Error %d|%d: Nepovoleny token(%d) v matematice.\n", p.odR, i, tokpole[p.odR][i].id);
                p.chyba = ERR_SEM_JINAK;
                return p;
            case TER_ZAC:
                nezavorky++;
                break;
            case TER_END:
                nezavorky++;
                break;
            //zavorky jsou OK, ale potrebuju zkontrolovat, ze je tam i neco krom nich
            case ZAV_ZAC:
                break;
            case ZAV_END:
                break;
            default:{
                //povoleny tokeny
                nezavorky++;
                break;
            }
        }
    }
    //kontrola, zda se ve vyraze nachazi i neco jiného, nez jen zavorky
    if(!nezavorky){
        fprintf(stderr, "Chybi matematicky vyraz na radku %d.\n", p.odR);
        p.chyba = ERR_SEM_JINAK;
        return p;
    }
    //Priprava na volani Convertu
    int tok_count = 0;
    while(tokpole[p.odR][tok_count++].id != KONEC_RADKU);

    token *Redpole = calloc(tok_count, sizeof(token));

    if (!Redpole){
        fprintf(stderr, "nepodarena alokace\n");
        p.chyba = ERR_SYS;
        return p;
    }
    int radek_mi_vratil;
    //Zde se vola convert a kontroluje se, zda v nem nenastala chyba
    if ((radek_mi_vratil = convert(tokpole, p.odR, Redpole, tok_count)) < 0){
        p.chyba = -radek_mi_vratil;
        return p;
    }

    p.doS += radek_mi_vratil;

    //Vnitrni zasobnik uschovavajici funkce, zavorky patrici k funkcim, carky jakozto oddelovace parametru a konstanty zduvodu optimalizace
    TStack zasobnik;
    TStack *zas = &zasobnik;
    zas_init(zas);

    //Zasobnik pro ternarni operatory. Uchovava se v nem informace o cislu labelu, kterym ternarni operator konci
    TStack terzassob;
    TStack *terzas = &terzassob;
    zas_init(terzas);

    //Zasobnik pro zpracovavani funkci.
    TStack zasobnik_fce;
    TStack *fce = &zasobnik_fce;
    zas_init(fce);

    bool byla_fce = false;  //Tato promenna udava, zda posledni zpracovany token byl identifikator funkce
    //Promenna fci udava pocet "otevrenych" funkci -> napr print( a(5, b(X)), "\n") v miste pracovavani tokenu X je fci == 3
    int fci = 0, zav_ignore = 0;
    //Cyklus zpracovavajici jednotlivé tokeny
    while(p.odS < p.doS){
        token t;
        //Rozhodovani se dle aktualniho tokenu
        switch(tokpole[p.odR][p.odS].id){
            case ZAV_ZAC:
                //pokud pred zavorkou nebyla fce, tak ji ignoruju
                if (byla_fce){
                    zas_push(zas, tokpole[p.odR][p.odS]);
                    byla_fce = false;
                    fci++;  //az v této chvili se "otevira" funkce - zpracovavame jeji parametry
                }
                //Pokud mam nacatou funkci a v ni je '(' a neni to zacatek dalsi funkce, tak k ni musim ignorovat jednu ')'
                else if(fci > 0){
                    zav_ignore++;
                }
                break;
            case ZAV_END:
                //Ignoruju ')' k predeslym ne-funkcnim '('
                byla_fce = false;
                if(zav_ignore > 0){
                    zav_ignore--;
                }
                //pokud pred ')' nemame otevrenou fci, tak ji ignoruju
                else if (fci > 0){
                    //Zde probiha vyhodnoceni funkci
                    token zeZas;
                    int operandy = 0, carky = 0; //tyto pocitadla slouzi pro naslednou kontrolu syntaxe
                    //Zde probiha zpetné vybirani carek a parametru (konstant) funkci. Nacitani skonci, kdyz dojedeme na '('
                    while((zeZas = zas_pop(zas)).id != ZAV_ZAC){
                        if(zeZas.id == CARKA){
                            carky++; //zvysime pocitadlo carek
                        }
                        else {
                            //Ulozime si operand (konstantu) na vedlejsi zasobnik - pro optimalizovani vestavenych fci - a zvysime pocitadlo operandu
                            zas_push(fce, zeZas);
                            operandy++;
                        }
                    }
                    //tady by mel bit token s jménem funkce - posledni jsme nacetli '('
                    zeZas = zas_pop(zas);
                    if(zeZas.id != IDENTIFIKATOR){
                        fprintf(stderr, "1-Syntakticka chyba na radku %d.\n", p.odR);
                        p.chyba = ERR_PARAM;
                        return p;
                    }
                    symbol *var = htab_data(tab_fce, zeZas.attr);
                    if(!var){
                        fprintf(stderr, "Syntakticka chyba na radku %d.\n", p.odR);
                        p.chyba = ERR_PARAM;
                        return p;
                    }

                    //Kontrola poctu carek
                    if(var->pocet_param != PROMENY_POC_PARAM && var->pocet_param != carky + 1 && var->pocet_param !=0 ){
                        fprintf(stderr, "Chybny pocet carek fce na radku %d.\n", p.odR);
                        p.chyba = ERR_PARAM;
                        return p;
                    }
                    //Kontrola poctu parametru
                    if(var->pocet_param != PROMENY_POC_PARAM && (var->pocet_param > (codezas + operandy) || operandy > var->pocet_param)){
                        fprintf(stderr, "Chybny pocet operandu fce na radku %d.\n", p.odR);
                        p.chyba = ERR_PARAM;
                        return p;
                    }

                    //Zde se vygeneruje kód volani funkci
                    if(!strcmp("print", zeZas.attr)){
                        //Pro funkci print nevyuzijeme nijak toho, ze vime, ze nejaky operand je konstanta, tak je napushujem na codezas
                        for(int i = 0; i < operandy ;i++){
                            token c = zas_pop(fce);
                            switch(c.id){
                                case KONSTANTA_D:
                                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", c.attr_d);
                                    codezas++;
                                    break;
                                case KONSTANTA_I:
                                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", c.attr_i);
                                    codezas++;
                                    break;
                                case KONSTANTA_S:
                                    pisNa(zapisovany_radek++, strlen(c.attr), "PUSHS string@%s", c.attr);
                                    codezas++;
                                    break;
                                case BTRUE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                    codezas++;
                                    break;
                                case BFALSE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                    codezas++;
                                    break;
                                case NIL:
                                    pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                    codezas++;
                                    break;
                                default:
                                    break;
                            }
                        }
                        codezas += operandy;
                        if(codezas <= 0){
                            fprintf(stderr, "Chybi operand/y vestavene fce print na radku %d.\n", p.odR);
                            p.chyba = ERR_PARAM;
                            return p;
                        }
                        v_print(&zapisovany_radek, carky + 1);
                        //odectu si pocet param printu ze zas a prictu +1 za ret hodnotu fce
                        codezas -= carky;
                    }
                    else{
                        if(!strcmp("length", zeZas.attr)){
                            //Pokud mam operand jako konstantu, zavolam zderivovanou funkci length pro konstantu. Stejne tak u ostatnich vestavenych fci...
                            if(operandy){
                                token c = v_length_K(zas_pop(fce));
                                if (c.id == KONEC_RADKU){
                                    fprintf(stderr, "Spatny typ parametru fce\n");
                                    p.chyba = ERR_SEM_OPER;
                                    return p;
                                }
                                else
                                    zas_push(zas, c);
                            }
                            else{
                                //jeden parametr, jeden vystup
                                v_length(&zapisovany_radek);
                            }
                        }
                        else if(!strcmp("not", zeZas.attr)){
                            if(operandy){
                                token c = v_not_K(zas_pop(fce));
                                if (c.id == KONEC_RADKU){
                                    fprintf(stderr, "Spatny typ parametru fce\n");
                                    p.chyba = ERR_SEM_OPER;
                                    return p;
                                }
                                else
                                    zas_push(zas, c);
                            }
                            else{
                                //jeden parametr, jeden vystup
                                v_not(&zapisovany_radek);
                            }
                        }
                        //U inputu je 0 parametru, neni co optimalizovat
                        else if(!strcmp("inputs", zeZas.attr)){
                            v_inputs(&zapisovany_radek);
                        }
                        else if(!strcmp("inputi", zeZas.attr)){
                            v_inputi(&zapisovany_radek);
                        }
                        else if(!strcmp("inputf", zeZas.attr)){
                            v_inputf(&zapisovany_radek);
                        }
                        else if(!strcmp("substr", zeZas.attr)){
                            if(operandy){
                                token a, b, c;
                                switch(operandy){
                                    case 3:
                                        a = zas_pop(fce);
                                        b = zas_pop(fce);
                                        c = v_substr_K_K_K(zas_pop(fce), b, a);
                                        if (c.id == KONEC_RADKU){
                                            fprintf(stderr, "Spatny typ parametru fce\n");
                                            p.chyba = ERR_SEM_OPER;
                                            return p;
                                        }
                                        else
                                            zas_push(zas, c);

                                        free(c.attr);
                                        break;
                                    case 2:
                                        a = zas_pop(fce);
                                        if(v_substr_P_K_K(&zapisovany_radek, zas_pop(fce), a)){
                                            fprintf(stderr, "Spatny typ parametru fce\n");
                                            p.chyba = ERR_SEM_OPER;
                                            return p;
                                        }
                                        //tato fce vezme 1 parametr z codezas a vrati 1 vysledek na codezas. Tudiz se codezas nemeni.
                                        break;
                                    case 1:
                                        if(v_substr_P_P_K(&zapisovany_radek, zas_pop(fce))){
                                            fprintf(stderr, "Spatny typ parametru fce\n");
                                            p.chyba = ERR_SEM_OPER;
                                            return p;
                                        }
                                        //tato fce veme 2 parametry z codezas a vrati 1 vysledek -> musim codezas o 1 snizit.
                                        codezas -= 1;
                                        break;
                                }
                            }
                            else{
                                codezas -= var->pocet_param - 1;
                                v_substr(&zapisovany_radek);
                            }
                        }
                        else if(!strcmp("ord", zeZas.attr)){
                            token a, c;
                            if(operandy == 2){
                                a = zas_pop(fce);
                                c = v_ord_K_K(zas_pop(fce), a);
                                if (c.id == KONEC_RADKU){
                                    fprintf(stderr, "Spatny typ parametru fce\n");
                                    p.chyba = ERR_SEM_OPER;
                                    return p;
                                }
                                else
                                    zas_push(zas, c);
                            }
                            else if(operandy == 1){
                                //jeden param zere, jeden vysledek dava -> codezas se nemeni
                                if(v_ord_P_K(&zapisovany_radek, zas_pop(fce))){
                                    fprintf(stderr, "Spatny typ parametru fce\n");
                                    p.chyba = ERR_SEM_OPER;
                                    return p;
                                }
                            }
                            else{
                                //2 parametry, jeden vysledek
                                codezas -= 1;
                                v_ord(&zapisovany_radek);
                            }
                        }
                        else if(!strcmp("chr", zeZas.attr)){
                            if(operandy){
                                token c = v_chr_K(zas_pop(fce));
                                if (c.id == KONEC_RADKU){
                                    fprintf(stderr, "Spatny typ parametru fce\n");
                                    p.chyba = ERR_SEM_OPER;
                                    return p;
                                }
                                else
                                    zas_push(zas, c);

                                free(c.attr);
                            }
                            else{
                                //jeden parametr, jeden vystup
                                v_chr(&zapisovany_radek);
                            }
                        }
                        else{
                            //Pro normalni funkce taky nijak nevyuzijeme toho, ze vime, ze nejaky parametr je konstanta, tak je vsechny napushujem na codezas
                            for(int i = 0; i < operandy ;i++){
                                token c = zas_pop(fce);
                                switch(c.id){
                                    case KONSTANTA_D:
                                        pisNa(zapisovany_radek++, 0, "PUSHS float@%a", c.attr_d);
                                        codezas++;
                                        break;
                                    case KONSTANTA_I:
                                        pisNa(zapisovany_radek++, 0, "PUSHS int@%d", c.attr_i);
                                        codezas++;
                                        break;
                                    case KONSTANTA_S:
                                        pisNa(zapisovany_radek++, strlen(c.attr), "PUSHS string@%s", c.attr);
                                        codezas++;
                                        break;
                                    case BTRUE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                        codezas++;
                                        break;
                                    case BFALSE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                        codezas++;
                                        break;
                                    case NIL:
                                        pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                        codezas++;
                                        break;
                                    default:
                                        break;
                                }
                            }
                            //MÁM fci s pevnym poctem parametru, odectu si je ze codezas a prictu +1 za ret hodnotu fce, stejne tak u ostatnich fci...
                            codezas -= var->pocet_param - 1;
                            //Funkce si sama musi vypopovat ze zas.
                            pisNa(zapisovany_radek++, strlen(zeZas.attr), "CALL FUNKCE-%s", zeZas.attr);
                        }
                    }
                    byla_fce = false;
                    fci--;
                }
                break;
            case CARKA:
            case BTRUE:
            case BFALSE:
            case NIL:
            case KONSTANTA_I:
            case KONSTANTA_D:
            case KONSTANTA_S:
                //Kdyz dostanu carku nebo konstantu, tak si ji pushnu na nas zasobnik
                byla_fce = false;
                if(zas_push(zas, tokpole[p.odR][p.odS])){
                    p.chyba = ERR_SYS;
                    return p;
                }
                break;
            case IDENTIFIKATOR:{
                symbol *var;
                //Pokud mam identifikator tak musim zkontrolovat, jestli mame fci
                if((var = htab_data(tab_fce, tokpole[p.odR][p.odS].attr))){
                    //mame fci -> pokud pred ni na zasobniku neco je, musim to dat na assemblerovy zasobnik
                    byla_fce = true;
                    if(fci > 0){
                        //Pokud mam otevrenou fci tak musime projit vsechny jeji parametry a dat je na assemblerovy zasobnik, protoze pracovat s nimi dale jako s konstantami by se nam nevyplatilo
                        token zeZas;
                        int carky = 0;
                        while((zeZas = zas_pop(zas)).id != ZAV_ZAC){
                            if(zeZas.id == CARKA){
                                carky++; //zvysime pocitadlo carek
                            }
                            else {
                                switch(zeZas.id){
                                    case KONSTANTA_D:
                                        pisNa(zapisovany_radek++, 0, "PUSHS float@%a", zeZas.attr_d);
                                        codezas++;
                                        break;
                                    case KONSTANTA_I:
                                        pisNa(zapisovany_radek++, 0, "PUSHS int@%d", zeZas.attr_i);
                                        codezas++;
                                        break;
                                    case KONSTANTA_S:
                                        pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS string@%s", zeZas.attr);
                                        codezas++;
                                        break;
                                    case BTRUE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                        codezas++;
                                        break;
                                    case BFALSE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                        codezas++;
                                        break;
                                    case NIL:
                                        pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                        codezas++;
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        //Na konci cyklu musim vratit '(' a vsechny carky
                        zas_push(zas, zeZas);
                        zeZas.id = CARKA;
                        for(int i = 0; i < carky; i++){
                            zas_push(zas, zeZas);
                        }
                    }
                    else{
                        if(!zas_empty(zas)){
                            token zeZas = zas_pop(zas);
                            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                                zas_push(zas, zeZas);
                            }
                            else{
                                switch(zeZas.id){
                                    case KONSTANTA_D:
                                        pisNa(zapisovany_radek++, 0, "PUSHS float@%a", zeZas.attr_d);
                                        codezas++;
                                        break;
                                    case KONSTANTA_I:
                                        pisNa(zapisovany_radek++, 0, "PUSHS int@%d", zeZas.attr_i);
                                        codezas++;
                                        break;
                                    case KONSTANTA_S:
                                        pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS string@%s", zeZas.attr);
                                        codezas++;
                                        break;
                                    case BTRUE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                        codezas++;
                                        break;
                                    case BFALSE:
                                        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                        codezas++;
                                        break;
                                    case NIL:
                                        pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                        codezas++;
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
//                    var->vysledkuPred = codezas;
                    zas_push(zas, tokpole[p.odR][p.odS]);
                    break;
                }//TODO asi nejaka chyba tu, falesne pozitivni chyba
                else if(!(var = htab_data(tab_lok, tokpole[p.odR][p.odS].attr))){
                    //neni to v lok tabulce ani v tabulce fci -> chyba
                    fprintf(stderr, "Nedefinovana promenna (Neni v tabulce) %s.\n", tokpole[p.odR][p.odS].attr);
                    p.chyba = ERR_SEM_DEF;
                    return p;
                }
                if(var->radek > p.odR){
                    //Kdyz je v tabulce, tak musim zjitit, jestli nahodou neni definova az na pozdejsim radku
                    fprintf(stderr, "Nedefinovana promenna (Je definovana az pozdeji) %s.\n", tokpole[p.odR][p.odS].attr);
                    p.chyba = ERR_SEM_DEF;
                    return p;
                }
                //Zde vime, ze mame platnou promennou (v predchozich ifech jsme skoncili na breaku nebo returnu)
                byla_fce = false;
                if(fci > 0){
                    //Pokud mam otevrenou fci tak musime projit vsechny jeji parametry a dat je na assemblerovy zasobnik, protoze pracovat s nimi dale jako s konstantami by se nam nevyplatilo
                    token zeZas;
                    int carky = 0;
                    while((zeZas = zas_pop(zas)).id != ZAV_ZAC){
                        if(zeZas.id == CARKA){
                            carky++; //zvysime pocitadlo carek
                        }
                        else {
                            switch(zeZas.id){
                                case KONSTANTA_D:
                                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", zeZas.attr_d);
                                    codezas++;
                                    break;
                                case KONSTANTA_I:
                                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", zeZas.attr_i);
                                    codezas++;
                                    break;
                                case KONSTANTA_S:
                                    pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS string@%s", zeZas.attr);
                                    codezas++;
                                    break;
                                case BTRUE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                    codezas++;
                                    break;
                                case BFALSE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                    codezas++;
                                    break;
                                case NIL:
                                    pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                    codezas++;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    //Na konci cyklu musim vratit '(' a vsechny carky
                    zas_push(zas, zeZas);
                    zeZas.id = CARKA;
                    for(int i = 0; i < carky; i++){
                        zas_push(zas, zeZas);
                    }
                }
                else{
                    if(!zas_empty(zas)){
                        token zeZas = zas_pop(zas);
                        if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                            zas_push(zas, zeZas);
                        }
                        else{
                            switch(zeZas.id){
                                case KONSTANTA_D:
                                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", zeZas.attr_d);
                                    codezas++;
                                    break;
                                case KONSTANTA_I:
                                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", zeZas.attr_i);
                                    codezas++;
                                    break;
                                case KONSTANTA_S:
                                    pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS string@%s", zeZas.attr);
                                    codezas++;
                                    break;
                                case BTRUE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                                    codezas++;
                                    break;
                                case BFALSE:
                                    pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                                    codezas++;
                                    break;
                                case NIL:
                                    pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                                    codezas++;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                //Promennou také zapiseme na kódovy zasobnik.
                pisNa(zapisovany_radek++, strlen(tokpole[p.odR][p.odS].attr), "PUSHS LF@%s", tokpole[p.odR][p.odS].attr);
                codezas++;
                break;
            }
            case PLUS:{
                byla_fce = false;
                p = ma_plus(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "Scitani vyhodilo chybu\n");
                    return p;
                }
                break;
            }
            case MINUS:
                byla_fce = false;
                p = ma_minus(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "Odcitani vyhodilo chybu\n");
                    return p;
                }
                break;
            case KRAT:
                byla_fce = false;
                p = ma_krat(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "Nasobeni vyhodilo chybu\n");
                    return p;
                }
                break;
            case DELENO:
                byla_fce = false;
                p = ma_deleno(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "Deleni vyhodilo chybu\n");
                    return p;
                }
                break;
            case AND:
            case AND_Z:
                byla_fce = false;
                p = ma_and(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "AND vyhodilo chybu\n");
                    return p;
                }
                break;
            case OR:
            case OR_Z:
                byla_fce = false;
                p = ma_or(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "OR vyhodilo chybu\n");
                    return p;
                }
                break;
            case NOT:
            case NOT_Z:
                //Kdyz dostanu jeden z tokenu NOT (lisi se prioritou) tak zde uz k nim pristupuji jako k funkcim... tak z nich udelam fci.
                tokpole[p.odR][p.odS].id = IDENTIFIKATOR;
                tokpole[p.odR][p.odS].attr = gS("not");
                p.odS--;
                break;
            case MENSI:
                byla_fce = false;
                p = ma_mensi(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "MENSI vyhodilo chybu\n");
                    return p;
                }
                break;
            case VETSI:
                byla_fce = false;
                p = ma_vetsi(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "VETSI vyhodilo chybu\n");
                    return p;
                }
                break;
            case VETSIROVNO:
                byla_fce = false;
                p = ma_vetsirovno(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "VETSIROVNO vyhodilo chybu\n");
                    return p;
                }
                break;
            case MENSIROVNO:
                byla_fce = false;
                p = ma_mensirovno(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "MENSIROVNO vyhodilo chybu\n");
                    return p;
                }
                break;
            case ROVNASE:
                byla_fce = false;
                p = ma_rovno(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "ROVNO vyhodilo chybu\n");
                    return p;
                }
                break;
            case NEROVNASE:
                byla_fce = false;
                p = ma_nerovno(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "NEROVNO vyhodilo chybu\n");
                    return p;
                }
                break;
            case TER_ZAC:
                byla_fce = false;
                //U ternarniho operatoru (pro vnoreni do sebe) si musim zapamatovat cislo labelu, ktery ma TER_END tisknout
                t.id = KONSTANTA_I;
                t.attr_i = p.lokal_label;
                zas_push(terzas,t);
                p = ma_terzac(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "TER_ZAC vyhodil chybu\n");
                    return p;
                }
                break;
            case TER_END:
                byla_fce = false;
                p = ma_terend(p, zas, &codezas);
                if(p.chyba){
                    fprintf(stderr, "TER_END vyhodil chybu\n");
                    return p;
                }
                t = zas_pop(terzas);
                p.lokal_label = t.attr_i;
                ternarniP++; //tady se zvetsi pocet "dokoncenych" ternaru
                break;
            default:
                break;
        }
        p.odS++;
    }


    //Pokud zas neni prazdny, tak by se na nem mela nachazet prave jedna hodnota a udela se finalni push na codezas
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);

        //Pokud zasobnik po popnuti 1 tokenu stale neni prazdny, tak se jedna o chybu
        if(!zas_empty(zas)){
            fprintf(stderr, "2-Syntakticka chyba na radku %d.\n", p.odR);
            p.chyba = ERR_SEM_DEF;
            return p;
        }

        //zbyla mi tam prave 1 vec (asi jakoze konstanta), tak sup s ni na zasobnik - jedna se o vysledek tohoto vyrazu
        switch (zeZas.id){
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS string@%s", zeZas.attr);
                codezas++;
                break;
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", zeZas.attr_d); //??
                codezas++;
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "PUSHS int@%lld", zeZas.attr_i);
                codezas++;
                break;
            case BTRUE:
                pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                codezas++;
                break;
            case BFALSE:
                pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                codezas++;
                break;
            case NIL:
                pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                codezas++;
                break;
            //Tento pripad by téz nemel nikdy nastat
            case IDENTIFIKATOR:
                if(htab_find(tab_fce, zeZas.attr)){
                    fprintf(stderr, "Chyba\n");
                    p.chyba = ERR_SYS;
                    return p;
                }
                pisNa(zapisovany_radek++, strlen(zeZas.attr), "PUSHS LF@%s", zeZas.attr);
                break;
            default:
                //nikdy by nemelo nastat
                fprintf(stderr, "Vnitrni chyba. Neocekavany token: %d\n", zeZas.id);
                p.chyba = ERR_SYS;
                return p;
                break;
        }
    }
    //Zde se nakonec musi aktualizovat zapisovany radek dle poctu ternarnich operatoru
    if(ternarniP)
        zapisovany_radek += ternarniP;
    return p;
}

//prevod scitani do ciloveho jazyka
sa_pos ma_plus(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Nacitani mozna konstant
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }
    //Secteni dvou konstant
    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        if(a.id == NIL || b.id == NIL || a.id == BTRUE || b.id == BTRUE || a.id == BFALSE || b.id == BFALSE){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //stringy
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                token c;
                int len = strlen(a.attr) + strlen(b.attr) + 1;
                c.attr = calloc(len, sizeof(char));
                if(!c.attr){
                    p.chyba = ERR_SYS;
                    return p;
                }
                //spojeni stringu
                strcpy(c.attr, a.attr);
                strcat(c.attr, b.attr);
                zas_push(zas, c);
                return p;
            }
            else{
                p.chyba = ERR_SEM_OPER;
                return p;
            }
        }
        //jeden string, chyba
        else if(b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            c.id = KONSTANTA_D;
            c.attr_d = 0;
            if(a.id == KONSTANTA_D)
                c.attr_d += a.attr_d;
            else
                c.attr_d += a.attr_i;
            if(b.id == KONSTANTA_D)
                c.attr_d += b.attr_d;
            else
                c.attr_d += b.attr_i;
            zas_push(zas, c);
        }
        //inty
        else{
            token c;
            c.id = KONSTANTA_I;
            c.attr_i = a.attr_i + b.attr_i;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_DEF;
        return p;
    }

    (*codezas)-=2;

    int l_string = labelN++, l_float_a = labelN++, l_float_b = labelN++, l_add = labelN++, l_end = labelN++;
    //2 Promenne
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        //Tady bude probihat full porovnavani obou, protoze oboje jsou promenny.
        //Teď otestovat typy a vygenerovat kod.

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@nil");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@nil");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@bool");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$string$%d GF@$type_a string@string", l_string);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@string");


        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$float_a$%d GF@$type_a string@float", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$float_b$%d GF@$type_b string@float", l_float_b);


        //tady vim, ze mam 2 inty
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_PLUS$plus$%d", l_end);

        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$string$%d", l_string);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 string@string GF@$type_b");
        //tady concat stringu
        pisNa(zapisovany_radek++, 0, "CONCAT GF@$mat_a GF@$mat_a GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_PLUS$end$%d", l_end);

        //tady mozna pretypuju b
        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$floats_a$%d string@float GF@$type_b", l_float_a);
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$floats_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_PLUS$plus$%d", l_end);
        //tady pretypuju a
        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        //Secteni
        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$plus$%d", l_end);
        pisNa(zapisovany_radek++, 0, "ADDS");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$end$%d", l_end);
    }
    else{
        //Reesni konstanty s promennou
        if (b.id == KONEC_RADKU)
            b = a;
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$add$%d GF@$type_a string@float", l_add);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$add$%d", l_add);
                if (b.attr_d != 0) {
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                    pisNa(zapisovany_radek++, 0, "ADDS");
                }
                break;
            case KONSTANTA_I:
                if (b.attr_i != 0) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$int$%d GF@$type_a string@int", l_add);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ EXIT4 GF@$type_a string@float", l_add);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                    pisNa(zapisovany_radek++, 0, "JUMP MAT_PLUS$end$%d", l_add);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$int$%d", l_add);
                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$end$%d", l_add);
                    pisNa(zapisovany_radek++, 0, "ADDS");
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_PLUS$int$%d GF@$type_a string@int", l_add);
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@float");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_PLUS$int$%d", l_add);

                }
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, strlen(b.attr), "CONCAT GF@$mat_a GF@$mat_a string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                break;
            case BTRUE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case BFALSE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//Odcitani
sa_pos ma_minus(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }


    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL || a.id == BTRUE || b.id == BTRUE || a.id == BFALSE || b.id == BFALSE){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //chyba pro stringy
        if(a.id == KONSTANTA_S || b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            c.id = KONSTANTA_D;
            c.attr_d = 0;
            if(a.id == KONSTANTA_D)
                c.attr_d += a.attr_d;
            else
                c.attr_d += a.attr_i;

            if(b.id == KONSTANTA_D)
                c.attr_d -= b.attr_d;
            else
                c.attr_d -= b.attr_i;

            zas_push(zas, c);
        }
        //inty
        else{
            token c;
            c.id = KONSTANTA_I;
            c.attr_i = a.attr_i - b.attr_i;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    int l_float_a = labelN++, l_float_b = labelN++, l_sub = labelN++, l_end = labelN++;
    //2 promenne na zas
    if(b.id == KONEC_RADKU){


        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        //Tady bude probihat full porovnavani obou, protoze oboje jsou promenny.
        //Teď otestovat typy a vygenerovat kod.

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@nil");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@nil");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@bool"); //nad boolem se nescita!
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@string");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@string");


        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$float_a$%d GF@$type_a string@float", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$float_b$%d GF@$type_b string@float", l_float_b);


        //tady vim, ze mam 2 inty
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_MINUS$minus$%d", l_end);

        //tady mozna pretypuju b
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$floats_a$%d string@float GF@$type_b", l_float_a);
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$floats_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_MINUS$minus$%d", l_end);
        //tady pretypuju a
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        //Secteni
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$minus$%d", l_end);
        pisNa(zapisovany_radek++, 0, "SUBS");
    }
    else{
        //Reseni konsanty a promenne
        bool znamenko = false;
        if (b.id == KONEC_RADKU){
            b = a;
            b.attr_d = -b.attr_d;
            b.attr_i = -b.attr_i;
            znamenko = true;
        }
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                if (b.attr_d == 0 && !znamenko) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$add$%d GF@$type_a string@float", l_sub);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$add$%d", l_sub);
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$add$%d GF@$type_a string@float", l_sub);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                    //tady scitam 2 floaty
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$add$%d", l_sub);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                    if (!znamenko)
                        pisNa(zapisovany_radek++, 0, "SUBS");
                    else
                        pisNa(zapisovany_radek++, 0, "ADDS");
                }
                break;
            case KONSTANTA_I:
                if (b.attr_i == 0 && !znamenko) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$add$%d GF@$type_a string@float", l_sub);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$add$%d", l_sub);
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MINUS$add$%d GF@$type_a string@float", l_sub);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                    pisNa(zapisovany_radek++, 0, "JUMP MAT_MINUS$int$%d", l_sub);
                    //tady scitam 2 floaty
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$add$%d", l_sub);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MINUS$int$%d", l_sub);
                    if (!znamenko)
                        pisNa(zapisovany_radek++, 0, "SUBS");
                    else
                        pisNa(zapisovany_radek++, 0, "ADDS");
                }
                break;
            case KONSTANTA_S:
                p.chyba = ERR_SEM_OPER;
                return p;
            case BTRUE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case BFALSE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}
//Nasobeni
sa_pos ma_krat(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL || a.id == BTRUE || b.id == BTRUE || a.id == BFALSE || b.id == BFALSE){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        if(a.id == KONSTANTA_S || b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            c.id = KONSTANTA_D;
            c.attr_d = 0;
            if(a.id == KONSTANTA_D)
                c.attr_d += a.attr_d;
            else
                c.attr_d += a.attr_i;
            if(b.id == KONSTANTA_D)
                c.attr_d *= b.attr_d;
            else
                c.attr_d *= b.attr_i;

            zas_push(zas, c);
        }
        //inty
        else{
            token c;
            c.id = KONSTANTA_I;
            c.attr_i = a.attr_i * b.attr_i;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas0
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta -> musim nacpat konstantu na code zas0
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    int l_float_a = labelN++, l_float_b = labelN++, l_mul = labelN++, l_end = labelN++;
    //2 promenne na zas
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        //Tady bude probihat full porovnavani obou, protoze oboje jsou promenny.
        //Teď otestovat typy a vygenerovat kod.

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@nil");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@nil");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@bool"); //nad boolem se nescita!
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@string");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@string");


        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$float_a$%d GF@$type_a string@float", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$float_b$%d GF@$type_b string@float", l_float_b);


        //tady vim, ze mam 2 inty
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_MUL$plus$%d", l_end);

        //tady mozna pretypuju b
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$floats_a$%d string@float GF@$type_b", l_float_a);
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$floats_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_MUL$plus$%d", l_end);
        //tady pretypuju a
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        //Secteni
        pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$plus$%d", l_end);
        pisNa(zapisovany_radek++, 0, "MULS");
    }
    else{
        //Reseni promenne s konstantou
        if (b.id == KONEC_RADKU)
            b = a;
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                if (b.attr_d == 0.0) {
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", 0.0);
                } else if (b.attr_d == 1) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                    pisNa(zapisovany_radek++, 0, "MULS");
                }
                break;
            case KONSTANTA_I:
                if (b.attr_i == 0) {
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "PUSHS int@0");
                    pisNa(zapisovany_radek++, 0, "JUMP MAT_MUL2$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", 0.0);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL2$add$%d", l_mul);
                } else if (b.attr_i == 1) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_MUL$add$%d GF@$type_a string@float", l_mul);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                    pisNa(zapisovany_radek++, 0, "JUMP MAT_MUL2$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_MUL2$add$%d", l_mul);
                    pisNa(zapisovany_radek++, 0, "MULS");
                }
                break;
            case KONSTANTA_S:
                p.chyba = ERR_SEM_OPER;
                return p;
            case BTRUE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case BFALSE:
                p.chyba = ERR_SEM_OPER;
                return p;
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//Deleni
sa_pos ma_deleno(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL || a.id == BTRUE || b.id == BTRUE || a.id == BFALSE || b.id == BFALSE){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        if(a.id == KONSTANTA_S || b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            c.id = KONSTANTA_D;
            c.attr_d = 0;
            if(a.id == KONSTANTA_D)
                c.attr_d += a.attr_d;
            else
                c.attr_d += a.attr_i;
            if(b.id == KONSTANTA_D){
                if(b.attr_d == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                c.attr_d /= b.attr_d;
            }
            else{
                if(b.attr_d == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                c.attr_d /= b.attr_i;
            }
            zas_push(zas, c);
        }
        //inty
        else{
            token c;
            c.id = KONSTANTA_I;
            if(b.attr_i == 0){
                fprintf(stderr, "Deleni nulou!\n");
                p.chyba = ERR_SEM_JINAK;
                return p;
            }
            c.attr_i = a.attr_i / b.attr_i;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                if(b.attr_d == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                (*codezas)++;
                break;
            case KONSTANTA_I:
                if(b.attr_i == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                if(a.attr_d == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                (*codezas)++;
                break;
            case KONSTANTA_I:
                if(a.attr_i == 0){
                    fprintf(stderr, "Deleni nulou!\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                (*codezas)++;
                break;
            case KONSTANTA_S:
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    int l_float_a = labelN++, l_float_b = labelN++, l_end = labelN++;
    //2 promenn na zas
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        //Tady bude probihat full porovnavani obou, protoze oboje jsou promenny.
        //Teď otestovat typy a vygenerovat kod.

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@nil");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@nil");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@bool"); //nad boolem se nescita!
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_a string@string");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT4 GF@$type_b string@string");


        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DIV$float_a$%d GF@$type_a string@float", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DIV$float_b$%d GF@$type_b string@float", l_float_b);


        //tady vim, ze mam 2 inty
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b int@0");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "IDIVS");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_DIV$end$%d", l_end);

        //tady mozna pretypuju b
        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DIV$floats_a$%d string@float GF@$type_b", l_float_a);
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b int@0", 0);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_DIV$nefloat_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$floats_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b float@%a", 0.0);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$nefloat_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_DIV$plus$%d", l_end);
        //tady pretypuju a
        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b float@%a", 0.0);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        //Secteni
        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$plus$%d", l_end);
        pisNa(zapisovany_radek++, 0, "DIVS");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_DIV$end$%d", l_end);
    }
    else if(a.id == KONEC_RADKU){
        //Promenna s konstantou
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                if(b.attr_d == 0){
                    fprintf(stderr, "Deleni nulou.\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DELENO$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$float_b$%d", l_float_b);
                if (b.attr_d != 1) {
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a",b.attr_d);
                    pisNa(zapisovany_radek++, 0, "DIVS");
                }
                break;
            case KONSTANTA_I:
                if(b.attr_i == 0){
                    fprintf(stderr, "Deleni nulou.\n");
                    p.chyba = ERR_SEM_JINAK;
                    return p;
                }
                if (b.attr_i == 1) {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DELENO$float_a$%d GF@$type_a string@float", l_float_a);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$float_a$%d", l_float_a);
                } else {
                    pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                    pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DELENO$float_a$%d GF@$type_a string@float", l_float_a);
                    pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d",b.attr_i);
                    pisNa(zapisovany_radek++, 0, "IDIVS");
                    pisNa(zapisovany_radek++, 0, "JUMP MAT_DELENO$end$%d", l_end);
                    pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$float_a$%d", l_float_a);
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                    pisNa(zapisovany_radek++, 0, "DIVS");
                }
                break;
            case KONSTANTA_S:
                fprintf(stderr, "Odcitani stringu s necim.\n");
                p.chyba = ERR_SEM_OPER;
                return p;
                break;
            default:
                fprintf(stderr, "bool nebo nil u deleni.%d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$end$%d", l_end);


        //vysledek je na code zas
    }else{
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");

        switch(a.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS float@%a",b.attr_d);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DELENO$float_b$%d GF@$type_b string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b int@0");
                pisNa(zapisovany_radek++, 0, "DIVS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_DELENO$end$%d", l_end);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b float@%a", 0.0);
                pisNa(zapisovany_radek++, 0, "DIVS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_DELENO$float_b$%d GF@$type_b string@float", l_float_a);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d",b.attr_i);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b int@0");
                pisNa(zapisovany_radek++, 0, "IDIVS");
                //tady scitam 2 inty
                pisNa(zapisovany_radek++, 0, "JUMP MAT_DELENO$end$%d", l_end);

                //tady se musi konstanta_i prevést na float
                pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$float_b$%d", l_float_a);
                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ $EXIT9 GF@$mat_b float@%a", 0.0);
                pisNa(zapisovany_radek++, 0, "DIVS");
                break;
            case KONSTANTA_S:
                fprintf(stderr, "Odcitani stringu s necim.\n");
                p.chyba = ERR_SEM_OPER;
                return p;
                break;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_DELENO$end$%d", l_end);


        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//logicky and
sa_pos ma_and(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                zas_push(zas, zeZas);
                b = a;
                a.id = KONEC_RADKU;
            }
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL || a.id == KONSTANTA_S || b.id == KONSTANTA_S || a.id == KONSTANTA_D || b.id == KONSTANTA_D || a.id == KONSTANTA_I || b.id == KONSTANTA_I){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        else if(a.id == BFALSE || b.id == BFALSE){
            token c;
            c.id = BFALSE;
            zas_push(zas, c);
        }
        else{
            //mam 2x true -> true
            token c;
            c.id = BTRUE;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case BTRUE:
                (*codezas)++;
                break;
            case BFALSE:
                (*codezas)++;
                break;
            case KONSTANTA_D:
            case KONSTANTA_I:
            case KONSTANTA_S:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_OPER;
        return p;
    }

    (*codezas)-=2;

    //2 promenne na zas
    if(b.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "ANDS");

    }
    else{
        //promenna a konstanta
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case BFALSE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");
                pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                break;
            case BTRUE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                break;
            default:
                fprintf(stderr, "And se spatnou konstantou.%d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }

        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//logicky or
sa_pos ma_or(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                zas_push(zas, zeZas);
                b = a;
                a.id = KONEC_RADKU;
            }
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL || a.id == KONSTANTA_S || b.id == KONSTANTA_S || a.id == KONSTANTA_D || b.id == KONSTANTA_D || a.id == KONSTANTA_I || b.id == KONSTANTA_I){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        else if(a.id == BTRUE || b.id == BTRUE){
            token c;
            c.id = BTRUE;
            zas_push(zas, c);
        }
        else{
            token c;
            c.id = BFALSE;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case BTRUE:
                (*codezas)++;
                break;
            case BFALSE:
                (*codezas)++;
                break;
            case KONSTANTA_D:
            case KONSTANTA_I:
            case KONSTANTA_S:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    //Mam 2 promenne na zas
    if(b.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@bool");

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");


        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "ORS");

    }
    else{
        //Promenna a konstanta
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case BFALSE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                break;
            case BTRUE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@bool");
                pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                break;
            default:
                fprintf(stderr, "Or se spatnou konstantou.%d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }

        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}
//operator vetsi
sa_pos ma_vetsi(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Mozna nacteni konstant
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL ){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                token c;
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) > 0 ) {
                    c.id = BTRUE;
                }
                else
                    c.id = BFALSE;
                zas_push(zas, c);
            }
            else{
                p.chyba = ERR_SEM_OPER;
                return p;
            }
        }
        else if(b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d >b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d > b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            //float a int
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i >b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i > b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            token c;
            if (a.attr_i > b.attr_i) {
                c.id = BTRUE;
            }
            else
                c.id = BFALSE;

            zas_push(zas, c);
        }
        else{
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_OPER;
        return p;
    }

    (*codezas)-=2;

    //Mam 2 promenne na zas
    int l_float = labelN++, l_float_b = labelN++, l_vetsi = labelN++, l_end = labelN++;
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int$%d GF@$type_a string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@float");


        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI$vetsi$%d", l_vetsi);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int2$%d GF@$type_b string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@float");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$vetsi$%d", l_vetsi);
        pisNa(zapisovany_radek++, 0, "GTS");
    }
    else if (b.id != KONEC_RADKU){
        //Promenna a konstanta
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
    }else{
        //Konstanta a promenna
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$end$%d", l_end);


        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//Operator mensi
sa_pos ma_mensi(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Mozna nacteni konstant
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL ){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                token c;
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) < 0 ) {
                    c.id = BTRUE;
                }
                else
                    c.id = BFALSE;
                zas_push(zas, c);
            }
            else{
                p.chyba = ERR_SEM_OPER;
                return p;
            }
        }
        else if(b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d < b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d < b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{   // pro napr 5.6 > true
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            //float a int
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i < b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i < b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{   // pro napr 5.6 > true
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            token c;
            if (a.attr_i < b.attr_i) {
                c.id = BTRUE;
            }
            else
                c.id = BFALSE;

            zas_push(zas, c);
        }
        else{
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    //2 promenne na zas
    int l_float = labelN++, l_float_b = labelN++, l_vetsi = labelN++, l_end = labelN++;
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){


        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int$%d GF@$type_a string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@float", l_float);


        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI$vetsi$%d", l_vetsi);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int2$%d GF@$type_b string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@float");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$vetsi$%d", l_vetsi);
        pisNa(zapisovany_radek++, 0, "LTS");
    }
    //promenna s konstantou
    else if (b.id != KONEC_RADKU){
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
    }else{
        //Konstanta s promennou
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "LTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$end$%d", l_end);

    }
    (*codezas)++;
    return p;
}

//operator porovnani
sa_pos ma_rovno(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Mozna nacte konstatnty
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                zas_push(zas, zeZas);
                b = a;
                a.id = KONEC_RADKU;
            }
            else
                b = zeZas;
    }
    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        token c;
        if(a.id == NIL && b.id == NIL ){
            c.id = BTRUE;
            zas_push(zas, c);
        }
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) == 0 )
                    c.id = BTRUE;
                else
                    c.id = BFALSE;
                zas_push(zas, c);
            }
            else{
                c.id = BFALSE;
                zas_push(zas, c);
            }
        }
        else if(b.id == KONSTANTA_S){
            c.id = BFALSE;
            zas_push(zas, c);
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d == b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d == b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{
                    c.id = BFALSE;
                    zas_push(zas, c);
                }
            }
            //float a int
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i == b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i == b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{
                    c.id = BFALSE;
                    zas_push(zas, c);
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            if (a.attr_i == b.attr_i) {
                c.id = BTRUE;
            }
            else
                c.id = BFALSE;
            zas_push(zas, c);
        }
        else if ((a.id == BTRUE && a.id == BTRUE) || (b.id == BFALSE && b.id == BFALSE)) {  // oba jsou bool
            c.id = BTRUE;
            zas_push(zas, c);
        }
        else{
            c.id = BFALSE;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
                (*codezas)++;
                break;
            case BFALSE:
                (*codezas)++;
                break;
            case NIL:   //i nil se musi pushnout
                (*codezas)++;
                break;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    int l_float = labelN++, l_float_a = labelN++, l_float_b = labelN++, l_rovno_P = labelN++, l_false = labelN++, l_end = labelN++;
    //2 promenne na zas
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$ruzne_typy$%d GF@$type_b GF@$type_a", l_rovno_P);

        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "EQS");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_rovno_P);

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$ruzne_typy$%d", l_rovno_P);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float$%d GF@$type_a string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_rovno_P);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_a$%d GF@$type_b string@int", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_rovno_P);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "EQS");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_rovno_P);

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@int", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_rovno_P);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "EQS");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$konec$%d", l_rovno_P);
    }
    else{
        //Konstanta s promennou
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@int", l_false);
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@int", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@float", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$float_b2$%d", l_end);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b2$%d", l_end);

                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@string", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case BTRUE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@bool", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case BFALSE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@bool", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "NOTS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case NIL:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@nil", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$false$%d", l_false);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@false");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$end$%d", l_end);


    }
    (*codezas)++;
    return p;
}

//Operator nerovno
sa_pos ma_nerovno(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Mozna nacteni konstant
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
                zas_push(zas, zeZas);
                b = a;
                a.id = KONEC_RADKU;
            }
            else
                b = zeZas;
    }
    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        token c;
        if(a.id == NIL && b.id == NIL ){
            c.id = BFALSE;
            zas_push(zas, c);
        }
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) == 0 ) {
                    c.id = BFALSE;
                }
                else
                    c.id = BTRUE;
                zas_push(zas, c);
            }
            else{
                c.id = BTRUE;
                zas_push(zas, c);
            }
        }
        else if(b.id == KONSTANTA_S){
            c.id = BTRUE;
            zas_push(zas, c);
        }
        //aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d == b.attr_d) {
                        c.id = BFALSE;
                    }
                    else
                        c.id = BTRUE;
                }
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d == b.attr_i) {
                        c.id = BFALSE;
                    }
                    else
                        c.id = BTRUE;
                }
                else{
                    c.id = BTRUE;
                    zas_push(zas, c);
                }
            }
            //float a int
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i == b.attr_i) {
                        c.id = BFALSE;
                    }
                    else
                        c.id = BTRUE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i == b.attr_d) {
                        c.id = BFALSE;
                    }
                    else
                        c.id = BTRUE;
                }
                else{
                    c.id = BTRUE;
                    zas_push(zas, c);
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            if (a.attr_i == b.attr_i) {
                c.id = BFALSE;
            }
            else
                c.id = BTRUE;
            zas_push(zas, c);
        }
        else if ((a.id == BTRUE && a.id == BTRUE) || (b.id == BFALSE && b.id == BFALSE)) {  // oba jsou bool
            c.id = BFALSE;
            zas_push(zas, c);
        }
        else{
            c.id = BTRUE;
            zas_push(zas, c);
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
                (*codezas)++;
                break;
            case BFALSE:
                (*codezas)++;
                break;
            case NIL:
                (*codezas)++;
                break;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    //2 promenne na zas
    int l_float = labelN++, l_float_a = labelN++, l_float_b = labelN++, l_nerovno_P = labelN++, l_false = labelN++, l_end = labelN++;
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$ruzne_typy$%d GF@$type_b GF@$type_a", l_nerovno_P);

        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "EQS");
        pisNa(zapisovany_radek++, 0, "NOTS");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_nerovno_P);

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$ruzne_typy$%d", l_nerovno_P);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float$%d GF@$type_a string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_nerovno_P);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_a$%d GF@$type_b string@int", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_nerovno_P);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_a$%d", l_float_a);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "EQS");
        pisNa(zapisovany_radek++, 0, "NOTS");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_nerovno_P);

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@int", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
        pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$konec$%d", l_nerovno_P);
        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "EQS");
        pisNa(zapisovany_radek++, 0, "NOTS");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$konec$%d", l_nerovno_P);
    }
    else{
        //Promenna s konstantou
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@int", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@float", l_false);
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_ROVNO$float_b$%d GF@$type_a string@int", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@float", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$float_b2$%d", l_end);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$float_b2$%d", l_end);

                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@string", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "EQS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case BTRUE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@bool", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "NOTS");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case BFALSE:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@bool", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            case NIL:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ MAT_ROVNO$false$%d GF@$type_a string@nil", l_false);
                pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                pisNa(zapisovany_radek++, 0, "JUMP MAT_ROVNO$end$%d", l_end);
                break;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$false$%d", l_false);
        pisNa(zapisovany_radek++, 0, "PUSHS bool@true");

        pisNa(zapisovany_radek++, 0, "LABEL MAT_ROVNO$end$%d", l_end);


        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//Operator vetsirovno
sa_pos ma_vetsirovno(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Mozna nacteni konstant
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL ){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                token c;
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) >= 0 ) {
                    c.id = BTRUE;
                }
                else
                    c.id = BFALSE;
                zas_push(zas, c);
            }
            else{
                p.chyba = ERR_SEM_OPER;
                return p;
            }
        }
        else if(b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }//aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d >= b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }//float a int
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d >= b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{   // pro napr 5.6 > true
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }//int a float
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i >= b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i >= b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{   // pro napr 5.6 > true
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            token c;
            if (a.attr_i >= b.attr_i) {
                c.id = BTRUE;
            }
            else
                c.id = BFALSE;

            zas_push(zas, c);
        }
        else{
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    //2 promenne na zas
    int l_float = labelN++, l_float_b = labelN++, l_vetsi = labelN++, l_end = labelN++;
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float$%d GF@$type_a string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int$%d GF@$type_a string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMP $EXIT4", l_end);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float$%d", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI$vetsi$%d", l_vetsi);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int2$%d GF@$type_b string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@float");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$vetsi$%d", l_vetsi);
        pisNa(zapisovany_radek++, 0, "LTS");
        pisNa(zapisovany_radek++, 0, "NOTS");
    }
    else if (b.id != KONEC_RADKU){
        //Promenna a konstanta
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                p.chyba = ERR_SEM_OPER;
                return p;
        }
    }else{
        //Konstanta a promenna
        b = a;
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "LTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                fprintf(stderr, "Nekompatibilni typy %d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$end$%d", l_end);


        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

//Operator mensirovno
sa_pos ma_mensirovno(sa_pos p, TStack *zas, int *codezas){
    token a, b;
    a.id = KONEC_RADKU;
    b.id = KONEC_RADKU;
    //Nacitani konstant, pokud jsou
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
            if(!zas_empty(zas)){
                token zeZas2 = zas_pop(zas);
                if ((zeZas2.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas2.attr)) || zeZas2.id == ZAV_ZAC || zeZas2.id == CARKA)
                    zas_push(zas, zeZas2);
                else
                    a = zeZas2;
            }
            if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA)
                zas_push(zas, zeZas);
            else
                b = zeZas;
    }

    if(a.id != KONEC_RADKU && b.id != KONEC_RADKU){
        //mam 2 konstanty
        if(a.id == NIL || b.id == NIL ){
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        //stringy
        if(a.id == KONSTANTA_S){
            if (b.id == KONSTANTA_S){
                token c;
                // porovnavam lexikograficky tzn. na prvni pozici rezezcu na ktere si neodpovadji se porovna jejich hodnota znaku (tak to dela i strcmp)
                if (strcmp(a.attr, b.attr) <= 0 ) {
                    c.id = BTRUE;
                }
                else
                    c.id = BFALSE;
                zas_push(zas, c);
            }
            else{
                p.chyba = ERR_SEM_OPER;
                return p;
            }
        }
        else if(b.id == KONSTANTA_S){
            p.chyba = ERR_SEM_OPER;
            return p;
        }//aspon 1 float
        else if(a.id == KONSTANTA_D || b.id == KONSTANTA_D){
            token c;
            if(a.id == KONSTANTA_D){
                if (b.id == KONSTANTA_D) {
                    if (a.attr_d <= b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_I) {
                    if (a.attr_d <= b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{   // pro napr 5.6 > true
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }//float a int
            else if (a.id == KONSTANTA_I) {
                if (b.id == KONSTANTA_I) {
                    if (a.attr_i <= b.attr_i) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else if (b.id == KONSTANTA_D) {
                    if (a.attr_i <= b.attr_d) {
                        c.id = BTRUE;
                    }
                    else
                        c.id = BFALSE;
                }
                else{
                    p.chyba = ERR_SEM_OPER;
                    return p;
                }
            }
            zas_push(zas, c);
        }
        else if (a.id == KONSTANTA_I && b.id == KONSTANTA_I) {  // oba jsou inty
            token c;
            if (a.attr_i <= b.attr_i)
                c.id = BTRUE;
            else
                c.id = BFALSE;

            zas_push(zas, c);
        }
        else{
            p.chyba = ERR_SEM_OPER;
            return p;
        }
        return p;
    }

    if(b.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(b.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'b', v 'a' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case KONSTANTA_D:
                (*codezas)++;
                break;
            case KONSTANTA_I:
                (*codezas)++;
                break;
            case KONSTANTA_S:
                (*codezas)++;
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                break;
        }
    }

    if(*codezas < 2){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_OPER;//ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=2;

    //2 promenne na zas
    int l_float = labelN++, l_float_b = labelN++, l_vetsi = labelN++, l_end = labelN++;
    if(b.id == KONEC_RADKU && a.id == KONEC_RADKU){

        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_b GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");

        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int$%d GF@$type_a string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@float");


        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float2$%d GF@$type_b string@float", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@int");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI$vetsi$%d", l_vetsi);


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int$%d", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$int2$%d GF@$type_b string@int", l_float);
        pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_b string@float");
        pisNa(zapisovany_radek++, 0, "INT2FLOATS");
        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$int2$%d", l_float);
        pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_b");


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$vetsi$%d", l_vetsi);
        pisNa(zapisovany_radek++, 0, "GTS");
        pisNa(zapisovany_radek++, 0, "NOTS");
    }
    else if (b.id != KONEC_RADKU){
        //Promenna a konstanta
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                fprintf(stderr, "Nekompatibilni typy pro porovnani %d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }
    }else{
        //konstanta a promenna
        pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
        pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

        switch(b.id){
            case KONSTANTA_D:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "INT2FLOATS");
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", b.attr_d);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_I:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ MAT_VETSI$float_b$%d GF@$type_a string@float", l_float_b);
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@int");
                pisNa(zapisovany_radek++, 0, "PUSHS int@%d", b.attr_i);
                pisNa(zapisovany_radek++, 0, "JUMP MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$float_b$%d", l_float_b);

                pisNa(zapisovany_radek++, 0, "PUSHS float@%a", (float)b.attr_i);
                pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI2$float_b$%d", l_float_b);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case KONSTANTA_S:
                pisNa(zapisovany_radek++, 0, "JUMPIFNEQ $EXIT4 GF@$type_a string@string");
                pisNa(zapisovany_radek++, strlen(b.attr), "PUSHS string@%s", b.attr);
                pisNa(zapisovany_radek++, 0, "PUSHS GF@$mat_a");
                pisNa(zapisovany_radek++, 0, "GTS");
                pisNa(zapisovany_radek++, 0, "NOTS");
                break;
            case BTRUE:
            case BFALSE:
            case NIL:
                p.chyba = ERR_SEM_OPER;
                return p;
            default:
                fprintf(stderr, "Nekompatibilni typy pro porovnani %d\n", b.id);
                p.chyba = ERR_SEM_OPER;
                return p;
        }


        pisNa(zapisovany_radek++, 0, "LABEL MAT_VETSI$end$%d", l_end);


        //vysledek je na code zas
    }
    (*codezas)++;
    return p;
}

sa_pos ma_terzac(sa_pos p, TStack *zas, int *codezas){
    token a;
    a.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
        if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
            zas_push(zas, zeZas);
        }
        else{
            a = zeZas;
        }

    }

    p.lokal_label=dejUID();

    if(a.id != KONEC_RADKU){
        //mam 1 promennou 'a', v 'b' je konstanta -> musim nacpat konstantu na code zas
        switch(a.id){
            case BFALSE:
            case NIL:
                pisNa(zapisovany_radek++, 0, "JUMPIFEQ TER_ELSE%d GF@$mat_a bool@false", p.lokal_label);
                break;
            default:
                //Pro ostatni jdu proste dal
                break;
        }
        return p;
    }

    if(*codezas < 1){
        fprintf(stderr, "Pravdepodobne chybi operand v matice.\n");
        p.chyba = ERR_SEM_JINAK;
        return p;
    }

    (*codezas)-=1;

    //mam to na codezas
    pisNa(zapisovany_radek++, 0, "POPS GF@$mat_a");
    pisNa(zapisovany_radek++, 0, "TYPE GF@$type_a GF@$mat_a");

    //Pro ostatni jdu proste dal
    pisNa(zapisovany_radek++, 0, "JUMPIFEQ TER_ELSE%d GF@$type_a string@nil", p.lokal_label);
    pisNa(zapisovany_radek++, 0, "JUMPIFEQ TER_ELSE%d GF@$mat_a bool@false", p.lokal_label);
    return p;
}

sa_pos ma_terend(sa_pos p, TStack *zas, int *codezas){token a;
    a.id = KONEC_RADKU;
    if(!zas_empty(zas)){
        token zeZas = zas_pop(zas);
        if ((zeZas.id == IDENTIFIKATOR && htab_find(tab_fce, zeZas.attr)) || zeZas.id == ZAV_ZAC || zeZas.id == CARKA){
            zas_push(zas, zeZas);
        }
        else{
            a = zeZas;
            switch(a.id){
                case KONSTANTA_D:
                    pisNa(zapisovany_radek++, 0, "PUSHS float@%a", a.attr_d);
                    (*codezas)++;
                    break;
                case KONSTANTA_I:
                    pisNa(zapisovany_radek++, 0, "PUSHS int@%d", a.attr_i);
                    (*codezas)++;
                    break;
                case KONSTANTA_S:
                    pisNa(zapisovany_radek++, strlen(a.attr), "PUSHS string@%s", a.attr);
                    (*codezas)++;
                    break;
                case BTRUE:
                    pisNa(zapisovany_radek++, 0, "PUSHS bool@true");
                    (*codezas)++;
                    break;
                case BFALSE:
                    pisNa(zapisovany_radek++, 0, "PUSHS bool@false");
                    (*codezas)++;
                    break;
                case NIL:
                    pisNa(zapisovany_radek++, 0, "PUSHS nil@nil");
                    (*codezas)++;
                    break;
                default:
                    break;
            }
        }

    }
    pisNa(zapisovany_radek++, 0, "JUMP TER_END%d", p.lokal_label);
    pisNa(zapisovany_radek++, 0, "LABEL TER_ELSE%d", p.lokal_label);
    pisNa(zapisovany_radek, 0, "LABEL TER_END%d", p.lokal_label);
    (void)zas;
    (void)codezas;
    return p;
}
