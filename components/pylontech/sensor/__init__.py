import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_CURRENT,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from .. import (
    CONF_BATTERY,
    CONF_PYLONTECH_ID,
    PYLONTECH_COMPONENT_SCHEMA,
    PylontechBattery,
    get_or_create_battery,
)

CONF_TEMPERATURE_LOW = "temperature_low"
CONF_TEMPERATURE_HIGH = "temperature_high"
CONF_CELL_VOLTAGE_LOW = "cell_voltage_low"
CONF_CELL_VOLTAGE_HIGH = "cell_voltage_high"
CONF_SOC = "soc"
CONF_COULOMB = "coulomb"
CONF_TOTAL_COULOMB = "total_coulomb"
CONF_MAX_VOLTAGE = "max_voltage"
CONF_CHARGE_TIMES = "charge_times"
CONF_DISCHARGE_SEC = "discharge_sec"
CONF_CHARGE_SEC = "charge_sec"
CONF_MOS_TEMPERATURE = "mos_temperature"
CONF_DEVICE_ADDRESS = "device_address"
CONF_CELL_NUMBER = "cell_number"
CONF_MAX_CHARGE_CURRENT = "max_charge_current"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_TOTAL_NUM = "total_num"
CONF_PRESENT_NUM = "present_num"
CONF_SLEEP_NUM = "sleep_num"
CONF_SYSTEM_VOLTAGE = "system_voltage"
CONF_SYSTEM_CURRENT = "system_current"
CONF_SYSTEM_RC = "system_rc"
CONF_SYSTEM_FCC = "system_fcc"
CONF_SYSTEM_SOC = "system_soc"
CONF_SYSTEM_SOH = "system_soh"
CONF_HIGHEST_VOLTAGE = "highest_voltage"
CONF_AVERAGE_VOLTAGE = "average_voltage"
CONF_LOWEST_VOLTAGE = "lowest_voltage"
CONF_HIGHEST_TEMPERATURE = "highest_temperature"
CONF_AVERAGE_TEMPERATURE = "average_temperature"
CONF_LOWEST_TEMPERATURE = "lowest_temperature"
CONF_RECOMMEND_CHARGE_VOLTAGE = "recommend_charge_voltage"
CONF_RECOMMEND_DISCHARGE_VOLTAGE = "recommend_discharge_voltage"
CONF_RECOMMEND_CHARGE_CURRENT = "recommend_charge_current"
CONF_RECOMMEND_DISCHARGE_CURRENT = "recommend_discharge_current"
CONF_CELL_VOLTAGES = "cell_voltages"
# Compatibility alias for early test YAMLs.
CONF_CELLS = "cells"
CONF_CELL_TEMPERATURES = "cell_temperatures"
CONF_GETPWR_VOLTAGE = "getpwr_voltage"
CONF_GETPWR_CURRENT = "getpwr_current"
CONF_GETPWR_TEMPERATURE = "getpwr_temperature"
CONF_GETPWR_RESIDUAL_CAPACITY = "getpwr_residual_capacity"
CONF_GETPWR_RESIDUAL_MA = "getpwr_residual_ma"
CONF_GETPWR_TAIL_1 = "getpwr_tail_1"
CONF_GETPWR_TAIL_2 = "getpwr_tail_2"
# Compatibility aliases for early test YAMLs.
CONF_GETPWR_UNKNOWN = "getpwr_unknown"
CONF_GETPWR_CYCLE = "getpwr_cycle"
CONF_STAT_CYCLE_COUNT = "stat_cycle_count"
CONF_STAT_COULOMB = "stat_coulomb"
CONF_BAT_CELL_CURRENTS = "bat_cell_currents"
CONF_BAT_CELL_SOCS = "bat_cell_socs"
CONF_BAT_CELL_COULOMBS = "bat_cell_coulombs"
CONF_SOH_CELL_COUNTS = "soh_cell_counts"
CONF_RESPONSE_TIME = "response_time"
CONF_CONSECUTIVE_FAILURES = "consecutive_failures"
CONF_TIMEOUT_COUNT = "timeout_count"
CONF_UART_QUEUE_HIGH_WATERMARK = "uart_queue_high_watermark"
CONF_UART_QUEUE_FULL_COUNT = "uart_queue_full_count"

STAT_SENSOR_FIELDS = {
    "stat_device_address": (0, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_data_items": (1, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_hisdata_items": (2, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_miscdata_items": (51, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_charge_count": (3, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_charge_cnt": (3, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_discharge_count": (4, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_discharge_cnt": (4, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_charge_times": (5, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_status_count": (6, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_status_cnt": (6, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_idle_times": (7, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_coc_times": (8, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_coc2_times": (9, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_doc_times": (10, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_doc2_times": (11, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_coca_times": (12, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_doca_times": (13, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_sc_times": (14, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bat_ov_times": (15, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bat_hv_times": (16, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bat_lv_times": (17, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bat_uv_times": (18, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bat_slp_times": (19, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_ov_times": (20, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_hv_times": (21, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_lv_times": (22, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_uv_times": (23, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_slp_times": (24, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_cot_times": (25, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_cut_times": (26, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_dot_times": (27, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_dut_times": (28, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_cht_times": (29, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_clt_times": (30, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_dht_times": (31, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_dlt_times": (32, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_shut_times": (33, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_reset_times": (34, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_rv_times": (35, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_input_ov_times": (36, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_soh_times": (37, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_bmicerr_times": (38, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_cycle_times": (39, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_soh": (40, sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_percent": (41, sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_pwr_coulomb": (42, sensor.sensor_schema(unit_of_measurement="Ah", accuracy_decimals=3, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_dsg_cap": (43, sensor.sensor_schema(unit_of_measurement="Ah", accuracy_decimals=3, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_ht_half_c_count": (44, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_ht_05c_cnt": (44, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lt_half_c_count": (45, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lt_05c_cnt": (45, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_ht_count": (46, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_ht_cnt": (46, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lt_count": (47, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lt_cnt": (47, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lv_count": (48, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lv_cnt": (48, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lifewarn_times": (49, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    "stat_lifealarm_times": (50, sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
}

SENSORS = {
    CONF_VOLTAGE: (
        "set_voltage_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_CURRENT: (
        "set_current_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_TEMPERATURE: (
        "set_temperature_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_TEMPERATURE_LOW: (
        "set_temperature_low_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_TEMPERATURE_HIGH: (
        "set_temperature_high_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_CELL_VOLTAGE_LOW: (
        "set_cell_voltage_low_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_CELL_VOLTAGE_HIGH: (
        "set_cell_voltage_high_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_SOC: (
        "set_soc_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_COULOMB: (
        "set_soc_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_TOTAL_COULOMB: (
        "set_total_coulomb_sensor",
        sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_MAX_VOLTAGE: (
        "set_max_voltage_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_CHARGE_TIMES: (
        "set_charge_times_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_DISCHARGE_SEC: (
        "set_discharge_sec_sensor",
        sensor.sensor_schema(
            unit_of_measurement="s",
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_CHARGE_SEC: (
        "set_charge_sec_sensor",
        sensor.sensor_schema(
            unit_of_measurement="s",
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_MOS_TEMPERATURE: (
        "set_mos_temperature_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_DEVICE_ADDRESS: (
        "set_device_address_sensor",
        sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT),
    ),
    CONF_CELL_NUMBER: (
        "set_cell_number_sensor",
        sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT),
    ),
    CONF_MAX_CHARGE_CURRENT: (
        "set_max_charge_current_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_MAX_DISCHARGE_CURRENT: (
        "set_max_discharge_current_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_VOLTAGE: (
        "set_getpwr_voltage_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_CURRENT: (
        "set_getpwr_current_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_TEMPERATURE: (
        "set_getpwr_temperature_sensor",
        sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_RESIDUAL_CAPACITY: (
        "set_getpwr_residual_capacity_sensor",
        sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_RESIDUAL_MA: (
        "set_getpwr_residual_ma_sensor",
        sensor.sensor_schema(
            unit_of_measurement="mA",
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_UNKNOWN: (
        "set_getpwr_tail_1_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_TAIL_1: (
        "set_getpwr_tail_1_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_CYCLE: (
        "set_getpwr_tail_2_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_GETPWR_TAIL_2: (
        "set_getpwr_tail_2_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_STAT_CYCLE_COUNT: (
        "set_stat_cycle_count_sensor",
        sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
    CONF_STAT_COULOMB: (
        "set_stat_coulomb_sensor",
        sensor.sensor_schema(
            unit_of_measurement="Ah",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    ),
}

SYSTEM_SENSORS = {
    CONF_TOTAL_NUM: ("set_total_num_sensor", sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_PRESENT_NUM: ("set_present_num_sensor", sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SLEEP_NUM: ("set_sleep_num_sensor", sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_VOLTAGE: ("set_system_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_CURRENT: ("set_system_current_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_AMPERE, accuracy_decimals=3, device_class=DEVICE_CLASS_CURRENT, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_RC: ("set_system_rc_sensor", sensor.sensor_schema(unit_of_measurement="Ah", accuracy_decimals=3, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_FCC: ("set_system_fcc_sensor", sensor.sensor_schema(unit_of_measurement="Ah", accuracy_decimals=3, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_SOC: ("set_system_soc_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, device_class=DEVICE_CLASS_BATTERY, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_SYSTEM_SOH: ("set_system_soh_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_HIGHEST_VOLTAGE: ("set_highest_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_AVERAGE_VOLTAGE: ("set_average_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_LOWEST_VOLTAGE: ("set_lowest_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_HIGHEST_TEMPERATURE: ("set_highest_temperature_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, accuracy_decimals=1, device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_AVERAGE_TEMPERATURE: ("set_average_temperature_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, accuracy_decimals=1, device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_LOWEST_TEMPERATURE: ("set_lowest_temperature_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, accuracy_decimals=1, device_class=DEVICE_CLASS_TEMPERATURE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_RECOMMEND_CHARGE_VOLTAGE: ("set_recommend_charge_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_RECOMMEND_DISCHARGE_VOLTAGE: ("set_recommend_discharge_voltage_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_VOLT, accuracy_decimals=3, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_RECOMMEND_CHARGE_CURRENT: ("set_recommend_charge_current_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_AMPERE, accuracy_decimals=3, device_class=DEVICE_CLASS_CURRENT, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_RECOMMEND_DISCHARGE_CURRENT: ("set_recommend_discharge_current_sensor", sensor.sensor_schema(unit_of_measurement=UNIT_AMPERE, accuracy_decimals=3, device_class=DEVICE_CLASS_CURRENT, state_class=STATE_CLASS_MEASUREMENT)),
    CONF_RESPONSE_TIME: ("set_response_time_sensor", sensor.sensor_schema(unit_of_measurement="ms", accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)),
    CONF_CONSECUTIVE_FAILURES: ("set_consecutive_failures_sensor", sensor.sensor_schema(accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)),
    CONF_TIMEOUT_COUNT: ("set_timeout_count_sensor", sensor.sensor_schema(accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)),
    CONF_UART_QUEUE_HIGH_WATERMARK: ("set_uart_queue_high_watermark_sensor", sensor.sensor_schema(accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)),
    CONF_UART_QUEUE_FULL_COUNT: ("set_uart_queue_full_count_sensor", sensor.sensor_schema(accuracy_decimals=0, entity_category=ENTITY_CATEGORY_DIAGNOSTIC)),
}

CONFIG_SCHEMA = PYLONTECH_COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PylontechBattery),
    }
).extend({cv.Optional(key): schema for key, (_, schema) in SENSORS.items()}).extend(
    {cv.Optional(key): schema for key, (_, schema) in STAT_SENSOR_FIELDS.items()}
).extend(
    {
        cv.Optional(CONF_CELL_VOLTAGES): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_VOLT,
                    accuracy_decimals=3,
                    device_class=DEVICE_CLASS_VOLTAGE,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_CELLS): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_VOLT,
                    accuracy_decimals=3,
                    device_class=DEVICE_CLASS_VOLTAGE,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_CELL_TEMPERATURES): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_CELSIUS,
                    accuracy_decimals=1,
                    device_class=DEVICE_CLASS_TEMPERATURE,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_BAT_CELL_CURRENTS): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement="mA",
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_BAT_CELL_SOCS): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_PERCENT,
                    accuracy_decimals=0,
                    device_class=DEVICE_CLASS_BATTERY,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_BAT_CELL_COULOMBS): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    unit_of_measurement="mAh",
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
        cv.Optional(CONF_SOH_CELL_COUNTS): cv.All(
            cv.ensure_list(
                sensor.sensor_schema(
                    accuracy_decimals=0,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
            ),
            cv.Length(min=1, max=16),
        ),
    }
).extend(
    {cv.Optional(key): schema for key, (_, schema) in SYSTEM_SENSORS.items()}
).extend(
    cv.COMPONENT_SCHEMA
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])
    has_battery_sensors = (
        any(key in config for key in SENSORS)
        or any(key in config for key in STAT_SENSOR_FIELDS)
        or CONF_CELL_VOLTAGES in config
        or CONF_CELLS in config
        or CONF_CELL_TEMPERATURES in config
        or CONF_BAT_CELL_CURRENTS in config
        or CONF_BAT_CELL_SOCS in config
        or CONF_BAT_CELL_COULOMBS in config
        or CONF_SOH_CELL_COUNTS in config
    )

    if has_battery_sensors:
        if CONF_BATTERY not in config:
            raise cv.Invalid("battery is required when declaring battery sensors")

        battery = await get_or_create_battery(config)

        for key, (setter, _) in SENSORS.items():
            if key in config:
                sens = await sensor.new_sensor(config[key])
                cg.add(getattr(battery, setter)(sens))

        for key, (field, _) in STAT_SENSOR_FIELDS.items():
            if key in config:
                sens = await sensor.new_sensor(config[key])
                cg.add(battery.set_stat_field_sensor(field, sens))

        cell_voltage_configs = config.get(CONF_CELL_VOLTAGES, config.get(CONF_CELLS))
        if cell_voltage_configs is not None:
            for index, cell_config in enumerate(cell_voltage_configs):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_cell_voltage_sensor(index + 1, sens))

        if CONF_CELL_TEMPERATURES in config:
            for index, cell_config in enumerate(config[CONF_CELL_TEMPERATURES]):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_cell_temperature_sensor(index + 1, sens))

        if CONF_BAT_CELL_CURRENTS in config:
            for index, cell_config in enumerate(config[CONF_BAT_CELL_CURRENTS]):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_bat_cell_current_sensor(index + 1, sens))

        if CONF_BAT_CELL_SOCS in config:
            for index, cell_config in enumerate(config[CONF_BAT_CELL_SOCS]):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_bat_cell_soc_sensor(index + 1, sens))

        if CONF_BAT_CELL_COULOMBS in config:
            for index, cell_config in enumerate(config[CONF_BAT_CELL_COULOMBS]):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_bat_cell_coulomb_sensor(index + 1, sens))

        if CONF_SOH_CELL_COUNTS in config:
            for index, cell_config in enumerate(config[CONF_SOH_CELL_COUNTS]):
                sens = await sensor.new_sensor(cell_config)
                cg.add(battery.set_soh_cell_count_sensor(index + 1, sens))

    for key, (setter, _) in SYSTEM_SENSORS.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(parent, setter)(sens))
