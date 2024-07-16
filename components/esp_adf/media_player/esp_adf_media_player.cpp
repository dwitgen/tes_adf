#include "esp_adf_media_player.h"

#ifdef USE_ESP_IDF

#include "esp_log.h"
#include "http_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

namespace esphome {
namespace esp_adf {

static const char *TAG = "ESPADFMediaPlayer";

void ESPADFMediaPlayer::register_component() {
  App.register_component(this);
  App.register_media_player(this);
}

void ESPADFMediaPlayer::setup() {
  // Initialize ESP-ADF components
  audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
  pipeline_ = audio_pipeline_init(&pipeline_cfg);

  configure_http_stream();
  configure_mp3_decoder();
  configure_i2s_stream_writer();
  configure_resample_filter();

  // Register elements to pipeline
  audio_pipeline_register(pipeline_, http_stream_reader_, "http");
  audio_pipeline_register(pipeline_, mp3_decoder_, "mp3");
  audio_pipeline_register(pipeline_, resample_filter_, "filter");
  audio_pipeline_register(pipeline_, i2s_stream_writer_, "i2s");

  // Link elements together
  const char *link_tag[4] = {"http", "mp3", "filter", "i2s"};
  audio_pipeline_link(pipeline_, &link_tag[0], 4);

  // Set URI for HTTP stream
  if (current_url_.has_value()) {
    audio_element_set_uri(http_stream_reader_, current_url_.value().c_str());
  }

  // Register the component with the App
  this->register_component();
}

void ESPADFMediaPlayer::loop() {
  // Handle ADF events and states
}

void ESPADFMediaPlayer::dump_config() {
  ESP_LOGI(TAG, "I2S Audio Media Player:");
}

void ESPADFMediaPlayer::set_dout_pin(uint8_t pin) {
  this->dout_pin_ = pin;
}

void ESPADFMediaPlayer::set_external_dac_channels(uint8_t channels) {
  this->external_dac_channels_ = channels;
}

media_player::MediaPlayerTraits ESPADFMediaPlayer::get_traits() {
  return media_player::MediaPlayerTraits();
}

void ESPADFMediaPlayer::start_() {
  audio_pipeline_run(pipeline_);
}

void ESPADFMediaPlayer::stop_() {
  audio_pipeline_stop(pipeline_);
  audio_pipeline_wait_for_stop(pipeline_);
}

void ESPADFMediaPlayer::play_() {
  audio_pipeline_pause(pipeline_);
  audio_pipeline_run(pipeline_);
}

//void ESPADFMediaPlayer::set_volume_(float volume, bool publish) {
  // Implement volume control based on your specific requirements
  // This function might not be necessary for ESP-ADF
  //if (publish) {
  //  this->publish_state();
  //}
//}

void ESPADFMediaPlayer::control(const media_player::MediaPlayerCall &call) {
  if (call.get_state().has_value()) {
    switch (call.get_state().value()) {
      case media_player::MEDIA_PLAYER_STATE_PLAYING:
        this->play_();
        break;
      case media_player::MEDIA_PLAYER_STATE_PAUSED:
      case media_player::MEDIA_PLAYER_STATE_STOPPED:
        this->stop_();
        break;
      default:
        break;
    }
  }
}

esp_err_t ESPADFMediaPlayer::configure_i2s_stream_writer() {
  i2s_driver_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM,
      .dma_buf_count = 8,
      .dma_buf_len = 1024,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0,
      .mclk_multiple = I2S_MCLK_MULTIPLE_256,
      .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT,
  };

  i2s_stream_cfg_t i2s_cfg = {
      .type = AUDIO_STREAM_WRITER,
      .i2s_config = i2s_config,
      .i2s_port = I2S_NUM_0,
      .use_alc = false,
      .volume = 0,
      .out_rb_size = I2S_STREAM_RINGBUFFER_SIZE,
      .task_stack = I2S_STREAM_TASK_STACK,
      .task_core = I2S_STREAM_TASK_CORE,
      .task_prio = I2S_STREAM_TASK_PRIO,
      .stack_in_ext = false,
      .multi_out_num = 0,
      .uninstall_drv = true,
      .need_expand = false,
      .expand_src_bits = I2S_BITS_PER_SAMPLE_16BIT,
  };

  i2s_stream_writer_ = i2s_stream_init(&i2s_cfg);
  if (i2s_stream_writer_ == NULL) {
    ESP_LOGE(TAG, "Failed to initialize I2S stream writer for HTTP");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "I2S stream writer for HTTP initialized");
  return ESP_OK;
}

esp_err_t ESPADFMediaPlayer::configure_http_stream() {
  http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
  http_stream_reader_ = http_stream_init(&http_cfg);
  if (http_stream_reader_ == NULL) {
    ESP_LOGE(TAG, "Failed to initialize HTTP stream reader");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "HTTP stream reader initialized");
  return ESP_OK;
}

esp_err_t ESPADFMediaPlayer::configure_mp3_decoder() {
  mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
  mp3_decoder_ = mp3_decoder_init(&mp3_cfg);
  if (mp3_decoder_ == NULL) {
    ESP_LOGE(TAG, "Failed to initialize MP3 decoder");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "MP3 decoder initialized");
  return ESP_OK;
}

esp_err_t ESPADFMediaPlayer::configure_resample_filter() {
  rsp_filter_cfg_t rsp_cfg = {
      .src_rate = 44100,
      .src_ch = 2,
      .dest_rate = 44100,
      .dest_bits = 16,
      .dest_ch = 1,
      .src_bits = 16,
      .mode = RESAMPLE_DECODE_MODE,
      .max_indata_bytes = RSP_FILTER_BUFFER_BYTE,
      .out_len_bytes = RSP_FILTER_BUFFER_BYTE,
      .type = ESP_RESAMPLE_TYPE_AUTO,
      .complexity = 2,
      .down_ch_idx = 0,
      .prefer_flag = ESP_RSP_PREFER_TYPE_SPEED,
      .out_rb_size = RSP_FILTER_RINGBUFFER_SIZE,
      .task_stack = RSP_FILTER_TASK_STACK,
      .task_core = RSP_FILTER_TASK_CORE,
      .task_prio = RSP_FILTER_TASK_PRIO,
      .stack_in_ext = true,
  };

  resample_filter_ = rsp_filter_init(&rsp_cfg);
  if (resample_filter_ == NULL) {
    ESP_LOGE(TAG, "Failed to initialize resample filter");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Resample filter initialized");
  return ESP_OK;
}

}  // namespace esp_adf
}  // namespace esphome

#endif  // USE_ESP_IDF
