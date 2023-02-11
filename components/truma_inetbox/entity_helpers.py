import esphome.final_validate as fv
from esphome import core
from esphome.const import CONF_ID

def count_id_usage(property_to_update, property_to_count, property_value):
    """Validator that counts a configuration property from another entity, for use with FINAL_VALIDATE_SCHEMA.
    If a property is already set, it will not be updated.
    """

    def _walk_config(config, path):
        walk = [path] if not isinstance(path, list) else path
        for item_or_index in walk:
            config = config[item_or_index]
        return config

    def _count_config_value(config, conf_name_list, conf_value):
        ret = 0

        if isinstance(config, (list, tuple)):
            for config_item in config:
                ret += _count_config_value(config_item,
                                           conf_name_list, conf_value)
            for conf_name in conf_name_list:
                if conf_name in config and isinstance(config, (tuple)) and config[0] == conf_name and isinstance(config[1], (core.ID)) and config[1].type is conf_value:
                    ret += 1
        elif isinstance(config, (dict)):
            for config_item in config.items():
                ret += _count_config_value(config_item,
                                           conf_name_list, conf_value)

        return ret

    def inherit_property(config):
        # Ensure `property_to_count` is a list
        property_to_count_list = [property_to_count] if not isinstance(
            property_to_count, list) else property_to_count

        # Split the property into its path and name
        if not isinstance(property_to_update, list):
            property_path, property = [], property_to_update
        else:
            property_path, property = property_to_update[:-
                                                         1], property_to_update[-1]

        # Check if the property is accessible
        try:
            config_part = _walk_config(config, property_path)
        except KeyError:
            return config

        # Only update the property if it does not exist yet
        if property not in config_part:
            fconf = fv.full_config.get()

            count = _count_config_value(
                fconf, property_to_count_list, property_value)

            path = fconf.get_path_for_id(config[CONF_ID])[:-1]
            this_config = _walk_config(
                fconf.get_config_for_path(path), property_path
            )
            this_config[property] = count
        return config

    return inherit_property
