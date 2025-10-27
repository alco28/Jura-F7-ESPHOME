"""Jura Coffee Machine Component for ESPHome."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, switch
from esphome.const import CONF_ID

CODEOWNERS = ["@alco28"]
DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor', 'text_sensor', 'switch']

# Use the uart component's CONF key to avoid mismatches
CONF_UART_ID = uart.CONF_UART_ID
CONF_DEBUG_SWITCH = 'debug_switch'

# Generate namespaces and C++ class binding
jura_coffee_ns = cg.esphome_ns.namespace('jura_coffee')
JuraCoffeeComponent = jura_coffee_ns.class_('JuraCoffee', cg.Component, uart.UARTDevice)

# Configuration schema: expect an id, uart reference, and optional debug switch
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(JuraCoffeeComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_DEBUG_SWITCH): cv.use_id(switch.Switch),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ code for the component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    uart_comp = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart_parent(uart_comp))

    if CONF_DEBUG_SWITCH in config:
        debug_switch = await cg.get_variable(config[CONF_DEBUG_SWITCH])
        cg.add(var.set_debug_switch(debug_switch))