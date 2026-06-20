import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button

from .. import CONF_PYLONTECH_ID, PylontechComponent, pylontech_ns

CONF_REQUEST_INFO = "request_info"
CONF_REQUEST_GETPWR = "request_getpwr"
CONF_REQUEST_STAT = "request_stat"
CONF_REQUEST_BAT = "request_bat"
CONF_REQUEST_SOH = "request_soh"
CONF_LOGIN_DEBUG = "login_debug"
CONF_INITIALIZE_US2000B = "initialize_us2000b"

PylontechInfoButton = pylontech_ns.class_(
    "PylontechInfoButton", button.Button, cg.Component
)
PylontechGetPowerButton = pylontech_ns.class_(
    "PylontechGetPowerButton", button.Button, cg.Component
)
PylontechStatButton = pylontech_ns.class_(
    "PylontechStatButton", button.Button, cg.Component
)
PylontechBatButton = pylontech_ns.class_(
    "PylontechBatButton", button.Button, cg.Component
)
PylontechSohButton = pylontech_ns.class_(
    "PylontechSohButton", button.Button, cg.Component
)
PylontechLoginDebugButton = pylontech_ns.class_(
    "PylontechLoginDebugButton", button.Button, cg.Component
)
PylontechUS2000BInitializationButton = pylontech_ns.class_(
    "PylontechUS2000BInitializationButton", button.Button, cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYLONTECH_ID): cv.use_id(PylontechComponent),
        cv.Optional(CONF_REQUEST_INFO): button.button_schema(
            PylontechInfoButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_REQUEST_GETPWR): button.button_schema(
            PylontechGetPowerButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_REQUEST_STAT): button.button_schema(
            PylontechStatButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_REQUEST_BAT): button.button_schema(
            PylontechBatButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_REQUEST_SOH): button.button_schema(
            PylontechSohButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_LOGIN_DEBUG): button.button_schema(
            PylontechLoginDebugButton
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_INITIALIZE_US2000B): button.button_schema(
            PylontechUS2000BInitializationButton
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])

    if CONF_REQUEST_INFO in config:
        btn = await button.new_button(config[CONF_REQUEST_INFO])
        await cg.register_component(btn, config[CONF_REQUEST_INFO])
        cg.add(btn.set_parent(parent))

    if CONF_REQUEST_GETPWR in config:
        btn = await button.new_button(config[CONF_REQUEST_GETPWR])
        await cg.register_component(btn, config[CONF_REQUEST_GETPWR])
        cg.add(btn.set_parent(parent))

    if CONF_REQUEST_STAT in config:
        btn = await button.new_button(config[CONF_REQUEST_STAT])
        await cg.register_component(btn, config[CONF_REQUEST_STAT])
        cg.add(btn.set_parent(parent))

    if CONF_REQUEST_BAT in config:
        btn = await button.new_button(config[CONF_REQUEST_BAT])
        await cg.register_component(btn, config[CONF_REQUEST_BAT])
        cg.add(btn.set_parent(parent))

    if CONF_REQUEST_SOH in config:
        btn = await button.new_button(config[CONF_REQUEST_SOH])
        await cg.register_component(btn, config[CONF_REQUEST_SOH])
        cg.add(btn.set_parent(parent))

    if CONF_LOGIN_DEBUG in config:
        btn = await button.new_button(config[CONF_LOGIN_DEBUG])
        await cg.register_component(btn, config[CONF_LOGIN_DEBUG])
        cg.add(btn.set_parent(parent))

    if CONF_INITIALIZE_US2000B in config:
        btn = await button.new_button(config[CONF_INITIALIZE_US2000B])
        await cg.register_component(btn, config[CONF_INITIALIZE_US2000B])
        cg.add(btn.set_parent(parent))
