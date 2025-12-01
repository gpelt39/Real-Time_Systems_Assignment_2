#include "tiebreak.h"

UBaseType_t uxTaskTieBreakerGet() {
    StaticTask_t** mypxCurrentTCB = (StaticTask_t**)(&pxCurrentTCB);
    return mypxCurrentTCB[0]->uxDummy5b;
}

void vTaskTieBreakerSet(UBaseType_t value) {
    StaticTask_t** mypxCurrentTCB = (StaticTask_t**)(&pxCurrentTCB);
    mypxCurrentTCB[0]->uxDummy5b = value;
    ((ListItem_t*)&(mypxCurrentTCB[0]->xDummy3))->xItemValue = value;
}
