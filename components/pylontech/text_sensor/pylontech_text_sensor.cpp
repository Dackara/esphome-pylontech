#include "pylontech_text_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pylontech {

static const char *const TAG = "pylontech.textsensor";

PylontechTextSensor::PylontechTextSensor(int8_t bat_num) { this->bat_num_ = bat_num; }

void PylontechTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech Text Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);
  LOG_TEXT_SENSOR("  ", "Base state", this->base_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Voltage state", this->voltage_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Current state", this->current_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Temperature state", this->temperature_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Date state", this->date_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Time state", this->time_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "B.V. state", this->bv_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "B.T. state", this->bt_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "MOS state", this->mos_state_text_sensor_);
}

void PylontechTextSensor::on_line_read(PylontechListener::LineContents *line) {
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->base_state_text_sensor_ != nullptr) {
    this->base_state_text_sensor_->publish_state(std::string(line->base_st));
  }
  if (this->voltage_state_text_sensor_ != nullptr) {
    this->voltage_state_text_sensor_->publish_state(std::string(line->volt_st));
  }
  if (this->current_state_text_sensor_ != nullptr) {
    this->current_state_text_sensor_->publish_state(std::string(line->curr_st));
  }
  if (this->temperature_state_text_sensor_ != nullptr) {
    this->temperature_state_text_sensor_->publish_state(std::string(line->temp_st));
  }
  if (this->date_state_text_sensor_ != nullptr) {
     this->date_state_text_sensor_->publish_state(std::string(line->year)+"-"+(line->month)+"-"+(line->day));
  }
  if (this->time_state_text_sensor_ != nullptr) {
      this->time_state_text_sensor_->publish_state(std::string(line->hour)+":"+(line->minute)+":"+(line->second));
  }
  if (this->bv_state_text_sensor_ != nullptr) {
    this->bv_state_text_sensor_->publish_state(std::string(line->bv_st));
  }
  if (this->bt_state_text_sensor_ != nullptr) {
    this->bt_state_text_sensor_->publish_state(std::string(line->bt_st));
  }
  if (this->mos_state_text_sensor_ != nullptr) {
    this->mos_state_text_sensor_->publish_state(std::string(line->mos_st));
  }
}

}  // namespace pylontech
}  // namespace esphome
