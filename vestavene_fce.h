#ifndef __VESTAVENE_FUNKCE_H_
#define __VESTAVENE_FUNKCE_H_

#include "scanner.h"
#include "stdbool.h"

void Exity(void);

void v_inputs(int *radek);
void v_inputi(int *radek);
void v_inputf(int *radek);

//promenny pocet stringu
//na zasobnik:  str[n]
//              str[n-1]
//              .....
//              str[1]
// (vrchol)     int(pocet)
void v_print(int *radek, unsigned pocet);

//na zasobnik:  string
int v_length(int *radek);

//na zasobnik:  n int(delka)
//              i int(odkud)
//              s string
void v_substr(int *radek);

//na zasobnik:  int(na_pozici)
//              string
int v_ord(int *radek);

void v_chr(int *radek);

void v_not(int *zapisovany_radek);

void generateIf(int *radek, int lokal_label);
void generateElse(int *radek, int lokal_label);
void generateEnd(int *radek, int lokal_label);

token v_length_K(token tok);
token v_not_K(token tok);
token v_chr_K(token tok);
token v_ord_K_K(token toks, token toki);
int v_ord_P_K(int *radek, token tok);
int v_substr_P_P_K(int *radek, token tokn);
int v_substr_P_K_K(int *radek, token toki, token tokn);
token v_substr_K_K_K(token toks, token toki, token tokn);

#endif
