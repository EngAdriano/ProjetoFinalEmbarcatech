#ifndef UI_ENERGY_H
#define UI_ENERGY_H

#include "pzem004t.h"

/* Inicializa UI (display + layout principal) */
void UI_Energy_Init(void);

/* Exibe tela de abertura */
void UI_Energy_ShowSplash(void);

/* Atualiza valores do PZEM */
void UI_Energy_Update(const pzem_data_t *data);

#endif
