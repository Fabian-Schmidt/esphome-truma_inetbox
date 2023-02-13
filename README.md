# ESPHome truma_inetbox component

ESPHome component to remote control Truma CP Plus Heater by simulating a Truma iNet box.

See [1](https://github.com/danielfett/inetbox.py) and [2](https://github.com/mc0110/inetbox2mqtt) for great documentation about how to connect an CP Plus to an ESP32.

## Acknowledgements

This project is based on the work of the [WomoLIN project](https://github.com/muccc/WomoLIN) and [mc0110 inetbox.py](https://github.com/danielfett/inetbox.py), especially the initial protocol decoding and the inet box log files.

## Example configuation

This example is just for connecting ESPHome to the CP Plus. See [truma.yaml](/truma.yaml) for an example config with all possible things configured.

```yaml
esphome:
  name: "esphome-truma"

external_components:
  - source: github://Fabian-Schmidt/esphome-truma_inetbox

esp32:
  board: mhetesp32devkit

uart:
  - id: lin_uart_bus
    baud_rate: 9600
    stop_bits: 2

truma_inetbox:
  uart_id: lin_uart_bus

binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED

sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
  - platform: truma_inetbox
    name: "Current Water Temperature"
    type: CURRENT_WATER_TEMPERATURE
  - platform: truma_inetbox
    name: "Target Room Temperature"
    type: TARGET_ROOM_TEMPERATURE
  - platform: truma_inetbox
    name: "Target Water Temperature"
    type: TARGET_WATER_TEMPERATURE
```

## ESPHome components

This project contains the following ESPHome components:

- `uart` will overwrite the default uart component to expose internal fields.
- `truma_inetbox` has the following settings:
  - `cs_pin` (optional) if you connect the pin of your lin driver chip.
  - `fault_pin` (optional) if you connect the pin of your lin driver chip.
  - `on_heater_message` (optional) [ESPHome Trigger](https://esphome.io/guides/automations.html) when a message from CP Plus is recieved.

### Binary sensor

Binary sensors are read-only.

```yaml
binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED
```

The following `type` values are available:

- `CP_PLUS_CONNECTED`
- `HEATER_ROOM`
- `HEATER_WATER`
- `HEATER_GAS`
- `HEATER_MIX_1`
- `HEATER_MIX_2`
- `HEATER_ELECTRICITY`
- `TIMER_ACTIVE`
- `TIMER_ROOM`
- `TIMER_WATER`

### Climate

Climate components support read and write.

```yaml
climate:
  - platform: truma_inetbox
    name: "Truma Room"
    type: ROOM
  - platform: truma_inetbox
    name: "Truma Water"
    type: WATER
```

The following `type` values are available:

- `ROOM`
- `WATER`

### Number

Number components support read and write.

```yaml
number:
  - platform: truma_inetbox
    name: "Target Room Temperature"
    type: TARGET_ROOM_TEMPERATURE
```

The following `type` values are available:

- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `ELECTRIC_POWER_LEVEL`

### Sensor

Sensors are read-only.

```yaml
sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
```

The following `type` values are available:

- `CURRENT_ROOM_TEMPERATURE`
- `CURRENT_WATER_TEMPERATURE`
- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `HEATING_MODE`
- `ELECTRIC_POWER_LEVEL`
- `ENERGY_MIX`
- `OPERATING_STATUS`

### Actions

The following [ESP Home actions](https://esphome.io/guides/automations.html#actions) are available:

- `truma_inetbox.heater.set_target_room_temperature`
  - `temperature` - Temperature between 5C and 30C. Below 5C will disable the Heater.
  - `heating_mode` - Optional set heating mode: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_target_water_temperature`
  - `temperature` - Set water temp as number: `0`, `40`, `60`, `80`.
- `truma_inetbox.heater.set_target_water_temperature_enum`
  - `temperature` - Set water temp as text: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_electric_power_level`
  - `watt` - Set electricity level to `0`, `900`, `1800`.
- `truma_inetbox.heater.set_energy_mix`
  - `energy_mix` - Set energy mix to: `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optional: Set electricity level to `0`, `900`, `1800`
- `truma_inetbox.timer.disable` - Disable the timer configuration.
- `truma_inetbox.timer.activate` - Set a new timer configuration.
  - `start` - Start time.
  - `stop` - Stop time.
  - `room_temperature` - Temperature between 5C and 30C.
  - `heating_mode` - Optional: Set heating mode: `"OFF"`, `ECO`, `HIGH`, `BOOST`.
  - `water_temperature` - Optional: Set water temp as number: `0`, `40`, `60`, `80`.
  - `energy_mix` - Optional: Set energy mix to: `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optional: Set electricity level to `0`, `900`, `1800`.
- `truma_inetbox.clock.set` - Update CP Plus from ESP Home. You *must* have another [clock source](https://esphome.io/#time-components) configured like Home Assistant Time, GPS or DS1307 RTC.

## TODO

- [ ] This file
- [ ] RP2040 support
- [ ] Testing of Combi 4E / Combi 6E and Alde devices (I only have access to an Combi 4)
- [ ] More Testing
