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

#include "scanner.h"
#include "scanner_fce.h"


#define ERR_LEX 1
#define ERR_SYNTAX 2
#define ERR_SYS 99

/**
 * Lexikalni analyzator - scanner.
 *
 * Popis funkce: Scanner provede pocatecni inicializaci datovych struktur a po te spusti hlavni nacitaci smycku.
 * Ta se sklada ze dvou cyklu, jenz prvni prochazi vstup po radcich a druhy po znacich.
 * Detailneji to funguje tak, ze ve vnejsim cyklu se zavola preprocessor, ktery nacte ze vstupu jeden radek.
 * Tento radek je pak zpracovan na tokeny vnitrnim cyklem.
 *
 * @param *radek - pres tento parametr vraci scanner pocet nactenych radku
 * @return - Vystupem scanneru je 2d pole tokenu
 *
 * @post - Kazdy radek vystupniho pole je ukoncen tokenem KONEC_RADKU
 */
token **scanner(int *radek){
    char *array;

    int znak = 0;

    int stav = START;
    int velikost = 64;

    char *slovo;
    int delkaSlova = 0;
    int velikostSlova = 1024;
    int lines = 1024;
    *radek = 0;
    //dynamicka alokace
    token *pole = calloc(sizeof(token), velikost);
    if (!pole)
        return (token **)-ERR_SYS;


    token **tok = calloc(sizeof(token*), lines);
    if (!tok) {
        free(pole);
        return (token **)-ERR_SYS;
    };
    slovo = calloc(sizeof(token), velikostSlova);
    if (!slovo){
        free(pole);
        free(tok);
        return (token **)-ERR_SYS;
    }

    //pruchod vstupu po radcich
    while ((array = preprocessor(stdin)) != NULL) {
        if (array == (void*)-ERR_SYS) {
            uvolniVse(tok, pole, slovo);;
            return (token **)-ERR_SYS;
        }
        if (array == (void*)-ERR_LEX) {
            uvolniVse(tok, pole, slovo);
            return (token **)-ERR_LEX;
        }
        if(*radek == lines){
            //realokace tok**
            lines *=2;
            token **toknew = realloc(tok, sizeof(token*)* lines);
            if(!toknew){
                uvolniVse(tok, pole, slovo);
                fprintf(stderr, "Alokace failnula v scanneru.\n");
                return (token **)-ERR_SYS;
            }
            tok = toknew;
        }
        znak = 0;
        int delkaRadku = strlen(array);
        int pocetTokenuNaRadku = 0;
        //pruchod vstupu znacich
        //Pro kazdy stav se vola prislusna stavova fce ze scanner_fce
        while(znak <= delkaRadku){
            switch(stav){
                case START:
                    stav = f_start(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case VYK_X:
                    stav = f_vyk_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case ROV_X:
                    stav = f_rov_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case ZOB_M_X:
                    stav = f_zob_m_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case ZOB_V_X:
                    stav = f_zob_v_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case STR_OTZ:
                    stav = f_str_otz(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case STR_SLASH:
                    stav = f_str_slash(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case STR_X:
                    stav = f_str_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case STR_XH:
                    stav = f_str_xh(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case FLOAT_X:
                    stav = f_float_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case FLOAT_E:
                    stav = f_float_e(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case FLOAT_SIGN:
                    stav = f_float_sign(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case FLOAT_BEZ_E:
                    stav = f_float_bez_e(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case FLOAT_END:
                    stav = f_float_end(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case INT:
                    stav = f_int(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case NULA_OTZ:
                    stav = f_nula_otz(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case NULA_X:
                    stav = f_nula_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case NULA_B:
                    stav = f_nula_b(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case INT_BIN:
                    stav = f_int_bin(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case INT_HEX:
                    stav = f_int_hex(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case INT_OCT:
                    stav = f_int_oct(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case IDENTIF:
                    stav = f_identif(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo, &delkaSlova, &velikostSlova);
                    break;
                case AND_X:
                    stav = f_and_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case OR_X:
                    stav = f_or_x(array, tok, &znak, &pocetTokenuNaRadku, &pole, &velikost, &slovo);
                    break;
                case ERROR:
                    //uvolniVse(tok, pole, slovo);
                    fprintf(stderr, "Lexikalni chyba.\n");
                    return (token **)-ERR_LEX;
                case ERROR_SYSTEM:
                    fprintf(stderr, "Systemova chyba v ramci scanneru.\n");
                    return (token **)-ERR_SYS;
                default:
                    break;
            }

            znak++;
        }
        //Zde se prekopirovava radek tokenu z pomocneho 1d pole 'pole' do vystupniho 2d pole 'tok'. Ve vystupnim poli bude radek tokenu mit minimalni dostatecnou velikost.
        if(pocetTokenuNaRadku != 0){
            tok[*radek] = malloc(sizeof(token) * (pocetTokenuNaRadku));
            if(!tok[*radek]){
                //uvolnit vse
                free(pole);
                for(int i = 0; i < (*radek); i++){
                    free(tok[i]);
                }
                return (token **)-ERR_SYS;
            }
            else {
                memcpy(tok[*radek], pole, sizeof(token) * pocetTokenuNaRadku);
            }
        }
        delete(array, false, NULL);
        (*radek)++;
    }
    (*radek)++;
    //Realokace na spravny pocet radku
    token **toknew = realloc(tok, sizeof(token*) * (*radek));
    if(!toknew){
        uvolniVse(tok, pole, slovo);
        fprintf(stderr, "Alokace failnula ve scanneru.\n");
        return (token **)-ERR_SYS;
    }
    tok = toknew;
    //Pridani koncoveho tokenu konec souboru.
    tok[(*radek) - 1] = calloc(2, sizeof(token));
    if (!tok[(*radek) - 1]){
        uvolniVse(tok, pole, slovo);
        fprintf(stderr, "Alokace failnula ve scanneru.\n");
        return (token **)-ERR_SYS;
    }
    tok[(*radek) - 1][0].id = FILEND;
    //Uvolneni pomocnych struktur
    free(pole);
    free(slovo);
    (*radek)--;
    return tok;
}
