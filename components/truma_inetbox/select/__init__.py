from esphome.components import select
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_ICON,
    CONF_OPTIONS,
    ICON_THERMOMETER,
)
from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

CONF_CLASS = "class"

TrumaSelect = truma_inetbox_ns.class_(
    "TrumaSelect", select.Select, cg.Component)

# `TRUMA_SELECT_TYPE` is a enum class and not a namespace but it works.
TRUMA_SELECT_TYPE_dummy_ns = truma_inetbox_ns.namespace("TRUMA_SELECT_TYPE")

CONF_SUPPORTED_TYPE = {
    "HEATER_FAN_MODE_COMBI": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterSelect", select.Select, cg.Component),
        CONF_TYPE: TRUMA_SELECT_TYPE_dummy_ns.HEATER_FAN_MODE,
        CONF_ICON: ICON_THERMOMETER,
        CONF_OPTIONS: ("Off", "Eco", "High", "Boost"),
    },
    "HEATER_FAN_MODE_VARIO_HEAT": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterSelect", select.Select, cg.Component),
        CONF_TYPE: TRUMA_SELECT_TYPE_dummy_ns.HEATER_FAN_MODE,
        CONF_ICON: ICON_THERMOMETER,
        CONF_OPTIONS: ("Off", "Night", "Auto", "Boost"),
    },
    "HEATER_ENERGY_MIX_GAS": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterSelect", select.Select, cg.Component),
        CONF_TYPE: TRUMA_SELECT_TYPE_dummy_ns.HEATER_ENERGY_MIX,
        CONF_ICON: ICON_THERMOMETER,
        CONF_OPTIONS: ("Gas", "Mix 1", "Mix 2", "Electric 1", "Electric 2"),
    },
    "HEATER_ENERGY_MIX_DIESEL": {
        CONF_CLASS: truma_inetbox_ns.class_("TrumaHeaterSelect", select.Select, cg.Component),
        CONF_TYPE: TRUMA_SELECT_TYPE_dummy_ns.HEATER_ENERGY_MIX,
        CONF_ICON: ICON_THERMOMETER,
        CONF_OPTIONS: ("Diesel", "Mix 1", "Mix 2", "Electric 1", "Electric 2"),
    },
}


def set_default_based_on_type():
    def set_defaults_(config):
        # update the class
        config[CONF_ID].type = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_CLASS]
        # set defaults based on sensor type:
        if CONF_ICON not in config:
            config[CONF_ICON] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_ICON]
        if CONF_OPTIONS not in config:
            config[CONF_OPTIONS] = CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_OPTIONS]
        return config

    return set_defaults_


CONFIG_SCHEMA = select.SELECT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(TrumaSelect),
        cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
        cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
        cv.Optional(CONF_OPTIONS): cv.All(
            cv.ensure_list(cv.string_strict), cv.Length(min=1)
        ),
    }
).extend(cv.COMPONENT_SCHEMA)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await select.register_select(
        var,
        config,
        options=config[CONF_OPTIONS]
    )
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_TYPE]))
