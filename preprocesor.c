/*  Soubor: Knihovna pro preprocesor
 *
 *  Autor:  Radek Duchoň, VUT FIT (xducho07)
 *
 *  Datum:  15.11.2018
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "preprocesor.h"


#define ERR_LEX 1
#define ERR_SYNTAX 2
#define ERR_SYS 99

char *preprocessor(FILE *fp);
//Pomocna funkce pro smazani pole a pripadne ohlaseni chyby
void *delete(char *array, bool err, void *navrat)
{
    if (array)
        free(array);
    if (err)
        fprintf(stderr, "Error: Memory could not been allocated.\n");//TODO chyba alokace
    return navrat;
}
//Funkce zahazuje duplicitni bile znaky, prazdne radky a komentare.
//Nasledne vrati 1 radek pro zpracovani scanneru.
char *preprocessor(FILE *fp)
{
    //Promenne pro nacitani znaku, aktualni velikost a max velikost nacitaneho retezce.
    int c, i = 0, size = 256;
    //Promenne znacici jestli se ma dalsi mezera smazat a jestli byl posledni znak smazan
    bool spaces = true, deleted = false;
    //Promenne pro urceni, jestli se nachazime ve stringu
    bool str = false, lomitko = false;
    //promenna s nacitanym retezcem
    char *line = calloc(size, sizeof(char));

    if (!line)
        return NULL;
    //Nacitani do konce souboru
    while ((c = fgetc(fp)) != EOF) {
        //Mimo string upravi \t na mezeru
        if (str == false && c == '\t')
            c = ' ';
        //Nacitani blokoveho komentare, pokud byl znak '=' prvnim na radku
        if (!deleted && c == '=' && i == 0) {
            if (fgetc(fp) != 'b' || fgetc(fp) != 'e' || fgetc(fp) != 'g' || fgetc(fp) != 'i' || fgetc(fp) != 'n') {
                fprintf(stderr, "Error: This should probably be a block comment, but it's not.\n");
                return delete(line, false, (void*)-ERR_LEX);
            }
            c = fgetc(fp);
            if (c != ' ' && c != '\t' && c != '\n') {
                fprintf(stderr, "Error: This should probably be a block comment, but it's not.\n");
                return delete(line, false, (void*)-ERR_LEX);
            }
            while (1) {
                while (c != '\n' && c != EOF)
                    c = fgetc(fp);
                //Nalezen EOF uvnitr blokoveho komentare
                if (c == EOF) {
                    fprintf(stderr, "Error: Block comment without end.\n");
                    return delete(line, false, (void*)-ERR_LEX);
                }
                //Vyhledava "=end" na zacatku kazdeho radku
                if (c == '\n') {
                    if ((c = fgetc(fp)) == '=' && (c = fgetc(fp)) == 'e' && (c = fgetc(fp)) == 'n' && (c = fgetc(fp)) == 'd') {
                        c = fgetc(fp);
                        if (c == ' ' || c == '\t' || c == '\n' || c == EOF) {
                            while (c != '\n' && c != EOF)
                                c = fgetc(fp);
                            break;
                        }
                    }
                }
            }
            //Zacne nacitat dalsi radek vstupu
            continue;
        }
        //Odmazava radkovy komentar
        if (c == '#' && str == false) {
            while (c != '\n' && c != EOF)
                c = fgetc(fp);

            deleted = false;
            //Zacne nove nacitani, pokud nebylo pred komentarem nic nacteno
            if (!i)
                continue;
            //Zaridi aby string nekonil zbytecnou mezerou.
            if (line[i-1] == ' ')
                line[i-1] = '\0';

            return line;
        }
        //Urcovani, jestli je vstup zrovna ve stringu
        if (str && !lomitko && c == '\\')
            lomitko = true;
        else if (!lomitko && c == '\"')
            str = !str;
        else
            lomitko = false;
        //Byl nalezen konec radku
        if (c == '\n' && str == false) {
            if (!i)
                continue;
            //Odstrani mezeru na konci stringu
            if (line[i-1] == ' ')
                line[i-1] = '\0';

            return line;
        } else {
            //Realokace pro pripadnou nutnou vetsi delku radku
            if (i == size) {
                size *= 2;
                char *ctrl = realloc(line, sizeof(char) * size);
                if (!ctrl)
                    return delete(line, true, (void*)-ERR_SYS);

                line = ctrl;
            }
            //Zapis do vystupniho retezc
            if (c != ' ' || spaces == false || str == true) {
                line[i++] = c;
            } else
                deleted = true;

            if (c == ' ')
                spaces = true;
            else
                spaces = false;
        }
    }
    //Pokud byl nalezen EOF na novem radku, vrati NULL, jinak co bylo pred nim.
    if (c == EOF) {
        if (i != 0)
            return line;
        free(line);
        return NULL;
    }

    return line;
}
