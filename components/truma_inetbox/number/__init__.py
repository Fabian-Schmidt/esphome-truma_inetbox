from esphome.components import number
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
    CONF_DEVICE_CLASS,
    UNIT_WATT,
    ICON_POWER,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_STEP,
)
from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

CONF_CLASS = "class"

TrumaNumber = truma_inetbox_ns.class_(
    "TrumaNumber", number.Number, cg.Component)

# `TRUMA_NUMBER_TYPE` is a enum class and not a namespace but it works.
TRUMA_NUMBER_TYPE_dummy_ns = truma_inetbox_ns.namespace("TRUMA_NUMBER_TYPE")

CONF_SUPPORTED_TYPE = {
    "TARGET_ROOM_TEMPERATURE": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterNumber", number.Number, cg.Component),
        CONF_TYPE: TRUMA_NUMBER_TYPE_dummy_ns.TARGET_ROOM_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
        CONF_MAX_VALUE: 30,
        # Values between 0 and 5 are handeld as off.
        CONF_MIN_VALUE: 0,
        CONF_STEP: 1,
    },
    "TARGET_WATER_TEMPERATURE": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterNumber", number.Number, cg.Component),
        CONF_TYPE: TRUMA_NUMBER_TYPE_dummy_ns.TARGET_WATER_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
        CONF_MAX_VALUE: 80,
        # Values between 0 and 40 are handeld as off.
        CONF_MIN_VALUE: 0,
        CONF_STEP: 20,
    },
    "ELECTRIC_POWER_LEVEL": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterNumber", number.Number, cg.Component),
        CONF_TYPE: TRUMA_NUMBER_TYPE_dummy_ns.ELECTRIC_POWER_LEVEL,
        CONF_UNIT_OF_MEASUREMENT: UNIT_WATT,
        CONF_ICON: ICON_POWER,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
        CONF_MAX_VALUE: 1800,
        CONF_MIN_VALUE: 0,
        CONF_STEP: 900,
    },

    "AIRCON_MANUAL_TEMPERATURE": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaAirconManualNumber", number.Number, cg.Component),
        CONF_TYPE: TRUMA_NUMBER_TYPE_dummy_ns.AIRCON_MANUAL_TEMPERATURE,
        CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
        CONF_ICON: ICON_THERMOMETER,
        CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
        CONF_MAX_VALUE: 31,
        # Values between 0 and 16 are handeld as off.
        CONF_MIN_VALUE: 15,
        CONF_STEP: 1,
    },
}


def set_default_based_on_type():
    def set_defaults_(config):
        # update the class
        config[CONF_ID].type = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_CLASS]
        # set defaults based on sensor type:
        if CONF_UNIT_OF_MEASUREMENT not in config:
            config[CONF_UNIT_OF_MEASUREMENT] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]
                                                                   ][CONF_UNIT_OF_MEASUREMENT]
        if CONF_ICON not in config:
            config[CONF_ICON] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_ICON]
        if CONF_DEVICE_CLASS not in config:
            config[CONF_DEVICE_CLASS] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]
                                                            ][CONF_DEVICE_CLASS]
        if CONF_MAX_VALUE not in config:
            config[CONF_MAX_VALUE] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]
                                                         ][CONF_MAX_VALUE]
        if CONF_MIN_VALUE not in config:
            config[CONF_MIN_VALUE] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]
                                                         ][CONF_MIN_VALUE]
        if CONF_STEP not in config:
            config[CONF_STEP] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_STEP]
        return config

    return set_defaults_


CONFIG_SCHEMA = number.NUMBER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(TrumaNumber),
        cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
        cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
        cv.Optional(CONF_MAX_VALUE): cv.float_,
        cv.Optional(CONF_MIN_VALUE): cv.float_,
        cv.Optional(CONF_STEP): cv.positive_float,
    }
).extend(cv.COMPONENT_SCHEMA)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await number.register_number(
        var,
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_TYPE]))
