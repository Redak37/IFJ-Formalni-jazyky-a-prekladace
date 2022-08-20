#ifndef __ZASOBNIKY_H_
#define __ZASOBNIKY_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct ArrayStack{
  uint64_t vel;
  uint64_t zap;
  void **data;
}ArStack;


bool ASInit(ArStack *s, unsigned size);

void **ASPush(ArStack *s, void* data);

void *ASTopPop(ArStack *s);
/*
void *ASTop(ArStack *s);
*/
bool ASSearch(ArStack *s, void* data);

// 0 - smaže pole // 1 - smaže obsah // 2 - smaže vše
void ASDestroy(ArStack *s, uint8_t destroy_level);
/*
int ASzap(ArStack *s);
*/
/*
ArStack *ASDeepCopy(ArStack *s);
*/
#endif
