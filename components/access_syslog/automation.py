import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

from . import AccessSyslog, access_syslog_ns

CONF_UID = "uid"
CONF_MESSAGE = "message"

SendOnlineAction = access_syslog_ns.class_("SendOnlineAction", automation.Action)
SendDoorOpenedAction = access_syslog_ns.class_(
    "SendDoorOpenedAction", automation.Action
)
SendUnlockedByAction = access_syslog_ns.class_(
    "SendUnlockedByAction", automation.Action
)
SendUnlockedByUidAction = access_syslog_ns.class_(
    "SendUnlockedByUidAction", automation.Action
)
SendMessageAction = access_syslog_ns.class_("SendMessageAction", automation.Action)


@automation.register_action(
    "access_syslog.send_online",
    SendOnlineAction,
    cv.Schema({cv.GenerateID(): cv.use_id(AccessSyslog)}),
    synchronous=True,
)
async def access_syslog_send_online(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "access_syslog.send_door_opened",
    SendDoorOpenedAction,
    cv.Schema({cv.GenerateID(): cv.use_id(AccessSyslog)}),
    synchronous=True,
)
async def access_syslog_send_door_opened(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "access_syslog.send_unlocked_by",
    SendUnlockedByAction,
    cv.Schema({cv.GenerateID(): cv.use_id(AccessSyslog)}),
    synchronous=True,
)
async def access_syslog_send_unlocked_by(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "access_syslog.send_unlocked_by_uid",
    SendUnlockedByUidAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AccessSyslog),
            cv.Required(CONF_UID): cv.templatable(cv.string),
        }
    ),
    synchronous=True,
)
async def access_syslog_send_unlocked_by_uid(config, action_id, template_arg, args):
    var = cg.new_Pvariable(
        action_id, template_arg, await cg.get_variable(config[CONF_ID])
    )
    template_ = await cg.templatable(config[CONF_UID], args, cg.std_string)
    cg.add(var.set_uid(template_))
    return var


@automation.register_action(
    "access_syslog.send_message",
    SendMessageAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AccessSyslog),
            cv.Required(CONF_MESSAGE): cv.templatable(cv.string),
        }
    ),
    synchronous=True,
)
async def access_syslog_send_message(config, action_id, template_arg, args):
    var = cg.new_Pvariable(
        action_id, template_arg, await cg.get_variable(config[CONF_ID])
    )
    template_ = await cg.templatable(config[CONF_MESSAGE], args, cg.std_string)
    cg.add(var.set_message(template_))
    return var
