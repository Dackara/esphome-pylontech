import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC

from .. import (
    CONF_BATTERY,
    CONF_PYLONTECH_ID,
    PYLONTECH_COMPONENT_SCHEMA,
    PylontechBattery,
    get_or_create_battery,
)

CONF_BASE_STATE = "base_state"
CONF_VOLTAGE_STATE = "voltage_state"
CONF_CURRENT_STATE = "current_state"
CONF_TEMPERATURE_STATE = "temperature_state"
CONF_COULOMB_STATE = "coulomb_state"
CONF_SOH_STATE = "soh_state"
CONF_HEATER_STATE = "heater_state"
CONF_PROTECT_ENA = "protect_ena"
CONF_BAT_EVENTS = "bat_events"
CONF_POWER_EVENTS = "power_events"
CONF_SYSTEM_FAULT = "system_fault"
CONF_SYSTEM_STATUS = "system_status"
CONF_SOH_RESPONSE = "soh_response"
CONF_MANUFACTURER = "manufacturer"
CONF_DEVICE_NAME = "device_name"
CONF_BOARD = "board"
CONF_BOARD_VERSION = "board_version"
CONF_MAIN_SOFT_VERSION = "main_soft_version"
CONF_SOFT_VERSION = "soft_version"
CONF_BOOT_VERSION = "boot_version"
CONF_COMM_VERSION = "comm_version"
CONF_RELEASE_DATE = "release_date"
CONF_BARCODE = "barcode"
CONF_SPECIFICATION = "specification"
CONF_EPON_PORT_RATE = "epon_port_rate"
CONF_CONSOLE_PORT_RATE = "console_port_rate"
CONF_INFO_RESPONSE = "info_response"
CONF_GETPWR_RESPONSE = "getpwr_response"
CONF_STAT_RESPONSE = "stat_response"
CONF_BAT_RESPONSE = "bat_response"
CONF_BATTERY_VOLTAGE_STATE = "battery_voltage_state"
CONF_BATTERY_TEMPERATURE_STATE = "battery_temperature_state"
CONF_MOS_STATE = "mos_state"
CONF_CELL_VOLTAGE_STATES = "cell_voltage_states"
CONF_CELL_TEMPERATURE_STATES = "cell_temperature_states"
CONF_BAT_CELL_BALANCE_STATES = "bat_cell_balance_states"
CONF_SOH_CELL_STATUSES = "soh_cell_statuses"
CONF_LAST_COMMAND = "last_command"
CONF_LAST_ERROR = "last_error"
CONF_CONFIGURED_ROLE = "configured_role"
CONF_OBSERVED_ROLE = "observed_role"
CONF_ROLE_VALIDATION = "role_validation"

TEXT_SENSORS = {
    CONF_BASE_STATE: "set_base_state_text_sensor",
    CONF_VOLTAGE_STATE: "set_voltage_state_text_sensor",
    CONF_CURRENT_STATE: "set_current_state_text_sensor",
    CONF_TEMPERATURE_STATE: "set_temperature_state_text_sensor",
    CONF_COULOMB_STATE: "set_coulomb_state_text_sensor",
    CONF_SOH_STATE: "set_soh_state_text_sensor",
    CONF_HEATER_STATE: "set_heater_state_text_sensor",
    CONF_PROTECT_ENA: "set_protect_ena_text_sensor",
    CONF_BAT_EVENTS: "set_bat_events_text_sensor",
    CONF_POWER_EVENTS: "set_power_events_text_sensor",
    CONF_SYSTEM_FAULT: "set_system_fault_text_sensor",
    CONF_MANUFACTURER: "set_manufacturer_text_sensor",
    CONF_DEVICE_NAME: "set_device_name_text_sensor",
    CONF_BOARD: "set_board_text_sensor",
    CONF_BOARD_VERSION: "set_board_version_text_sensor",
    CONF_MAIN_SOFT_VERSION: "set_main_soft_version_text_sensor",
    CONF_SOFT_VERSION: "set_soft_version_text_sensor",
    CONF_BOOT_VERSION: "set_boot_version_text_sensor",
    CONF_COMM_VERSION: "set_comm_version_text_sensor",
    CONF_RELEASE_DATE: "set_release_date_text_sensor",
    CONF_BARCODE: "set_barcode_text_sensor",
    CONF_SPECIFICATION: "set_specification_text_sensor",
    CONF_EPON_PORT_RATE: "set_epon_port_rate_text_sensor",
    CONF_CONSOLE_PORT_RATE: "set_console_port_rate_text_sensor",
    CONF_INFO_RESPONSE: "set_info_response_text_sensor",
    CONF_GETPWR_RESPONSE: "set_getpwr_response_text_sensor",
    CONF_STAT_RESPONSE: "set_stat_response_text_sensor",
    CONF_BAT_RESPONSE: "set_bat_response_text_sensor",
    CONF_SOH_RESPONSE: "set_soh_response_text_sensor",
    CONF_BATTERY_VOLTAGE_STATE: "set_battery_voltage_state_text_sensor",
    CONF_BATTERY_TEMPERATURE_STATE: "set_battery_temperature_state_text_sensor",
    CONF_MOS_STATE: "set_mos_state_text_sensor",
}

SYSTEM_TEXT_SENSORS = {
    CONF_SYSTEM_STATUS: "set_system_status_text_sensor",
    CONF_LAST_COMMAND: "set_last_command_text_sensor",
    CONF_LAST_ERROR: "set_last_error_text_sensor",
    CONF_CONFIGURED_ROLE: "set_configured_role_text_sensor",
    CONF_OBSERVED_ROLE: "set_observed_role_text_sensor",
    CONF_ROLE_VALIDATION: "set_role_validation_text_sensor",
}

CELL_TEXT_SENSOR_LISTS = {
    CONF_CELL_VOLTAGE_STATES: "set_cell_voltage_state_text_sensor",
    CONF_CELL_TEMPERATURE_STATES: "set_cell_temperature_state_text_sensor",
    CONF_BAT_CELL_BALANCE_STATES: "set_bat_cell_balance_state_text_sensor",
    CONF_SOH_CELL_STATUSES: "set_soh_cell_status_text_sensor",
}

def validate_text_sensor_config(config):
    has_battery_text_sensors = any(key in config for key in TEXT_SENSORS) or any(
        key in config for key in CELL_TEXT_SENSOR_LISTS
    )
    if has_battery_text_sensors and CONF_BATTERY not in config:
        raise cv.Invalid("battery is required when declaring battery text sensors")
    return config


CONFIG_SCHEMA = cv.All(PYLONTECH_COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PylontechBattery),
    }
).extend(
    {
        cv.Optional(key): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        )
        for key in TEXT_SENSORS
    }
).extend(
    {
        cv.Optional(key): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        )
        for key in SYSTEM_TEXT_SENSORS
    }
).extend(
    {
        cv.Optional(key): cv.All(
            cv.ensure_list(
                text_sensor.text_sensor_schema(
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC
                )
            ),
            cv.Length(min=1, max=16),
        )
        for key in CELL_TEXT_SENSOR_LISTS
    }
).extend(cv.COMPONENT_SCHEMA), validate_text_sensor_config)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])

    has_battery_text_sensors = any(key in config for key in TEXT_SENSORS) or any(
        key in config for key in CELL_TEXT_SENSOR_LISTS
    )
    if has_battery_text_sensors:
        battery = await get_or_create_battery(config)

        for key, setter in TEXT_SENSORS.items():
            if key in config:
                sens = await text_sensor.new_text_sensor(config[key])
                cg.add(getattr(battery, setter)(sens))

        for key, setter in CELL_TEXT_SENSOR_LISTS.items():
            if key in config:
                for index, cell_config in enumerate(config[key]):
                    sens = await text_sensor.new_text_sensor(cell_config)
                    cg.add(getattr(battery, setter)(index + 1, sens))

    for key, setter in SYSTEM_TEXT_SENSORS.items():
        if key in config:
            sens = await text_sensor.new_text_sensor(config[key])
            cg.add(getattr(parent, setter)(sens))
