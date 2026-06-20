import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@Dackara"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "text_sensor", "binary_sensor", "switch", "button"]
MULTI_CONF = True

CONF_BATTERY = "battery"
CONF_ROLE = "role"
CONF_PYLONTECH_ID = "pylontech_id"
CONF_INFO_INTERVAL = "info_interval"
CONF_ENABLE_INFO = "enable_info"
CONF_INFO_STARTUP_DELAY = "info_startup_delay"
CONF_ENABLE_GETPWR = "enable_getpwr"
CONF_ENABLE_STAT = "enable_stat"
CONF_STAT_INTERVAL = "stat_interval"
CONF_ENABLE_BAT = "enable_bat"
CONF_ENABLE_SOH = "enable_soh"
CONF_SLOW_INTERVAL = "slow_interval"
CONF_PUBLISH_ONLY_CHANGES = "publish_only_changes"
CONF_ENABLE_LOGIN_DEBUG_RECOVERY = "enable_login_debug_recovery"
CONF_LOGIN_DEBUG_FAILURE_THRESHOLD = "login_debug_failure_threshold"
CONF_LOGIN_DEBUG_RECOVERY_INTERVAL = "login_debug_recovery_interval"
CONF_ENABLE_US2000B_INITIALIZATION = "enable_us2000b_initialization"
CONF_MEMORY_MODE = "memory_mode"

pylontech_ns = cg.esphome_ns.namespace("pylontech")

PylontechComponent = pylontech_ns.class_(
    "PylontechComponent", cg.PollingComponent, uart.UARTDevice
)
PylontechBattery = pylontech_ns.class_("PylontechBattery", cg.Component)

PylontechRole = pylontech_ns.enum("PylontechRole")
ROLES = {
    "master": PylontechRole.PYLONTECH_ROLE_MASTER,
    "slave": PylontechRole.PYLONTECH_ROLE_SLAVE,
}

PylontechMemoryMode = pylontech_ns.enum("PylontechMemoryMode")
MEMORY_MODES = {
    "internal": PylontechMemoryMode.PYLONTECH_MEMORY_INTERNAL,
    "auto": PylontechMemoryMode.PYLONTECH_MEMORY_AUTO,
    "psram": PylontechMemoryMode.PYLONTECH_MEMORY_PSRAM,
}


def validate_memory_mode(value):
    value = cv.one_of(*MEMORY_MODES, lower=True)(value)
    if value == "psram":
        return cv.requires_component("psram")(value)
    return value

PYLONTECH_PARENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYLONTECH_ID): cv.use_id(PylontechComponent),
    }
)

PYLONTECH_COMPONENT_SCHEMA = PYLONTECH_PARENT_SCHEMA.extend(
    {
        cv.Optional(CONF_BATTERY): cv.int_range(min=1, max=16),
    }
)

_BATTERY_INSTANCES = {}


async def get_or_create_battery(config):
    key = (str(config[CONF_PYLONTECH_ID]), config[CONF_BATTERY])
    battery = _BATTERY_INSTANCES.get(key)
    if battery is not None:
        return battery

    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])
    battery = cg.new_Pvariable(config[CONF_ID], config[CONF_BATTERY])
    _BATTERY_INSTANCES[key] = battery
    await cg.register_component(battery, config)
    cg.add(parent.register_battery(battery))
    return battery

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechComponent),
            cv.Optional(CONF_ROLE, default="master"): cv.enum(ROLES, lower=True),
            cv.Optional(CONF_ENABLE_INFO, default=False): cv.boolean,
            cv.Optional(CONF_INFO_INTERVAL, default="24h"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_INFO_STARTUP_DELAY, default="0s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_ENABLE_GETPWR, default=False): cv.boolean,
            cv.Optional(CONF_ENABLE_STAT, default=False): cv.boolean,
            cv.Optional(CONF_ENABLE_BAT, default=False): cv.boolean,
            cv.Optional(CONF_ENABLE_SOH, default=False): cv.boolean,
            cv.Optional(CONF_PUBLISH_ONLY_CHANGES, default=False): cv.boolean,
            cv.Optional(CONF_ENABLE_LOGIN_DEBUG_RECOVERY, default=False): cv.boolean,
            cv.Optional(CONF_LOGIN_DEBUG_FAILURE_THRESHOLD, default=3): cv.int_range(min=1, max=20),
            cv.Optional(CONF_LOGIN_DEBUG_RECOVERY_INTERVAL, default="24h"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_ENABLE_US2000B_INITIALIZATION, default=False): cv.boolean,
            cv.Optional(CONF_MEMORY_MODE, default="internal"): validate_memory_mode,
            cv.Optional(CONF_SLOW_INTERVAL): cv.positive_time_period_milliseconds,
            # Deprecated: use slow_interval now that the slow loop hosts more than stat.
            cv.Optional(CONF_STAT_INTERVAL, default="300s"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], MEMORY_MODES[config[CONF_MEMORY_MODE]])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_role(config[CONF_ROLE]))
    cg.add(var.set_enable_info(config[CONF_ENABLE_INFO]))
    cg.add(var.set_info_interval(config[CONF_INFO_INTERVAL]))
    cg.add(var.set_info_startup_delay(config[CONF_INFO_STARTUP_DELAY]))
    cg.add(var.set_enable_getpwr(config[CONF_ENABLE_GETPWR]))
    cg.add(var.set_enable_stat(config[CONF_ENABLE_STAT]))
    cg.add(var.set_enable_bat(config[CONF_ENABLE_BAT]))
    cg.add(var.set_enable_soh(config[CONF_ENABLE_SOH]))
    cg.add(var.set_publish_only_changes(config[CONF_PUBLISH_ONLY_CHANGES]))
    cg.add(var.set_enable_login_debug_recovery(config[CONF_ENABLE_LOGIN_DEBUG_RECOVERY]))
    cg.add(var.set_login_debug_failure_threshold(config[CONF_LOGIN_DEBUG_FAILURE_THRESHOLD]))
    cg.add(var.set_login_debug_recovery_interval(config[CONF_LOGIN_DEBUG_RECOVERY_INTERVAL]))
    cg.add(var.set_enable_us2000b_initialization(config[CONF_ENABLE_US2000B_INITIALIZATION]))
    cg.add(var.set_slow_interval(config.get(CONF_SLOW_INTERVAL, config[CONF_STAT_INTERVAL])))
