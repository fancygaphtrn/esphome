#include "kilovault_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace kilovault_bms_ble {

static const char *const TAG = "kilovault_bms_ble.switch";

void KilovaultSwitch::dump_config() { LOG_SWITCH("", "KilovaultBmsBle Switch", this); }
void KilovaultSwitch::write_state(bool state) {
  // this->parent_->write_register(this->holding_register_, (uint16_t) state);
}

}  // namespace kilovault_bms_ble
}  // namespace esphome
