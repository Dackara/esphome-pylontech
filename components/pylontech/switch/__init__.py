import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from .. import CONF_PYLONTECH_ID, PylontechComponent, pylontech_ns

CONF_FORCE_SLAVE_MODE = "force_slave_mode"

PylontechForceSlaveSwitch = pylontech_ns.class_(
    "PylontechForceSlaveSwitch", switch.Switch, cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYLONTECH_ID): cv.use_id(PylontechComponent),
        cv.Optional(CONF_FORCE_SLAVE_MODE): switch.switch_schema(PylontechForceSlaveSwitch).extend(
            cv.COMPONENT_SCHEMA
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_PYLONTECH_ID])

    if CONF_FORCE_SLAVE_MODE in config:
        sw = await switch.new_switch(config[CONF_FORCE_SLAVE_MODE])
        await cg.register_component(sw, config[CONF_FORCE_SLAVE_MODE])
        cg.add(sw.set_parent(parent))
