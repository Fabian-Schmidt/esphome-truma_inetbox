from esphome.components import climate
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
)
from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

TrumaClimate = truma_inetbox_ns.class_(
    "TrumaClimate", climate.Climate, cg.Component)

CONF_SUPPORTED_TYPE = {
    "ROOM": truma_inetbox_ns.class_("TrumaRoomClimate", climate.Climate, cg.Component),
    "WATER": truma_inetbox_ns.class_("TrumaWaterClimate", climate.Climate, cg.Component),
}


def set_default_based_on_type():
    def set_defaults_(config):
        # update the class
        config[CONF_ID].type = CONF_SUPPORTED_TYPE[config[CONF_TYPE]]
        return config

    return set_defaults_


CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(TrumaClimate),
        cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
        cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)
FINAL_VALIDATE_SCHEMA = set_default_based_on_type()


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])
