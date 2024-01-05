import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@Dackara"]
AUTO_LOAD = ["binary_sensor", "text_sensor", "sensor"]
MULTI_CONF = True

CONF_PYLONTECH_ID = "pylontech_id"

pylontech_ns = cg.esphome_ns.namespace("pylontech")*

PYLONTECH_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_PYLONTECH_ID): cv.use_id(PylontechComponent),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema({cv.GenerateID(): cv.declare_id(PylontechComponent),})
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
