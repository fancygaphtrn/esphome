#include "kilovault_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace kilovault_bms_ble {

static const char *const TAG = "kilovault_bms_ble";

//static const uint16_t KILOVAULT_BMS_SERVICE_UUID = 0xFA00;
static const uint16_t KILOVAULT_BMS_SERVICE_UUID = 0xFFE0;
static const uint16_t KILOVAULT_BMS_NOTIFY_CHARACTERISTIC_UUID = 0xFFE4;   // handle 0x12

static const uint16_t KILOVAULT_BMS_CONTROL_CHARACTERISTIC_UUID = 0xFA02;  // handle 0x15

static const uint16_t MAX_RESPONSE_SIZE = 121;

static const uint8_t KILOVAULT_PKT_START_A = 0xB0;
static const uint8_t KILOVAULT_PKT_START_B = 0xB0;
static const uint8_t KILOVAULT_ADDRESS = 0x16;
static const uint8_t KILOVAULT_PKT_END_1 = 0x52;
static const uint8_t KILOVAULT_PKT_END_2 = 0x52;

void KilovaultBmsBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                      esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;

      // this->publish_state_(this->voltage_sensor_, NAN);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {

      auto *char_notify =
          this->parent_->get_characteristic(KILOVAULT_BMS_SERVICE_UUID, KILOVAULT_BMS_NOTIFY_CHARACTERISTIC_UUID);
      if (char_notify == nullptr) {
        ESP_LOGE(TAG, "[%s] No notify service found at device, not an KILOVAULT BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_notify_handle_ = char_notify->handle;

      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                                      char_notify->handle);
      if (status) {
        ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }

      auto *char_command =
          this->parent_->get_characteristic(KILOVAULT_BMS_SERVICE_UUID, KILOVAULT_BMS_CONTROL_CHARACTERISTIC_UUID);
      if (char_command == nullptr) {
        ESP_LOGE(TAG, "[%s] No control service found at device, not an KILOVAULT BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_command_handle_ = char_command->handle;
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;

      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGVV(TAG, "Notification received (handle 0x%02X): %s", param->notify.handle,
                format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

      this->assemble_(param->notify.value, param->notify.value_len);
      break;
    }
    default:
      break;
  }
}

void KilovaultBmsBle::assemble_(const uint8_t *data, uint16_t length) {

  auto asciiToInt = [] ( const uint8_t c) {
	uint8_t v = c;
	if ((c >= 48) && (c <= 57))
		v = (c - 48);
	else if ((c >= 65) && (c <= 70))
		v = (c - 65 + 10);
	else if ((c >= 97) && (c <= 102))
		v = (c - 97 + 10);   
	return v;
  };

  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Maximum response size exceeded");
    this->frame_buffer_.clear();
  }

  // Flush buffer on every preamble
  if (data[0] == KILOVAULT_PKT_START_A || data[0] == KILOVAULT_PKT_START_B) {
    this->frame_buffer_.clear();
  }

  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);
  //ESP_LOGW(TAG, "frame_buffer: %s", format_hex_pretty(this->frame_buffer_).c_str());

  if (this->frame_buffer_.size() == MAX_RESPONSE_SIZE) {

    for(int i=0; i < this->frame_buffer_.size(); i++){
      this->frame_buffer_[i] = asciiToInt(this->frame_buffer_[i]);
    }

    this->on_kilovault_bms_ble_data_(this->frame_buffer_);
    this->frame_buffer_.clear();
  }
}

void KilovaultBmsBle::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED ) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

}

void KilovaultBmsBle::on_kilovault_bms_ble_data_(const std::vector<uint8_t> &data) {

  this->decode_status_data_(data);

}

void KilovaultBmsBle::decode_status_data_(const std::vector<uint8_t> &data) {
  auto kilovault_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 2]) << 12) | (uint16_t(data[i + 3]) << 8) | (uint16_t(data[i + 0]) << 4) | (uint16_t(data[i + 1]) << 0);
  };
  auto kilovault_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(kilovault_get_16bit(i + 4)) << 16) | (uint32_t(kilovault_get_16bit(i + 0)) << 0);
  };

  //ESP_LOGI(TAG, "Status frame (%d+4 bytes):", data.size());
  //ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  int32_t current = kilovault_get_32bit(9);
  if (current > 2147483647) {
     current = current - 4294967295;
  } 
  
  this->publish_state_(this->current_sensor_, (float) current * 0.001f);

  //  8    4  0xCE 0x61 0x00 0x00  Total voltage                    V     0.001f
  float voltage = kilovault_get_16bit(1) * 0.001f;
  this->publish_state_(this->voltage_sensor_, voltage);

  float power = voltage * ((float) current * 0.001f);
  this->publish_state_(this->power_sensor_, power);
  
  this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
  this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

  this->publish_state_(this->total_capacity_sensor_, kilovault_get_32bit(17) * 0.001f);

  this->publish_state_(this->current_capacity_sensor_, (kilovault_get_32bit(17) * 0.001f) * (kilovault_get_16bit(29) * 0.01f));
  
  this->publish_state_(this->cycles_sensor_, kilovault_get_16bit(25));

  this->publish_state_(this->state_of_charge_sensor_, kilovault_get_16bit(29));
  
  // temp is in Kelvin covert to Celius
  int16_t temp = kilovault_get_16bit(33);
  this->publish_state_(this->temperature_sensor_, (temp * 0.1f) - 273.15);

  this->publish_state_(this->status_sensor_, kilovault_get_16bit(37));

  this->publish_state_(this->afestatus_sensor_, kilovault_get_16bit(41));

  this->decode_cell_voltages_data_(data);

  this->publish_state_(this->battery_mac_text_sensor_, this->parent_->address_str().c_str());

}

void KilovaultBmsBle::decode_cell_voltages_data_(const std::vector<uint8_t> &data) {
  auto kilovault_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 2]) << 12) | (uint16_t(data[i + 3]) << 8) | (uint16_t(data[i + 0]) << 4) | (uint16_t(data[i + 1]) << 0);
  };
  uint8_t offset = 1;
  uint8_t cells = 4;
  uint8_t start = 41;

  //ESP_LOGI(TAG, "Cell voltages frame (chunk %d, %d+4 bytes):", data[2] - 36, data.size());
  //ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  this->min_cell_voltage_ = 100.0f;
  this->max_cell_voltage_ = -100.0f;
  
  for (uint8_t i = 1; i <= cells; i++) {
    float cell_voltage = kilovault_get_16bit(start + (i * 4)) * 0.001f;
    if (cell_voltage > 0 && cell_voltage < this->min_cell_voltage_) {
      this->min_cell_voltage_ = cell_voltage;
      this->min_voltage_cell_ = i ;
    }
    if (cell_voltage > this->max_cell_voltage_) {
      this->max_cell_voltage_ = cell_voltage;
      this->max_voltage_cell_ = i ;
    }
    this->publish_state_(this->cells_[i-1].cell_voltage_sensor_, cell_voltage);
  }

  this->publish_state_(this->min_cell_voltage_sensor_, this->min_cell_voltage_);
  this->publish_state_(this->max_cell_voltage_sensor_, this->max_cell_voltage_);
  this->publish_state_(this->max_voltage_cell_sensor_, (float) this->max_voltage_cell_);
  this->publish_state_(this->min_voltage_cell_sensor_, (float) this->min_voltage_cell_);
  this->publish_state_(this->delta_cell_voltage_sensor_, this->max_cell_voltage_ - this->min_cell_voltage_);
}

void KilovaultBmsBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "KilovaultBmsBle:");


  LOG_SENSOR("", "Voltage", voltage_sensor_);
  LOG_SENSOR("", "Current", current_sensor_);
  LOG_SENSOR("", "Power", power_sensor_);
  LOG_SENSOR("", "Charging power", charging_power_sensor_);
  LOG_SENSOR("", "Discharging power", discharging_power_sensor_);
  LOG_SENSOR("", "State of charge", state_of_charge_sensor_);
  LOG_SENSOR("", "Cycles", cycles_sensor_);
  LOG_SENSOR("", "Total capacity", total_capacity_sensor_);
  LOG_SENSOR("", "Current capacity", current_capacity_sensor_);
  LOG_SENSOR("", "Status", status_sensor_);
  LOG_SENSOR("", "Status", afestatus_sensor_);
  LOG_SENSOR("", "Min cell voltage", min_cell_voltage_sensor_);
  LOG_SENSOR("", "Max cell voltage", max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min voltage cell", min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max voltage cell", max_voltage_cell_sensor_);
  LOG_SENSOR("", "Delta cell voltage", delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Temperature", temperature_sensor_);
  LOG_SENSOR("", "Cell Voltage 1", this->cells_[0].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 2", this->cells_[1].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 3", this->cells_[2].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 4", this->cells_[3].cell_voltage_sensor_);

  LOG_TEXT_SENSOR("", "Battery MAC", this->battery_mac_text_sensor_);
}

void KilovaultBmsBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void KilovaultBmsBle::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void KilovaultBmsBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void KilovaultBmsBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void KilovaultBmsBle::write_register(uint8_t address, uint16_t value) {
  // this->send_command_(KILOVAULT_CMD_WRITE, KILOVAULT_CMD_MOS);  // @TODO: Pass value
}

bool KilovaultBmsBle::send_command_(uint8_t start_of_frame, uint8_t function, uint8_t value) {
  uint8_t frame[9];
  uint8_t data_len = 1;

  frame[0] = start_of_frame;
  frame[1] = KILOVAULT_ADDRESS;
  frame[2] = function;
  frame[3] = data_len;
  frame[4] = value;
  auto crc = chksum_(frame + 1, 4);
  frame[5] = crc >> 0;
  frame[6] = crc >> 8;
  frame[7] = KILOVAULT_PKT_END_1;
  frame[8] = KILOVAULT_PKT_END_2;

  ESP_LOGV(TAG, "Send command (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(frame, sizeof(frame)).c_str());

  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }

  return (status == 0);
}



}  // namespace kilovault_bms_ble
}  // namespace esphome
