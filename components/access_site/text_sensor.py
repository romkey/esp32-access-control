import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import AccessSite

DEPENDENCIES = ["access_site"]

CONF_ACCESS_SITE_ID = "access_site_id"
CONF_CONTROLLER_NAME = "controller_name"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
            cv.GenerateID(CONF_ACCESS_SITE_ID): cv.use_id(AccessSite),
            cv.Optional(CONF_CONTROLLER_NAME): text_sensor.text_sensor_schema(),
        }
    ),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ACCESS_SITE_ID])
    if CONF_CONTROLLER_NAME in config:
        sens = await text_sensor.new_text_sensor(config[CONF_CONTROLLER_NAME])
        cg.add(parent.set_name_sensor(sens))
