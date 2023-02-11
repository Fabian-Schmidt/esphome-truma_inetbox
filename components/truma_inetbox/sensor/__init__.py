from esphome.components import sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    DEVICE_CLASS_TEMPERATURE,
    CONF_UNIT_OF_MEASUREMENT,
    UNIT_CELSIUS,
    CONF_ICON,
    ICON_THERMOMETER,
    STATE_CLASS_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    CONF_DEVICE_CLASS,
    UNIT_WATT,
    UNIT_EMPTY,
    ICON_GAS_CYLINDER,
    ICON_POWER,
)
from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

TrumaSensor = truma_inetbox_ns.class_(
    "TrumaSensor", sensor.Sensor, cg.Component)

# `TRUMA_SENSOR_TYPE` is a enum class and not a namespace but it works.
TRUMA_SENSOR_TYPE_dummy_ns = truma_inetbox_ns.namespace("TRUMA_SENSOR_TYPE")

# 0 - C++ enum
# 1 - CONF_UNIT_OF_MEASUREMENT
# 2 - CONF_ICON
# 3 - CONF_ACCURACY_DECIMALS
# 4 - CONF_DEVICE_CLASS
CONF_SUPPORTED_TYPE = {
    "CURRENT_ROOM_TEMPERATURE": (TRUMA_SENSOR_TYPE_dummy_ns.CURRENT_ROOM_TEMPERATURE, UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    "CURRENT_WATER_TEMPERATURE": (TRUMA_SENSOR_TYPE_dummy_ns.CURRENT_WATER_TEMPERATURE, UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE),
    "TARGET_ROOM_TEMPERATURE": (TRUMA_SENSOR_TYPE_dummy_ns.TARGET_ROOM_TEMPERATURE, UNIT_CELSIUS, ICON_THERMOMETER, 0, DEVICE_CLASS_TEMPERATURE),
    "TARGET_WATER_TEMPERATURE": (TRUMA_SENSOR_TYPE_dummy_ns.TARGET_WATER_TEMPERATURE, UNIT_CELSIUS, ICON_THERMOMETER, 0, DEVICE_CLASS_TEMPERATURE),
    "HEATING_MODE": (TRUMA_SENSOR_TYPE_dummy_ns.HEATING_MODE, UNIT_EMPTY, ICON_THERMOMETER, 0, DEVICE_CLASS_TEMPERATURE),
    "ELECTRIC_POWER_LEVEL": (TRUMA_SENSOR_TYPE_dummy_ns.ELECTRIC_POWER_LEVEL, UNIT_WATT, ICON_POWER, 0, DEVICE_CLASS_TEMPERATURE),
    "ENERGY_MIX": (TRUMA_SENSOR_TYPE_dummy_ns.ENERGY_MIX, UNIT_EMPTY, ICON_GAS_CYLINDER, 0, DEVICE_CLASS_TEMPERATURE),
    "OPERATING_STATUS": (TRUMA_SENSOR_TYPE_dummy_ns.OPERATING_STATUS, UNIT_EMPTY, ICON_POWER, 0, DEVICE_CLASS_TEMPERATURE),
}


def set_default_based_on_type():
    def set_defaults_(config):
        # set defaults based on sensor type:
        if CONF_UNIT_OF_MEASUREMENT not in config:
            config[CONF_UNIT_OF_MEASUREMENT] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][1]
        if CONF_ICON not in config:
            config[CONF_ICON] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][2]
        if CONF_ACCURACY_DECIMALS not in config:
            config[CONF_ACCURACY_DECIMALS] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][3]
        if CONF_DEVICE_CLASS not in config:
            config[CONF_DEVICE_CLASS] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][4]
        return config

    return set_defaults_


CONFIG_SCHEMA = sensor.sensor_schema(
    state_class=STATE_CLASS_MEASUREMENT
).extend(
    {
        cv.GenerateID(): cv.declare_id(TrumaSensor),
        cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
        cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][0]))
