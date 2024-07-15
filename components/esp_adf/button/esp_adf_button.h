#ifndef ESP_ADF_BUTTON_H
#define ESP_ADF_BUTTON_H

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
  static void button_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
  static void handle_button_event(int32_t id, int32_t event_type);  // Add this declaration
  static void volume_up();
  static void volume_down();
  static void handle_mode_button();
  static void handle_play_button();
  static void handle_set_button();
  static void handle_rec_button();

  void set_volume(int volume);
  int get_current_volume();

 private:
  static const char *const TAG;
  int volume_ = 50;  // Default volume level
};

}  // namespace esp_adf
}  // namespace esphome

#endif // ESP_ADF_BUTTON_H
