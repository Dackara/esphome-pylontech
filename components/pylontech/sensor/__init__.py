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

CONF_TEMPERATURE_LOW = "temperature_low"
CONF_TEMPERATURE_HIGH = "temperature_high"
CONF_CELL_LOW = "cell_low"
CONF_CELL_HIGH = "cell_high"
CONF_CAPACITY = "capacity"
CONF_MOS_TEMPERATURE = "mos_temperature"

CONF_VOLTAGE_LOW = "voltage_low"
CONF_VOLTAGE_HIGH = "voltage_high"
CONF_COULOMB = "coulomb"


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
    CONF_CELL_LOW: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_CELL_HIGH: sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_CAPACITY: sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    CONF_MOS_TEMPERATURE: sensor.sensor_schema(
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
