import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_TEMPERATURE,
    UNIT_VOLT,
    UNIT_AMPERE,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_BATTERY,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    CONF_ID,
)

from .. import (
    CONF_PYLONTECH_ID,
    PYLONTECH_COMPONENT_SCHEMA,
    CONF_BATTERY,
    pylontech_ns,
)

PylontechSensor = pylontech_ns.class_("PylontechSensor", cg.Component)

CONF_UNIT_AMPERE_HOUR = "Ah"
# pwr sensors
CONF_TEMPERATURE_LOW = "temperature_low"
CONF_TEMPERATURE_HIGH = "temperature_high"
CONF_CELL_LOW = "cell_low"
CONF_CELL_HIGH = "cell_high"
CONF_COULOMB = "coulomb"
CONF_VOLTAGE_LOW = "voltage_low"
CONF_VOLTAGE_HIGH = "voltage_high"
CONF_MOS_TEMPERATURE = "mos_temperature"

# pwrsys sensors
CONF_TOTAL_NUM = "value_total_num"
CONF_VALUE_PRESENT_NUM = "value_present_num"
CONF_SLEEP_NUM = "value_sleep_num"
CONF_SYSTEM_VOLT = "value_system_volt"
CONF_SYSTEM_CURR = "value_system_curr"
CONF_SYSTEM_RC = "value_system_rc"
CONF_SYSTEM_FCC = "value_system_fcc"
CONF_SYSTEM_SOC = "value_system_soc"
CONF_SYSTEM_SOH = "value_system_soh"
CONF_HIGHEST_VOLTAGE = "value_highest_voltage"
CONF_AVERAGE_VOLTAGE = "value_average_voltage"
CONF_LOWEST_VOLTAGE = "value_lowest_voltage"
CONF_HIGEST_TEMPERATURE = "value_highest_temperature"
CONF_AVERAGE_TEMPERATURE = "value_average_temperature"
CONF_LOWEST_TEMPERATUREE = "value_lowest_temperature"
CONF_RECOMMEND_CHG_VOLTAGE = "value_recommend_chg_voltage"
CONF_RECOMMEND_DSG_VOLTAGE = "value_recommend_dsg_voltage"
CONF_RECOMMEND_CHG_CURRENT = "value_recommend_chg_current"
CONF_RECOMMEND_DSG_CURRENT = "value_recommend_dsg_current"
CONF_SYSTEM_RECOMMEND_CHG_VOLTAGE = "value_system_recommend_chg_voltage"
CONF_SYSTEM_RECOMMEND_DSG_VOLTAGE = "value_system_recommend_dsg_voltage"
CONF_SYSTEM_RECOMMEND_CHG_CURRENT = "value_system_recommend_chg_current"
CONF_SYSTEM_RECOMMEND_DSG_CURRENT = "value_system_recommend_dsg_current"

TYPES: dict[str, cv.Schema] = {
    CONF_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_TEMPERATURE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_TEMPERATURE_LOW: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_TEMPERATURE_HIGH: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_VOLTAGE_LOW: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_VOLTAGE_HIGH: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_COULOMB: sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    CONF_MOS_TEMPERATURE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),

    # pwrsys sensor
    CONF_TOTAL_NUM: sensor.sensor_schema(
        accuracy_decimals=0,
    ),
    CONF_VALUE_PRESENT_NUM: sensor.sensor_schema(
        accuracy_decimals=0,
    ),
    CONF_SLEEP_NUM: sensor.sensor_schema(
        accuracy_decimals=0,
    ),
    CONF_SYSTEM_VOLT: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_SYSTEM_CURR: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_SYSTEM_RC: sensor.sensor_schema(
        unit_of_measurement=AMPERE_HOUR,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_SYSTEM_FCC: sensor.sensor_schema(
        unit_of_measurement=AMPERE_HOUR,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_SYSTEM_SOC: sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    CONF_SYSTEM_SOH: sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    CONF_HIGHEST_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_AVERAGE_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_LOWEST_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_HIGEST_TEMPERATURE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_AVERAGE_TEMPERATURE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_LOWEST_TEMPERATUREE: sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_RECOMMEND_CHG_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_RECOMMEND_DSG_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_RECOMMEND_CHG_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_RECOMMEND_DSG_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_SYSTEM_RECOMMEND_CHG_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_SYSTEM_RECOMMEND_DSG_VOLTAGE: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_SYSTEM_RECOMMEND_CHG_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_SYSTEM_RECOMMEND_DSG_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
}

CONFIG_SCHEMA = PYLONTECH_COMPONENT_SCHEMA.extend(
    {cv.GenerateID(): cv.declare_id(PylontechSensor)}
).extend({cv.Optional(type): schema for type, schema in TYPES.items()})


async def to_code(config):
    paren = await cg.get_variable(config[CONF_PYLONTECH_ID])
    bat = cg.new_Pvariable(config[CONF_ID], config[CONF_BATTERY])

    for marker in TYPES:
        if marker_config := config.get(marker):
            sens = await sensor.new_sensor(marker_config)
            cg.add(getattr(bat, f"set_{marker}_sensor")(sens))

    cg.add(paren.register_listener(bat))
