import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import AccessDb

DEPENDENCIES = ["access_db"]

CONF_ACCESS_DB_ID = "access_db_id"
CONF_HEALTHY = "healthy"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(binary_sensor.BinarySensor),
            cv.GenerateID(CONF_ACCESS_DB_ID): cv.use_id(AccessDb),
            cv.Optional(CONF_HEALTHY): binary_sensor.binary_sensor_schema(),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ACCESS_DB_ID])
    if CONF_HEALTHY in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_HEALTHY])
        cg.add(parent.set_healthy_binary_sensor(sens))
