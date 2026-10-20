import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import PushAuth

DEPENDENCIES = ["push_auth"]

CONF_PUSH_AUTH_ID = "push_auth_id"
CONF_LAST_RESULT = "last_result"
CONF_LAST_ATTEMPT = "last_attempt"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
            cv.GenerateID(CONF_PUSH_AUTH_ID): cv.use_id(PushAuth),
            cv.Optional(CONF_LAST_RESULT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_LAST_ATTEMPT): text_sensor.text_sensor_schema(),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PUSH_AUTH_ID])
    if CONF_LAST_RESULT in config:
        sens = await text_sensor.new_text_sensor(config[CONF_LAST_RESULT])
        cg.add(parent.set_last_result_sensor(sens))
    if CONF_LAST_ATTEMPT in config:
        sens = await text_sensor.new_text_sensor(config[CONF_LAST_ATTEMPT])
        cg.add(parent.set_last_attempt_sensor(sens))
