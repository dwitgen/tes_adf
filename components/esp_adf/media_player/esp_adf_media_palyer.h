#pragma once

#ifdef USE_ESP_IDF

#include "../esp_adf.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "esphome/components/media_player/media_player.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"

#include <audio_element.h>
#include <audio_pipeline.h>
#include <audio_hal.h>
#include "esp_peripherals.h"
#include "periph_adc_button.h"
#include "input_key_service.h"
#include <board.h>
#include <esp_event.h>  

namespace esphome {
namespace esp_adf {

class ESPADFMediaPlayer : public Component, public media_player::MediaPlayer, public I2SAudioOut {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_dout_pin(uint8_t pin);
  void set_mute_pin(GPIOPin *mute_pin);
  void set_external_dac_channels(uint8_t channels);
  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override;

 protected:
  void control(const media_player::MediaPlayerCall &call) override;
  void start_();
  void stop_();
  void play_();
  void set_volume_(float volume, bool publish = true);
  void mute_();
  void unmute_();

 private:
  esp_err_t configure_i2s_stream_writer();
  esp_err_t configure_http_stream();
  esp_err_t configure_mp3_decoder();
  esp_err_t configure_resample_filter();

  audio_pipeline_handle_t pipeline_;
  audio_element_handle_t http_stream_reader_;
  audio_element_handle_t mp3_decoder_;
  audio_element_handle_t resample_filter_;
  audio_element_handle_t i2s_stream_writer_;

  uint8_t dout_pin_{0};
  GPIOPin *mute_pin_{nullptr};
  bool muted_{false};
  float unmuted_volume_{0};
  uint8_t external_dac_channels_{0};
  optional<std::string> current_url_{};
};

}  // namespace esp_adf
}  // namespace esphome

#endif  // USE_ESP_IDF
