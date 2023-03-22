from typing import Optional

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import pins, automation
from esphome.components import uart, time
from esphome.const import (
    CONF_ID,
    CONF_NUMBER,
    CONF_BAUD_RATE,
    CONF_UART_ID,
    CONF_RX_PIN,
    CONF_TX_PIN,
    CONF_INVERTED,
    CONF_CS_PIN,
    CONF_TEMPERATURE,
    CONF_TRIGGER_ID,
    CONF_STOP,
    CONF_TIME_ID,
    CONF_TIME,
)
from esphome.components.uart import (
    CONF_STOP_BITS,
    CONF_DATA_BITS,
    CONF_PARITY,
    KEY_UART_DEVICES,
)
from esphome.core import CORE
from .entity_helpers import count_id_usage

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@Fabian-Schmidt"]

CONF_TRUMA_INETBOX_ID = "truma_inetbox_id"
CONF_LIN_CHECKSUM = "lin_checksum"
CONF_FAULT_PIN = "fault_pin"
CONF_OBSERVER_MODE = "observer_mode"
CONF_NUMBER_OF_CHILDREN = "number_of_children"
CONF_ON_HEATER_MESSAGE = "on_heater_message"

truma_inetbox_ns = cg.esphome_ns.namespace("truma_inetbox")
StatusFrameHeater = truma_inetbox_ns.struct("StatusFrameHeater")
StatusFrameHeaterConstPtr = StatusFrameHeater.operator("ptr").operator("const")
TrumaINetBoxApp = truma_inetbox_ns.class_(
    "TrumaiNetBoxApp", cg.PollingComponent, uart.UARTDevice
)
TrumaiNetBoxAppHeaterMessageTrigger = truma_inetbox_ns.class_(
    "TrumaiNetBoxAppHeaterMessageTrigger",
    automation.Trigger.template(StatusFrameHeaterConstPtr),
)

# `LIN_CHECKSUM` is a enum class and not a namespace but it works.
LIN_CHECKSUM_dummy_ns = truma_inetbox_ns.namespace("LIN_CHECKSUM")

CONF_SUPPORTED_LIN_CHECKSUM = {
    "VERSION_1": LIN_CHECKSUM_dummy_ns.LIN_CHECKSUM_VERSION_1,
    "VERSION_2": LIN_CHECKSUM_dummy_ns.LIN_CHECKSUM_VERSION_2,
}

# [RP2040] Hardware serial of uart validation:
#   constexpr uint32_t valid_tx_uart_0 = __bitset({0, 12, 16, 28});
#   constexpr uint32_t valid_tx_uart_1 = __bitset({4, 8, 20, 24});
#   constexpr uint32_t valid_rx_uart_0 = __bitset({1, 13, 17, 29});
#   constexpr uint32_t valid_rx_uart_1 = __bitset({5, 9, 21, 25});
CONF_RP2040_HARDWARE_UART = {
    CONF_TX_PIN: {
        # Pin : Hardware UART number
        0: 0,
        12: 0,
        16: 0,
        28: 0,
        4: 1,
        8: 1,
        20: 1,
        24: 1,
    },
    CONF_RX_PIN: {
        # Pin : Hardware UART number
        1: 0,
        13: 0,
        17: 0,
        29: 0,
        5: 1,
        9: 1,
        21: 1,
        25: 1,
    }
}


def final_validate_device_schema(
    name: str,
    *,
    baud_rate: Optional[int] = None,
    require_tx: bool = False,
    require_rx: bool = False,
    stop_bits: Optional[int] = None,
    data_bits: Optional[int] = None,
    parity: str = None,
    require_hardware_uart: Optional[bool] = None,
):
    def validate_baud_rate(value):
        if value != baud_rate:
            raise cv.Invalid(
                f"Component {name} required baud rate {baud_rate} for the uart bus"
            )
        return value

    def validate_pin(opt, device):
        def validator(value):
            if opt in device:
                raise cv.Invalid(
                    f"The uart {opt} is used both by {name} and {device[opt]}, "
                    f"but can only be used by one. Please create a new uart bus for {name}."
                )
            device[opt] = name
            return value

        return validator

    def validate_stop_bits(value):
        if value != stop_bits:
            raise cv.Invalid(
                f"Component {name} required stop bits {stop_bits} for the uart bus"
            )
        return value

    def validate_data_bits(value):
        if value != data_bits:
            raise cv.Invalid(
                f"Component {name} required data bits {data_bits} for the uart bus"
            )
        return value

    def validate_parity(value):
        if value != parity:
            raise cv.Invalid(
                f"Component {name} required parity {parity} for the uart bus"
            )
        return value

    def validate_hardware_uart(opt, opt2=None, declaration_config=None):
        def validator(value):
            if (CORE.is_rp2040):
                if value[CONF_INVERTED]:
                    raise cv.Invalid(
                        f"Component {name} required Hardware UART. Inverted is not supported by Hardware UART.")
                if value[CONF_NUMBER] not in CONF_RP2040_HARDWARE_UART[opt]:
                    raise cv.Invalid(
                        f"Component {name} required Hardware UART. {opt} is not a Hardware UART pin.")
                if opt2 and declaration_config and CONF_RP2040_HARDWARE_UART[opt2][declaration_config[opt2][CONF_NUMBER]] != CONF_RP2040_HARDWARE_UART[opt][value[CONF_NUMBER]]:
                    raise cv.Invalid(
                        f"Component {name} required Hardware UART. {opt} and {opt2} are not a matching Hardware UART pin set.")

            return value
        return validator

    def validate_hub(hub_config):
        hub_schema = {}
        uart_id = hub_config[CONF_ID]
        devices = fv.full_config.get().data.setdefault(KEY_UART_DEVICES, {})
        device = devices.setdefault(uart_id, {})

        if require_tx:
            hub_schema[
                cv.Required(
                    CONF_TX_PIN,
                    msg=f"Component {name} requires this uart bus to declare a tx_pin",
                )
            ] = validate_pin(CONF_TX_PIN, device)
        if require_rx:
            hub_schema[
                cv.Required(
                    CONF_RX_PIN,
                    msg=f"Component {name} requires this uart bus to declare a rx_pin",
                )
            ] = validate_pin(CONF_RX_PIN, device)
        if baud_rate is not None:
            hub_schema[cv.Required(CONF_BAUD_RATE)] = validate_baud_rate
        if stop_bits is not None:
            hub_schema[cv.Required(CONF_STOP_BITS)] = validate_stop_bits
        if data_bits is not None:
            hub_schema[cv.Required(CONF_DATA_BITS)] = validate_data_bits
        if parity is not None:
            hub_schema[cv.Required(CONF_PARITY)] = validate_parity
        if require_hardware_uart is not None:
            fconf = fv.full_config.get()
            path = fconf.get_path_for_id(uart_id)[:-1]
            declaration_config = fconf.get_config_for_path(path)
            hub_schema[cv.Required(CONF_TX_PIN)] = validate_hardware_uart(
                CONF_TX_PIN)
            hub_schema[cv.Required(CONF_RX_PIN)] = validate_hardware_uart(
                CONF_RX_PIN, CONF_TX_PIN, declaration_config)
        return cv.Schema(hub_schema, extra=cv.ALLOW_EXTRA)(hub_config)

    return cv.Schema(
        {cv.Required(CONF_UART_ID)
                     : fv.id_declaration_match_schema(validate_hub)},
        extra=cv.ALLOW_EXTRA,
    )


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TrumaINetBoxApp),
            cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_LIN_CHECKSUM, "VERSION_2"): cv.enum(CONF_SUPPORTED_LIN_CHECKSUM, upper=True),
            cv.Optional(CONF_CS_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_FAULT_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_OBSERVER_MODE): cv.boolean,
            cv.Optional(CONF_ON_HEATER_MESSAGE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TrumaiNetBoxAppHeaterMessageTrigger),
                }
            ),
        }
    )
    # Polling is for presenting data to sensors.
    # Reading and communication is done in a seperate thread/core.
    .extend(cv.polling_component_schema("500ms"))
    .extend(uart.UART_DEVICE_SCHEMA),
    cv.only_on(["esp32", "rp2040"]),
)
FINAL_VALIDATE_SCHEMA = cv.All(
    final_validate_device_schema(
        "truma_inetbox", baud_rate=9600, require_tx=True, require_rx=True, stop_bits=2, data_bits=8, parity="NONE", require_hardware_uart=True),
)

async def to_code(config):
    if CORE.using_esp_idf:
        # Run interrupt on core 0. ESP Home runs on core 1.
        cg.add_build_flag("-DARDUINO_SERIAL_EVENT_TASK_RUNNING_CORE=0")
        # Default Stack Size is 2048. Not enough for my operation.
        cg.add_build_flag("-DARDUINO_SERIAL_EVENT_TASK_STACK_SIZE=4096")

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    if (CONF_TIME_ID in config):
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(time_))

    if CONF_LIN_CHECKSUM in config:
        cg.add(var.set_lin_checksum(
            CONF_SUPPORTED_LIN_CHECKSUM[config[CONF_LIN_CHECKSUM]]))

    if CONF_CS_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_CS_PIN])
        cg.add(var.set_cs_pin(pin))

    if CONF_FAULT_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FAULT_PIN])
        cg.add(var.set_fault_pin(pin))

    if CONF_OBSERVER_MODE in config:
        cg.add(var.set_observer_mode(config[CONF_OBSERVER_MODE]))

    for conf in config.get(CONF_ON_HEATER_MESSAGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(StatusFrameHeaterConstPtr, "message")], conf
        )


# AUTOMATION

CONF_ENERGY_MIX = "energy_mix"
CONF_ELECTRIC_POWER_LEVEL = "electric_power_level"
CONF_HEATING_MODE = "heating_mode"
CONF_WATT = "watt"
CONF_START = "start"
CONF_ROOM_TEMPERATURE = "room_temperature"
CONF_WATER_TEMPERATURE = "water_temperature"

HeaterRoomTempAction = truma_inetbox_ns.class_(
    "HeaterRoomTempAction", automation.Action)
HeaterWaterTempAction = truma_inetbox_ns.class_(
    "HeaterWaterTempAction", automation.Action)
HeaterWaterTempEnumAction = truma_inetbox_ns.class_(
    "HeaterWaterTempEnumAction", automation.Action)
HeaterElecPowerLevelAction = truma_inetbox_ns.class_(
    "HeaterElecPowerLevelAction", automation.Action)
HeaterEnergyMixAction = truma_inetbox_ns.class_(
    "HeaterEnergyMixAction", automation.Action)
AirconManualTempAction = truma_inetbox_ns.class_(
    "AirconManualTempAction", automation.Action)
TimerDisableAction = truma_inetbox_ns.class_(
    "TimerDisableAction", automation.Action)
TimerActivateAction = truma_inetbox_ns.class_(
    "TimerActivateAction", automation.Action)
WriteTimeAction = truma_inetbox_ns.class_("WriteTimeAction", automation.Action)

# `EnergyMix` is a enum class and not a namespace but it works.
EnergyMix_dummy_ns = truma_inetbox_ns.namespace("EnergyMix")

CONF_SUPPORTED_ENERGY_MIX = {
    "NONE": EnergyMix_dummy_ns.ENERGY_MIX_NONE,
    "GAS": EnergyMix_dummy_ns.ENERGY_MIX_GAS,
    "DIESEL": EnergyMix_dummy_ns.ENERGY_MIX_DIESEL,
    "ELECTRICITY": EnergyMix_dummy_ns.ENERGY_MIX_ELECTRICITY,
    "MIX": EnergyMix_dummy_ns.ENERGY_MIX_MIX,
}

# `ElectricPowerLevel` is a enum class and not a namespace but it works.
ElectricPowerLevel_dummy_ns = truma_inetbox_ns.namespace("ElectricPowerLevel")

CONF_SUPPORTED_ELECTRIC_POWER_LEVEL = {
    "0": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_0,
    "0W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_0,
    "0 W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_0,
    "900": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_900,
    "900W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_900,
    "900 W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_900,
    "1800": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1800W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1800 W": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1.8kW": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1,8kW": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1.8 kW": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
    "1,8 kW": ElectricPowerLevel_dummy_ns.ELECTRIC_POWER_LEVEL_1800,
}

# `HeatingMode` is a enum class and not a namespace but it works.
HeatingMode_dummy_ns = truma_inetbox_ns.namespace("HeatingMode")

CONF_SUPPORTED_HEATING_MODE = {
    "OFF": HeatingMode_dummy_ns.HEATING_MODE_OFF,
    "ECO": HeatingMode_dummy_ns.HEATING_MODE_ECO,
    "NIGHT": HeatingMode_dummy_ns.HEATING_MODE_VARIO_HEAT_NIGHT,
    "AUTO": HeatingMode_dummy_ns.HEATING_MODE_VARIO_HEAT_AUTO,
    "HIGH": HeatingMode_dummy_ns.HEATING_MODE_HIGH,
    "BOOST": HeatingMode_dummy_ns.HEATING_MODE_BOOST,
}

# `TargetTemp` is a enum class and not a namespace but it works.
TargetTemp_dummy_ns = truma_inetbox_ns.namespace("TargetTemp")

CONF_SUPPORTED_WATER_TEMPERATURE = {
    "OFF": TargetTemp_dummy_ns.TARGET_TEMP_OFF,
    "ECO": TargetTemp_dummy_ns.TARGET_TEMP_WATER_ECO,
    "HIGH": TargetTemp_dummy_ns.TARGET_TEMP_WATER_HIGH,
    "BOOST": TargetTemp_dummy_ns.TARGET_TEMP_WATER_BOOST,
}


@automation.register_action(
    "truma_inetbox.heater.set_target_room_temperature",
    HeaterRoomTempAction,
    automation.maybe_conf(
        CONF_TEMPERATURE,
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_TEMPERATURE): cv.templatable(cv.int_range(min=0, max=30)),
            cv.Optional(CONF_HEATING_MODE, "OFF"): cv.templatable(cv.enum(CONF_SUPPORTED_HEATING_MODE, upper=True)),
        }
    ),
)
async def truma_inetbox_heater_set_target_room_temperature_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_TEMPERATURE], args, cg.uint8)
    cg.add(var.set_temperature(template_))

    template_ = await cg.templatable(config[CONF_HEATING_MODE], args, cg.uint16)
    cg.add(var.set_heating_mode(template_))

    return var


@automation.register_action(
    "truma_inetbox.heater.set_target_water_temperature",
    HeaterWaterTempAction,
    automation.maybe_conf(
        CONF_TEMPERATURE,
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_TEMPERATURE): cv.templatable(cv.int_range(min=0, max=80)),
        }
    ),
)
async def truma_inetbox_heater_set_target_water_temperature_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_TEMPERATURE], args, cg.uint8)
    cg.add(var.set_temperature(template_))

    return var


@automation.register_action(
    "truma_inetbox.heater.set_target_water_temperature_enum",
    HeaterWaterTempEnumAction,
    automation.maybe_conf(
        CONF_TEMPERATURE,
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_TEMPERATURE): cv.templatable(cv.enum(CONF_SUPPORTED_WATER_TEMPERATURE, upper=True))
        }
    ),
)
async def truma_inetbox_heater_set_target_water_temperature_enum_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_TEMPERATURE], args, cg.uint16)
    cg.add(var.set_temperature(template_))

    return var


@automation.register_action(
    "truma_inetbox.heater.set_electric_power_level",
    HeaterElecPowerLevelAction,
    automation.maybe_conf(
        CONF_WATT,
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_WATT): cv.templatable(cv.int_range(min=0, max=1800))
        }
    ),
)
async def truma_inetbox_heater_set_electric_power_level_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_WATT], args, cg.uint16)
    cg.add(var.set_watt(template_))

    return var


@automation.register_action(
    "truma_inetbox.heater.set_energy_mix",
    HeaterEnergyMixAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_ENERGY_MIX): cv.templatable(cv.enum(CONF_SUPPORTED_ENERGY_MIX, upper=True)),
            cv.Optional(CONF_WATT, 0): cv.templatable(cv.enum(CONF_SUPPORTED_ELECTRIC_POWER_LEVEL, upper=True)),
        }
    ),
)
async def truma_inetbox_heater_set_energy_mix_level_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_ENERGY_MIX], args, cg.uint8)
    cg.add(var.set_energy_mix(template_))

    template_ = await cg.templatable(config[CONF_WATT], args, cg.uint16)
    cg.add(var.set_watt(template_))

    return var


@automation.register_action(
    "truma_inetbox.aircon.manual.set_target_temperature",
    AirconManualTempAction,
    automation.maybe_conf(
        CONF_TEMPERATURE,
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_TEMPERATURE): cv.templatable(cv.int_range(min=0, max=31)),
        }
    ),
)
async def truma_inetbox_aircon_manual_set_target_temperature_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_TEMPERATURE], args, cg.uint8)
    cg.add(var.set_temperature(template_))

    return var


@automation.register_action(
    "truma_inetbox.timer.disable",
    TimerDisableAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
        }
    ),
)
async def truma_inetbox_timer_disable_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "truma_inetbox.timer.activate",
    TimerActivateAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_START): cv.templatable(cv.int_range(min=0, max=1440)),
            cv.Required(CONF_STOP): cv.templatable(cv.int_range(min=0, max=1440)),
            cv.Required(CONF_ROOM_TEMPERATURE): cv.templatable(cv.int_range(min=0, max=30)),
            cv.Optional(CONF_HEATING_MODE, "OFF"): cv.templatable(cv.enum(CONF_SUPPORTED_HEATING_MODE, upper=True)),
            cv.Optional(CONF_WATER_TEMPERATURE, 0): cv.templatable(cv.int_range(min=0, max=80)),
            cv.Optional(CONF_ENERGY_MIX, "NONE"): cv.templatable(cv.enum(CONF_SUPPORTED_ENERGY_MIX, upper=True)),
            cv.Optional(CONF_WATT, 0): cv.templatable(cv.enum(CONF_SUPPORTED_ELECTRIC_POWER_LEVEL, upper=True)),

        }
    ),
)
async def truma_inetbox_timer_activate_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    template_ = await cg.templatable(config[CONF_START], args, cg.uint16)
    cg.add(var.set_start(template_))

    template_ = await cg.templatable(config[CONF_STOP], args, cg.uint16)
    cg.add(var.set_stop(template_))

    template_ = await cg.templatable(config[CONF_ROOM_TEMPERATURE], args, cg.uint8)
    cg.add(var.set_room_temperature(template_))

    template_ = await cg.templatable(config[CONF_HEATING_MODE], args, cg.uint16)
    cg.add(var.set_heating_mode(template_))

    template_ = await cg.templatable(config[CONF_WATER_TEMPERATURE], args, cg.uint8)
    cg.add(var.set_water_temperature(template_))

    template_ = await cg.templatable(config[CONF_ENERGY_MIX], args, cg.uint8)
    cg.add(var.set_energy_mix(template_))

    template_ = await cg.templatable(config[CONF_WATT], args, cg.uint16)
    cg.add(var.set_watt(template_))
    return var


@automation.register_action(
    "truma_inetbox.clock.set",
    WriteTimeAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(TrumaINetBoxApp),
        },
        cv.requires_component(CONF_TIME),
    ),
)
async def truma_inetbox_clock_set_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
