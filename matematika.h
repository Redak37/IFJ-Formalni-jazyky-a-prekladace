#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "gassembler.h"
#include <stdbool.h>

#include "symtable.h"

#define ERR_LEX 1           //Lexikalni chyba
#define ERR_SYNTAX 2        //chyba syntax, např. "něco chybí, něčeho je moc"
#define ERR_SEM_DEF 3       //chyba nedefinovaná proměnná/fce, redefinice
#define ERR_SEM_OPER 4      //typová chyba ve výrazech
#define ERR_PARAM 5         //špatný počet param fce
#define ERR_SEM_JINAK 6     //jiná sémantická chyba
#define ERR_SYS 99          //systémová chyba - např malloc

sa_pos MA(sa_pos p, token **tokpole, htab_t tab_lok);



#ifdef blablablab
static int delkaRadku(int lines, int cislo_radku, token **pole);
static bool jetuEOF(sa_pos p, int lines, token **pole);
static bool jsemMimo(sa_pos p, int lines, token **pole);
static sa_pos posunHlavu(sa_pos p, int posun, int lines, token **pole);
static sa_pos posunZarazku(sa_pos p, int posun, int lines, token **pole);
static bool semNaZaraz(sa_pos p);
static sa_pos zarazkaEND(sa_pos p, int lines, token **pole);
static sa_pos zarazkaELS(sa_pos p, int lines, token **pole, TOKTYPE tok);
#endif

#endif
