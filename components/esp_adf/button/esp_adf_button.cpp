#include "esp_adf_button.h"
//#include "esp_adf_speaker.h"
#include <board.h>
#include <audio_hal.h>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include "esp_peripherals.h"
#include "periph_adc_button.h"
#include "input_key_service.h"

namespace esphome {
namespace esp_adf {

const char *const ButtonHandler::TAG = "esp_adf.button";

void ButtonHandler::setup() {
      // Initialize the peripheral set with increased queue size
    ESP_LOGI(TAG, "Initializing peripheral set...");
    esp_periph_config_t periph_cfg = {
        .task_stack = 16384, //8192,
        .task_prio = 10, //5,
        .task_core = 0,
        .extern_stack = false
    };
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    if (!set) {
        ESP_LOGE(TAG, "Failed to initialize peripheral set");
        return;
    }

    // Initialize the audio board keys
    ESP_LOGI(TAG, "Initializing audio board keys...");
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = {
        .based_cfg = {
            .task_stack = ADC_BUTTON_STACK_SIZE, //4 * 1024, // INPUT_KEY_SERVICE_TASK_STACK_SIZE,
            .task_prio = ADC_BUTTON_TASK_PRIORITY, //10, //INPUT_KEY_SERVICE_TASK_PRIORITY,
            .task_core = ADC_BUTTON_TASK_CORE_ID, //INPUT_KEY_SERVICE_TASK_ON_CORE,
            .task_func = nullptr,
            .extern_stack = false,
            .service_start = nullptr,
            .service_stop = nullptr,
            .service_destroy = nullptr,
            .service_ioctl = nullptr,
            .service_name = nullptr,
            .user_data = nullptr
        },
        .handle = set
    };
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, ButtonHandler::input_key_service_cb, this); 
}

/*esp_err_t ButtonHandler::input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx) {
    ESPADFSpeaker *instance = static_cast<ESPADFSpeaker*>(ctx);
    int32_t id = static_cast<int32_t>(reinterpret_cast<uintptr_t>(evt->data));

    // Read the ADC value
    int adc_value = adc1_get_raw(ADC1_CHANNEL_3);  // Replace with your ADC channel
    ESP_LOGI("ButtonHandler", "Button event callback received: id=%d, event type=%d, ADC value=%d", id, evt->type, adc_value);

    instance->handle_button_event(id, evt->type);
    return ESP_OK;
}*/

void ButtonHandler::handle_button_event(int32_t id, int32_t event_type) {
    ESP_LOGI("ButtonHandler", "Handle Button event received: id=%d", id);
    if (event_type != 1 && event_type != 3) { // Only process the event if the event_type is 1 click action or 3 long press action
        ESP_LOGI("ButtonHandler", "Ignoring event with type: %d", event_type);
        return;
    }
    uint32_t current_time = millis();
    static uint32_t last_button_press[7] = {0};
    uint32_t debounce_time = 200;

    if (id == BUTTON_MODE_ID) {
        debounce_time = 500;
    }

    if (current_time - last_button_press[id] > debounce_time) {
        switch (id) {
            case 0:
                ESP_LOGI("ButtonHandler", "Unknown Button detected");
                //volume_down();
                break;
            case 1:
                ESP_LOGI("ButtonHandler", "Record button detected");
                handle_rec_button();
                break;
            case 2:
                ESP_LOGI("ButtonHandler", "Set button detected");
                handle_set_button();
                break;
            case 3:
                ESP_LOGI("ButtonHandler", "Play button detected");
                handle_play_button();
                break;
            case 4:
                ESP_LOGI("ButtonHandler", "Mode button detected");
                handle_mode_button();
                break;
            case 5:
                ESP_LOGI("ButtonHandler", "Volume down detected");
                // instance->volume_down(); // Comment out or handle appropriately
                break;
            case 6:
                ESP_LOGI("ButtonHandler", "Volume up detected");
                // instance->volume_up(); // Comment out or handle appropriately
                break;
            default:
                ESP_LOGW("ButtonHandler", "Unhandled button event id: %d", id);
                break;
        }
        last_button_press[id] = current_time;
    }
}

void ButtonHandler::volume_up() {
    ESP_LOGI(TAG, "Volume up button pressed");
    //int current_volume = this->get_current_volume();
    //this->set_volume(current_volume + 10);
}

void ButtonHandler::volume_down() {
    ESP_LOGI(TAG, "Volume down button pressed");
    //int current_volume = this->get_current_volume();
    //this->set_volume(current_volume - 10);
}

void ButtonHandler::handle_mode_button() {
    // Implementation of mode button handling
}

void ButtonHandler::handle_play_button() {
    // Implementation of play button handling
}

void ButtonHandler::handle_set_button() {
    // Implementation of set button handling
}

void ButtonHandler::handle_rec_button() {
    // Implementation of record button handling
}

}  // namespace esp_adf
}  // namespace esphome
