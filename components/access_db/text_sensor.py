import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import AccessDb

DEPENDENCIES = ["access_db"]

CONF_ACCESS_DB_ID = "access_db_id"
CONF_STATUS = "status"
CONF_LAST_SYNC = "last_sync"
CONF_LAST_ERROR = "last_error"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
            cv.GenerateID(CONF_ACCESS_DB_ID): cv.use_id(AccessDb),
            cv.Optional(CONF_STATUS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_LAST_SYNC): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_LAST_ERROR): text_sensor.text_sensor_schema(),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ACCESS_DB_ID])
    if CONF_STATUS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_STATUS])
        cg.add(parent.set_status_text_sensor(sens))
    if CONF_LAST_SYNC in config:
        sens = await text_sensor.new_text_sensor(config[CONF_LAST_SYNC])
        cg.add(parent.set_last_sync_text_sensor(sens))
    if CONF_LAST_ERROR in config:
        sens = await text_sensor.new_text_sensor(config[CONF_LAST_ERROR])
        cg.add(parent.set_last_error_text_sensor(sens))
