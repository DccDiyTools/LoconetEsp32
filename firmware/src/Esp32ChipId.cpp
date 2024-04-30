#include <Arduino.h>

#if CONFIG_IDF_TARGET_ESP32
  #include <rom/spi_flash.h>
#elif CONFIG_IDF_TARGET_ESP32S2  // ESP32-S2
  #include "esp32s2/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32S3  // ESP32-S3
  #include "esp32s3/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32C3  // ESP32-C3
  #include "esp32c3/rom/spi_flash.h"
#endif

uint32_t ESP_getFlashChipId(void)
{
  uint32_t id = g_rom_flashchip.device_id;
  id = ((id & 0xff) << 16) | ((id >> 16) & 0xff) | (id & 0xff00);
  return id;
}

uint32_t ESP_getFlashChipRealSize(void)
{
  uint32_t id = (ESP_getFlashChipId() >> 16) & 0xFF;
  return 2 << (id - 1);
}