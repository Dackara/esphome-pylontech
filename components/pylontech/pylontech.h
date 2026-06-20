#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/button/button.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"

#include <string>
#include <vector>

namespace esphome {
namespace pylontech {

static const uint8_t PYLONTECH_NUM_BUFFERS = 24;
static const uint16_t PYLONTECH_MAX_LINE_LENGTH = 1024;
static const uint16_t PYLONTECH_MAX_INFO_RESPONSE_LENGTH = 1536;
static const uint16_t PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH = 240;
static const uint16_t PYLONTECH_RESPONSE_LOG_CHUNK_LENGTH = 180;
static const uint16_t PYLONTECH_MAX_BYTES_PER_LOOP = 512;
static const uint8_t PYLONTECH_TEXT_SENSOR_MAX_LEN = 16;
static const uint8_t PYLONTECH_MAX_CELLS = 16;
static const uint8_t PYLONTECH_STAT_FIELD_COUNT = 52;

enum PylontechRole {
  PYLONTECH_ROLE_MASTER = 0,
  PYLONTECH_ROLE_SLAVE,
};

enum PylontechMemoryMode {
  PYLONTECH_MEMORY_INTERNAL = 0,
  PYLONTECH_MEMORY_AUTO,
  PYLONTECH_MEMORY_PSRAM,
};

using PylontechBuffer = std::basic_string<char, std::char_traits<char>, RAMAllocator<char>>;

struct PylontechPowerLine {
  int battery_address{0};
  int voltage_mv{0};
  int current_ma{0};
  int temperature_mc{0};
  int temperature_low_mc{0};
  int temperature_high_mc{0};
  int voltage_low_mv{0};
  int voltage_high_mv{0};
  int soc_percent{0};
  int mos_temperature_mc{0};
  bool has_mos_temperature{false};
  std::string base_state;
  std::string voltage_state;
  std::string current_state;
  std::string temperature_state;
  std::string battery_voltage_state;
  std::string battery_temperature_state;
  std::string mos_state;
};

class PylontechBattery : public Component {
 public:
  explicit PylontechBattery(uint8_t address) : address_(address) {}

  void dump_config() override;
  uint8_t address() const { return this->address_; }
  void set_publish_only_changes(bool enable) { this->publish_only_changes_ = enable; }
  void publish_power_line(const PylontechPowerLine &line);
  void publish_voltage(float voltage) const;
  void publish_current(float current) const;
  void publish_temperature(float temperature) const;
  void publish_soc(float soc) const;
  void publish_total_coulomb(float total_coulomb) const;
  void publish_max_voltage(float max_voltage) const;
  void publish_charge_times(float charge_times) const;
  void publish_discharge_sec(float discharge_sec) const;
  void publish_charge_sec(float charge_sec) const;
  void publish_device_address(float address) const;
  void publish_cell_number(float cell_number) const;
  void publish_max_charge_current(float current) const;
  void publish_max_discharge_current(float current) const;
  void publish_base_state(const std::string &state) const;
  void publish_voltage_state(const std::string &state) const;
  void publish_current_state(const std::string &state) const;
  void publish_temperature_state(const std::string &state) const;
  void publish_coulomb_state(const std::string &state) const;
  void publish_soh_state(const std::string &state) const;
  void publish_heater_state(const std::string &state) const;
  void publish_protect_ena(const std::string &state) const;
  void publish_bat_events(const std::string &state) const;
  void publish_power_events(const std::string &state) const;
  void publish_system_fault(const std::string &state) const;
  void publish_manufacturer(const std::string &value) const;
  void publish_device_name(const std::string &value) const;
  void publish_board(const std::string &value) const;
  void publish_board_version(const std::string &value) const;
  void publish_main_soft_version(const std::string &value) const;
  void publish_soft_version(const std::string &value) const;
  void publish_boot_version(const std::string &value) const;
  void publish_comm_version(const std::string &value) const;
  void publish_release_date(const std::string &value) const;
  void publish_barcode(const std::string &value) const;
  void publish_specification(const std::string &value) const;
  void publish_epon_port_rate(const std::string &value) const;
  void publish_console_port_rate(const std::string &value) const;
  void publish_info_response(const std::string &value) const;
  void publish_getpwr_response(const std::string &value) const;
  void publish_stat_response(const std::string &value) const;
  void publish_soh_response(const std::string &value) const;
  void publish_getpwr_summary(int voltage_mv, int current_ma, int temperature_mc, int residual_ma,
                              const std::string &base_state, const std::string &voltage_state,
                              const std::string &current_state, const std::string &temperature_state) const;
  void publish_getpwr_tail(int tail_1_value, int tail_2_value) const;
  void publish_stat_cycle_count(float value) const;
  void publish_stat_coulomb(float value) const;
  void publish_stat_field(uint8_t field, float value) const;
  void publish_cell_detail(uint8_t cell, int voltage_mv, int temperature_mc, const std::string &voltage_state,
                           const std::string &temperature_state) const;
  void publish_bat_cell_detail(uint8_t cell, int voltage_mv, int current_ma, int temperature_mc, int soc_percent,
                               int coulomb_mah, const std::string &voltage_state,
                               const std::string &temperature_state, const std::string &balance_state) const;
  void publish_soh_cell_detail(uint8_t cell, int voltage_mv, int soh_count, const std::string &soh_status) const;
  void publish_bat_response(const std::string &value) const;
  bool has_info_response_text_sensor() const { return this->info_response_text_sensor_ != nullptr; }
  bool has_getpwr_response_text_sensor() const { return this->getpwr_response_text_sensor_ != nullptr; }
  bool has_stat_response_text_sensor() const { return this->stat_response_text_sensor_ != nullptr; }
  bool has_bat_response_text_sensor() const { return this->bat_response_text_sensor_ != nullptr; }
  bool has_soh_response_text_sensor() const { return this->soh_response_text_sensor_ != nullptr; }

  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }
  void set_temperature_low_sensor(sensor::Sensor *sensor) { this->temperature_low_sensor_ = sensor; }
  void set_temperature_high_sensor(sensor::Sensor *sensor) { this->temperature_high_sensor_ = sensor; }
  void set_cell_voltage_low_sensor(sensor::Sensor *sensor) { this->cell_voltage_low_sensor_ = sensor; }
  void set_cell_voltage_high_sensor(sensor::Sensor *sensor) { this->cell_voltage_high_sensor_ = sensor; }
  void set_soc_sensor(sensor::Sensor *sensor) { this->soc_sensor_ = sensor; }
  void set_total_coulomb_sensor(sensor::Sensor *sensor) { this->total_coulomb_sensor_ = sensor; }
  void set_max_voltage_sensor(sensor::Sensor *sensor) { this->max_voltage_sensor_ = sensor; }
  void set_charge_times_sensor(sensor::Sensor *sensor) { this->charge_times_sensor_ = sensor; }
  void set_discharge_sec_sensor(sensor::Sensor *sensor) { this->discharge_sec_sensor_ = sensor; }
  void set_charge_sec_sensor(sensor::Sensor *sensor) { this->charge_sec_sensor_ = sensor; }
  void set_device_address_sensor(sensor::Sensor *sensor) { this->device_address_sensor_ = sensor; }
  void set_cell_number_sensor(sensor::Sensor *sensor) { this->cell_number_sensor_ = sensor; }
  void set_max_charge_current_sensor(sensor::Sensor *sensor) { this->max_charge_current_sensor_ = sensor; }
  void set_max_discharge_current_sensor(sensor::Sensor *sensor) { this->max_discharge_current_sensor_ = sensor; }
  void set_mos_temperature_sensor(sensor::Sensor *sensor) { this->mos_temperature_sensor_ = sensor; }
  void set_getpwr_voltage_sensor(sensor::Sensor *sensor) { this->getpwr_voltage_sensor_ = sensor; }
  void set_getpwr_current_sensor(sensor::Sensor *sensor) { this->getpwr_current_sensor_ = sensor; }
  void set_getpwr_temperature_sensor(sensor::Sensor *sensor) { this->getpwr_temperature_sensor_ = sensor; }
  void set_getpwr_residual_capacity_sensor(sensor::Sensor *sensor) { this->getpwr_residual_capacity_sensor_ = sensor; }
  void set_getpwr_residual_ma_sensor(sensor::Sensor *sensor) { this->getpwr_residual_ma_sensor_ = sensor; }
  void set_getpwr_tail_1_sensor(sensor::Sensor *sensor) { this->getpwr_tail_1_sensor_ = sensor; }
  void set_getpwr_tail_2_sensor(sensor::Sensor *sensor) { this->getpwr_tail_2_sensor_ = sensor; }
  void set_stat_cycle_count_sensor(sensor::Sensor *sensor) { this->stat_cycle_count_sensor_ = sensor; }
  void set_stat_coulomb_sensor(sensor::Sensor *sensor) { this->stat_coulomb_sensor_ = sensor; }
  void set_stat_field_sensor(uint8_t field, sensor::Sensor *sensor) {
    if (field < PYLONTECH_STAT_FIELD_COUNT)
      this->stat_field_sensors_[field] = sensor;
  }
  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->cell_voltage_sensors_[cell - 1] = sensor;
  }
  void set_cell_temperature_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->cell_temperature_sensors_[cell - 1] = sensor;
  }
  void set_bat_cell_current_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->bat_cell_current_sensors_[cell - 1] = sensor;
  }
  void set_bat_cell_soc_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->bat_cell_soc_sensors_[cell - 1] = sensor;
  }
  void set_bat_cell_coulomb_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->bat_cell_coulomb_sensors_[cell - 1] = sensor;
  }
  void set_soh_cell_count_sensor(uint8_t cell, sensor::Sensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->soh_cell_count_sensors_[cell - 1] = sensor;
  }

  void set_base_state_text_sensor(text_sensor::TextSensor *sensor) { this->base_state_text_sensor_ = sensor; }
  void set_voltage_state_text_sensor(text_sensor::TextSensor *sensor) { this->voltage_state_text_sensor_ = sensor; }
  void set_current_state_text_sensor(text_sensor::TextSensor *sensor) { this->current_state_text_sensor_ = sensor; }
  void set_temperature_state_text_sensor(text_sensor::TextSensor *sensor) { this->temperature_state_text_sensor_ = sensor; }
  void set_coulomb_state_text_sensor(text_sensor::TextSensor *sensor) { this->coulomb_state_text_sensor_ = sensor; }
  void set_soh_state_text_sensor(text_sensor::TextSensor *sensor) { this->soh_state_text_sensor_ = sensor; }
  void set_heater_state_text_sensor(text_sensor::TextSensor *sensor) { this->heater_state_text_sensor_ = sensor; }
  void set_protect_ena_text_sensor(text_sensor::TextSensor *sensor) { this->protect_ena_text_sensor_ = sensor; }
  void set_bat_events_text_sensor(text_sensor::TextSensor *sensor) { this->bat_events_text_sensor_ = sensor; }
  void set_power_events_text_sensor(text_sensor::TextSensor *sensor) { this->power_events_text_sensor_ = sensor; }
  void set_system_fault_text_sensor(text_sensor::TextSensor *sensor) { this->system_fault_text_sensor_ = sensor; }
  void set_manufacturer_text_sensor(text_sensor::TextSensor *sensor) { this->manufacturer_text_sensor_ = sensor; }
  void set_device_name_text_sensor(text_sensor::TextSensor *sensor) { this->device_name_text_sensor_ = sensor; }
  void set_board_text_sensor(text_sensor::TextSensor *sensor) { this->board_text_sensor_ = sensor; }
  void set_board_version_text_sensor(text_sensor::TextSensor *sensor) { this->board_version_text_sensor_ = sensor; }
  void set_main_soft_version_text_sensor(text_sensor::TextSensor *sensor) { this->main_soft_version_text_sensor_ = sensor; }
  void set_soft_version_text_sensor(text_sensor::TextSensor *sensor) { this->soft_version_text_sensor_ = sensor; }
  void set_boot_version_text_sensor(text_sensor::TextSensor *sensor) { this->boot_version_text_sensor_ = sensor; }
  void set_comm_version_text_sensor(text_sensor::TextSensor *sensor) { this->comm_version_text_sensor_ = sensor; }
  void set_release_date_text_sensor(text_sensor::TextSensor *sensor) { this->release_date_text_sensor_ = sensor; }
  void set_barcode_text_sensor(text_sensor::TextSensor *sensor) { this->barcode_text_sensor_ = sensor; }
  void set_specification_text_sensor(text_sensor::TextSensor *sensor) { this->specification_text_sensor_ = sensor; }
  void set_epon_port_rate_text_sensor(text_sensor::TextSensor *sensor) { this->epon_port_rate_text_sensor_ = sensor; }
  void set_console_port_rate_text_sensor(text_sensor::TextSensor *sensor) { this->console_port_rate_text_sensor_ = sensor; }
  void set_info_response_text_sensor(text_sensor::TextSensor *sensor) { this->info_response_text_sensor_ = sensor; }
  void set_getpwr_response_text_sensor(text_sensor::TextSensor *sensor) { this->getpwr_response_text_sensor_ = sensor; }
  void set_stat_response_text_sensor(text_sensor::TextSensor *sensor) { this->stat_response_text_sensor_ = sensor; }
  void set_bat_response_text_sensor(text_sensor::TextSensor *sensor) { this->bat_response_text_sensor_ = sensor; }
  void set_soh_response_text_sensor(text_sensor::TextSensor *sensor) { this->soh_response_text_sensor_ = sensor; }
  void set_battery_voltage_state_text_sensor(text_sensor::TextSensor *sensor) {
    this->battery_voltage_state_text_sensor_ = sensor;
  }
  void set_battery_temperature_state_text_sensor(text_sensor::TextSensor *sensor) {
    this->battery_temperature_state_text_sensor_ = sensor;
  }
  void set_mos_state_text_sensor(text_sensor::TextSensor *sensor) { this->mos_state_text_sensor_ = sensor; }
  void set_cell_voltage_state_text_sensor(uint8_t cell, text_sensor::TextSensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->cell_voltage_state_text_sensors_[cell - 1] = sensor;
  }
  void set_cell_temperature_state_text_sensor(uint8_t cell, text_sensor::TextSensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->cell_temperature_state_text_sensors_[cell - 1] = sensor;
  }
  void set_bat_cell_balance_state_text_sensor(uint8_t cell, text_sensor::TextSensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->bat_cell_balance_state_text_sensors_[cell - 1] = sensor;
  }
  void set_soh_cell_status_text_sensor(uint8_t cell, text_sensor::TextSensor *sensor) {
    if (cell >= 1 && cell <= PYLONTECH_MAX_CELLS)
      this->soh_cell_status_text_sensors_[cell - 1] = sensor;
  }

 protected:
  void publish_sensor_if_changed(sensor::Sensor *sensor, float value) const;

  uint8_t address_;
  bool publish_only_changes_{false};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *temperature_low_sensor_{nullptr};
  sensor::Sensor *temperature_high_sensor_{nullptr};
  sensor::Sensor *cell_voltage_low_sensor_{nullptr};
  sensor::Sensor *cell_voltage_high_sensor_{nullptr};
  sensor::Sensor *soc_sensor_{nullptr};
  sensor::Sensor *total_coulomb_sensor_{nullptr};
  sensor::Sensor *max_voltage_sensor_{nullptr};
  sensor::Sensor *charge_times_sensor_{nullptr};
  sensor::Sensor *discharge_sec_sensor_{nullptr};
  sensor::Sensor *charge_sec_sensor_{nullptr};
  sensor::Sensor *device_address_sensor_{nullptr};
  sensor::Sensor *cell_number_sensor_{nullptr};
  sensor::Sensor *max_charge_current_sensor_{nullptr};
  sensor::Sensor *max_discharge_current_sensor_{nullptr};
  sensor::Sensor *mos_temperature_sensor_{nullptr};
  sensor::Sensor *getpwr_voltage_sensor_{nullptr};
  sensor::Sensor *getpwr_current_sensor_{nullptr};
  sensor::Sensor *getpwr_temperature_sensor_{nullptr};
  sensor::Sensor *getpwr_residual_capacity_sensor_{nullptr};
  sensor::Sensor *getpwr_residual_ma_sensor_{nullptr};
  sensor::Sensor *getpwr_tail_1_sensor_{nullptr};
  sensor::Sensor *getpwr_tail_2_sensor_{nullptr};
  sensor::Sensor *stat_cycle_count_sensor_{nullptr};
  sensor::Sensor *stat_coulomb_sensor_{nullptr};
  sensor::Sensor *stat_field_sensors_[PYLONTECH_STAT_FIELD_COUNT]{};
  sensor::Sensor *cell_voltage_sensors_[PYLONTECH_MAX_CELLS]{};
  sensor::Sensor *cell_temperature_sensors_[PYLONTECH_MAX_CELLS]{};
  sensor::Sensor *bat_cell_current_sensors_[PYLONTECH_MAX_CELLS]{};
  sensor::Sensor *bat_cell_soc_sensors_[PYLONTECH_MAX_CELLS]{};
  sensor::Sensor *bat_cell_coulomb_sensors_[PYLONTECH_MAX_CELLS]{};
  sensor::Sensor *soh_cell_count_sensors_[PYLONTECH_MAX_CELLS]{};

  text_sensor::TextSensor *base_state_text_sensor_{nullptr};
  text_sensor::TextSensor *voltage_state_text_sensor_{nullptr};
  text_sensor::TextSensor *current_state_text_sensor_{nullptr};
  text_sensor::TextSensor *temperature_state_text_sensor_{nullptr};
  text_sensor::TextSensor *coulomb_state_text_sensor_{nullptr};
  text_sensor::TextSensor *soh_state_text_sensor_{nullptr};
  text_sensor::TextSensor *heater_state_text_sensor_{nullptr};
  text_sensor::TextSensor *protect_ena_text_sensor_{nullptr};
  text_sensor::TextSensor *bat_events_text_sensor_{nullptr};
  text_sensor::TextSensor *power_events_text_sensor_{nullptr};
  text_sensor::TextSensor *system_fault_text_sensor_{nullptr};
  text_sensor::TextSensor *manufacturer_text_sensor_{nullptr};
  text_sensor::TextSensor *device_name_text_sensor_{nullptr};
  text_sensor::TextSensor *board_text_sensor_{nullptr};
  text_sensor::TextSensor *board_version_text_sensor_{nullptr};
  text_sensor::TextSensor *main_soft_version_text_sensor_{nullptr};
  text_sensor::TextSensor *soft_version_text_sensor_{nullptr};
  text_sensor::TextSensor *boot_version_text_sensor_{nullptr};
  text_sensor::TextSensor *comm_version_text_sensor_{nullptr};
  text_sensor::TextSensor *release_date_text_sensor_{nullptr};
  text_sensor::TextSensor *barcode_text_sensor_{nullptr};
  text_sensor::TextSensor *specification_text_sensor_{nullptr};
  text_sensor::TextSensor *epon_port_rate_text_sensor_{nullptr};
  text_sensor::TextSensor *console_port_rate_text_sensor_{nullptr};
  text_sensor::TextSensor *info_response_text_sensor_{nullptr};
  text_sensor::TextSensor *getpwr_response_text_sensor_{nullptr};
  text_sensor::TextSensor *stat_response_text_sensor_{nullptr};
  text_sensor::TextSensor *bat_response_text_sensor_{nullptr};
  text_sensor::TextSensor *soh_response_text_sensor_{nullptr};
  text_sensor::TextSensor *battery_voltage_state_text_sensor_{nullptr};
  text_sensor::TextSensor *battery_temperature_state_text_sensor_{nullptr};
  text_sensor::TextSensor *mos_state_text_sensor_{nullptr};
  text_sensor::TextSensor *cell_voltage_state_text_sensors_[PYLONTECH_MAX_CELLS]{};
  text_sensor::TextSensor *cell_temperature_state_text_sensors_[PYLONTECH_MAX_CELLS]{};
  text_sensor::TextSensor *bat_cell_balance_state_text_sensors_[PYLONTECH_MAX_CELLS]{};
  text_sensor::TextSensor *soh_cell_status_text_sensors_[PYLONTECH_MAX_CELLS]{};
  mutable uint32_t last_cell_voltage_state_hashes_[PYLONTECH_MAX_CELLS]{};
  mutable uint32_t last_cell_temperature_state_hashes_[PYLONTECH_MAX_CELLS]{};
  mutable uint32_t last_bat_cell_balance_state_hashes_[PYLONTECH_MAX_CELLS]{};
  mutable uint32_t last_soh_cell_status_hashes_[PYLONTECH_MAX_CELLS]{};
};

class PylontechComponent : public PollingComponent, public uart::UARTDevice {
 public:
  explicit PylontechComponent(PylontechMemoryMode memory_mode = PYLONTECH_MEMORY_INTERNAL);

  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_role(PylontechRole role) { this->role_ = role; }
  void set_enable_info(bool enable);
  void set_info_interval(uint32_t interval) { this->info_interval_ = interval; }
  void set_info_startup_delay(uint32_t delay);
  void set_enable_getpwr(bool enable);
  void set_enable_stat(bool enable);
  void set_enable_bat(bool enable);
  void set_enable_soh(bool enable);
  void set_publish_only_changes(bool enable);
  void set_enable_login_debug_recovery(bool enable) { this->enable_login_debug_recovery_ = enable; }
  void set_login_debug_failure_threshold(uint8_t threshold) { this->login_debug_failure_threshold_ = threshold; }
  void set_login_debug_recovery_interval(uint32_t interval) { this->login_debug_recovery_interval_ = interval; }
  void set_enable_us2000b_initialization(bool enable) { this->enable_us2000b_initialization_ = enable; }
  void set_stat_interval(uint32_t interval) { this->slow_interval_ = interval; }
  void set_slow_interval(uint32_t interval) { this->slow_interval_ = interval; }
  void register_battery(PylontechBattery *battery);
  void set_force_slave_mode(bool force_slave);
  bool is_slave_mode() const { return this->role_ == PYLONTECH_ROLE_SLAVE; }
  bool request_info_update();
  bool request_getpwr_update();
  bool request_stat_update();
  bool request_bat_update();
  bool request_soh_update();
  bool request_login_debug();
  bool request_us2000b_initialization();

  void set_total_num_sensor(sensor::Sensor *sensor) { this->total_num_sensor_ = sensor; }
  void set_present_num_sensor(sensor::Sensor *sensor) { this->present_num_sensor_ = sensor; }
  void set_sleep_num_sensor(sensor::Sensor *sensor) { this->sleep_num_sensor_ = sensor; }
  void set_system_voltage_sensor(sensor::Sensor *sensor) { this->system_voltage_sensor_ = sensor; }
  void set_system_current_sensor(sensor::Sensor *sensor) { this->system_current_sensor_ = sensor; }
  void set_system_rc_sensor(sensor::Sensor *sensor) { this->system_rc_sensor_ = sensor; }
  void set_system_fcc_sensor(sensor::Sensor *sensor) { this->system_fcc_sensor_ = sensor; }
  void set_system_soc_sensor(sensor::Sensor *sensor) { this->system_soc_sensor_ = sensor; }
  void set_system_soh_sensor(sensor::Sensor *sensor) { this->system_soh_sensor_ = sensor; }
  void set_highest_voltage_sensor(sensor::Sensor *sensor) { this->highest_voltage_sensor_ = sensor; }
  void set_average_voltage_sensor(sensor::Sensor *sensor) { this->average_voltage_sensor_ = sensor; }
  void set_lowest_voltage_sensor(sensor::Sensor *sensor) { this->lowest_voltage_sensor_ = sensor; }
  void set_highest_temperature_sensor(sensor::Sensor *sensor) { this->highest_temperature_sensor_ = sensor; }
  void set_average_temperature_sensor(sensor::Sensor *sensor) { this->average_temperature_sensor_ = sensor; }
  void set_lowest_temperature_sensor(sensor::Sensor *sensor) { this->lowest_temperature_sensor_ = sensor; }
  void set_recommend_charge_voltage_sensor(sensor::Sensor *sensor) { this->recommend_charge_voltage_sensor_ = sensor; }
  void set_recommend_discharge_voltage_sensor(sensor::Sensor *sensor) {
    this->recommend_discharge_voltage_sensor_ = sensor;
  }
  void set_recommend_charge_current_sensor(sensor::Sensor *sensor) { this->recommend_charge_current_sensor_ = sensor; }
  void set_recommend_discharge_current_sensor(sensor::Sensor *sensor) {
    this->recommend_discharge_current_sensor_ = sensor;
  }
  void set_system_status_text_sensor(text_sensor::TextSensor *sensor) { this->system_status_text_sensor_ = sensor; }
  void set_protocol_online_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->protocol_online_binary_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_last_command_text_sensor(text_sensor::TextSensor *sensor) {
    this->last_command_text_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_last_error_text_sensor(text_sensor::TextSensor *sensor) {
    this->last_error_text_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_configured_role_text_sensor(text_sensor::TextSensor *sensor) { this->configured_role_text_sensor_ = sensor; }
  void set_observed_role_text_sensor(text_sensor::TextSensor *sensor) { this->observed_role_text_sensor_ = sensor; }
  void set_role_validation_text_sensor(text_sensor::TextSensor *sensor) { this->role_validation_text_sensor_ = sensor; }
  void set_response_time_sensor(sensor::Sensor *sensor) {
    this->response_time_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_consecutive_failures_sensor(sensor::Sensor *sensor) {
    this->consecutive_failures_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_timeout_count_sensor(sensor::Sensor *sensor) {
    this->timeout_count_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_uart_queue_high_watermark_sensor(sensor::Sensor *sensor) {
    this->uart_queue_high_watermark_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }
  void set_uart_queue_full_count_sensor(sensor::Sensor *sensor) {
    this->uart_queue_full_count_sensor_ = sensor;
    this->diagnostics_enabled_ = true;
  }

 protected:
  void publish_sensor_if_changed(sensor::Sensor *sensor, float value) const;
  void abort_active_sequences_();
  void on_fast_command_success_();
  void begin_diagnostic_command_(const char *command, uint8_t address = 0);
  void complete_diagnostic_command_(const char *command, uint8_t address = 0);
  void timeout_diagnostic_command_();
  void fail_diagnostic_command_(const char *error);
  void publish_diagnostic_label_(text_sensor::TextSensor *sensor, const char *command, uint8_t address) const;
  void update_uart_queue_diagnostics_();
  void publish_configured_role_();
  void observe_role_(PylontechRole role);
  bool looks_like_slave_power_line_(const std::string &line) const;

  void process_line_(std::string &line);
  void dispatch_line_(const std::string &line);
  bool parse_master_power_line_(const std::string &line, PylontechPowerLine *out);
  bool process_system_line_(const std::string &line);
  bool process_info_byte_(uint8_t data);
  bool process_info_line_(const std::string &line);
  bool process_getpwr_line_(const std::string &line);
  bool process_stat_line_(const std::string &line);
  bool process_bat_line_(const std::string &line);
  bool process_soh_line_(const std::string &line);
  bool process_slave_power_line_(const std::string &line);
  bool extract_key_value_(const std::string &line, std::string *key, std::string *value) const;
  int extract_int_(const std::string &value) const;
  void publish_slave_number_(void (PylontechBattery::*publisher)(float) const, float value);
  void publish_slave_text_(void (PylontechBattery::*publisher)(const std::string &) const, const std::string &value);
  void publish_info_number_(void (PylontechBattery::*publisher)(float) const, float value);
  void publish_info_text_(void (PylontechBattery::*publisher)(const std::string &) const, const std::string &value);
  void publish_info_response_();
  void publish_getpwr_response_();
  void publish_stat_response_();
  void publish_bat_response_();
  void publish_soh_response_();
  void publish_getpwr_tail_from_response_();
  void publish_stat_number_(void (PylontechBattery::*publisher)(float) const, float value);
  void publish_stat_field_(uint8_t field, float value);
  void publish_getpwr_summary_(int voltage_mv, int current_ma, int temperature_mc, int residual_ma,
                               const std::string &base_state, const std::string &voltage_state,
                               const std::string &current_state, const std::string &temperature_state);
  void publish_getpwr_tail_(int tail_1_value, int tail_2_value);
  void publish_getpwr_cell_detail_(uint8_t cell, int voltage_mv, int temperature_mc,
                                   const std::string &voltage_state, const std::string &temperature_state);
  void publish_bat_cell_detail_(uint8_t cell, int voltage_mv, int current_ma, int temperature_mc, int soc_percent,
                                int coulomb_mah, const std::string &voltage_state,
                                const std::string &temperature_state, const std::string &balance_state);
  void publish_soh_cell_detail_(uint8_t cell, int voltage_mv, int soh_count, const std::string &soh_status);
  bool append_limited_response_(PylontechBuffer *buffer, bool *truncated, const std::string &line);
  bool wants_info_response_(uint8_t address) const;
  bool wants_getpwr_response_(uint8_t address) const;
  bool wants_stat_response_(uint8_t address) const;
  bool wants_bat_response_(uint8_t address) const;
  bool wants_soh_response_(uint8_t address) const;
  void log_response_chunks_(const char *prefix, uint8_t address, const std::string &response) const;
  void start_response_capture_(const char *command);
  void clear_response_capture_(const char *reason);
  void append_response_capture_line_(const std::string &line);
  void schedule_next_automatic_info_(uint32_t delay);
  void schedule_next_automatic_slow_(uint32_t delay);
  bool start_info_sequence_();
  bool start_getpwr_sequence_();
  bool start_stat_sequence_();
  bool start_bat_sequence_();
  bool start_soh_sequence_();
  bool start_slow_sequence_();
  void send_info_for_address_(uint8_t address);
  void send_getpwr_for_address_(uint8_t address);
  void send_stat_for_address_(uint8_t address);
  void send_bat_for_address_(uint8_t address);
  void send_soh_for_address_(uint8_t address);
  void clear_info_context_();
  void clear_getpwr_context_();
  void clear_stat_context_();
  void clear_bat_context_();
  void clear_soh_context_();
  void process_pending_requests_();
  void process_pending_console_request_();
  void process_pending_info_request_();
  void process_pending_stat_request_();
  void process_pending_bat_request_();
  void process_pending_soh_request_();
  void process_pending_slow_request_();
  bool has_battery_address_(uint8_t address) const;
  uint8_t next_battery_address_(uint8_t after) const;
  bool command_busy_() const;
  bool start_login_debug_();
  bool start_us2000b_initialization_();
  void finish_us2000b_initialization_();

  PylontechRole role_{PYLONTECH_ROLE_MASTER};
  bool enable_info_{false};
  bool setup_complete_{false};
  bool first_fast_response_received_{false};
  PylontechMemoryMode memory_mode_{PYLONTECH_MEMORY_INTERNAL};
  bool has_observed_role_{false};
  PylontechRole observed_role_{PYLONTECH_ROLE_MASTER};
  uint32_t info_interval_{86400000};
  uint32_t info_startup_delay_{0};
  uint32_t info_busy_until_ms_{0};
  bool enable_getpwr_{false};
  bool publish_only_changes_{false};
  uint32_t getpwr_busy_until_ms_{0};
  bool enable_stat_{false};
  bool enable_bat_{false};
  bool enable_soh_{false};
  uint32_t slow_interval_{300000};
  uint32_t stat_busy_until_ms_{0};
  uint32_t bat_busy_until_ms_{0};
  uint32_t soh_busy_until_ms_{0};
  bool pending_info_request_{false};
  bool pending_stat_request_{false};
  bool pending_bat_request_{false};
  bool pending_soh_request_{false};
  bool pending_slow_request_{false};
  bool pending_bat_after_stat_{false};
  bool pending_soh_after_stat_{false};
  bool pending_soh_after_bat_{false};
  bool active_slow_sequence_{false};
  bool pending_pwr_after_pwrsys_{false};
  bool pending_getpwr_after_pwr_{false};
  bool enable_login_debug_recovery_{false};
  bool enable_us2000b_initialization_{false};
  bool login_debug_active_{false};
  bool us2000b_initialization_active_{false};
  bool pending_login_debug_{false};
  bool pending_us2000b_initialization_{false};
  bool login_debug_recovery_has_run_{false};
  uint8_t login_debug_failure_threshold_{3};
  uint8_t fast_command_failures_{0};
  uint32_t login_debug_recovery_interval_{86400000};
  uint32_t last_login_debug_recovery_ms_{0};
  uint32_t us2000b_original_baud_rate_{115200};
  bool diagnostics_enabled_{false};
  bool uart_queue_full_latched_{false};
  const char *diagnostic_active_command_{nullptr};
  uint8_t diagnostic_active_address_{0};
  uint32_t diagnostic_command_started_ms_{0};
  uint32_t diagnostic_timeout_count_{0};
  uint32_t diagnostic_uart_queue_full_count_{0};
  uint8_t diagnostic_consecutive_failures_{0};
  uint8_t diagnostic_uart_queue_high_watermark_{0};
  uint8_t active_info_address_{0};
  uint8_t next_info_address_{0};
  uint8_t active_getpwr_address_{0};
  uint8_t active_stat_address_{0};
  uint8_t active_bat_address_{0};
  uint8_t active_soh_address_{0};
  uint8_t next_getpwr_address_{0};
  uint8_t next_stat_address_{0};
  uint8_t next_bat_address_{0};
  uint8_t next_soh_address_{0};
  uint8_t active_getpwr_cell_{0};
  bool active_getpwr_tail_published_{false};
  bool active_getpwr_tail_pending_{false};
  int active_getpwr_tail_1_value_{0};
  uint8_t active_getpwr_summary_field_count_{0};
  std::string active_getpwr_summary_fields_[8];
  uint8_t active_getpwr_cell_field_{0};
  int active_getpwr_cell_voltage_mv_{0};
  int active_getpwr_cell_temperature_mc_{0};
  std::string active_getpwr_cell_voltage_state_;
  std::string active_getpwr_cell_temperature_state_;
  PylontechBuffer info_response_buffer_;
  PylontechBuffer info_full_response_buffer_;
  bool info_response_truncated_{false};
  PylontechBuffer getpwr_response_buffer_;
  bool getpwr_response_truncated_{false};
  PylontechBuffer stat_response_buffer_;
  bool stat_response_truncated_{false};
  PylontechBuffer bat_response_buffer_;
  bool bat_response_truncated_{false};
  PylontechBuffer soh_response_buffer_;
  bool soh_response_truncated_{false};
  std::string response_capture_command_;
  std::vector<PylontechBuffer> buffer_;
  uint8_t buffer_index_write_{0};
  uint8_t buffer_index_read_{0};
  std::vector<PylontechBattery *> batteries_;

  sensor::Sensor *total_num_sensor_{nullptr};
  sensor::Sensor *present_num_sensor_{nullptr};
  sensor::Sensor *sleep_num_sensor_{nullptr};
  sensor::Sensor *system_voltage_sensor_{nullptr};
  sensor::Sensor *system_current_sensor_{nullptr};
  sensor::Sensor *system_rc_sensor_{nullptr};
  sensor::Sensor *system_fcc_sensor_{nullptr};
  sensor::Sensor *system_soc_sensor_{nullptr};
  sensor::Sensor *system_soh_sensor_{nullptr};
  sensor::Sensor *highest_voltage_sensor_{nullptr};
  sensor::Sensor *average_voltage_sensor_{nullptr};
  sensor::Sensor *lowest_voltage_sensor_{nullptr};
  sensor::Sensor *highest_temperature_sensor_{nullptr};
  sensor::Sensor *average_temperature_sensor_{nullptr};
  sensor::Sensor *lowest_temperature_sensor_{nullptr};
  sensor::Sensor *recommend_charge_voltage_sensor_{nullptr};
  sensor::Sensor *recommend_discharge_voltage_sensor_{nullptr};
  sensor::Sensor *recommend_charge_current_sensor_{nullptr};
  sensor::Sensor *recommend_discharge_current_sensor_{nullptr};
  text_sensor::TextSensor *system_status_text_sensor_{nullptr};
  binary_sensor::BinarySensor *protocol_online_binary_sensor_{nullptr};
  text_sensor::TextSensor *last_command_text_sensor_{nullptr};
  text_sensor::TextSensor *last_error_text_sensor_{nullptr};
  text_sensor::TextSensor *configured_role_text_sensor_{nullptr};
  text_sensor::TextSensor *observed_role_text_sensor_{nullptr};
  text_sensor::TextSensor *role_validation_text_sensor_{nullptr};
  sensor::Sensor *response_time_sensor_{nullptr};
  sensor::Sensor *consecutive_failures_sensor_{nullptr};
  sensor::Sensor *timeout_count_sensor_{nullptr};
  sensor::Sensor *uart_queue_high_watermark_sensor_{nullptr};
  sensor::Sensor *uart_queue_full_count_sensor_{nullptr};
};

class PylontechForceSlaveSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;

  PylontechComponent *parent_{nullptr};
};

class PylontechInfoButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;

  PylontechComponent *parent_{nullptr};
};

class PylontechGetPowerButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;

  PylontechComponent *parent_{nullptr};
};

class PylontechStatButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;

  PylontechComponent *parent_{nullptr};
};

class PylontechBatButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;

  PylontechComponent *parent_{nullptr};
};

class PylontechSohButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;

  PylontechComponent *parent_{nullptr};
};

class PylontechLoginDebugButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;
  PylontechComponent *parent_{nullptr};
};

class PylontechUS2000BInitializationButton : public button::Button, public Component {
 public:
  void set_parent(PylontechComponent *parent) { this->parent_ = parent; }
  void dump_config() override;

 protected:
  void press_action() override;
  PylontechComponent *parent_{nullptr};
};

}  // namespace pylontech
}  // namespace esphome
