import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID, ICON_EMPTY

from . import CONF_KILOVAULT_BMS_BLE_ID, KilovaultBmsBle

DEPENDENCIES = ["kilovault_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_BATTERY_MAC = "battery_mac"

TEXT_SENSORS = [
    CONF_BATTERY_MAC,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_KILOVAULT_BMS_BLE_ID): cv.use_id(KilovaultBmsBle),
        cv.Optional(CONF_BATTERY_MAC): text_sensor.TEXT_SENSOR_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                cv.Optional(CONF_ICON, default=ICON_EMPTY): cv.icon,
            }
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_KILOVAULT_BMS_BLE_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
