import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@romkey"]
DEPENDENCIES = []

access_site_ns = cg.esphome_ns.namespace("access_site")
AccessSite = access_site_ns.class_("AccessSite", cg.Component)

CONF_NAME = "name"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AccessSite),
            cv.Required(CONF_NAME): cv.string,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_name(config[CONF_NAME]))
