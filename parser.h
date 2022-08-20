#ifndef __NEO_PARSER_H_
#define __NEO_PARSER_H_

#include "scanner.h"

typedef enum neterminaly {
    PROG,   //S
    LISTV,  //SV
    LISTIF, //SIF
    ARG,    //A
    NARG,   //Ai
    PRIKAZ, //P
    BLOKV,  //Q
    IDN1    //IDN1
}neterminaly_t;

int LLSA(token **tokpole, neterminaly_t net);



#endif
