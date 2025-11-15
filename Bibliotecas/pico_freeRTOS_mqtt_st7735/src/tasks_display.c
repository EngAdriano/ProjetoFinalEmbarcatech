#include "tasks_display.h"
#include <stdio.h>
#include "st7735.h"

void task_display(void *pvParameters) {
    (void) pvParameters;
    st7735_init();
    st7735_fill_screen(0x0000);
    st7735_set_cursor(0, 0);
    st7735_write_string("Pico W Project", 1);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
