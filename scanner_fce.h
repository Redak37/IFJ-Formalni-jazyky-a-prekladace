/*  Knihovna Scanner_Fce
 *
 *  Autor:  Jan Juda, VUT FIT (xjudaj00)
 *          Jan Dolezal, VUT FIT (xdolez81)
 *  Upravil:  Josef Oškera, VUT FIT (xosker03)
 *            Radek Duchoň, VUT FIT (xducho07)
 *
 *  Datum: podzim 2018
 *
 */

#ifndef __SCANNER_FCE_H_
#define __SCANNER_FCE_H_

#include "scanner.h"

//Hlavicky pomocnych funkci
void uvolniVse(token **tok, token *pole, char *slovo);
int realokovaniPole(token **tok, token **pole, char *slovo, int *pocetTokenuNaRadku, int *velikost);
int realokujSlovo(token **tok, token *pole, char **slovo, int *delkaSlova, int *velikostSlova);

//Hlavicky stavovych funkci - vice info v scanner_fce.c
int f_start(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_vyk_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_rov_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_zob_v_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_zob_m_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_identif(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_int_oct(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_nula_otz(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_int_hex(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_int_bin(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_nula_b(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_nula_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_int(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_float_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_float_e(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_float_sign(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_float_end(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_float_bez_e(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_str_otz(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_str_slash(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_str_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_str_xh(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo, int *delkaSlova, int *velikostSlova);
int f_and_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);
int f_or_x(char *array, token **tok, int *znak, int *pocetTokenuNaRadku, token **pole, int *velikost, char **slovo);

#endif
