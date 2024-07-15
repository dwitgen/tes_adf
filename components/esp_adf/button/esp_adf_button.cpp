#include "esp_adf_button.h"

#ifdef USE_ESP_IDF

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
    ESP_LOGI(TAG, "Setting up ButtonHandler...");
      // Initialize the peripheral set with increased queue size
    ESP_LOGI(TAG, "Initializing peripheral set...");

    uint32_t volume_sensor_key = 0;
    for (auto *sensor : App.get_sensors()) {
        if (sensor->get_name() == "generic_volume_sensor") {
            volume_sensor_key = sensor->get_object_id_hash();
            break;
        }
    }

    if (volume_sensor_key != 0) {
        this->volume_sensor = App.get_sensor_by_key(volume_sensor_key, true);
        ESP_LOGI(TAG, "Internal generic volume sensor initialized successfully: %s", this->volume_sensor->get_name().c_str());
    } else {
        ESP_LOGE(TAG, "Failed to find key for internal generic volume sensor");
    }

    if (this->volume_sensor == nullptr) {
        ESP_LOGE(TAG, "Failed to get internal generic volume sensor component");
    } else {
        ESP_LOGI(TAG, "Internal generic volume sensor initialized correctly");
    }

    this->set_volume(volume_);

    int initial_volume = this->get_current_volume();
    this->set_volume(initial_volume);
    
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
    ESP_LOGI(TAG, "ButtonHandler setup completed");
}

esp_err_t ButtonHandler::input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx) {
    ButtonHandler *instance = static_cast<ButtonHandler*>(ctx);  // Use ButtonHandler instance
    int32_t id = static_cast<int32_t>(reinterpret_cast<uintptr_t>(evt->data));

    ESP_LOGI(TAG, "Button event callback received: id=%d, event type=%d", id, evt->type);

    instance->handle_button_event(id, evt->type);
    return ESP_OK;
}

void ButtonHandler::set_volume(int volume) {
    ESP_LOGI(TAG, "Setting volume to %d", volume);
    
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    this->volume_ = volume;

    audio_board_handle_t board_handle = audio_board_init();
    esp_err_t err = audio_hal_set_volume(board_handle->audio_hal, volume);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting volume: %s", esp_err_to_name(err));
    }

    if (this->volume_sensor != nullptr) {
        this->volume_sensor->publish_state(this->volume_);
    } else {
        ESP_LOGE(TAG, "Volume sensor is not initialized");
    }
}

int ButtonHandler::get_current_volume() {
    /*audio_board_handle_t board_handle = audio_board_init();
    if (board_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to initialize audio board");
        return 0;
    }*/

    int current_volume = 0;
    esp_err_t read_err = audio_hal_get_volume(board_handle->audio_hal, &current_volume);
    if (read_err == ESP_OK) {
        ESP_LOGI(TAG, "Current device volume: %d", current_volume);
    } else {
        ESP_LOGE(TAG, "Error reading current volume: %s", esp_err_to_name(read_err));
    }

    return current_volume;
}

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
                break;
            case BUTTON_REC_ID:
                ESP_LOGI("ButtonHandler", "Record button detected");
                handle_rec_button();
                break;
            case BUTTON_SET_ID:
                ESP_LOGI("ButtonHandler", "Set button detected");
                handle_set_button();
                break;
            case BUTTON_PLAY_ID:
                ESP_LOGI("ButtonHandler", "Play button detected");
                handle_play_button();
                break;
            case BUTTON_MODE_ID:
                ESP_LOGI("ButtonHandler", "Mode button detected");
                handle_mode_button();
                break;
            case BUTTON_VOLDOWN_ID:
                ESP_LOGI("ButtonHandler", "Volume down detected");
                volume_down(); // Comment out or handle appropriately
                break;
            case BUTTON_VOLUP_ID:
                ESP_LOGI("ButtonHandler", "Volume up detected");
                volume_up(); // Comment out or handle appropriately
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
    int current_volume = this->get_current_volume();
    this->set_volume(current_volume + 5);
}

void ButtonHandler::volume_down() {
    ESP_LOGI(TAG, "Volume down button pressed");
    int current_volume = this->get_current_volume();
    this->set_volume(current_volume - 5);
}

void ButtonHandler::handle_mode_button() {
     ESP_LOGI(TAG, "Mode button pressed");
    // Implementation of mode button handling
}

void ButtonHandler::handle_play_button() {
    ESP_LOGI(TAG, "Play button pressed");
    // Implementation of play button handling
}

void ButtonHandler::handle_set_button() {
    ESP_LOGI(TAG, "Set button pressed");
    // Implementation of set button handling
}

void ButtonHandler::handle_rec_button() {
    ESP_LOGI(TAG, "Record button pressed");
    // Implementation of record button handling
}

}  // namespace esp_adf
}  // namespace esphome
#endif  // USE_ESP_IDF
