import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@romkey"]
DEPENDENCIES = []

push_auth_ns = cg.esphome_ns.namespace("push_auth")
PushAuth = push_auth_ns.class_("PushAuth", cg.Component)

CONF_TOKEN = "token"
CONF_HEADER = "header"
CONF_ALT_HEADER = "alt_header"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PushAuth),
            cv.Required(CONF_TOKEN): cv.string,
            cv.Optional(CONF_HEADER, default="Authorization"): cv.string,
            cv.Optional(CONF_ALT_HEADER): cv.string,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_token(config[CONF_TOKEN]))
    cg.add(var.set_header(config[CONF_HEADER]))
    if CONF_ALT_HEADER in config:
        cg.add(var.set_alt_header(config[CONF_ALT_HEADER]))
