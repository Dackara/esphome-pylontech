#include "pylontech_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pylontech {

static const char *const TAG = "pylontech.sensor";

PylontechSensor::PylontechSensor(int8_t bat_num) { this->bat_num_ = bat_num; }

void PylontechSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Temperature low", this->temperature_low_sensor_);
  LOG_SENSOR("  ", "Temperature high", this->temperature_high_sensor_);
  LOG_SENSOR("  ", "Voltage low", this->voltage_low_sensor_);
  LOG_SENSOR("  ", "Voltage high", this->voltage_high_sensor_);
  LOG_SENSOR("  ", "Coulomb", this->coulomb_sensor_);
  LOG_SENSOR("  ", "MOS Temperature", this->mos_temperature_sensor_);
}

void PylontechSensor::on_line_read(PylontechListener::LineContents *line) {
  // pwr sensor
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->voltage_sensor_ != nullptr) {
    this->voltage_sensor_->publish_state(((float) line->volt) / 1000.0f);
  }
  if (this->current_sensor_ != nullptr) {
    this->current_sensor_->publish_state(((float) line->curr) / 1000.0f);
  }
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(((float) line->tempr) / 1000.0f);
  }
  if (this->temperature_low_sensor_ != nullptr) {
    this->temperature_low_sensor_->publish_state(((float) line->tlow) / 1000.0f);
  }
  if (this->temperature_high_sensor_ != nullptr) {
    this->temperature_high_sensor_->publish_state(((float) line->thigh) / 1000.0f);
  }
  if (this->voltage_low_sensor_ != nullptr) {
    this->voltage_low_sensor_->publish_state(((float) line->vlow) / 1000.0f);
  }
  if (this->voltage_high_sensor_ != nullptr) {
    this->voltage_high_sensor_->publish_state(((float) line->vhigh) / 1000.0f);
  }
  if (this->coulomb_sensor_ != nullptr) {
    this->coulomb_sensor_->publish_state(line->coulomb);
  }
  if (this->mos_temperature_sensor_ != nullptr) {
    this->mos_temperature_sensor_->publish_state(((float) line->mostempr) / 1000.0f);
  }

  //pwrsys sensor
  if (this->value_total_num_sensor_ != nullptr) {
    this->value_total_num_sensor_->publish_state(line->value_total_num);
  }
  if (this->value_present_num_sensor_ != nullptr) {
    this->value_present_num_sensor_->publish_state(line->value_present_num);
  }
  if (this->value_sleep_num_sensor_ != nullptr) {
    this->value_sleep_num_sensor_->publish_state(line->value_sleep_num);
  }
  if (this->value_system_volt_sensor_ != nullptr) {
    this->value_system_volt_sensor_->publish_state(((float) line->value_system_volt) / 1000.0f);
  }
  if (this->value_system_curr_sensor_ != nullptr) {
    this->value_system_curr_sensor_->publish_state(((float) line->value_system_curr) / 1000.0f);
  }
  if (this->value_system_rc_sensor_ != nullptr) {
    this->value_system_rc_sensor_->publish_state(((float) line->value_system_rc) / 1000.0f);
  }
  if (this->value_system_fcc_sensor_ != nullptr) {
    this->value_system_fcc_sensor_->publish_state(((float) line->value_system_fcc) / 1000.0f);
  }
  if (this->value_system_soc_sensor_ != nullptr) {
    this->value_system_soc_sensor_->publish_state(line->value_system_soc);
  }
  if (this->value_system_soh_sensor_ != nullptr) {
    this->value_system_soh_sensor_->publish_state(line->value_system_soh);
  }
  if (this->value_highest_voltage_sensor_ != nullptr) {
    this->value_highest_voltage_sensor_->publish_state(((float) line->value_highest_voltage) / 1000.0f);
  }
  if (this->value_highest_voltage_sensor_ != nullptr) {
    this->value_highest_voltage_sensor_->publish_state(((float) line->value_highest_voltage) / 1000.0f);
  }
  if (this->value_average_voltage_sensor_ != nullptr) {
    this->value_average_voltage_sensor_->publish_state(((float) line->value_average_voltage) / 1000.0f);
  }
  if (this->value_lowest_voltage_sensor_ != nullptr) {
    this->value_lowest_voltage_sensor_->publish_state(((float) line->value_lowest_voltage) / 1000.0f);
  }
  if (this->value_highest_temperature_sensor_ != nullptr) {
    this->value_highest_temperature_sensor_->publish_state(((float) line->value_highest_temperature) / 1000.0f);
  }
  if (this->value_average_temperature_sensor_ != nullptr) {
    this->value_average_temperature_sensor_->publish_state(((float) line->value_average_temperature) / 1000.0f);
  }
  if (this->value_lowest_temperature_sensor_ != nullptr) {
    this->value_lowest_temperature_sensor_->publish_state(((float) line->value_lowest_temperature) / 1000.0f);
  }
  if (this->value_recommend_chg_voltage_sensor_ != nullptr) {
    this->value_recommend_chg_voltage_sensor_->publish_state(((float) line->value_recommend_chg_voltage) / 1000.0f);
  }
  if (this->value_recommend_dsg_voltage_sensor_ != nullptr) {
    this->value_recommend_dsg_voltage_sensor_->publish_state(((float) line->value_recommend_dsg_voltage) / 1000.0f);
  }
  if (this->value_recommend_chg_current_sensor_ != nullptr) {
    this->value_recommend_chg_current_sensor_->publish_state(((float) line->value_recommend_chg_current) / 1000.0f);
  }
  if (this->value_recommend_dsg_current_sensor_ != nullptr) {
    this->value_recommend_dsg_current_sensor_->publish_state(((float) line->value_recommend_dsg_current) / 1000.0f);
  }
  if (this->value_system_recommend_chg_voltage_sensor_ != nullptr) {
    this->value_system_recommend_chg_voltage_sensor_->publish_state(((float) line->value_system_recommend_chg_voltage) / 1000.0f);
  }
  if (this->value_system_recommend_dsg_voltage_sensor_ != nullptr) {
    this->value_system_recommend_dsg_voltage_sensor_->publish_state(((float) line->value_system_recommend_dsg_voltage) / 1000.0f);
  }
  if (this->value_system_recommend_chg_current_sensor_ != nullptr) {
    this->value_system_recommend_chg_current_sensor_->publish_state(((float) line->value_system_recommend_chg_current) / 1000.0f);
  }
  if (this->value_system_recommend_dsg_current_sensor_ != nullptr) {
    this->value_system_recommend_dsg_current_sensor_->publish_state(((float) line->value_system_recommend_dsg_current) / 1000.0f);
  }
}

}  // namespace pylontech
}  // namespace esphome
