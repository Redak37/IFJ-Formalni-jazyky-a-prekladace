/*  Knihovna Scanner_Fce
 *
 *  Autor:  Jan Juda, VUT FIT (xjudaj00)
 *          Jan Dolezal, VUT FIT (xdolez81)
 *  Upravil:  Josef Oškera, VUT FIT (xosker03)
 *            Radek Duchoň, VUT FIT (xducho07)

 *  Datum: 20.11.2018
 *
 */

#include "scanner_fce.h"

#include "matematika.h"


/**
 * Pomocna funkce na uvolneni vsech alokovanych struktur. (Typicky pri chybe)
 * Uvolni vsechny zadane parametry.
 *
 * @param tok - 2d pole tokenu
 * @param pole - 1d pole tokenu
 * @param slovo - string (pole znaku)
 */
void uvolniVse(token **tok, token * pole, char *slovo){
    free(pole);
    free(slovo);
    int i = 0;
    while(tok[i] != NULL){
        free(tok[i++]);
        //i++;
    }
    free(tok);
}

/**
 * Funkce na dynamicke realokovani pole tokenu.
 * Toto pole docasne obsahuje jeden radek tokenu ze vstupniho souboru.
 *
 * @param tok - vystupni 2d pole tokenu, tento parametr se predava pro uvolneni v pripade chyby
 * @param pole - ukazatel na 1d pole tokenu, ktere se bude realokovat
 * @param slovo - string, tento parametr se predava pro uvolneni v pripade chyby
 * @param pocetTokenuNaRadku - cislo, udavajici aktualni pocet tokenu na radku
 * @param velikost - cislo, udavajici aktualni naalokovanou velikost token * pole
 */
int realokovaniPole(token **tok, token ** pole, char *slovo, int *pocetTokenuNaRadku, int *velikost){
    if(*pocetTokenuNaRadku == *velikost){
        (*velikost) *= 2;
        token * new_pole = realloc(*pole, (*velikost) * sizeof(token));
        if(!new_pole){
            //uvolnit vse
            uvolniVse(tok, *pole, slovo);
            return ERR_SYS;
        }
        else{
            *pole = new_pole;
        }
    }
    return 0;
}

/**
 * Fukce na dynamicke realokovani retezce.
 * V tomto retezci se docasne uchovova nazev identifikatoru nebo obsah retezcoveho literalu.
 *
 * @param tok - vystupni 2d pole tokenu, tento parametr se predava pro uvolneni v pripade chyby
 * @param pole - pomocne 1d pole tokenu, tento parametr se predava pro uvolneni v pripade chyby
 * @param slovo - ukazatel na retezec, ktery ma byt realokovan
 * @param delkaSlova - cislo, udavajici aktualni pouzitou delku retezce
 * @param velikostSlova - cislo, udavajici aktualni
 */
int realokujSlovo(token **tok, token * pole, char **slovo, int *delkaSlova, int *velikostSlova){
    if(*delkaSlova == *velikostSlova){
        (*velikostSlova) *= 2;
        char * new_slovo = realloc(*slovo, (*velikostSlova) * sizeof(char));
        if(!new_slovo){
            //uvolnit vse
            uvolniVse(tok, pole, *slovo);
            return ERR_SYS;
        }
        else{
            *slovo = new_slovo;
        }
    }
    return 0;
}

/**
 * SOUHRNA HLAVICKA PRO VSECHNY STAVOVE FCE.
 *
 * @param array - Retezec obsahujici 1 radek vstupniho souboru
 * @param tok - vystupni 2d pole tokenu
 * @param znak - index aktualne spracovavaneho znaku v retezci array
 * @param pocetTokenuNaRadku - cislo, udavajici aktualni pocet tokenu v token * pole
 * @param pole - pomocne 1d pole tokenu, docasne obsahuje vsechny tokeny tohoto radku
 * @param velikost - cislo, udavajici naalokovanou velikost token * pole
 * @param slovo - pomocny retezec, docasne obsahuje jmeno prave nacitaneho identifikatoru nebo obsah retezcoveho literalu
 * @param delkaSlova - cislo, udavajici aktualni pouzitou delku retezce slovo, (tento parametr se u nekterych fci nevyskytuje, protoze neni treba)
 * @param velikostSlova - cislo, udavajici naalokovanou velikost char * slovo, (tento parametr se u nekterych fci nevyskytuje, protoze neni treba)
 * @return Stavove funkce vraci dalsi stav (next state).
 */

int f_start(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav = START;
    switch(array[*znak]){
        case '+':
            (*pole)[*pocetTokenuNaRadku].id = PLUS;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '-':
            (*pole)[*pocetTokenuNaRadku].id = MINUS;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '/':
            (*pole)[*pocetTokenuNaRadku].id = DELENO;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '(':
            (*pole)[*pocetTokenuNaRadku].id = ZAV_ZAC;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case ')':
            (*pole)[*pocetTokenuNaRadku].id = ZAV_END;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '?':
            (*pole)[*pocetTokenuNaRadku].id = TER_ZAC;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case ':':
            (*pole)[*pocetTokenuNaRadku].id = TER_END;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
            break;
        case ',':
            (*pole)[*pocetTokenuNaRadku].id = CARKA;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '*':
            (*pole)[*pocetTokenuNaRadku].id = KRAT;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case '>':
            stav = ZOB_V_X;
            break;
        case '<':
            stav = ZOB_M_X;
            break;
        case '=':
            stav = ROV_X;
            break;
        case '!':
            stav = VYK_X;
            break;
        case '\"':
            stav = STR_OTZ;
            break;
        case '&':
            stav = AND_X;
            break;
        case '|':
            stav = OR_X;
            break;
        case '0':
            stav = NULA_OTZ;
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            stav = INT;
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '_':
            stav = IDENTIF;
            (*znak)--;
            break;
        case '\0':
            (*pole)[*pocetTokenuNaRadku].id = KONEC_RADKU;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        case ' ':
        case '\t':
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav VYK_X.
 * Pokud je dalsi znak =, tak vygeneruje token NEROVNASE.
 */
int f_vyk_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '=':
            (*pole)[*pocetTokenuNaRadku].id = NEROVNASE;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            (*pole)[*pocetTokenuNaRadku].id = NOT;
            (*pocetTokenuNaRadku)++;
            stav = START;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav ROV_X.
 * Pokud je dalsi znak =, tak vygeneruje token ROVNASE.
 */
int f_rov_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '=':
            (*pole)[*pocetTokenuNaRadku].id = ROVNASE;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            (*pole)[*pocetTokenuNaRadku].id = PRIRAD;
            (*pocetTokenuNaRadku)++;
            stav = START;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav ZOB_V_X.
 * Pokud je dalsi znak =, tak vygeneruje token VETSIROVNO, jinak generuje token VETSI
 */
int f_zob_v_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '=':
            (*pole)[*pocetTokenuNaRadku].id = VETSIROVNO;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            (*pole)[*pocetTokenuNaRadku].id = VETSI;
            (*pocetTokenuNaRadku)++;
            stav = START;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav ZOB_M_X.
 * Pokud je dalsi znak =, tak vygeneruje token MENSIROVNO, jinak generuje token MENSI
 */
int f_zob_m_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '=':
            (*pole)[*pocetTokenuNaRadku].id = MENSIROVNO;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            (*pole)[*pocetTokenuNaRadku].id = MENSI;
            (*pocetTokenuNaRadku)++;
            stav = START;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav IDENTIF.
 * Generuje token IDENTIFIKATOR pokud jsme nacetli cely identifikator, jinak nacita dalsi znaky identifikatoru a uklada si je do stringu slovo
 */
int f_identif(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '!':
        case '?':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;

            (*pole)[*pocetTokenuNaRadku].id = IDENTIFIKATOR;
            (*pole)[*pocetTokenuNaRadku].attr = calloc(((*delkaSlova) + 1), sizeof(char));
            if (!((*pole)[*pocetTokenuNaRadku].attr)){
                uvolniVse(tok, *pole, *slovo);
            }
            memcpy((*pole)[*pocetTokenuNaRadku].attr, (*slovo), (*delkaSlova)); //ukoncovaci nula z callocu
            (*delkaSlova) = 0;

            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            if((array[*znak]>= 'a' && array[*znak]<= 'z') || (array[*znak]>= 'A' && array[*znak]<= 'Z') || (array[*znak]>= '0' && array[*znak]<= '9') || array[*znak] == '_'){
                (*slovo)[*delkaSlova] = array[*znak];
                (*delkaSlova)++;
                stav = IDENTIF;
                break;
            }
            (*slovo)[*delkaSlova] = '\0';
            if(!strcmp((*slovo), "def")){
                (*pole)[*pocetTokenuNaRadku].id = DEF;
            }
            else if(!strcmp((*slovo), "end")){
                (*pole)[*pocetTokenuNaRadku].id = END;
            }
            else if(!strcmp((*slovo), "if")){
                (*pole)[*pocetTokenuNaRadku].id = IF;
            }
            else if(!strcmp((*slovo), "then")){
                (*pole)[*pocetTokenuNaRadku].id = THEN;
            }
            else if(!strcmp((*slovo), "else")){
                (*pole)[*pocetTokenuNaRadku].id = ELSE;
            }
            else if(!strcmp((*slovo), "while")){
                (*pole)[*pocetTokenuNaRadku].id = WHILE;
            }
            else if(!strcmp((*slovo), "do")){
                (*pole)[*pocetTokenuNaRadku].id = DO;
            }
            else if(!strcmp((*slovo), "true")){
                (*pole)[*pocetTokenuNaRadku].id = BTRUE;
            }
            else if(!strcmp((*slovo), "false")){
                (*pole)[*pocetTokenuNaRadku].id = BFALSE;
            }
            else if(!strcmp((*slovo), "not")){
                (*pole)[*pocetTokenuNaRadku].id = NOT;
            }
            else if(!strcmp((*slovo), "and")){
                (*pole)[*pocetTokenuNaRadku].id = AND;
            }
            else if(!strcmp((*slovo), "or")){
                (*pole)[*pocetTokenuNaRadku].id = OR;
            }
            else if(!strcmp((*slovo), "nil")){
                (*pole)[*pocetTokenuNaRadku].id = NIL;
            }
            else if(!strcmp((*slovo), "elsif")){
                (*pole)[*pocetTokenuNaRadku].id = ELSEIF;
            }
            else{
                (*pole)[*pocetTokenuNaRadku].id = IDENTIFIKATOR;
                (*pole)[*pocetTokenuNaRadku].attr = calloc((*delkaSlova) + 1, sizeof(char));
                if (!((*pole)[*pocetTokenuNaRadku].attr)){
                    uvolniVse(tok, *pole, *slovo);
                }
                memcpy((*pole)[*pocetTokenuNaRadku].attr, (*slovo), (*delkaSlova)); //ukoncovaci nula z callocu

            }
            //Techto dalsich 5 radku se provede vzdy
            (*znak)--;
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav NULA_OTZ.
 * Z tohoto stavu se muze prejit do mnoha dalsich stavu specifikujicich konkretni typ literalu vyjadrujici cislo.
 */
int f_nula_otz(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case 'e':
        case 'E':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_E;
            break;
        case '.':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_X;
            break;
        case 'b':
            //Nepotrebujem strtolovat to 0b nebo 0x
            (*delkaSlova) = 0;
            stav = NULA_B;
            break;
        case 'x':
            (*delkaSlova) = 0;
            stav = NULA_X;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = INT_OCT;
            break;
        default:
            (*znak)--;
            stav = START;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_I;
            (*pole)[*pocetTokenuNaRadku].attr_i = 0;
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav INT_OCT.
 * Tento stav zpracovava cisla v osmickove soustave.
 */
int f_int_oct(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = INT_OCT;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_I;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_i = strtol ((*slovo), NULL, 8);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav INT_HEX.
 * Zpracovava hexadecimalni cisla.
 */
int f_int_hex(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_I;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_i = strtol ((*slovo), NULL, 16);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            if((array[*znak]>= 'a' && array[*znak]<= 'f') || (array[*znak]>= 'A' && array[*znak]<= 'F') || (array[*znak]>= '0' && array[*znak]<= '9')){
                (*slovo)[*delkaSlova] = array[*znak];
                (*delkaSlova)++;
                stav = INT_HEX;
                break;
            }
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav INT_BIN.
 * Zpracovava binarni cisla.
 */
int f_int_bin(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = INT_BIN;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_I;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_i = strtol ((*slovo), NULL, 2);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav NULA_B.
 * Zde je zacatek zpracovavani binarnich cisel.
 */
int f_nula_b(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
            stav = INT_BIN;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav NULA_X.
 * Zde je zacatek zpracovavani hexadecimalnich cisel.
 */
int f_nula_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    if((array[*znak]>= 'a' && array[*znak]<= 'f') || (array[*znak]>= 'A' && array[*znak]<= 'F') || (array[*znak]>= '0' && array[*znak]<= '9')){
        stav = INT_HEX;
    }
    else{
        stav = ERROR;
        (*znak)--;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav INT.
 * Zde se zpracovavaji desitkove inty.
 */
int f_int(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = INT;
            break;
        case 'e':
        case 'E':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_E;
            break;
        case '.':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_X;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_I;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_i = strtol ((*slovo), NULL, 10);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav FLOAT_X.
 * Dostali jsme desetinnou tecku. Ocekava se cislice, jinak nastane chyba.
 */
int f_float_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    if((array[*znak]>= '0' && array[*znak]<= '9')){
        (*slovo)[*delkaSlova] = array[*znak];
        (*delkaSlova)++;
        stav = FLOAT_BEZ_E;
    }
    else{
        stav = ERROR;
        (*znak)--;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav FLOAT_E.
 * Dostali jsme exponent. Ocekava se znamenko, jinak nastane chyba.
 */
int f_float_e(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '-':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_SIGN;
            break;
        case '+':   //to plus se taky zapisovat nemusi
            stav = FLOAT_SIGN;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav FLOAT_SIGN.
 * Dostali jsme znamenko. Ocekava se cislice, jinak nastane chyba.
 */
int f_float_sign(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    if((array[*znak]>= '0' && array[*znak]<= '9')){
        (*slovo)[*delkaSlova] = array[*znak];
        (*delkaSlova)++;
        stav = FLOAT_END;
    }
    else{
        stav = ERROR;
        (*znak)--;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav FLOAT_END.
 * Konecny stav zpracovavani floatu s exponentem.
 */
int f_float_end(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_END;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_D;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_d = strtod((*slovo), NULL);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav FLOAT_BEZ_E.
 * Konecny stav zpracovavani floatu bez exponentu.
 */
int f_float_bez_e(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_BEZ_E;
            break;
        case 'e':
        case 'E':
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = FLOAT_E;
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '<':
        case '>':
        case '!':
        case ')':
        case '&':
        case '|':
        case '?':
        case ':':
        case ',':
        case '\0':
        case ' ':
        case '\t':
            (*znak)--;
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_D;
            (*slovo)[*delkaSlova] = '\0';
            (*delkaSlova)++;
            (*pole)[*pocetTokenuNaRadku].attr_d = strtod((*slovo), NULL);
            (*delkaSlova) = 0;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav STR_OTZ.
 * V tomto stavu probiha hlavni zpracovavani retezcovych literalu.
 */
int f_str_otz(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '\0':
        case '\n':
            stav = ERROR;
            (*znak)--;
            break;
        case '\\':
            stav = STR_SLASH;
            break;
        case '#':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '3';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '5';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case ' ':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '3';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '2';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case '\t':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '9';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case '\"':
            (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_S;
            (*pole)[*pocetTokenuNaRadku].attr = calloc(((*delkaSlova) + 1), sizeof(char));
            if (!((*pole)[*pocetTokenuNaRadku].attr)){
                uvolniVse(tok, *pole, *slovo);
            }
            memcpy((*pole)[*pocetTokenuNaRadku].attr, (*slovo), (*delkaSlova)); //ukoncovaci nula z callocu
            (*delkaSlova) = 0;

            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            (*slovo)[*delkaSlova] = array[*znak];
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav STR_SLASH.
 * Zpracovavani escape sekvenci.
 */
int f_str_slash(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    switch(array[*znak]){
        case '\"':
            (*slovo)[*delkaSlova] = '\"';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case 'n':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '1';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case 't':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '9';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case 's':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '3';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '2';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case '\\':
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '0';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '9';
            (*delkaSlova)++;
            (*slovo)[*delkaSlova] = '2';
            (*delkaSlova)++;
            stav = STR_OTZ;
            break;
        case 'x':
            stav = STR_X;
            break;
        default:
//            (*slovo)[*delkaSlova] = array[*znak];
//            (*delkaSlova)++;
            //Nechame to normalne zpracovat v STR_OTZ
            (*znak)--;
            stav = STR_OTZ;
            break;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav STR_X.
 * Zpracovavani escape sekvenci ve tvaru \x. Čast prvni.
 */
int f_str_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    if((array[*znak]>= 'a' && array[*znak]<= 'f') || (array[*znak]>= 'A' && array[*znak]<= 'F') || (array[*znak]>= '0' && array[*znak]<= '9')){
        (*slovo)[*delkaSlova] = array[*znak]; //pismenko normalne ulozim do vystupniho slova, pak si ho ale musim oddelat
        (*delkaSlova)++;
        stav = STR_XH;
    }
    else{
        stav = ERROR;
        (*znak)--;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav STR_XH.
 * Zpracovavani escape sekvenci ve tvaru \x. Čast druha.
 */
int f_str_xh(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova){
    int stav;
    char a;
    char b[4]={0};
    if((array[*znak]>= 'a' && array[*znak]<= 'f') || (array[*znak]>= 'A' && array[*znak]<= 'F') || (array[*znak]>= '0' && array[*znak]<= '9')){
        (*slovo)[*delkaSlova] = array[*znak]; //pismenko normalne ulozim do vystupniho slova, pak se ale musi oddelat
        (*delkaSlova)++;
        (*slovo)[*delkaSlova] = '\0';
        a = strtol(&((*slovo)[*delkaSlova - 2]), NULL, 16);
        if(a <=32 || a == 92 || a == 35){
            sprintf(b,"%03d", (int)a);
            (*delkaSlova)-=2;
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            for(int i = 0; i < 3; i++){
                (*slovo)[*delkaSlova] = b[i];
                (*delkaSlova)++;
            }
        }
        else{
            (*delkaSlova)-=2;
            (*slovo)[*delkaSlova] = a;
            (*delkaSlova)++;
        }
        stav = STR_OTZ;
    }
    else if(array[*znak] == '\"'){
        (*slovo)[*delkaSlova] = '\0';
        a = strtol(&((*slovo)[*delkaSlova - 1]), NULL, 16);
        (*delkaSlova)--;
        if(a <=32 || a == 92 || a ==35){
            sprintf(b,"%03d", (int)a);
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            for(int i = 0; i < 3; i++){
                (*slovo)[*delkaSlova] = b[i];
                (*delkaSlova)++;
            }
        }
        else{
            (*delkaSlova)--;
            (*slovo)[*delkaSlova] = a;
            (*delkaSlova)++;
        }

        (*pole)[*pocetTokenuNaRadku].id = KONSTANTA_S;
        (*pole)[*pocetTokenuNaRadku].attr = calloc(sizeof(char), (*delkaSlova) + 1);
        if (!((*pole)[*pocetTokenuNaRadku].attr)){
            uvolniVse(tok, *pole, *slovo);
        }
        memcpy((*pole)[*pocetTokenuNaRadku].attr, (*slovo), (*delkaSlova)); //ukoncovaci nula z callocu
        (*delkaSlova) = 0;

        (*pocetTokenuNaRadku)++;
        stav = START;
    }
    else{
        (*znak)--;
        (*slovo)[*delkaSlova] = '\0';
        a = strtol(&((*slovo)[*delkaSlova - 1]), NULL, 16);
        (*delkaSlova)--;
        if(a <=32 || a == 92 || a ==35){
            sprintf(b,"%03d", (int)a);
            (*slovo)[*delkaSlova] = '\\';
            if(realokujSlovo(tok, *pole, slovo, delkaSlova + 4, velikostSlova))
                return ERROR_SYSTEM;
            (*delkaSlova)++;
            for(int i = 0; i < 3; i++){
                (*slovo)[*delkaSlova] = b[i];
                (*delkaSlova)++;
            }
        }
        else {
            (*delkaSlova)--;
            (*slovo)[*delkaSlova] = a;
            (*delkaSlova)++;
        }
        stav = STR_OTZ;
    }
    if(realokujSlovo(tok, *pole, slovo, delkaSlova, velikostSlova))
        return ERROR_SYSTEM;
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav AND_X.
 * Ocekava se znak &.
 */
int f_and_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '&':
            (*pole)[*pocetTokenuNaRadku].id = AND_Z;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}

/**
 * Stavova funkce pro stav OR_X.
 * Ocekava se znak |.
 */
int f_or_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo){
    int stav;
    switch(array[*znak]){
        case '|':
            (*pole)[*pocetTokenuNaRadku].id = OR_Z;
            (*pocetTokenuNaRadku)++;
            stav = START;
            break;
        default:
            stav = ERROR;
            (*znak)--;
            break;
    }
    if(realokovaniPole(tok, pole, *slovo, pocetTokenuNaRadku, velikost))
        return ERROR_SYSTEM;
    return stav;
}
