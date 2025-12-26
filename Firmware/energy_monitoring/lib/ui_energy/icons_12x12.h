#ifndef ICONS_12X12_H
#define ICONS_12X12_H

#include <stdint.h>

#ifdef __cplusplus
  #include <cstdint>
  using std::uint16_t;
#else
  #include <stdint.h>
#endif

/* ===============================
 * Ícone WiFi (12x12)
 * =============================== */
/*
static const uint16_t icon_wifi_12x12[] = {
    0x0000,
    0x0180,
    0x03C0,
    0x07E0,
    0x0FF0,
    0x1C38,
    0x381C,
    0x0000,
    0x0180,
    0x0180,
    0x0000,
    0x0000
};*/

static const uint16_t icon_wifi_12x12[] = {
    0x0FF0,  // #### ####
    0x1FF8,  // ##### ###
    0x300C,  // ##    ##
    0x07E0,  //  ######
    0x0FF0,  // #### ####
    0x180C,  // ##      ##
    0x03C0,  //   ####
    0x0660,  //  ##  ##
    0x0180,  //    ##
    0x0180,  //    ##
    0x0000,
    0x0000
};


/* ===============================
 * Ícone MQTT (12x12)
 * Estilo "nós conectados"
 * =============================== */
static const uint16_t icon_mqtt_12x12[] = {
    0x0000,
    0x0600,
    0x0900,
    0x1080,
    0x2040,
    0x2040,
    0x1080,
    0x0900,
    0x0600,
    0x0000,
    0x0000,
    0x0000
};

#endif /* ICONS_12X12_H */
