import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT

CODEOWNERS = ["@romkey"]
DEPENDENCIES = [
    "network",
    "web_server",
    "web_server_base",
    "push_auth",
    "access_db",
    "json",
]
AUTO_LOAD = ["web_server_base"]

access_syslog_ns = cg.esphome_ns.namespace("access_syslog")
AccessSyslog = access_syslog_ns.class_("AccessSyslog", cg.Component)

CONF_SERVER = "server"
CONF_SITE_ID = "site_id"
CONF_ACCESS_DB_ID = "access_db_id"
CONF_AUTH_ID = "auth_id"
CONF_UNLOCK_PATH = "unlock_path"
CONF_HOSTNAME = "hostname"
CONF_FACILITY = "facility"


def validate_server(value):
    """An IPv4 address or a hostname; IPv6 literals are not supported."""
    value = cv.string_strict(value).strip()
    if not value:
        raise cv.Invalid("server must not be empty")
    if ":" in value or "/" in value or " " in value or "[" in value:
        raise cv.Invalid(
            "server must be an IPv4 address or a hostname (IPv6 addresses, "
            "ports and URLs are not supported; set the port with `port:`)"
        )
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AccessSyslog),
            cv.Required(CONF_SERVER): validate_server,
            cv.Optional(CONF_PORT, default=514): cv.port,
            cv.Required(CONF_ACCESS_DB_ID): cv.use_id("access_db.AccessDb"),
            cv.Required(CONF_AUTH_ID): cv.use_id("push_auth.PushAuth"),
            cv.Optional(CONF_SITE_ID): cv.use_id("access_site.AccessSite"),
            cv.Optional(CONF_UNLOCK_PATH, default="/access-unlock"): cv.string,
            cv.Optional(CONF_HOSTNAME, default="access-control"): cv.string,
            cv.Optional(CONF_FACILITY, default=16): cv.int_range(min=0, max=23),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on(["esp32", "esp32_s2", "esp32_s3", "esp32_c3", "esp32_c6"]),
)


async def to_code(config):
    access_db = await cg.get_variable(config[CONF_ACCESS_DB_ID])
    auth = await cg.get_variable(config[CONF_AUTH_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_server(config[CONF_SERVER]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_hostname(config[CONF_HOSTNAME]))
    cg.add(var.set_facility(config[CONF_FACILITY]))
    cg.add(var.set_access_db(access_db))
    cg.add(var.set_auth(auth))
    cg.add(var.set_unlock_path(config[CONF_UNLOCK_PATH]))
    if CONF_SITE_ID in config:
        site = await cg.get_variable(config[CONF_SITE_ID])
        cg.add(var.set_site(site))


from . import automation  # noqa: F401
