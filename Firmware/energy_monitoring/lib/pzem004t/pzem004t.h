#ifndef PZEM004T_H
#define PZEM004T_H

#include "pico/stdlib.h"

typedef struct {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
} pzem_data_t;

void pzem_init();
bool pzem_read(pzem_data_t *out);

#endif
