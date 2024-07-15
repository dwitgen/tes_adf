#include "esp_adf.h"
#include "esphome/core/defines.h"

#ifdef USE_ESP_IDF

#ifdef USE_ESP_ADF_BOARD
#include <board.h>
#endif

#include "esphome/core/log.h"
#include "button/esp_adf_button.h" 

namespace esphome {
namespace esp_adf {

static const char *const TAG = "esp_adf";

ButtonHandler button_handler;

void ESPADF::setup() {
#ifdef USE_ESP_ADF_BOARD
  ESP_LOGI(TAG, "Start codec chip");
  audio_board_handle_t board_handle = audio_board_init();
  audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
#endif
// Call ButtonHandler setup
  ESP_LOGI(TAG, "Setting up ButtonHandler");
  button_handler.setup();
  ESP_LOGI(TAG, "ButtonHandler setup completed");
}


float ESPADF::get_setup_priority() const { return setup_priority::HARDWARE; }

}  // namespace esp_adf
}  // namespace esphome

#endif  // USE_ESP_IDF
