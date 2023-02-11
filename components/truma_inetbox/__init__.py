import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.components import sensor, uart, time
from esphome.const import (
    CONF_ID,
    CONF_CS_PIN,
    CONF_TEMPERATURE,
    CONF_ON_MESSAGE,
    CONF_TRIGGER_ID,
    CONF_STOP,
    CONF_TIME_ID,
)
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
    automation.Trigger.template(cg.int_, cg.const_char_ptr, cg.const_char_ptr),
)

# `LIN_CHECKSUM` is a enum class and not a namespace but it works.
LIN_CHECKSUM_dummy_ns = truma_inetbox_ns.namespace("LIN_CHECKSUM")

CONF_SUPPORTED_LIN_CHECKSUM = {
    "VERSION_1": LIN_CHECKSUM_dummy_ns.LIN_CHECKSUM_VERSION_1,
    "VERSION_2": LIN_CHECKSUM_dummy_ns.LIN_CHECKSUM_VERSION_2,
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TrumaINetBoxApp),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
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
    cv.only_with_arduino,
    cv.only_on(["esp32"]),
)
FINAL_VALIDATE_SCHEMA = cv.All(
    uart.final_validate_device_schema(
        # TODO: Validate 2 Stop bits are configured.
        "truma_inetbox", baud_rate=9600, require_tx=True, require_rx=True
    ),
    count_id_usage(CONF_NUMBER_OF_CHILDREN, [
                   CONF_TRUMA_INETBOX_ID, CONF_ID], TrumaINetBoxApp),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_NUMBER_OF_CHILDREN])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
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

    template_ = await cg.templatable(config[CONF_HEATING_MODE], args, cg.uint8)
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

    template_ = await cg.templatable(config[CONF_HEATING_MODE], args, cg.uint8)
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
        }
    ),
)
async def truma_inetbox_clock_set_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
