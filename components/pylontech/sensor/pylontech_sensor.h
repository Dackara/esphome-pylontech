#pragma once

#include "../pylontech.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace pylontech {

class PylontechSensor : public PylontechListener, public Component {
 public:
  PylontechSensor(int8_t bat_num);
  void dump_config() override;

// pwr sensor
  SUB_SENSOR(voltage)
  SUB_SENSOR(current)
  SUB_SENSOR(temperature)
  SUB_SENSOR(temperature_low)
  SUB_SENSOR(temperature_high)
  SUB_SENSOR(voltage_low)
  SUB_SENSOR(voltage_high)
  SUB_SENSOR(coulomb)
  SUB_SENSOR(mos_temperature)

// pwrsys sensor
  SUB_SENSOR(value_total_num)
  SUB_SENSOR(value_present_num)
  SUB_SENSOR(value_sleep_num)
  SUB_SENSOR(value_system_volt)
  SUB_SENSOR(value_system_curr)
  SUB_SENSOR(value_system_rc)
  SUB_SENSOR(value_system_fcc)
  SUB_SENSOR(value_system_soc)
  SUB_SENSOR(value_system_soh)
  SUB_SENSOR(value_highest_voltage)
  SUB_SENSOR(value_average_voltage)
  SUB_SENSOR(value_lowest_voltage)
  SUB_SENSOR(value_highest_temperature)
  SUB_SENSOR(value_average_temperature)
  SUB_SENSOR(value_lowest_temperature)
  SUB_SENSOR(value_recommend_chg_voltage)
  SUB_SENSOR(value_recommend_dsg_voltage)
  SUB_SENSOR(value_recommend_chg_current)
  SUB_SENSOR(value_recommend_dsg_current)
  SUB_SENSOR(value_system_recommend_chg_voltage)
  SUB_SENSOR(value_system_recommend_dsg_voltage)
  SUB_SENSOR(value_system_recommend_chg_current)
  SUB_SENSOR(value_system_recommend_dsg_current)

  void on_line_read(LineContents *line) override;

 protected:
  int8_t bat_num_;
};

}  // namespace pylontech
}  // namespace esphome
