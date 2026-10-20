import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32, web_server_base
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID
from esphome.const import CONF_ID

CODEOWNERS = ["@romkey"]
DEPENDENCIES = ["web_server", "web_server_base", "push_auth", "json"]
AUTO_LOAD = ["web_server_base"]

access_db_ns = cg.esphome_ns.namespace("access_db")
AccessDb = access_db_ns.class_("AccessDb", cg.Component)

CONF_AUTH_ID = "auth_id"
CONF_PATH = "path"
CONF_MAX_BODY_SIZE = "max_body_size"
CONF_PARTITION_LABEL = "partition_label"
CONF_MOUNT_PATH = "mount_path"

DEFAULT_PARTITION_SIZE = 0x80000  # 512 KB

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AccessDb),
            cv.GenerateID(CONF_AUTH_ID): cv.use_id("push_auth.PushAuth"),
            cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(
                web_server_base.WebServerBase
            ),
            cv.Optional(CONF_PATH, default="/access-db"): cv.string,
            cv.Optional(CONF_MAX_BODY_SIZE, default=131072): cv.int_range(
                min=1024, max=524288
            ),
            cv.Optional(CONF_PARTITION_LABEL, default="storage"): cv.string,
            cv.Optional(CONF_MOUNT_PATH, default="/storage"): cv.string,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    esp32.add_idf_component(name="joltwallet/littlefs", ref="1.20.3")
    esp32.add_partition(
        config[CONF_PARTITION_LABEL], "data", "littlefs", DEFAULT_PARTITION_SIZE
    )

    auth = await cg.get_variable(config[CONF_AUTH_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_auth(auth))
    cg.add(var.set_path(config[CONF_PATH]))
    cg.add(var.set_max_body_size(config[CONF_MAX_BODY_SIZE]))
    cg.add(var.set_partition_label(config[CONF_PARTITION_LABEL]))
    cg.add(var.set_mount_path(config[CONF_MOUNT_PATH]))
