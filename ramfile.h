#ifndef __RAMFILE_H_
#define __RAMFILE_H_

/*  Knihovna RamFile
 *
 *  Autor:  Josef Oškera, VUT FIT (xosker03)
 *          Jan Juda, VUT FIT (xjudaj00)
 *
 *  Datum: 14.11.2018
 *
 */


void ramFileSwap(int *zapisovany_radek, int novy_stav);

void pisKonec(int max_delka_plus, const char *fmt, ...);

int pisNa(int cislo_radku, int max_delka_plus, const char *fmt, ...);

char *vypisNa(int cislo);

int vymazNa(int cislo);

int ramFilePocetRadku(void);

int dejUID(void);

#endif
