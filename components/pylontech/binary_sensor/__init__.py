import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from .. import CONF_PYLONTECH_ID, PYLONTECH_PARENT_SCHEMA

CONF_PROTOCOL_ONLINE = "protocol_online"

CONFIG_SCHEMA = PYLONTECH_PARENT_SCHEMA.extend(
    {
        cv.Optional(CONF_PROTOCOL_ONLINE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])
    if CONF_PROTOCOL_ONLINE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_PROTOCOL_ONLINE])
        cg.add(parent.set_protocol_online_binary_sensor(sens))
