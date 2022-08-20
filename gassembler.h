#ifndef __GENERATOR_ASSERMBLERU_H_
#define __GENERATOR_ASSERMBLERU_H_

#include "scanner.h"
#include <stdbool.h>
#include "symtable.h"

typedef enum stavy {
    SPROG,
    SVYRAZ,
    SLISTV,
    SBLOKV
}stavy_t;


typedef struct ridici_pozice{
    int odR;
    int odS;
    int doR;
    int doS;
    int chyba;
    int lokal_label;
    int vnoreni;
} sa_pos;


sa_pos GA(sa_pos p,int lines, int hloubka, token **tokpole, stavy_t net);

int delkaRadku(int lines, int cislo_radku, token **pole);
bool jetuEOF(sa_pos p, int lines, token **pole);
sa_pos posunHlavu(sa_pos p, int posun, int lines, token **pole);
void DefVar(int *zapisovany_radek, char *key, symbol *data);


#endif
