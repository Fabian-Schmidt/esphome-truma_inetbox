#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

u_int8_t addr_parity(const u_int8_t PID) {
  u_int8_t P0 = ((PID >> 0) + (PID >> 1) + (PID >> 2) + (PID >> 4)) & 1;
  u_int8_t P1 = ~((PID >> 1) + (PID >> 3) + (PID >> 4) + (PID >> 5)) & 1;
  return (P0 | (P1 << 1));
}

// sum = 0 LIN 1.X CRC, sum = PID LIN 2.X CRC Enhanced
u_int8_t data_checksum(const u_int8_t *message, u_int8_t length, uint16_t sum) {
  for (u_int8_t i = 0; i < length; i++) {
    sum += message[i];

    if (sum >= 256)
      sum -= 255;
  }
  return (~sum);
}

float temp_code_to_decimal(u_int16_t val, float zero) {
  if (val == 0) {
    return zero;
  }
  return ((float) val) / 10.0f - 273.0f;
}

float water_temp_200_fix(float val) {
  if (val == 200) {
    return 80;
  }
  return val;
}

float temp_code_to_decimal(TargetTemp val, float zero) { return temp_code_to_decimal((u_int16_t) val, zero); }

TargetTemp decimal_to_temp(u_int8_t val) { return (TargetTemp) ((((u_int16_t) val) + 273) * 10); }

TargetTemp decimal_to_temp(float val) { return (TargetTemp) ((val + 273) * 10); }

TargetTemp decimal_to_room_temp(u_int8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 5) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 30) {
    return TargetTemp::TARGET_TEMP_ROOM_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_room_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 5) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 30) {
    return TargetTemp::TARGET_TEMP_ROOM_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_aircon_manual_temp(u_int8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_aircon_manual_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_aircon_auto_temp(u_int8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_aircon_auto_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_water_temp(u_int8_t val) {
  if (val < 40) {
    return TargetTemp::TARGET_TEMP_OFF;
  } else if (val >= 40 && val < 60) {
    return TargetTemp::TARGET_TEMP_WATER_ECO;
  } else if (val >= 60 && val < 80) {
    return TargetTemp::TARGET_TEMP_WATER_HIGH;
  } else {
    return TargetTemp::TARGET_TEMP_WATER_BOOST;
  }
}

TargetTemp decimal_to_water_temp(float val) {
  if (std::isnan(val) || val < 40) {
    return TargetTemp::TARGET_TEMP_OFF;
  } else if (val >= 40 && val < 60) {
    return TargetTemp::TARGET_TEMP_WATER_ECO;
  } else if (val >= 60 && val < 80) {
    return TargetTemp::TARGET_TEMP_WATER_HIGH;
  } else {
    return TargetTemp::TARGET_TEMP_WATER_BOOST;
  }
}

const std::string operating_status_to_str(OperatingStatus val) {
  if (val == OperatingStatus::OPERATING_STATUS_OFF) {
    return "OFF";
  } else if (val == OperatingStatus::OPERATING_STATUS_WARNING) {
    return "WARNING";
  } else if (val == OperatingStatus::OPERATING_STATUS_START_OR_COOL_DOWN) {
    return "START/COOL DOWN";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_5) {
    return "ON (5)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_6) {
    return "ON (6)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_7) {
    return "ON (7)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_8) {
    return "ON (8)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_9) {
    return "ON (9)";
  } else {
    return esphome::str_snprintf("ON %u", 6, (uint8_t) val);
  }
}

ElectricPowerLevel decimal_to_el_power_level(u_int16_t val) {
  if (val >= 1800) {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800;
  } else if (val >= 900) {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900;
  } else {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0;
  }
}

}  // namespace truma_inetbox
}  // namespace esphome