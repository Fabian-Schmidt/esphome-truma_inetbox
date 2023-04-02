from esphome.components import sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    DEVICE_CLASS_POWER,
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

CONF_CLASS = "class"

TrumaSensor = truma_inetbox_ns.class_(
    "TrumaSensor", sensor.Sensor, cg.Component)

# `TRUMA_SENSOR_TYPE` is a enum class and not a namespace but it works.
TRUMA_SENSOR_TYPE_dummy_ns = truma_inetbox_ns.namespace("TRUMA_SENSOR_TYPE")

CONF_SUPPORTED_TYPE = {
    "CURRENT_ROOM_TEMPERATURE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.CURRENT_ROOM_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_ACCURACY_DECIMALS: 1,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
    },
    "CURRENT_WATER_TEMPERATURE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.CURRENT_WATER_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
    },
    "TARGET_ROOM_TEMPERATURE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.TARGET_ROOM_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
    },
    "TARGET_WATER_TEMPERATURE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.TARGET_WATER_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
    },
    "HEATING_MODE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.HEATING_MODE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_EMPTY,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "ELECTRIC_POWER_LEVEL": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.ELECTRIC_POWER_LEVEL,
        CONF_UNIT_OF_MEASUREMENT: UNIT_WATT,
        CONF_ICON: ICON_POWER,
        CONF_ACCURACY_DECIMALS: 0,
        CONF_DEVICE_CLASS: DEVICE_CLASS_POWER,
    },
    "ENERGY_MIX": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.ENERGY_MIX,
        CONF_UNIT_OF_MEASUREMENT: UNIT_EMPTY,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "OPERATING_STATUS": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.OPERATING_STATUS,
        CONF_UNIT_OF_MEASUREMENT: UNIT_EMPTY,
        CONF_ACCURACY_DECIMALS: 0,
    },
    "HEATER_ERROR_CODE": {
        CONF_CLASS: TRUMA_SENSOR_TYPE_dummy_ns.HEATER_ERROR_CODE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_EMPTY,
        CONF_ACCURACY_DECIMALS: 0,
    },
}


def set_default_based_on_type():
    def set_defaults_(config):
        sensor_type = CONF_SUPPORTED_TYPE[config[CONF_TYPE]]
        # set defaults based on sensor type:
        if CONF_UNIT_OF_MEASUREMENT in sensor_type and CONF_UNIT_OF_MEASUREMENT not in config:
            config[CONF_UNIT_OF_MEASUREMENT] = sensor_type[CONF_UNIT_OF_MEASUREMENT]
        if CONF_ICON in sensor_type and CONF_ICON not in config:
            config[CONF_ICON] = sensor_type[CONF_ICON]
        if CONF_ACCURACY_DECIMALS in sensor_type and CONF_ACCURACY_DECIMALS not in config:
            config[CONF_ACCURACY_DECIMALS] = sensor_type[CONF_ACCURACY_DECIMALS]
        if CONF_DEVICE_CLASS in sensor_type and CONF_DEVICE_CLASS not in config:
            config[CONF_DEVICE_CLASS] = sensor_type[CONF_DEVICE_CLASS]
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

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_CLASS]))
