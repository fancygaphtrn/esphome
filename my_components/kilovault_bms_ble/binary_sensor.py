import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID

from . import CONF_KILOVAULT_BMS_BLE_ID, KilovaultBmsBle

DEPENDENCIES = ["kilovault_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"

BINARY_SENSORS = [
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_KILOVAULT_BMS_BLE_ID): cv.use_id(KilovaultBmsBle),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_KILOVAULT_BMS_BLE_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
