Jura Coffee ESPHome Custom Component

Overview

This directory contains a custom ESPHome component to decode Jura coffee machine counters and status via a UART-based protocol. The component is designed to be used as an `external_components` local source in your ESPHome YAML.

Files

- `jura_coffee.h` - Header with class declaration (constructor and public methods).
- `jura_coffee.cpp` - Implementation of the decoding logic, UART read/write, parsing and publishing to sensors.
- `custom_component.yaml` - Example external_components entry (kept for reference).

Sensor mapping

The component expects 11 numeric sensors (counters) and 2 text sensors for tray/tank status. The YAML should define these sensors and pass them to the component in the exact order shown below.

Constructor parameter / YAML id mapping (order matters):

1. `num_single_espresso` - Single espressos made
2. `num_double_espresso` - Double espressos made
3. `num_coffee` - Single coffees made
4. `num_double_coffee` - Double coffees made
5. `num_ristretto` - Single ristretto made
6. `num_single_capuccino` - Single cappuccino made
7. `num_double_ristretto` - Double ristretto made
8. `num_brewunit_movements` - Brew unit motor movements
9. `num_clean` - Cleanings performed
10. `num_descaling` - Descalings performed
11. `num_coffee_Grounds` - Coffee grounds count (from EEPROM)

Text sensors (tray/tank):
- `tray_status` - "Present" / "Missing"
- `tank_status` - "OK" / "Fill Tank"

How to include in your YAML

Example snippet (already present in `jura-01.yaml` in this repo):

external_components:
  - source:
      type: local
      path: external_components/jura_coffee

esphome:
  includes:
    - external_components/jura_coffee/jura_coffee.h

# register the custom platform (example)
- platform: custom
  lambda: |-
    auto *jura = new JuraCoffee(id(uart_bus), id(num_single_espresso), id(num_double_espresso), id(num_coffee), id(num_double_coffee), id(num_ristretto),
                                id(num_single_capuccino), id(num_double_ristretto), id(num_brewunit_movements), id(num_clean), id(num_descaling),
                                id(num_coffee_Grounds), id(tray_status), id(tank_status));
    App.register_component(jura);
    return {id(num_single_espresso), id(num_double_espresso), id(num_coffee), id(num_double_coffee), id(num_ristretto), id(num_single_capuccino), id(num_double_ristretto), id(num_brewunit_movements), id(num_clean), id(num_descaling), id(num_coffee_Grounds)};

Build / validation (PowerShell)

From the repository root (where `jura-01.yaml` is located) run:

# Validate the YAML
esphome config .\jura-01.yaml

# Compile the firmware (downloads toolchain + compiles)
esphome compile .\jura-01.yaml

If you use Home Assistant's ESPHome Add-on you can also validate/compile/upload via the Add-on UI.

Troubleshooting

- If compilation fails due to missing includes, ensure the `esphome` includes path points to `external_components/jura_coffee/jura_coffee.h`.
- If UART timing or decoding seems wrong: check wiring, ensure TX/RX pins and ground are correct, and try increasing/decreasing delays in `fetchData` (in `jura_coffee.cpp`).
- If sensors publish `nan` frequently, the component may time out while reading; increase the timeout or check the cable/connections.

Notes

- The component uses low-level bit encoding/decoding specific to some Jura models. Behavior may vary between models.
- If you want the counters to start at 0 instead of `NAN` as initial state, change the template sensor lambdas in your YAML accordingly.

License & attribution

Original author credit is kept in `jura-01.yaml` header. Update as needed.
