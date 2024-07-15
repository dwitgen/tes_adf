//#ifndef ESP_ADF_BUTTON_H
//#define ESP_ADF_BUTTON_H

#pragma once

#ifdef USE_ESP_IDF

#include "../esp_adf.h"
#include <esp_event.h>  // Include the header for esp_event_base_t

#include <board.h>
#include "esp_peripherals.h"
#include "periph_adc_button.h"
#include "input_key_service.h"
#include "esp_log.h"

namespace esphome {
namespace esp_adf {

//class ESPADFSpeaker;  // Forward declaration

class ButtonHandler {
 public:
  
  void setup(); // override;
 
  static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);
  
  void volume_up();
  void volume_down();
  void handle_mode_button();
  void handle_play_button();
  void handle_set_button();
  void handle_rec_button();

  void set_volume(int volume);
  int get_current_volume();

 protected:
  static void button_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
  void handle_button_event(int32_t id, int32_t event_type);  // Add this declaration

 private:
  static const char *const TAG;
  int volume_ = 50;  // Default volume level
};

}  // namespace esp_adf
}  // namespace esphome
#endif  // USE_ESP_IDF
//#endif // ESP_ADF_BUTTON_H
