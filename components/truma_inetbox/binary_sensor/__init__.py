from esphome.components import binary_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_UPDATE_INTERVAL
)
from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

TrumaSensor = truma_inetbox_ns.class_(
    "TrumaBinarySensor", binary_sensor.BinarySensor, cg.Component)

# `TRUMA_BINARY_SENSOR_TYPE` is a enum class and not a namespace but it works.
TRUMA_BINARY_SENSOR_TYPE_dummy_ns = truma_inetbox_ns.namespace(
    "TRUMA_BINARY_SENSOR_TYPE")

# 0 - C++ class
# 1 - C++ enum
CONF_SUPPORTED_TYPE = {
    # TrumaCpPlusBinarySensor
    "CP_PLUS_CONNECTED": (truma_inetbox_ns.class_("TrumaCpPlusBinarySensor", binary_sensor.BinarySensor, cg.PollingComponent), None),
    # TrumaHeaterBinarySensor
    "HEATER_ROOM": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_ROOM),
    "HEATER_WATER": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_WATER),
    "HEATER_GAS": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_GAS),
    "HEATER_DIESEL": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_DIESEL),
    "HEATER_MIX_1": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_MIX_1),
    "HEATER_MIX_2": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_MIX_2),
    "HEATER_ELECTRICITY": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_ELECTRICITY),
    "HEATER_HAS_ERROR": (truma_inetbox_ns.class_("TrumaHeaterBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.HEATER_HAS_ERROR),
    # TrumaTimerBinarySensor
    "TIMER_ACTIVE": (truma_inetbox_ns.class_("TrumaTimerBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.TIMER_ACTIVE),
    "TIMER_ROOM": (truma_inetbox_ns.class_("TrumaTimerBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.TIMER_ROOM),
    "TIMER_WATER": (truma_inetbox_ns.class_("TrumaTimerBinarySensor", binary_sensor.BinarySensor, cg.Component), TRUMA_BINARY_SENSOR_TYPE_dummy_ns.TIMER_WATER),
}


def set_default_based_on_type():
    def set_defaults_(config):
        # Update type based on configuration
        config[CONF_ID].type = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][0]

        # set defaults based on sensor type:
        if config[CONF_TYPE] == "CP_PLUS_CONNECTED":
            if CONF_UPDATE_INTERVAL not in config:
                config[CONF_UPDATE_INTERVAL] = 500  # 0.5 seconds
        return config

    return set_defaults_


CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(TrumaSensor)
    .extend({
            cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
            cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
            }).extend(cv.COMPONENT_SCHEMA)
)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])

    if CONF_SUPPORTED_TYPE[config[CONF_TYPE]][1]:
        cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][1]))
