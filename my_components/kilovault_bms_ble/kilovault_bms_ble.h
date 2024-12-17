#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace kilovault_bms_ble {

namespace espbt = esphome::esp32_ble_tracker;

class KilovaultBmsBle : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }


  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charging_power_sensor(sensor::Sensor *charging_power_sensor) {
    charging_power_sensor_ = charging_power_sensor;
  }
  void set_discharging_power_sensor(sensor::Sensor *discharging_power_sensor) {
    discharging_power_sensor_ = discharging_power_sensor;
  }
  void set_cycles_sensor(sensor::Sensor *cycles_sensor) {
    cycles_sensor_ = cycles_sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor *state_of_charge_sensor) {
    state_of_charge_sensor_ = state_of_charge_sensor;
  }
  void set_total_capacity_sensor(sensor::Sensor *total_capacity_sensor) {
    total_capacity_sensor_ = total_capacity_sensor;
  }
  void set_current_capacity_sensor(sensor::Sensor *current_capacity_sensor) {
    current_capacity_sensor_ = current_capacity_sensor;
  }
  void set_status_sensor(sensor::Sensor *status_sensor) { status_sensor_ = status_sensor; }
  void set_afestatus_sensor(sensor::Sensor *afestatus_sensor) { afestatus_sensor_ = afestatus_sensor; }

  void set_min_cell_voltage_sensor(sensor::Sensor *min_cell_voltage_sensor) {
    min_cell_voltage_sensor_ = min_cell_voltage_sensor;
  }
  void set_max_cell_voltage_sensor(sensor::Sensor *max_cell_voltage_sensor) {
    max_cell_voltage_sensor_ = max_cell_voltage_sensor;
  }
  void set_min_voltage_cell_sensor(sensor::Sensor *min_voltage_cell_sensor) {
    min_voltage_cell_sensor_ = min_voltage_cell_sensor;
  }
  void set_max_voltage_cell_sensor(sensor::Sensor *max_voltage_cell_sensor) {
    max_voltage_cell_sensor_ = max_voltage_cell_sensor;
  }
  void set_delta_cell_voltage_sensor(sensor::Sensor *delta_cell_voltage_sensor) {
    delta_cell_voltage_sensor_ = delta_cell_voltage_sensor;
  }
  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *cell_voltage_sensor) {
    this->cells_[cell].cell_voltage_sensor_ = cell_voltage_sensor;
  }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) {
    temperature_sensor_ = temperature_sensor;
  }

  void set_battery_mac_text_sensor(text_sensor::TextSensor *battery_mac_text_sensor) {
    battery_mac_text_sensor_ = battery_mac_text_sensor;
  }

  void set_message_text_sensor(text_sensor::TextSensor *message_text_sensor) {
    message_text_sensor_ = message_text_sensor;
  }

  void write_register(uint8_t address, uint16_t value);

 protected:

  sensor::Sensor *voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *charging_power_sensor_;
  sensor::Sensor *discharging_power_sensor_;
  sensor::Sensor *cycles_sensor_;
  sensor::Sensor *state_of_charge_sensor_;
  sensor::Sensor *total_capacity_sensor_;
  sensor::Sensor *current_capacity_sensor_;
  sensor::Sensor *status_sensor_;
  sensor::Sensor *afestatus_sensor_;
  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;
  sensor::Sensor *delta_cell_voltage_sensor_;
  sensor::Sensor *temperature_sensor_;

  text_sensor::TextSensor *battery_mac_text_sensor_;
  text_sensor::TextSensor *message_text_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[4];

  std::vector<uint8_t> frame_buffer_;
  uint16_t char_notify_handle_;
  uint16_t char_command_handle_;
  uint8_t next_command_{5};

  float min_cell_voltage_{100.0f};
  float max_cell_voltage_{-100.0f};
  uint8_t max_voltage_cell_{0};
  uint8_t min_voltage_cell_{0};

  void assemble_(const uint8_t *data, uint16_t length);
  void on_kilovault_bms_ble_data_(const std::vector<uint8_t> &data);
  void decode_status_data_(const std::vector<uint8_t> &data);
  void decode_general_info_data_(const std::vector<uint8_t> &data);
  void decode_cell_voltages_data_(const std::vector<uint8_t> &data);
  void decode_protect_ic_data_(const std::vector<uint8_t> &data);
  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void publish_state_(switch_::Switch *obj, const bool &state);
  bool send_command_(uint8_t start_of_frame, uint8_t function, uint8_t value = 0x00);

  uint16_t chksum_(const uint8_t data[], const uint16_t len) {
    uint16_t checksum = 0x00;
    for (uint16_t i = 0; i < len; i++) {
      checksum = checksum + data[i];
    }
    return checksum;
  }
};

}  // namespace kilovault_bms_ble
}  // namespace esphome

#endif
