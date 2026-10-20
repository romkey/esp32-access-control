import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import STATE_CLASS_TOTAL_INCREASING

from . import PushAuth

DEPENDENCIES = ["push_auth"]

CONF_PUSH_AUTH_ID = "push_auth_id"
CONF_FAILURE_COUNT = "failure_count"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(sensor.Sensor),
            cv.GenerateID(CONF_PUSH_AUTH_ID): cv.use_id(PushAuth),
            cv.Optional(CONF_FAILURE_COUNT): sensor.sensor_schema(
                unit_of_measurement="failures",
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
            ),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PUSH_AUTH_ID])
    if CONF_FAILURE_COUNT in config:
        sens = await sensor.new_sensor(config[CONF_FAILURE_COUNT])
        cg.add(parent.set_failure_count_sensor(sens))
