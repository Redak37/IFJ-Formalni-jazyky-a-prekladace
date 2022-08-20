/*  Knihovna Symtable
 *
 *  Autor:  Josef Oškera, VUT FIT (xosker03)
 *
 *  Datum:  10.11.2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "zasobniky.h"



////////////////////////////////// special array stack /////////////////////////

bool ASInit(ArStack *s, unsigned size){
    assert(s);
    assert(size);
    s->data = malloc(size * sizeof(void*));
    if (s->data){
        s->vel = size;
        s->zap = 0;
        return true;
    } else {
        s->vel = 0;
        s->zap = 0;
        return false;
    }
}

void **ASPush(ArStack *s, void* data){
    assert(s);
    if(s->vel == s->zap){
        void *tmp = realloc(s->data, s->vel * 2 * sizeof(void*));
        if(tmp){
            s->data = tmp;
            s->vel *= 2;
        } else {
            fprintf(stderr, "AS Chyba neni ram\n");
            return NULL;
        }
    }

    s->data[s->zap] = data;
    s->zap++;
    return &s->data[s->zap - 1];
}


void *ASTopPop(ArStack *s){
    assert(s);
    if (s->zap == 0){
        return NULL;
    }
    return s->data[s->zap--];
}
bool ASSearch(ArStack *s, void* data){
    assert(s);
    if (!data){
      return false;
    }
    uint64_t zap = s->zap;
    for(uint64_t i = 0; i < zap; ++i){
        if (s->data[i] == data){
            return true;
        }
    }
    return false;
}

// 0 - smaže pole // 1 - smaže obsah // 2 - smaže vše
void ASDestroy(ArStack *s, uint8_t destroy_level){
    assert(s);
    uint64_t zap = s->zap;
    if (destroy_level > 0){
        for(uint64_t i = 0; i < zap; ++i){
            free(s->data[i]);
        }
    }
    if (destroy_level > 1 || destroy_level == 0){
        free(s->data);
        s->vel = 0;
        s->zap = 0;
        s->data = NULL;
    }
}
