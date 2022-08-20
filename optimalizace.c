/*  Soubor: Knihovna pro optimalizace
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *
 *  Datum:  15.11.2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ramfile.h"
#include "optimalizace.h"
#include "opti2.h"

int optimalizuj(void)
{
    int lines = ramFilePocetRadku();
    char *str, *str2, *prom;
    bool prepis = true;
    bool zmeneno = false;
    //Odstrani zbytecne LABELy
    for (int i = 0; i < lines; ++i) {
        str2 = vypisNa(i);
        str = strstr(str2, "LABEL");
        if (str != NULL && ((str != str2 && str[-1] != '@') || str == str2)) {
            if (strstr(str, "LABEL") && !strstr(str, "FUNKCE")) {
                prom = &str[6];
                for (int j = 0; j < lines; ++j) {
                    str2 = vypisNa(j);
                    if (strstr(str2, "JUMP") && strstr(str2, prom)) {
                        prepis = false;
                        break;
                    }
                }
                if (prepis) {

                    vymazNa(i);
                    --lines;
                    --i;
                    continue;
                }
                prepis = true;
            }
        }
    }
    //Odstrani zbytecne MOVE
    for (int i = 0; i < lines; ++i) {
        str = vypisNa(i);
        if (str == NULL)
            break;
        if (strstr(str, "MOVE")) {
            char *prom = calloc(strlen(str) + 1, sizeof(char));
            strcpy(prom, str);
            for (int j = 6; prom[j]; ++j) {
                if (prom[j] == ' ') {
                    prom[j+1] = '\0';
                    break;
                }
            }
            str2 = vypisNa(i+1);
            for (int j = i + 2; !strstr(str2, "JUMP") && j < lines && !strstr(str2, "RETURN"); ++j) {
                if (str == NULL)
                    break;
                if (strstr(str2, &prom[5])) {
                    if (strstr(str2, "PUSHS") || strstr(str2, "WRITE") || strstr(str2, "READ"))
                        break;
                    if (strstr(str2, prom) || strstr(str2, "POPS")) {
                        vymazNa(i);
                        --lines;
                        --i;
                        zmeneno = true;
                        break;
                    }

                }
                str2 = vypisNa(j);
            }
            free(prom);
        }
    }
    if (zmeneno)
        return opt2(0, ramFilePocetRadku());
    return 0;
}
