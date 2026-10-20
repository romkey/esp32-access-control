import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from . import AccessDb

DEPENDENCIES = ["access_db"]

CONF_ACCESS_DB_ID = "access_db_id"
CONF_VERSION = "version"
CONF_ENTRY_COUNT = "entry_count"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(sensor.Sensor),
            cv.GenerateID(CONF_ACCESS_DB_ID): cv.use_id(AccessDb),
            cv.Optional(CONF_VERSION): sensor.sensor_schema(
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_ENTRY_COUNT): sensor.sensor_schema(
                accuracy_decimals=0,
            ),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ACCESS_DB_ID])
    if CONF_VERSION in config:
        sens = await sensor.new_sensor(config[CONF_VERSION])
        cg.add(parent.set_version_sensor(sens))
    if CONF_ENTRY_COUNT in config:
        sens = await sensor.new_sensor(config[CONF_ENTRY_COUNT])
        cg.add(parent.set_entry_count_sensor(sens))
