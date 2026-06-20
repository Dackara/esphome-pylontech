#include "pylontech.h"
#include "esphome/core/log.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace esphome {
namespace pylontech {

static const char *const TAG = "pylontech";
static const uint8_t ASCII_LF = 0x0A;

static RAMAllocator<char> make_buffer_allocator(PylontechMemoryMode mode) {
  if (mode == PYLONTECH_MEMORY_INTERNAL) {
    return RAMAllocator<char>(RAMAllocator<char>::ALLOC_INTERNAL);
  }
  return RAMAllocator<char>(RAMAllocator<char>::ALLOC_EXTERNAL | RAMAllocator<char>::ALLOC_INTERNAL);
}

PylontechComponent::PylontechComponent(PylontechMemoryMode memory_mode)
    : memory_mode_(memory_mode),
      info_response_buffer_(make_buffer_allocator(memory_mode)),
      info_full_response_buffer_(make_buffer_allocator(memory_mode)),
      getpwr_response_buffer_(make_buffer_allocator(memory_mode)),
      stat_response_buffer_(make_buffer_allocator(memory_mode)),
      bat_response_buffer_(make_buffer_allocator(memory_mode)),
      soh_response_buffer_(make_buffer_allocator(memory_mode)) {
  this->buffer_.reserve(PYLONTECH_NUM_BUFFERS);
  for (uint8_t i = 0; i < PYLONTECH_NUM_BUFFERS; i++) {
    this->buffer_.emplace_back(make_buffer_allocator(memory_mode));
  }
}

static bool deadline_pending(uint32_t now, uint32_t deadline) {
  return deadline != 0 && static_cast<int32_t>(deadline - now) > 0;
}

static uint32_t fnv1a_hash(const std::string &value) {
  uint32_t hash = 2166136261u;
  for (unsigned char c : value) {
    hash ^= c;
    hash *= 16777619u;
  }
  return hash == 0 ? 1 : hash;
}

static void publish_text_if_changed(text_sensor::TextSensor *sensor, const std::string &value) {
  if (sensor != nullptr && sensor->state != value) {
    sensor->publish_state(value);
  }
}

static void publish_sensor_value(sensor::Sensor *sensor, float value, bool only_changes) {
  if (sensor == nullptr) {
    return;
  }
  if (only_changes && ((std::isnan(sensor->state) && std::isnan(value)) || sensor->state == value)) {
    return;
  }
  sensor->publish_state(value);
}

void PylontechBattery::publish_sensor_if_changed(sensor::Sensor *sensor, float value) const {
  publish_sensor_value(sensor, value, this->publish_only_changes_);
}

void PylontechComponent::publish_sensor_if_changed(sensor::Sensor *sensor, float value) const {
  publish_sensor_value(sensor, value, this->publish_only_changes_);
}

void PylontechComponent::publish_diagnostic_label_(text_sensor::TextSensor *sensor, const char *command,
                                                    uint8_t address) const {
  if (sensor == nullptr || command == nullptr) {
    return;
  }
  char label[24];
  if (address > 0) {
    std::snprintf(label, sizeof(label), "%s %u", command, address);
  } else {
    std::snprintf(label, sizeof(label), "%s", command);
  }
  publish_text_if_changed(sensor, label);
}

void PylontechComponent::begin_diagnostic_command_(const char *command, uint8_t address) {
  if (!this->diagnostics_enabled_) {
    return;
  }
  this->diagnostic_active_command_ = command;
  this->diagnostic_active_address_ = address;
  this->diagnostic_command_started_ms_ = millis();
}

void PylontechComponent::complete_diagnostic_command_(const char *command, uint8_t address) {
  if (!this->diagnostics_enabled_) {
    return;
  }
  this->publish_diagnostic_label_(this->last_command_text_sensor_, command, address);
  if (this->diagnostic_active_command_ != nullptr) {
    this->publish_sensor_if_changed(this->response_time_sensor_, millis() - this->diagnostic_command_started_ms_);
  }
  this->diagnostic_active_command_ = nullptr;
  this->diagnostic_active_address_ = 0;
  this->diagnostic_consecutive_failures_ = 0;
  this->publish_sensor_if_changed(this->consecutive_failures_sensor_, 0);
  if (this->protocol_online_binary_sensor_ != nullptr && !this->protocol_online_binary_sensor_->state) {
    this->protocol_online_binary_sensor_->publish_state(true);
  }
}

void PylontechComponent::timeout_diagnostic_command_() {
  if (!this->diagnostics_enabled_ || this->diagnostic_active_command_ == nullptr) {
    return;
  }
  char error[40];
  if (this->diagnostic_active_address_ > 0) {
    std::snprintf(error, sizeof(error), "Timeout: %s %u", this->diagnostic_active_command_,
                  this->diagnostic_active_address_);
  } else {
    std::snprintf(error, sizeof(error), "Timeout: %s", this->diagnostic_active_command_);
  }
  publish_text_if_changed(this->last_error_text_sensor_, error);
  this->diagnostic_timeout_count_++;
  this->diagnostic_consecutive_failures_++;
  this->publish_sensor_if_changed(this->timeout_count_sensor_, this->diagnostic_timeout_count_);
  this->publish_sensor_if_changed(this->consecutive_failures_sensor_, this->diagnostic_consecutive_failures_);
  if (this->protocol_online_binary_sensor_ != nullptr && this->diagnostic_consecutive_failures_ >= 3 &&
      this->protocol_online_binary_sensor_->state) {
    this->protocol_online_binary_sensor_->publish_state(false);
  }
  this->diagnostic_active_command_ = nullptr;
  this->diagnostic_active_address_ = 0;
}

void PylontechComponent::fail_diagnostic_command_(const char *error) {
  if (!this->diagnostics_enabled_) {
    return;
  }
  publish_text_if_changed(this->last_error_text_sensor_, error);
  this->diagnostic_consecutive_failures_++;
  this->publish_sensor_if_changed(this->consecutive_failures_sensor_, this->diagnostic_consecutive_failures_);
  if (this->protocol_online_binary_sensor_ != nullptr && this->diagnostic_consecutive_failures_ >= 3 &&
      this->protocol_online_binary_sensor_->state) {
    this->protocol_online_binary_sensor_->publish_state(false);
  }
  this->diagnostic_active_command_ = nullptr;
  this->diagnostic_active_address_ = 0;
}

void PylontechComponent::update_uart_queue_diagnostics_() {
  if (this->uart_queue_high_watermark_sensor_ == nullptr) {
    return;
  }
  uint8_t depth = (this->buffer_index_write_ + PYLONTECH_NUM_BUFFERS - this->buffer_index_read_) %
                  PYLONTECH_NUM_BUFFERS;
  if (depth > this->diagnostic_uart_queue_high_watermark_) {
    this->diagnostic_uart_queue_high_watermark_ = depth;
    this->publish_sensor_if_changed(this->uart_queue_high_watermark_sensor_, depth);
  }
}

void PylontechBattery::dump_config() {
  ESP_LOGCONFIG(TAG, "  Battery listener address: %u", this->address_);
  LOG_SENSOR("    ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("    ", "Current", this->current_sensor_);
  LOG_SENSOR("    ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("    ", "Coulomb", this->soc_sensor_);
}

void PylontechBattery::publish_power_line(const PylontechPowerLine &line) {
  ESP_LOGV(TAG,
           "Publishing battery %d: %.3fV %.3fA %.1fC coulomb=%d%% cell_low=%.3fV cell_high=%.3fV",
           line.battery_address, line.voltage_mv / 1000.0f, line.current_ma / 1000.0f,
           line.temperature_mc / 1000.0f, line.soc_percent, line.voltage_low_mv / 1000.0f,
           line.voltage_high_mv / 1000.0f);

  publish_sensor_if_changed(this->voltage_sensor_, line.voltage_mv / 1000.0f);
  publish_sensor_if_changed(this->current_sensor_, line.current_ma / 1000.0f);
  publish_sensor_if_changed(this->temperature_sensor_, line.temperature_mc / 1000.0f);
  publish_sensor_if_changed(this->temperature_low_sensor_, line.temperature_low_mc / 1000.0f);
  publish_sensor_if_changed(this->temperature_high_sensor_, line.temperature_high_mc / 1000.0f);
  publish_sensor_if_changed(this->cell_voltage_low_sensor_, line.voltage_low_mv / 1000.0f);
  publish_sensor_if_changed(this->cell_voltage_high_sensor_, line.voltage_high_mv / 1000.0f);
  publish_sensor_if_changed(this->soc_sensor_, line.soc_percent);
  publish_sensor_if_changed(this->mos_temperature_sensor_,
                            line.has_mos_temperature ? line.mos_temperature_mc / 1000.0f : NAN);

  publish_text_if_changed(this->base_state_text_sensor_, line.base_state);
  publish_text_if_changed(this->voltage_state_text_sensor_, line.voltage_state);
  publish_text_if_changed(this->current_state_text_sensor_, line.current_state);
  publish_text_if_changed(this->temperature_state_text_sensor_, line.temperature_state);
  publish_text_if_changed(this->battery_voltage_state_text_sensor_, line.battery_voltage_state);
  publish_text_if_changed(this->battery_temperature_state_text_sensor_, line.battery_temperature_state);
  publish_text_if_changed(this->mos_state_text_sensor_, line.mos_state);
}

void PylontechBattery::publish_voltage(float voltage) const {
  publish_sensor_if_changed(this->voltage_sensor_, voltage);
}

void PylontechBattery::publish_current(float current) const {
  publish_sensor_if_changed(this->current_sensor_, current);
}

void PylontechBattery::publish_temperature(float temperature) const {
  publish_sensor_if_changed(this->temperature_sensor_, temperature);
}

void PylontechBattery::publish_soc(float soc) const {
  publish_sensor_if_changed(this->soc_sensor_, soc);
}

void PylontechBattery::publish_total_coulomb(float total_coulomb) const {
  publish_sensor_if_changed(this->total_coulomb_sensor_, total_coulomb);
}

void PylontechBattery::publish_max_voltage(float max_voltage) const {
  publish_sensor_if_changed(this->max_voltage_sensor_, max_voltage);
}

void PylontechBattery::publish_charge_times(float charge_times) const {
  publish_sensor_if_changed(this->charge_times_sensor_, charge_times);
}

void PylontechBattery::publish_discharge_sec(float discharge_sec) const {
  publish_sensor_if_changed(this->discharge_sec_sensor_, discharge_sec);
}

void PylontechBattery::publish_charge_sec(float charge_sec) const {
  publish_sensor_if_changed(this->charge_sec_sensor_, charge_sec);
}

void PylontechBattery::publish_device_address(float address) const {
  publish_sensor_if_changed(this->device_address_sensor_, address);
}

void PylontechBattery::publish_cell_number(float cell_number) const {
  publish_sensor_if_changed(this->cell_number_sensor_, cell_number);
}

void PylontechBattery::publish_max_charge_current(float current) const {
  publish_sensor_if_changed(this->max_charge_current_sensor_, current);
}

void PylontechBattery::publish_max_discharge_current(float current) const {
  publish_sensor_if_changed(this->max_discharge_current_sensor_, current);
}

void PylontechBattery::publish_base_state(const std::string &state) const {
  publish_text_if_changed(this->base_state_text_sensor_, state);
}

void PylontechBattery::publish_voltage_state(const std::string &state) const {
  publish_text_if_changed(this->voltage_state_text_sensor_, state);
}

void PylontechBattery::publish_current_state(const std::string &state) const {
  publish_text_if_changed(this->current_state_text_sensor_, state);
}

void PylontechBattery::publish_temperature_state(const std::string &state) const {
  publish_text_if_changed(this->temperature_state_text_sensor_, state);
}

void PylontechBattery::publish_coulomb_state(const std::string &state) const {
  publish_text_if_changed(this->coulomb_state_text_sensor_, state);
}

void PylontechBattery::publish_soh_state(const std::string &state) const {
  publish_text_if_changed(this->soh_state_text_sensor_, state);
}

void PylontechBattery::publish_heater_state(const std::string &state) const {
  publish_text_if_changed(this->heater_state_text_sensor_, state);
}

void PylontechBattery::publish_protect_ena(const std::string &state) const {
  publish_text_if_changed(this->protect_ena_text_sensor_, state);
}

void PylontechBattery::publish_bat_events(const std::string &state) const {
  publish_text_if_changed(this->bat_events_text_sensor_, state);
}

void PylontechBattery::publish_power_events(const std::string &state) const {
  publish_text_if_changed(this->power_events_text_sensor_, state);
}

void PylontechBattery::publish_system_fault(const std::string &state) const {
  publish_text_if_changed(this->system_fault_text_sensor_, state);
}

void PylontechBattery::publish_manufacturer(const std::string &value) const {
  publish_text_if_changed(this->manufacturer_text_sensor_, value);
}

void PylontechBattery::publish_device_name(const std::string &value) const {
  publish_text_if_changed(this->device_name_text_sensor_, value);
}

void PylontechBattery::publish_board(const std::string &value) const {
  publish_text_if_changed(this->board_text_sensor_, value);
}

void PylontechBattery::publish_board_version(const std::string &value) const {
  publish_text_if_changed(this->board_version_text_sensor_, value);
}

void PylontechBattery::publish_main_soft_version(const std::string &value) const {
  publish_text_if_changed(this->main_soft_version_text_sensor_, value);
}

void PylontechBattery::publish_soft_version(const std::string &value) const {
  publish_text_if_changed(this->soft_version_text_sensor_, value);
}

void PylontechBattery::publish_boot_version(const std::string &value) const {
  publish_text_if_changed(this->boot_version_text_sensor_, value);
}

void PylontechBattery::publish_comm_version(const std::string &value) const {
  publish_text_if_changed(this->comm_version_text_sensor_, value);
}

void PylontechBattery::publish_release_date(const std::string &value) const {
  publish_text_if_changed(this->release_date_text_sensor_, value);
}

void PylontechBattery::publish_barcode(const std::string &value) const {
  publish_text_if_changed(this->barcode_text_sensor_, value);
}

void PylontechBattery::publish_specification(const std::string &value) const {
  publish_text_if_changed(this->specification_text_sensor_, value);
}

void PylontechBattery::publish_epon_port_rate(const std::string &value) const {
  publish_text_if_changed(this->epon_port_rate_text_sensor_, value);
}

void PylontechBattery::publish_console_port_rate(const std::string &value) const {
  publish_text_if_changed(this->console_port_rate_text_sensor_, value);
}

void PylontechBattery::publish_info_response(const std::string &value) const {
  publish_text_if_changed(this->info_response_text_sensor_, value);
}

void PylontechBattery::publish_getpwr_response(const std::string &value) const {
  publish_text_if_changed(this->getpwr_response_text_sensor_, value);
}

void PylontechBattery::publish_stat_response(const std::string &value) const {
  publish_text_if_changed(this->stat_response_text_sensor_, value);
}

void PylontechBattery::publish_bat_response(const std::string &value) const {
  publish_text_if_changed(this->bat_response_text_sensor_, value);
}

void PylontechBattery::publish_soh_response(const std::string &value) const {
  publish_text_if_changed(this->soh_response_text_sensor_, value);
}

void PylontechBattery::publish_stat_cycle_count(float value) const {
  publish_sensor_if_changed(this->stat_cycle_count_sensor_, value);
}

void PylontechBattery::publish_stat_coulomb(float value) const {
  publish_sensor_if_changed(this->stat_coulomb_sensor_, value);
}

void PylontechBattery::publish_stat_field(uint8_t field, float value) const {
  if (field < PYLONTECH_STAT_FIELD_COUNT)
    publish_sensor_if_changed(this->stat_field_sensors_[field], value);
}

void PylontechBattery::publish_getpwr_summary(int voltage_mv, int current_ma, int temperature_mc, int residual_ma,
                                              const std::string &base_state, const std::string &voltage_state,
                                              const std::string &current_state,
                                              const std::string &temperature_state) const {
  publish_sensor_if_changed(this->getpwr_voltage_sensor_, voltage_mv / 1000.0f);
  publish_sensor_if_changed(this->getpwr_current_sensor_, current_ma / 1000.0f);
  publish_sensor_if_changed(this->getpwr_temperature_sensor_, temperature_mc / 1000.0f);
  publish_sensor_if_changed(this->getpwr_residual_capacity_sensor_, residual_ma / 1000.0f);
  publish_sensor_if_changed(this->getpwr_residual_ma_sensor_, residual_ma);

  if (!base_state.empty())
    publish_text_if_changed(this->base_state_text_sensor_, base_state);
  if (!voltage_state.empty())
    publish_text_if_changed(this->voltage_state_text_sensor_, voltage_state);
  if (!current_state.empty())
    publish_text_if_changed(this->current_state_text_sensor_, current_state);
  if (!temperature_state.empty())
    publish_text_if_changed(this->temperature_state_text_sensor_, temperature_state);
}

void PylontechBattery::publish_getpwr_tail(int tail_1_value, int tail_2_value) const {
  publish_sensor_if_changed(this->getpwr_tail_1_sensor_, tail_1_value);
  publish_sensor_if_changed(this->getpwr_tail_2_sensor_, tail_2_value);
}

void PylontechBattery::publish_cell_detail(uint8_t cell, int voltage_mv, int temperature_mc,
                                           const std::string &voltage_state,
                                           const std::string &temperature_state) const {
  if (cell < 1 || cell > PYLONTECH_MAX_CELLS)
    return;
  auto *voltage_sensor = this->cell_voltage_sensors_[cell - 1];
  publish_sensor_if_changed(voltage_sensor, voltage_mv / 1000.0f);
  auto *temperature_sensor = this->cell_temperature_sensors_[cell - 1];
  publish_sensor_if_changed(temperature_sensor, temperature_mc / 1000.0f);
  auto *voltage_state_sensor = this->cell_voltage_state_text_sensors_[cell - 1];
  uint32_t voltage_state_hash = voltage_state.empty() ? 0 : fnv1a_hash(voltage_state);
  if (voltage_state_sensor != nullptr && voltage_state_hash != 0 &&
      this->last_cell_voltage_state_hashes_[cell - 1] != voltage_state_hash) {
    this->last_cell_voltage_state_hashes_[cell - 1] = voltage_state_hash;
    voltage_state_sensor->publish_state(voltage_state);
  }
  auto *temperature_state_sensor = this->cell_temperature_state_text_sensors_[cell - 1];
  uint32_t temperature_state_hash = temperature_state.empty() ? 0 : fnv1a_hash(temperature_state);
  if (temperature_state_sensor != nullptr && temperature_state_hash != 0 &&
      this->last_cell_temperature_state_hashes_[cell - 1] != temperature_state_hash) {
    this->last_cell_temperature_state_hashes_[cell - 1] = temperature_state_hash;
    temperature_state_sensor->publish_state(temperature_state);
  }
}

void PylontechBattery::publish_bat_cell_detail(uint8_t cell, int voltage_mv, int current_ma, int temperature_mc,
                                               int soc_percent, int coulomb_mah,
                                               const std::string &voltage_state,
                                               const std::string &temperature_state,
                                               const std::string &balance_state) const {
  if (cell < 1 || cell > PYLONTECH_MAX_CELLS)
    return;

  this->publish_cell_detail(cell, voltage_mv, temperature_mc, voltage_state, temperature_state);

  auto *current_sensor = this->bat_cell_current_sensors_[cell - 1];
  publish_sensor_if_changed(current_sensor, current_ma);
  auto *soc_sensor = this->bat_cell_soc_sensors_[cell - 1];
  publish_sensor_if_changed(soc_sensor, soc_percent);
  auto *coulomb_sensor = this->bat_cell_coulomb_sensors_[cell - 1];
  publish_sensor_if_changed(coulomb_sensor, coulomb_mah);
  auto *balance_sensor = this->bat_cell_balance_state_text_sensors_[cell - 1];
  uint32_t balance_state_hash = balance_state.empty() ? 0 : fnv1a_hash(balance_state);
  if (balance_sensor != nullptr && balance_state_hash != 0 &&
      this->last_bat_cell_balance_state_hashes_[cell - 1] != balance_state_hash) {
    this->last_bat_cell_balance_state_hashes_[cell - 1] = balance_state_hash;
    balance_sensor->publish_state(balance_state);
  }
}

void PylontechBattery::publish_soh_cell_detail(uint8_t cell, int voltage_mv, int soh_count,
                                               const std::string &soh_status) const {
  if (cell < 1 || cell > PYLONTECH_MAX_CELLS)
    return;

  auto *voltage_sensor = this->cell_voltage_sensors_[cell - 1];
  publish_sensor_if_changed(voltage_sensor, voltage_mv / 1000.0f);
  auto *count_sensor = this->soh_cell_count_sensors_[cell - 1];
  publish_sensor_if_changed(count_sensor, soh_count);
  auto *status_sensor = this->soh_cell_status_text_sensors_[cell - 1];
  uint32_t soh_status_hash = soh_status.empty() ? 0 : fnv1a_hash(soh_status);
  if (status_sensor != nullptr && soh_status_hash != 0 &&
      this->last_soh_cell_status_hashes_[cell - 1] != soh_status_hash) {
    this->last_soh_cell_status_hashes_[cell - 1] = soh_status_hash;
    status_sensor->publish_state(soh_status);
  }
}

void PylontechComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Pylontech component...");
  this->check_uart_settings(115200, 1, uart::UART_CONFIG_PARITY_NONE, 8);
  this->info_response_buffer_.reserve(160);
  uint16_t flushed = 0;
  while (this->available() != 0) {
    this->read();
    flushed++;
  }
  ESP_LOGV(TAG, "UART buffer flushed: %u bytes discarded", flushed);
  if (this->protocol_online_binary_sensor_ != nullptr) {
    this->protocol_online_binary_sensor_->publish_state(false);
  }
  publish_text_if_changed(this->last_error_text_sensor_, "None");
  this->publish_sensor_if_changed(this->consecutive_failures_sensor_, 0);
  this->publish_sensor_if_changed(this->timeout_count_sensor_, 0);
  this->publish_sensor_if_changed(this->uart_queue_high_watermark_sensor_, 0);
  this->publish_sensor_if_changed(this->uart_queue_full_count_sensor_, 0);
  this->publish_configured_role_();
  publish_text_if_changed(this->observed_role_text_sensor_, "Unknown");
  publish_text_if_changed(this->role_validation_text_sensor_, "Not checked");
  this->setup_complete_ = true;
  if (this->enable_us2000b_initialization_) {
    ESP_LOGW(TAG, "US2000B-only initialization enabled; runtime UART baud switching is required");
    this->set_timeout("automatic_us2000b_initialization", 1000,
                      [this]() { this->request_us2000b_initialization(); });
  }
  if (this->enable_info_) {
    ESP_LOGV(TAG, "Automatic info waiting for the first successful fast-loop response");
  } else {
    ESP_LOGV(TAG, "Automatic info disabled");
  }
  if (this->enable_getpwr_) {
    ESP_LOGV(TAG, "GETPWR enabled in fast update loop");
  } else {
    ESP_LOGV(TAG, "GETPWR disabled in fast update loop");
  }
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(10000);
  } else {
    ESP_LOGV(TAG, "Automatic slow loop disabled");
  }
}

void PylontechComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech:");
  ESP_LOGCONFIG(TAG, "  Role: %s", this->role_ == PYLONTECH_ROLE_MASTER ? "master" : "slave");
  const char *memory_mode = this->memory_mode_ == PYLONTECH_MEMORY_INTERNAL
                                ? "internal"
                                : (this->memory_mode_ == PYLONTECH_MEMORY_PSRAM ? "psram" : "auto");
  ESP_LOGCONFIG(TAG, "  Buffer memory mode: %s", memory_mode);
  ESP_LOGCONFIG(TAG, "  Automatic info: %s", YESNO(this->enable_info_));
  ESP_LOGCONFIG(TAG, "  Info interval: %ums", this->info_interval_);
  ESP_LOGCONFIG(TAG, "  Info startup delay: %ums", this->info_startup_delay_);
  ESP_LOGCONFIG(TAG, "  GETPWR in fast loop: %s", YESNO(this->enable_getpwr_));
  ESP_LOGCONFIG(TAG, "  Automatic stat: %s", YESNO(this->enable_stat_));
  ESP_LOGCONFIG(TAG, "  Automatic bat: %s", YESNO(this->enable_bat_));
  ESP_LOGCONFIG(TAG, "  Automatic soh: %s", YESNO(this->enable_soh_));
  ESP_LOGCONFIG(TAG, "  Publish only changes: %s", YESNO(this->publish_only_changes_));
  ESP_LOGCONFIG(TAG, "  Login debug recovery: %s", YESNO(this->enable_login_debug_recovery_));
  ESP_LOGCONFIG(TAG, "  US2000B initialization: %s", YESNO(this->enable_us2000b_initialization_));
  ESP_LOGCONFIG(TAG, "  Slow interval: %ums", this->slow_interval_);
  uint8_t slow_command_count = static_cast<uint8_t>(this->enable_stat_) + static_cast<uint8_t>(this->enable_bat_) +
                               static_cast<uint8_t>(this->enable_soh_);
  if (this->slow_interval_ < 30000 && slow_command_count > 1) {
    ESP_LOGW(TAG, "Slow interval below 30s with %u slow commands enabled may delay the fast loop",
             slow_command_count);
  }
  ESP_LOGCONFIG(TAG, "  Battery listener count: %u", static_cast<unsigned>(this->batteries_.size()));
  LOG_SENSOR("  ", "System voltage", this->system_voltage_sensor_);
  LOG_SENSOR("  ", "System current", this->system_current_sensor_);
  LOG_SENSOR("  ", "System SOC", this->system_soc_sensor_);
  LOG_SENSOR("  ", "System SOH", this->system_soh_sensor_);
  LOG_TEXT_SENSOR("  ", "System status", this->system_status_text_sensor_);
  LOG_BINARY_SENSOR("  ", "Protocol online", this->protocol_online_binary_sensor_);
  LOG_TEXT_SENSOR("  ", "Last command", this->last_command_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Last error", this->last_error_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Configured role", this->configured_role_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Observed role", this->observed_role_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Role validation", this->role_validation_text_sensor_);
  LOG_SENSOR("  ", "Response time", this->response_time_sensor_);
  LOG_SENSOR("  ", "Consecutive failures", this->consecutive_failures_sensor_);
  LOG_SENSOR("  ", "Timeout count", this->timeout_count_sensor_);
  LOG_SENSOR("  ", "UART queue high watermark", this->uart_queue_high_watermark_sensor_);
  LOG_SENSOR("  ", "UART queue pause count", this->uart_queue_full_count_sensor_);
  for (auto *battery : this->batteries_) {
    if (battery != nullptr) {
      battery->dump_config();
    }
  }
  LOG_UPDATE_INTERVAL(this);
}

void PylontechComponent::abort_active_sequences_() {
  const char *timeouts[] = {"automatic_info",          "automatic_slow",       "clear_info_context",
                            "clear_getpwr_context",    "clear_stat_context",   "clear_bat_context",
                            "clear_soh_context",       "send_next_info",       "send_next_getpwr",
                            "send_next_stat",          "send_next_bat",        "send_next_soh",
                            "response_capture_timeout", "send_pwr_after_pwrsys_complete",
                            "send_getpwr_after_pwr_complete", "login_debug_complete",
                            "automatic_us2000b_initialization", "us2000b_send_frame", "us2000b_restore_baud",
                            "us2000b_complete", "login_debug_retry_fast", "us2000b_retry_fast"};
  for (const char *timeout : timeouts) {
    this->cancel_timeout(timeout);
  }

  this->info_busy_until_ms_ = 0;
  this->getpwr_busy_until_ms_ = 0;
  this->stat_busy_until_ms_ = 0;
  this->bat_busy_until_ms_ = 0;
  this->soh_busy_until_ms_ = 0;
  this->active_info_address_ = 0;
  this->active_getpwr_address_ = 0;
  this->active_stat_address_ = 0;
  this->active_bat_address_ = 0;
  this->active_soh_address_ = 0;
  this->next_info_address_ = 0;
  this->next_getpwr_address_ = 0;
  this->next_stat_address_ = 0;
  this->next_bat_address_ = 0;
  this->next_soh_address_ = 0;
  this->pending_info_request_ = false;
  this->pending_stat_request_ = false;
  this->pending_bat_request_ = false;
  this->pending_soh_request_ = false;
  this->pending_slow_request_ = false;
  this->pending_bat_after_stat_ = false;
  this->pending_soh_after_stat_ = false;
  this->pending_soh_after_bat_ = false;
  this->pending_pwr_after_pwrsys_ = false;
  this->pending_getpwr_after_pwr_ = false;
  this->active_slow_sequence_ = false;
  this->response_capture_command_.clear();
  this->login_debug_active_ = false;
#if defined(USE_ESP32) || defined(USE_ESP8266)
  if (this->us2000b_initialization_active_) {
    this->parent_->set_baud_rate(this->us2000b_original_baud_rate_);
    this->parent_->load_settings(false);
  }
#endif
  this->us2000b_initialization_active_ = false;
  this->pending_login_debug_ = false;
  this->pending_us2000b_initialization_ = false;
  this->diagnostic_active_command_ = nullptr;
  this->diagnostic_active_address_ = 0;
  this->fast_command_failures_ = 0;
  if (this->protocol_online_binary_sensor_ != nullptr) {
    this->protocol_online_binary_sensor_->publish_state(false);
  }
  this->info_response_buffer_.clear();
  this->info_full_response_buffer_.clear();
  this->getpwr_response_buffer_.clear();
  this->stat_response_buffer_.clear();
  this->bat_response_buffer_.clear();
  this->soh_response_buffer_.clear();
  for (auto &buffer : this->buffer_) {
    buffer.clear();
  }
  this->buffer_index_read_ = 0;
  this->buffer_index_write_ = 0;
  while (this->available() > 0) {
    this->read();
  }
  this->first_fast_response_received_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(10000);
  }
}

void PylontechComponent::on_fast_command_success_() {
  if (this->first_fast_response_received_) {
    return;
  }
  this->first_fast_response_received_ = true;
  if (this->enable_info_) {
    this->schedule_next_automatic_info_(this->info_startup_delay_);
  }
}

void PylontechComponent::set_force_slave_mode(bool force_slave) {
  this->abort_active_sequences_();
  this->role_ = force_slave ? PYLONTECH_ROLE_SLAVE : PYLONTECH_ROLE_MASTER;
  this->has_observed_role_ = false;
  this->publish_configured_role_();
  publish_text_if_changed(this->observed_role_text_sensor_, "Unknown");
  publish_text_if_changed(this->role_validation_text_sensor_, "Not checked");
  ESP_LOGI(TAG, "Runtime role changed to %s", this->is_slave_mode() ? "slave" : "master");
}

void PylontechComponent::publish_configured_role_() {
  publish_text_if_changed(this->configured_role_text_sensor_,
                          this->role_ == PYLONTECH_ROLE_MASTER ? "Master" : "Slave");
}

void PylontechComponent::observe_role_(PylontechRole role) {
  const bool changed = !this->has_observed_role_ || this->observed_role_ != role;
  this->has_observed_role_ = true;
  this->observed_role_ = role;
  publish_text_if_changed(this->observed_role_text_sensor_, role == PYLONTECH_ROLE_MASTER ? "Master" : "Slave");
  publish_text_if_changed(this->role_validation_text_sensor_, role == this->role_ ? "OK" : "Mismatch");
  if (changed && role != this->role_) {
    ESP_LOGW(TAG, "Configured role is %s, but the PWR response looks like %s mode",
             this->role_ == PYLONTECH_ROLE_MASTER ? "master" : "slave",
             role == PYLONTECH_ROLE_MASTER ? "master" : "slave");
  }
}

bool PylontechComponent::looks_like_slave_power_line_(const std::string &line) const {
  std::string key;
  std::string value;
  if (!this->extract_key_value_(line, &key, &value)) {
    return false;
  }
  return key == "Voltage" || key == "Current" || key == "Temperature" || key == "Coulomb" ||
         key == "Total Coulomb" || key == "Max Voltage" || key == "Charge Times" ||
         key == "Discharge Sec." || key == "Charge Sec." || key == "Basic Status" ||
         key == "Volt Status" || key == "Current Status" || key == "Tmpr. Status" ||
         key == "Coul. Status" || key == "Soh. Status" || key == "Heater Status" ||
         key == "Protect ENA" || key == "Bat Events" || key == "Power Events" || key == "System Fault";
}

void PylontechComponent::set_enable_info(bool enable) {
  this->enable_info_ = enable;
  ESP_LOGV(TAG, "Automatic info option set to %s", YESNO(enable));
  if (!this->setup_complete_) {
    return;
  }
  if (enable && this->first_fast_response_received_) {
    this->schedule_next_automatic_info_(this->info_startup_delay_);
  } else {
    this->cancel_timeout("automatic_info");
  }
}

void PylontechComponent::set_info_startup_delay(uint32_t delay) {
  this->info_startup_delay_ = delay;
  ESP_LOGV(TAG, "Automatic info startup delay set to %ums", delay);
  if (this->setup_complete_ && this->enable_info_ && this->first_fast_response_received_) {
    this->schedule_next_automatic_info_(delay);
  }
}

void PylontechComponent::set_enable_getpwr(bool enable) {
  this->enable_getpwr_ = enable;
  ESP_LOGV(TAG, "GETPWR fast-loop option set to %s", YESNO(enable));
}

void PylontechComponent::set_enable_stat(bool enable) {
  this->enable_stat_ = enable;
  ESP_LOGV(TAG, "Automatic stat option set to %s", YESNO(enable));
  if (!this->setup_complete_) {
    return;
  }
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(10000);
  } else {
    this->cancel_timeout("automatic_slow");
  }
}

void PylontechComponent::set_enable_bat(bool enable) {
  this->enable_bat_ = enable;
  ESP_LOGV(TAG, "Automatic bat option set to %s", YESNO(enable));
  if (!this->setup_complete_) {
    return;
  }
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(10000);
  } else {
    this->cancel_timeout("automatic_slow");
  }
}

void PylontechComponent::set_enable_soh(bool enable) {
  this->enable_soh_ = enable;
  ESP_LOGV(TAG, "Automatic soh option set to %s", YESNO(enable));
  if (!this->setup_complete_) {
    return;
  }
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(10000);
  } else {
    this->cancel_timeout("automatic_slow");
  }
}

void PylontechComponent::set_publish_only_changes(bool enable) {
  this->publish_only_changes_ = enable;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr) {
      battery->set_publish_only_changes(enable);
    }
  }
}

void PylontechComponent::register_battery(PylontechBattery *battery) {
  if (battery == nullptr) {
    return;
  }
  battery->set_publish_only_changes(this->publish_only_changes_);
  this->batteries_.push_back(battery);
}

void PylontechComponent::update() {
  const uint32_t now = millis();
  if (this->command_busy_()) {
    ESP_LOGV(TAG, "Skipping regular update while another Pylontech sequence is active");
    return;
  }

  if (!this->is_slave_mode()) {
    ESP_LOGV(TAG, "TX: pwrsys");
    this->pending_pwr_after_pwrsys_ = true;
    this->pending_getpwr_after_pwr_ = this->enable_getpwr_;
    this->start_response_capture_("pwrsys");
    this->write_str("pwrsys\n");
  } else {
    ESP_LOGV(TAG, "TX: pwr (slave mode)");
    this->pending_getpwr_after_pwr_ = this->enable_getpwr_;
    this->start_response_capture_("pwr");
    this->write_str("pwr\n");
  }
}

void PylontechComponent::schedule_next_automatic_info_(uint32_t delay) {
  if (!this->enable_info_) {
    ESP_LOGV(TAG, "Automatic info not scheduled because it is disabled");
    return;
  }
  this->cancel_timeout("automatic_info");
  ESP_LOGV(TAG, "Automatic info scheduled in %ums", delay);
  this->set_timeout("automatic_info", delay, [this]() {
    if (this->command_busy_()) {
      ESP_LOGV(TAG, "Automatic info queued because another Pylontech sequence is active");
      this->pending_info_request_ = true;
      return;
    }
    ESP_LOGV(TAG, "Starting automatic info sequence");
    if (!this->start_info_sequence_()) {
      ESP_LOGW(TAG, "Automatic info sequence could not start; retrying in 10s");
      this->schedule_next_automatic_info_(10000);
      return;
    }
    this->schedule_next_automatic_info_(this->info_interval_);
  });
}

bool PylontechComponent::request_info_update() {
  if (this->command_busy_()) {
    ESP_LOGV(TAG, "Info request queued because another Pylontech sequence is already active");
    this->pending_info_request_ = true;
    return false;
  }
  if (this->enable_info_) {
    this->schedule_next_automatic_info_(this->info_interval_);
  }
  return this->start_info_sequence_();
}

bool PylontechComponent::request_getpwr_update() {
  if (this->command_busy_()) {
    ESP_LOGV(TAG, "Getpwr request ignored because another Pylontech sequence is already active");
    return false;
  }
  return this->start_getpwr_sequence_();
}

void PylontechComponent::schedule_next_automatic_slow_(uint32_t delay) {
  if (!this->enable_stat_ && !this->enable_bat_ && !this->enable_soh_) {
    ESP_LOGV(TAG, "Automatic slow loop not scheduled because it is disabled");
    return;
  }
  this->cancel_timeout("automatic_slow");
  ESP_LOGV(TAG, "Automatic slow loop scheduled in %ums", delay);
  this->set_timeout("automatic_slow", delay, [this]() {
    if (this->command_busy_()) {
      ESP_LOGV(TAG, "Automatic slow loop queued because another Pylontech sequence is active");
      this->pending_slow_request_ = true;
      return;
    }
    ESP_LOGV(TAG, "Starting automatic slow loop sequence");
    if (!this->start_slow_sequence_()) {
      ESP_LOGW(TAG, "Automatic slow loop could not start; retrying in 10s");
      this->schedule_next_automatic_slow_(10000);
      return;
    }
  });
}

bool PylontechComponent::request_stat_update() {
  if (this->command_busy_()) {
    if (!this->pending_stat_request_) {
      ESP_LOGV(TAG, "Stat request queued because another Pylontech sequence is already active");
    } else {
      ESP_LOGV(TAG, "Stat request already queued");
    }
    this->pending_stat_request_ = true;
    return false;
  }
  this->pending_stat_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  return this->start_stat_sequence_();
}

bool PylontechComponent::request_bat_update() {
  if (this->command_busy_()) {
    if (!this->pending_bat_request_) {
      ESP_LOGV(TAG, "Bat request queued because another Pylontech sequence is already active");
    } else {
      ESP_LOGV(TAG, "Bat request already queued");
    }
    this->pending_bat_request_ = true;
    return false;
  }
  this->pending_bat_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  return this->start_bat_sequence_();
}

bool PylontechComponent::request_soh_update() {
  if (this->command_busy_()) {
    if (!this->pending_soh_request_) {
      ESP_LOGV(TAG, "Soh request queued because another Pylontech sequence is already active");
    } else {
      ESP_LOGV(TAG, "Soh request already queued");
    }
    this->pending_soh_request_ = true;
    return false;
  }
  this->pending_soh_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  return this->start_soh_sequence_();
}

bool PylontechComponent::request_login_debug() {
  if (this->command_busy_()) {
    this->pending_login_debug_ = true;
    return false;
  }
  return this->start_login_debug_();
}

bool PylontechComponent::request_us2000b_initialization() {
  if (this->command_busy_()) {
    this->pending_us2000b_initialization_ = true;
    return false;
  }
  return this->start_us2000b_initialization_();
}

bool PylontechComponent::start_login_debug_() {
  if (this->login_debug_active_ || this->us2000b_initialization_active_) {
    return false;
  }
  this->login_debug_active_ = true;
  this->pending_login_debug_ = false;
  this->last_login_debug_recovery_ms_ = millis();
  this->login_debug_recovery_has_run_ = true;
  publish_text_if_changed(this->last_command_text_sensor_, "LOGIN DEBUG");
  ESP_LOGI(TAG, "TX: login debug");
  this->write_str("login debug\n");
  this->set_timeout("login_debug_complete", 500, [this]() {
    this->login_debug_active_ = false;
    this->set_timeout("login_debug_retry_fast", 250, [this]() {
      if (!this->command_busy_()) {
        this->update();
      }
    });
    this->process_pending_requests_();
  });
  return true;
}

bool PylontechComponent::start_us2000b_initialization_() {
#if defined(USE_ESP32) || defined(USE_ESP8266)
  if (this->us2000b_initialization_active_ || this->login_debug_active_) {
    return false;
  }
  static const uint8_t US2000B_INIT_FRAME[] = {0x7E, 0x32, 0x30, 0x30, 0x31, 0x34, 0x36, 0x38,
                                               0x32, 0x43, 0x30, 0x30, 0x34, 0x38, 0x35, 0x32,
                                               0x30, 0x46, 0x43, 0x43, 0x33, 0x0D};
  this->us2000b_initialization_active_ = true;
  this->pending_us2000b_initialization_ = false;
  this->last_login_debug_recovery_ms_ = millis();
  this->login_debug_recovery_has_run_ = true;
  this->us2000b_original_baud_rate_ = this->parent_->get_baud_rate();
  if (this->us2000b_original_baud_rate_ != 115200) {
    ESP_LOGW(TAG, "US2000B initialization expects normal operation at 115200 baud, configured baud is %u",
             this->us2000b_original_baud_rate_);
  }
  publish_text_if_changed(this->last_command_text_sensor_, "US2000B INIT");
  ESP_LOGW(TAG, "Starting US2000B-only initialization at 1200 baud");
  this->parent_->set_baud_rate(1200);
  this->parent_->load_settings(false);
  this->set_timeout("us2000b_send_frame", 100, [this]() {
    this->write_array(US2000B_INIT_FRAME, sizeof(US2000B_INIT_FRAME));
  });
  this->set_timeout("us2000b_restore_baud", 600, [this]() {
    this->parent_->set_baud_rate(115200);
    this->parent_->load_settings(false);
    this->write_str("login debug\n");
  });
  this->set_timeout("us2000b_complete", 1200, [this]() { this->finish_us2000b_initialization_(); });
  return true;
#else
  ESP_LOGE(TAG, "US2000B initialization requires a platform supporting runtime UART reload");
  return false;
#endif
}

void PylontechComponent::finish_us2000b_initialization_() {
  this->us2000b_initialization_active_ = false;
  this->fast_command_failures_ = 0;
  ESP_LOGI(TAG, "US2000B initialization sequence complete");
  this->set_timeout("us2000b_retry_fast", 250, [this]() {
    if (!this->command_busy_()) {
      this->update();
    }
  });
  this->process_pending_requests_();
}

bool PylontechComponent::start_info_sequence_() {
  const uint32_t now = millis();
  ESP_LOGV(TAG, "Preparing info sequence for %u declared battery listener(s)",
           static_cast<unsigned>(this->batteries_.size()));
  if (this->is_slave_mode()) {
    this->next_info_address_ = 0;
    this->send_info_for_address_(1);
    this->info_busy_until_ms_ = now + 3000;
    this->set_timeout("clear_info_context", 3000, [this]() { this->clear_info_context_(); });
    return true;
  }

  uint8_t first_address = this->next_battery_address_(0);
  if (first_address == 0) {
    return false;
  }
  this->next_info_address_ = this->next_battery_address_(first_address);
  this->send_info_for_address_(first_address);
  uint32_t timeout = (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000;
  this->info_busy_until_ms_ = now + timeout;
  this->set_timeout("clear_info_context", timeout, [this]() { this->clear_info_context_(); });
  return true;
}

void PylontechComponent::send_info_for_address_(uint8_t address) {
  this->active_info_address_ = address;
  this->begin_diagnostic_command_("INFO", address);
  this->info_response_buffer_.clear();
  this->info_full_response_buffer_.clear();
  this->info_response_truncated_ = false;
  if (this->wants_info_response_(address)) {
    this->info_full_response_buffer_.reserve(PYLONTECH_MAX_INFO_RESPONSE_LENGTH);
  }

  if (this->is_slave_mode() || address <= 1) {
    ESP_LOGV(TAG, "TX: info (battery %u, line parser v4)", address);
    this->write_str("info\n");
    return;
  }

  char command[16];
  std::snprintf(command, sizeof(command), "info %u\n", address);
  ESP_LOGV(TAG, "TX: info %u (line parser v4)", address);
  this->write_str(command);
}

bool PylontechComponent::start_getpwr_sequence_() {
  const uint32_t now = millis();
  ESP_LOGV(TAG, "Preparing getpwr sequence for %u declared battery listener(s)",
           static_cast<unsigned>(this->batteries_.size()));

  if (this->is_slave_mode()) {
    this->next_getpwr_address_ = 0;
    this->send_getpwr_for_address_(1);
    this->getpwr_busy_until_ms_ = now + 1500;
    this->set_timeout("clear_getpwr_context", 1500, [this]() { this->clear_getpwr_context_(); });
    return true;
  }

  uint8_t first_address = this->next_battery_address_(0);
  if (first_address == 0) {
    return false;
  }
  this->next_getpwr_address_ = this->next_battery_address_(first_address);
  this->send_getpwr_for_address_(first_address);
  this->getpwr_busy_until_ms_ = now + (static_cast<uint32_t>(this->batteries_.size()) * 2500) + 2000;
  this->set_timeout("clear_getpwr_context", (static_cast<uint32_t>(this->batteries_.size()) * 2500) + 2000,
                    [this]() { this->clear_getpwr_context_(); });
  return true;
}

void PylontechComponent::send_getpwr_for_address_(uint8_t address) {
  this->active_getpwr_address_ = address;
  this->begin_diagnostic_command_("GETPWR", address);
  this->active_getpwr_cell_ = 0;
  this->active_getpwr_tail_published_ = false;
  this->active_getpwr_tail_pending_ = false;
  this->active_getpwr_tail_1_value_ = 0;
  this->active_getpwr_summary_field_count_ = 0;
  this->active_getpwr_cell_field_ = 0;
  this->active_getpwr_cell_voltage_mv_ = 0;
  this->active_getpwr_cell_temperature_mc_ = 0;
  this->active_getpwr_cell_voltage_state_.clear();
  this->active_getpwr_cell_temperature_state_.clear();
  this->getpwr_response_buffer_.clear();
  this->getpwr_response_truncated_ = false;
  if (this->wants_getpwr_response_(address)) {
    this->getpwr_response_buffer_.reserve(PYLONTECH_MAX_INFO_RESPONSE_LENGTH);
  }

  if (address <= 1) {
    ESP_LOGV(TAG, "TX: getpwr (battery %u)", address);
    this->write_str("getpwr\n");
    return;
  }

  char command[16];
  std::snprintf(command, sizeof(command), "getpwr %u\n", address);
  ESP_LOGV(TAG, "TX: getpwr %u", address);
  this->write_str(command);
}

bool PylontechComponent::start_stat_sequence_() {
  const uint32_t now = millis();
  ESP_LOGV(TAG, "Preparing stat sequence for %u declared battery listener(s)",
           static_cast<unsigned>(this->batteries_.size()));

  if (this->is_slave_mode()) {
    this->next_stat_address_ = 0;
    this->send_stat_for_address_(1);
    this->stat_busy_until_ms_ = now + 1500;
    this->set_timeout("clear_stat_context", 1500, [this]() { this->clear_stat_context_(); });
    return true;
  }

  uint8_t first_address = this->next_battery_address_(0);
  if (first_address == 0) {
    return false;
  }
  this->next_stat_address_ = this->next_battery_address_(first_address);
  this->send_stat_for_address_(first_address);
  this->stat_busy_until_ms_ = now + (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000;
  this->set_timeout("clear_stat_context", (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000,
                    [this]() { this->clear_stat_context_(); });
  return true;
}

void PylontechComponent::send_stat_for_address_(uint8_t address) {
  this->active_stat_address_ = address;
  this->begin_diagnostic_command_("STAT", address);
  this->stat_response_buffer_.clear();
  this->stat_response_truncated_ = false;
  if (this->wants_stat_response_(address)) {
    this->stat_response_buffer_.reserve(PYLONTECH_MAX_INFO_RESPONSE_LENGTH);
  }

  if (this->is_slave_mode() || address <= 1) {
    ESP_LOGV(TAG, "TX: stat (battery %u)", address);
    this->write_str("stat\n");
    return;
  }

  char command[16];
  std::snprintf(command, sizeof(command), "stat %u\n", address);
  ESP_LOGV(TAG, "TX: stat %u", address);
  this->write_str(command);
}

bool PylontechComponent::start_slow_sequence_() {
  this->pending_bat_after_stat_ = false;
  this->pending_soh_after_stat_ = false;
  this->pending_soh_after_bat_ = false;
  this->active_slow_sequence_ = true;
  if (this->enable_stat_) {
    this->pending_bat_after_stat_ = this->enable_bat_;
    this->pending_soh_after_stat_ = !this->enable_bat_ && this->enable_soh_;
    this->pending_soh_after_bat_ = this->enable_bat_ && this->enable_soh_;
    if (this->start_stat_sequence_()) {
      return true;
    }
    this->active_slow_sequence_ = false;
    return false;
  }
  if (this->enable_bat_) {
    this->pending_soh_after_bat_ = this->enable_soh_;
    if (this->start_bat_sequence_()) {
      return true;
    }
    this->active_slow_sequence_ = false;
    return false;
  }
  if (this->enable_soh_) {
    if (this->start_soh_sequence_()) {
      return true;
    }
    this->active_slow_sequence_ = false;
    return false;
  }
  this->active_slow_sequence_ = false;
  return false;
}

bool PylontechComponent::start_bat_sequence_() {
  const uint32_t now = millis();
  ESP_LOGV(TAG, "Preparing bat sequence for %u declared battery listener(s)",
           static_cast<unsigned>(this->batteries_.size()));

  if (this->is_slave_mode()) {
    this->next_bat_address_ = 0;
    this->send_bat_for_address_(1);
    this->bat_busy_until_ms_ = now + 2000;
    this->set_timeout("clear_bat_context", 2000, [this]() { this->clear_bat_context_(); });
    return true;
  }

  uint8_t first_address = this->next_battery_address_(0);
  if (first_address == 0) {
    return false;
  }
  this->next_bat_address_ = this->next_battery_address_(first_address);
  this->send_bat_for_address_(first_address);
  this->bat_busy_until_ms_ = now + (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000;
  this->set_timeout("clear_bat_context", (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000,
                    [this]() { this->clear_bat_context_(); });
  return true;
}

void PylontechComponent::send_bat_for_address_(uint8_t address) {
  this->active_bat_address_ = address;
  this->begin_diagnostic_command_("BAT", address);
  this->bat_response_buffer_.clear();
  this->bat_response_truncated_ = false;
  if (this->wants_bat_response_(address)) {
    this->bat_response_buffer_.reserve(PYLONTECH_MAX_INFO_RESPONSE_LENGTH);
  }

  if (this->is_slave_mode() || address <= 1) {
    ESP_LOGV(TAG, "TX: bat (battery %u)", address);
    this->write_str("bat\n");
    return;
  }

  char command[16];
  std::snprintf(command, sizeof(command), "bat %u\n", address);
  ESP_LOGV(TAG, "TX: bat %u", address);
  this->write_str(command);
}

bool PylontechComponent::start_soh_sequence_() {
  const uint32_t now = millis();
  if (this->is_slave_mode()) {
    this->next_soh_address_ = 0;
    this->send_soh_for_address_(1);
    this->soh_busy_until_ms_ = now + 3000;
    this->set_timeout("clear_soh_context", 3000, [this]() { this->clear_soh_context_(); });
    return true;
  }

  uint8_t first_address = this->next_battery_address_(0);
  if (first_address == 0) {
    return false;
  }
  this->next_soh_address_ = this->next_battery_address_(first_address);
  this->send_soh_for_address_(first_address);
  this->soh_busy_until_ms_ = now + (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000;
  this->set_timeout("clear_soh_context", (static_cast<uint32_t>(this->batteries_.size()) * 3000) + 2000,
                    [this]() { this->clear_soh_context_(); });
  return true;
}

void PylontechComponent::send_soh_for_address_(uint8_t address) {
  this->active_soh_address_ = address;
  this->begin_diagnostic_command_("SOH", address);
  this->soh_response_buffer_.clear();
  this->soh_response_truncated_ = false;
  if (this->wants_soh_response_(address)) {
    this->soh_response_buffer_.reserve(PYLONTECH_MAX_INFO_RESPONSE_LENGTH);
  }

  if (this->is_slave_mode() || address <= 1) {
    ESP_LOGV(TAG, "TX: soh (battery %u)", address);
    this->write_str("soh\n");
    return;
  }

  char command[16];
  std::snprintf(command, sizeof(command), "soh %u\n", address);
  ESP_LOGV(TAG, "TX: soh %u", address);
  this->write_str(command);
}

bool PylontechComponent::has_battery_address_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address) {
      return true;
    }
  }
  return false;
}

uint8_t PylontechComponent::next_battery_address_(uint8_t after) const {
  uint8_t next = 0;
  for (auto *battery : this->batteries_) {
    if (battery == nullptr || battery->address() <= after) {
      continue;
    }
    if (next == 0 || battery->address() < next) {
      next = battery->address();
    }
  }
  return next;
}

void PylontechComponent::clear_info_context_() {
  ESP_LOGV(TAG, "Info sequence complete");
  this->cancel_timeout("send_next_info");
  this->timeout_diagnostic_command_();
  this->active_info_address_ = 0;
  this->next_info_address_ = 0;
  this->info_response_buffer_.clear();
  this->info_full_response_buffer_.clear();
  this->info_response_truncated_ = false;
  this->process_pending_requests_();
}

void PylontechComponent::loop() {
  uint8_t data;
  uint16_t bytes_read = 0;

  while (this->available() > 0 && bytes_read < PYLONTECH_MAX_BYTES_PER_LOOP) {
    // Keep one slot free so a burst cannot wrap the ring and overwrite unread lines.
    // Remaining bytes stay in the UART RX buffer and are consumed on the next loop.
    const uint8_t next_write = (this->buffer_index_write_ + 1) % PYLONTECH_NUM_BUFFERS;
    if (next_write == this->buffer_index_read_ && this->buffer_[this->buffer_index_write_].empty()) {
      if (this->diagnostics_enabled_ && !this->uart_queue_full_latched_) {
        this->uart_queue_full_latched_ = true;
        this->diagnostic_uart_queue_full_count_++;
        this->publish_sensor_if_changed(this->uart_queue_full_count_sensor_,
                                        this->diagnostic_uart_queue_full_count_);
      }
      break;
    }
    if (this->diagnostics_enabled_) {
      this->uart_queue_full_latched_ = false;
    }
    if (this->read_byte(&data)) {
      bytes_read++;
      if (this->active_info_address_ != 0) {
        this->process_info_byte_(data);
        continue;
      }

      this->buffer_[this->buffer_index_write_] += static_cast<char>(data);
      if (data == ASCII_LF ||
          this->buffer_[this->buffer_index_write_].length() >= PYLONTECH_MAX_LINE_LENGTH) {
        ESP_LOGVV(TAG, "RX line buffered (%u bytes): %s",
                  static_cast<unsigned>(this->buffer_[this->buffer_index_write_].length()),
                  this->buffer_[this->buffer_index_write_].c_str());
        this->buffer_index_write_ = (this->buffer_index_write_ + 1) % PYLONTECH_NUM_BUFFERS;
        this->buffer_[this->buffer_index_write_].clear();
        this->update_uart_queue_diagnostics_();
      }
    }
  }

  uint8_t queue_depth = (this->buffer_index_write_ + PYLONTECH_NUM_BUFFERS - this->buffer_index_read_) %
                        PYLONTECH_NUM_BUFFERS;
  uint8_t process_limit = queue_depth >= 16 ? 4 : 2;
  uint8_t processed = 0;
  while (this->buffer_index_read_ != this->buffer_index_write_ && processed < process_limit) {
    const auto &queued_line = this->buffer_[this->buffer_index_read_];
    std::string line(queued_line.data(), queued_line.size());
    this->process_line_(line);
    this->buffer_[this->buffer_index_read_].clear();
    this->buffer_index_read_ = (this->buffer_index_read_ + 1) % PYLONTECH_NUM_BUFFERS;
    processed++;
  }
}

void PylontechComponent::process_line_(std::string &line) {
  std::string current;
  auto dispatch_record = [this](const std::string &raw_record) {
    size_t first = raw_record.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
      return;
    }
    size_t last = raw_record.find_last_not_of(" \t\r\n");
    this->dispatch_line_(raw_record.substr(first, last - first + 1));
  };

  for (char c : line) {
    if (c == '\r' || c == '\n') {
      if (!current.empty()) {
        dispatch_record(current);
      }
      current.clear();
    } else {
      current += c;
    }
  }
  dispatch_record(current);
}

void PylontechComponent::dispatch_line_(const std::string &line) {
  ESP_LOGV(TAG, "RX: %s", line.c_str());
  this->append_response_capture_line_(line);
  if (line.find("Unknown command") != std::string::npos) {
    this->fail_diagnostic_command_("Unknown command");
  }

  if (this->process_info_line_(line)) {
    return;
  }

  if (this->process_getpwr_line_(line)) {
    return;
  }

  if (this->process_stat_line_(line)) {
    return;
  }

  if (this->process_bat_line_(line)) {
    return;
  }

  if (this->process_soh_line_(line)) {
    return;
  }

  const bool processing_fast_power =
      this->response_capture_command_ == "pwr" || this->response_capture_command_ == "pwrsys";

  if (this->is_slave_mode()) {
    if (processing_fast_power && this->looks_like_slave_power_line_(line)) {
      this->observe_role_(PYLONTECH_ROLE_SLAVE);
      this->process_slave_power_line_(line);
      return;
    }
    PylontechPowerLine detected_master_line;
    if (processing_fast_power && this->parse_master_power_line_(line, &detected_master_line)) {
      this->observe_role_(PYLONTECH_ROLE_MASTER);
      return;
    }
    if (!this->process_slave_power_line_(line)) {
      ESP_LOGVV(TAG, "RX ignored in slave mode");
    }
    return;
  }

  if (this->process_system_line_(line)) {
    if (processing_fast_power) {
      this->observe_role_(PYLONTECH_ROLE_MASTER);
    }
    return;
  }

  if (processing_fast_power && this->looks_like_slave_power_line_(line)) {
    this->observe_role_(PYLONTECH_ROLE_SLAVE);
    return;
  }

  PylontechPowerLine power_line;
  if (!this->parse_master_power_line_(line, &power_line)) {
    ESP_LOGVV(TAG, "RX ignored: not a pwr data row");
    return;
  }
  if (processing_fast_power) {
    this->observe_role_(PYLONTECH_ROLE_MASTER);
  }

  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == power_line.battery_address) {
      ESP_LOGV(TAG, "RX parsed row for declared battery %d", power_line.battery_address);
      battery->publish_power_line(power_line);
      published = true;
    }
  }

  if (!published) {
    ESP_LOGV(TAG, "RX parsed row for battery %d, but no sensor block is declared for it",
             power_line.battery_address);
  }
}

bool PylontechComponent::parse_master_power_line_(const std::string &line, PylontechPowerLine *out) {
  const char *start = line.c_str();
  while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
    start++;
  }
  if (*start < '0' || *start > '9') {
    return false;
  }

  char base_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char voltage_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char current_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char temperature_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char battery_voltage_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char battery_temperature_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char mos_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};

  int parsed = std::sscanf(
      line.c_str(),
      "%d %d %d %d %d %d %d %d %15s %15s %15s %15s %d%% %*d-%*d-%*d %*d:%*d:%*d %15s %15s %d %15s",
      &out->battery_address, &out->voltage_mv, &out->current_ma, &out->temperature_mc,
      &out->temperature_low_mc, &out->temperature_high_mc, &out->voltage_low_mv, &out->voltage_high_mv,
      base_state, voltage_state, current_state, temperature_state, &out->soc_percent, battery_voltage_state,
      battery_temperature_state, &out->mos_temperature_mc, mos_state);

  if (parsed < 13) {
    ESP_LOGV(TAG, "Not a complete Pylontech pwr row: parsed %d fields from: %s", parsed, start);
    return false;
  }

  out->base_state = base_state;
  out->voltage_state = voltage_state;
  out->current_state = current_state;
  out->temperature_state = temperature_state;
  if (parsed >= 15) {
    out->battery_voltage_state = battery_voltage_state;
    out->battery_temperature_state = battery_temperature_state;
  }
  if (parsed >= 16) {
    out->has_mos_temperature = true;
  }
  if (parsed >= 17) {
    out->mos_state = mos_state;
  }

  ESP_LOGV(TAG,
           "Parsed pwr row: bat=%d fields=%d volt=%dmV curr=%dmA temp=%dmC coulomb=%d%% states=%s/%s/%s/%s",
           out->battery_address, parsed, out->voltage_mv, out->current_ma, out->temperature_mc,
           out->soc_percent, out->base_state.c_str(), out->voltage_state.c_str(),
           out->current_state.c_str(), out->temperature_state.c_str());

  return out->battery_address > 0;
}

bool PylontechComponent::process_system_line_(const std::string &line) {
  std::string key;
  std::string value;
  if (!this->extract_key_value_(line, &key, &value)) {
    const std::string marker = "System is";
    size_t pos = line.find(marker);
    if (pos == std::string::npos) {
      return false;
    }
    key = marker;
    value = line.substr(pos + marker.length());
    const char *whitespace = " :\t\r\n";
    size_t first = value.find_first_not_of(whitespace);
    value = first == std::string::npos ? "" : value.substr(first);
    size_t last = value.find_last_not_of(" \t\r\n");
    if (last != std::string::npos) {
      value = value.substr(0, last + 1);
    }
  }

  if (key == "System is") {
    ESP_LOGV(TAG, "Parsed pwrsys field: system_status=%s", value.c_str());
    publish_text_if_changed(this->system_status_text_sensor_, value);
    return true;
  }
  if (key == "Total Num") {
    float total = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed pwrsys field: total_num=%.0f", total);
    publish_sensor_if_changed(this->total_num_sensor_, total);
    return true;
  }
  if (key == "Present Num") {
    float present = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed pwrsys field: present_num=%.0f", present);
    publish_sensor_if_changed(this->present_num_sensor_, present);
    return true;
  }
  if (key == "Sleep Num") {
    float sleep = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed pwrsys field: sleep_num=%.0f", sleep);
    publish_sensor_if_changed(this->sleep_num_sensor_, sleep);
    return true;
  }
  if (key == "System Volt") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: system_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->system_voltage_sensor_, voltage);
    return true;
  }
  if (key == "System Curr") {
    float current = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: system_current=%.3fA", current);
    publish_sensor_if_changed(this->system_current_sensor_, current);
    return true;
  }
  if (key == "System RC") {
    float rc = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: system_rc=%.3fAh", rc);
    publish_sensor_if_changed(this->system_rc_sensor_, rc);
    return true;
  }
  if (key == "System FCC") {
    float fcc = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: system_fcc=%.3fAh", fcc);
    publish_sensor_if_changed(this->system_fcc_sensor_, fcc);
    return true;
  }
  if (key == "System SOC") {
    float soc = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed pwrsys field: system_soc=%.0f%%", soc);
    publish_sensor_if_changed(this->system_soc_sensor_, soc);
    return true;
  }
  if (key == "System SOH") {
    float soh = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed pwrsys field: system_soh=%.0f%%", soh);
    publish_sensor_if_changed(this->system_soh_sensor_, soh);
    return true;
  }
  if (key == "Highest voltage") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: highest_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->highest_voltage_sensor_, voltage);
    return true;
  }
  if (key == "Average voltage") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: average_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->average_voltage_sensor_, voltage);
    return true;
  }
  if (key == "Lowest voltage") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: lowest_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->lowest_voltage_sensor_, voltage);
    return true;
  }
  if (key == "Highest temperature") {
    float temperature = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: highest_temperature=%.1fC", temperature);
    publish_sensor_if_changed(this->highest_temperature_sensor_, temperature);
    return true;
  }
  if (key == "Average temperature") {
    float temperature = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: average_temperature=%.1fC", temperature);
    publish_sensor_if_changed(this->average_temperature_sensor_, temperature);
    return true;
  }
  if (key == "Lowest temperature") {
    float temperature = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: lowest_temperature=%.1fC", temperature);
    publish_sensor_if_changed(this->lowest_temperature_sensor_, temperature);
    return true;
  }
  if (key == "Recommend chg voltage") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: recommend_charge_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->recommend_charge_voltage_sensor_, voltage);
    return true;
  }
  if (key == "Recommend dsg voltage") {
    float voltage = std::abs(this->extract_int_(value)) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: recommend_discharge_voltage=%.3fV", voltage);
    publish_sensor_if_changed(this->recommend_discharge_voltage_sensor_, voltage);
    return true;
  }
  if (key == "Recommend chg current") {
    float current = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: recommend_charge_current=%.3fA", current);
    publish_sensor_if_changed(this->recommend_charge_current_sensor_, current);
    return true;
  }
  if (key == "Recommend dsg current") {
    float current = std::abs(this->extract_int_(value)) / 1000.0f;
    ESP_LOGV(TAG, "Parsed pwrsys field: recommend_discharge_current=%.3fA", current);
    publish_sensor_if_changed(this->recommend_discharge_current_sensor_, current);
    return true;
  }

  return false;
}

bool PylontechComponent::process_info_byte_(uint8_t data) {
  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  if (this->wants_info_response_(this->active_info_address_)) {
    if (this->info_full_response_buffer_.length() < PYLONTECH_MAX_INFO_RESPONSE_LENGTH) {
      this->info_full_response_buffer_ += static_cast<char>(data);
    } else {
      this->info_response_truncated_ = true;
    }
  }

  if (data == '\r' || data == '\n') {
    std::string line = trim(std::string(this->info_response_buffer_.data(), this->info_response_buffer_.size()));
    this->info_response_buffer_.clear();
    if (line.empty()) {
      return true;
    }

    ESP_LOGV(TAG, "INFO RX line parser v4 battery %u: %s", this->active_info_address_, line.c_str());
    if (line == "Command completed successfully") {
      uint8_t completed_address = this->active_info_address_;
      this->complete_diagnostic_command_("INFO", completed_address);
      this->publish_info_response_();
      ESP_LOGV(TAG, "Info response complete for battery %u", completed_address);
      this->active_info_address_ = 0;
      if (this->next_info_address_ != 0) {
        uint8_t next_address = this->next_info_address_;
        this->next_info_address_ = this->next_battery_address_(next_address);
        this->set_timeout("send_next_info", 250,
                          [this, next_address]() { this->send_info_for_address_(next_address); });
      } else {
        this->cancel_timeout("clear_info_context");
        this->info_busy_until_ms_ = 0;
        this->clear_info_context_();
      }
      return true;
    }

    this->process_info_line_(line);
    return true;
  }

  this->info_response_buffer_ += static_cast<char>(data);
  if (this->info_response_buffer_.length() >= PYLONTECH_MAX_LINE_LENGTH) {
    ESP_LOGW(TAG, "Info line too long for battery %u, dropping partial line", this->active_info_address_);
    this->info_response_buffer_.clear();
  }
  return true;
}

bool PylontechComponent::process_info_line_(const std::string &line) {
  if (this->active_info_address_ == 0) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  auto normalize_key = [&trim](const std::string &text) {
    std::string normalized;
    bool previous_space = false;
    for (unsigned char c : trim(text)) {
      if (c == ' ' || c == '\t') {
        if (!normalized.empty() && !previous_space) {
          normalized += ' ';
        }
        previous_space = true;
      } else {
        normalized += static_cast<char>(std::tolower(c));
        previous_space = false;
      }
    }
    return trim(normalized);
  };

  std::string key;
  std::string value;
  if (!this->extract_key_value_(line, &key, &value)) {
    ESP_LOGV(TAG, "INFO ignored battery %u: %s", this->active_info_address_, line.c_str());
    return true;
  }

  std::string normalized_key = normalize_key(key);
  ESP_LOGV(TAG, "INFO field battery %u: key='%s' value='%s'", this->active_info_address_, key.c_str(),
           value.c_str());

  if (normalized_key == "device address") {
    int reported_address = this->extract_int_(value);
    if (reported_address != this->active_info_address_) {
      ESP_LOGW(TAG, "INFO address mismatch: requested battery %u, response reports %d", this->active_info_address_,
               reported_address);
    }
    this->publish_info_number_(&PylontechBattery::publish_device_address, reported_address);
  } else if (normalized_key == "cell number") {
    this->publish_info_number_(&PylontechBattery::publish_cell_number, this->extract_int_(value));
  } else if (normalized_key == "max charge curr") {
    this->publish_info_number_(&PylontechBattery::publish_max_charge_current, this->extract_int_(value) / 1000.0f);
  } else if (normalized_key == "max dischg curr") {
    this->publish_info_number_(&PylontechBattery::publish_max_discharge_current,
                               std::abs(this->extract_int_(value)) / 1000.0f);
  } else if (normalized_key == "manufacturer" || normalized_key == "manufacture") {
    this->publish_info_text_(&PylontechBattery::publish_manufacturer, value);
  } else if (normalized_key == "device name") {
    this->publish_info_text_(&PylontechBattery::publish_device_name, value);
  } else if (normalized_key == "board version") {
    this->publish_info_text_(&PylontechBattery::publish_board_version, value);
  } else if (normalized_key == "board") {
    this->publish_info_text_(&PylontechBattery::publish_board, value);
  } else if (normalized_key == "main soft version" || normalized_key == "main software version") {
    this->publish_info_text_(&PylontechBattery::publish_main_soft_version, value);
  } else if (normalized_key == "soft version") {
    this->publish_info_text_(&PylontechBattery::publish_soft_version, value);
  } else if (normalized_key == "boot version") {
    this->publish_info_text_(&PylontechBattery::publish_boot_version, value);
  } else if (normalized_key == "comm version") {
    this->publish_info_text_(&PylontechBattery::publish_comm_version, value);
  } else if (normalized_key == "release date") {
    this->publish_info_text_(&PylontechBattery::publish_release_date, value);
  } else if (normalized_key == "barcode") {
    this->publish_info_text_(&PylontechBattery::publish_barcode, value);
  } else if (normalized_key == "specification") {
    this->publish_info_text_(&PylontechBattery::publish_specification, value);
  } else if (normalized_key == "eponport rate" || normalized_key == "epon port rate") {
    this->publish_info_text_(&PylontechBattery::publish_epon_port_rate, value);
  } else if (normalized_key == "console port rate") {
    this->publish_info_text_(&PylontechBattery::publish_console_port_rate, value);
  } else {
    ESP_LOGV(TAG, "INFO unhandled field battery %u: key='%s' value='%s'", this->active_info_address_, key.c_str(),
             value.c_str());
  }

  return true;
}

bool PylontechComponent::process_getpwr_line_(const std::string &line) {
  if (this->active_getpwr_address_ == 0) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  std::string cleaned = trim(line);
  if (cleaned.empty()) {
    return true;
  }
  if (this->wants_getpwr_response_(this->active_getpwr_address_)) {
    this->append_limited_response_(&this->getpwr_response_buffer_, &this->getpwr_response_truncated_, cleaned);
  }

  if (cleaned == "Command completed successfully") {
    uint8_t completed_address = this->active_getpwr_address_;
    this->complete_diagnostic_command_("GETPWR", completed_address);
    if (this->wants_getpwr_response_(this->active_getpwr_address_)) {
      this->publish_getpwr_tail_from_response_();
    }
    this->publish_getpwr_response_();
    ESP_LOGV(TAG, "Getpwr response complete for battery %u", completed_address);
    this->active_getpwr_address_ = 0;
    this->active_getpwr_cell_ = 0;
    this->active_getpwr_tail_published_ = false;
    this->active_getpwr_tail_pending_ = false;
    this->active_getpwr_tail_1_value_ = 0;
    this->active_getpwr_summary_field_count_ = 0;
    this->active_getpwr_cell_field_ = 0;
    this->active_getpwr_cell_voltage_mv_ = 0;
    this->active_getpwr_cell_temperature_mc_ = 0;
    this->active_getpwr_cell_voltage_state_.clear();
    this->active_getpwr_cell_temperature_state_.clear();
    if (this->next_getpwr_address_ != 0) {
      uint8_t next_address = this->next_getpwr_address_;
      this->next_getpwr_address_ = this->next_battery_address_(next_address);
      ESP_LOGV(TAG, "GETPWR sequence next battery: completed=%u next=%u", completed_address, next_address);
      this->set_timeout("send_next_getpwr", 250, [this, next_address]() { this->send_getpwr_for_address_(next_address); });
    } else {
      this->cancel_timeout("clear_getpwr_context");
      this->getpwr_busy_until_ms_ = 0;
      this->clear_getpwr_context_();
    }
    return true;
  }

  const bool has_separator = cleaned.find('#') != std::string::npos;
  if (!has_separator) {
    ESP_LOGV(TAG, "GETPWR ignored battery %u: %s", this->active_getpwr_address_, cleaned.c_str());
    return true;
  }

  std::string field;
  field.reserve(24);
  bool saw_summary = this->active_getpwr_cell_ > 0 || this->active_getpwr_summary_field_count_ >= 8;

  auto publish_cell = [this](int voltage_mv, int temperature_mc, const std::string &voltage_state,
                             const std::string &temperature_state) {
    if (voltage_mv < 2000 || voltage_mv > 4500) {
      return;
    }
    this->active_getpwr_cell_++;
    ESP_LOGV(TAG, "Parsed getpwr cell: battery=%u cell=%u voltage=%.3fV temperature=%.1fC states=%s/%s",
             this->active_getpwr_address_, this->active_getpwr_cell_, voltage_mv / 1000.0f,
             temperature_mc / 1000.0f, voltage_state.c_str(), temperature_state.c_str());
    this->publish_getpwr_cell_detail_(this->active_getpwr_cell_, voltage_mv, temperature_mc, voltage_state,
                                      temperature_state);
  };

  auto handle_field = [this, &trim, &saw_summary, &publish_cell](const std::string &raw_field) {
    std::string value = trim(raw_field);
    if (value.empty()) {
      return;
    }

    if (!saw_summary && this->active_getpwr_summary_field_count_ < 8) {
      this->active_getpwr_summary_fields_[this->active_getpwr_summary_field_count_++] = value;
      if (this->active_getpwr_summary_field_count_ == 8) {
        int voltage_mv = this->extract_int_(this->active_getpwr_summary_fields_[0]);
        if (voltage_mv > 10000) {
          int current_ma = this->extract_int_(this->active_getpwr_summary_fields_[1]);
          int temperature_mc = this->extract_int_(this->active_getpwr_summary_fields_[2]);
          int residual_ma = this->extract_int_(this->active_getpwr_summary_fields_[3]);
          ESP_LOGV(TAG,
                   "Parsed getpwr summary: battery=%u voltage=%.3fV current=%.3fA temperature=%.1fC residual=%dmA",
                   this->active_getpwr_address_, voltage_mv / 1000.0f, current_ma / 1000.0f,
                   temperature_mc / 1000.0f, residual_ma);
          this->publish_getpwr_summary_(voltage_mv, current_ma, temperature_mc, residual_ma,
                                        this->active_getpwr_summary_fields_[4],
                                        this->active_getpwr_summary_fields_[5],
                                        this->active_getpwr_summary_fields_[6],
                                        this->active_getpwr_summary_fields_[7]);
          saw_summary = true;
        } else {
          this->active_getpwr_summary_field_count_ = 0;
        }
      }
      return;
    }

    if (!saw_summary) {
      this->active_getpwr_cell_voltage_mv_ = this->extract_int_(value);
      publish_cell(this->active_getpwr_cell_voltage_mv_, 0, "", "");
      return;
    }

    if (this->active_getpwr_tail_pending_) {
      int tail_2_value = this->extract_int_(value);
      ESP_LOGV(TAG, "GETPWR parsed tail: battery=%u tail_1=%d tail_2=%d", this->active_getpwr_address_,
               this->active_getpwr_tail_1_value_, tail_2_value);
      this->publish_getpwr_tail_(this->active_getpwr_tail_1_value_, tail_2_value);
      this->active_getpwr_tail_pending_ = false;
      return;
    }

    if (this->active_getpwr_cell_field_ == 0) {
      int candidate = this->extract_int_(value);
      if (candidate < 2000 || candidate > 4500) {
        this->active_getpwr_tail_1_value_ = candidate;
        this->active_getpwr_tail_pending_ = true;
        return;
      }
      this->active_getpwr_cell_voltage_mv_ = candidate;
      this->active_getpwr_cell_temperature_mc_ = 0;
      this->active_getpwr_cell_voltage_state_.clear();
      this->active_getpwr_cell_temperature_state_.clear();
      this->active_getpwr_cell_field_ = 1;
      return;
    }
    if (this->active_getpwr_cell_field_ == 1) {
      this->active_getpwr_cell_temperature_mc_ = this->extract_int_(value);
      this->active_getpwr_cell_field_ = 2;
      return;
    }
    if (this->active_getpwr_cell_field_ == 2) {
      this->active_getpwr_cell_voltage_state_ = value;
      this->active_getpwr_cell_field_ = 3;
      return;
    }

    this->active_getpwr_cell_temperature_state_ = value;
    publish_cell(this->active_getpwr_cell_voltage_mv_, this->active_getpwr_cell_temperature_mc_,
                 this->active_getpwr_cell_voltage_state_, this->active_getpwr_cell_temperature_state_);
    this->active_getpwr_cell_field_ = 0;
  };

  for (char c : cleaned) {
    if (c == '#') {
      handle_field(field);
      field.clear();
    } else {
      field += c;
    }
  }
  handle_field(field);
  return true;
}

bool PylontechComponent::process_stat_line_(const std::string &line) {
  if (this->active_stat_address_ == 0) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  std::string cleaned = trim(line);
  if (cleaned.empty()) {
    return true;
  }
  ESP_LOGV(TAG, "STAT RX battery %u: %s", this->active_stat_address_, cleaned.c_str());

  if (this->wants_stat_response_(this->active_stat_address_)) {
    this->append_limited_response_(&this->stat_response_buffer_, &this->stat_response_truncated_, cleaned);
  }

  if (cleaned == "Command completed successfully") {
    uint8_t completed_address = this->active_stat_address_;
    this->complete_diagnostic_command_("STAT", completed_address);
    this->publish_stat_response_();
    ESP_LOGV(TAG, "STAT response complete for battery %u", completed_address);
    this->active_stat_address_ = 0;
    if (this->next_stat_address_ != 0) {
      uint8_t next_address = this->next_stat_address_;
      this->next_stat_address_ = this->next_battery_address_(next_address);
      ESP_LOGV(TAG, "STAT sequence next battery: completed=%u next=%u", completed_address, next_address);
      this->set_timeout("send_next_stat", 250, [this, next_address]() { this->send_stat_for_address_(next_address); });
    } else {
      this->cancel_timeout("clear_stat_context");
      this->stat_busy_until_ms_ = 0;
      this->clear_stat_context_();
    }
    return true;
  }

  struct StatField {
    const char *key;
    uint8_t field;
    float scale;
  };
  static const StatField STAT_FIELDS[] = {
      {"Device address", 0, 1.0f},   {"Data Items", 1, 1.0f},       {"HisData Items", 2, 1.0f},
      {"Charge Cnt.", 3, 1.0f},      {"Discharge Cnt.", 4, 1.0f},   {"Charge Times", 5, 1.0f},
      {"Status Cnt.", 6, 1.0f},      {"Idle Times", 7, 1.0f},       {"COC Times", 8, 1.0f},
      {"COC2 Times", 9, 1.0f},       {"DOC Times", 10, 1.0f},       {"DOC2 Times", 11, 1.0f},
      {"COCA Times", 12, 1.0f},      {"DOCA Times", 13, 1.0f},      {"SC Times", 14, 1.0f},
      {"Bat OV Times", 15, 1.0f},    {"Bat HV Times", 16, 1.0f},    {"Bat LV Times", 17, 1.0f},
      {"Bat UV Times", 18, 1.0f},    {"Bat SLP Times", 19, 1.0f},   {"Pwr OV Times", 20, 1.0f},
      {"Pwr HV Times", 21, 1.0f},    {"Pwr LV Times", 22, 1.0f},    {"Pwr UV Times", 23, 1.0f},
      {"Pwr SLP Times", 24, 1.0f},   {"COT Times", 25, 1.0f},       {"CUT Times", 26, 1.0f},
      {"DOT Times", 27, 1.0f},       {"DUT Times", 28, 1.0f},       {"CHT Times", 29, 1.0f},
      {"CLT Times", 30, 1.0f},       {"DHT Times", 31, 1.0f},       {"DLT Times", 32, 1.0f},
      {"Shut Times", 33, 1.0f},      {"Reset Times", 34, 1.0f},     {"RV Times", 35, 1.0f},
      {"Input OV Times", 36, 1.0f},  {"SOH Times", 37, 1.0f},       {"BMICERR Times", 38, 1.0f},
      {"CYCLE Times", 39, 1.0f},     {"SOH", 40, 1.0f},             {"Pwr Percent", 41, 1.0f},
      {"Pwr Coulomb", 42, 3600000.0f}, {"Dsg Cap", 43, 3600000.0f}, {"HT@0.5C Cnt", 44, 1.0f},
      {"LT@0.5C Cnt", 45, 1.0f},     {"HT Cnt", 46, 1.0f},          {"LT Cnt", 47, 1.0f},
      {"LV Cnt", 48, 1.0f},          {"LifeWarn Times", 49, 1.0f},  {"LifeAlarm Times", 50, 1.0f},
      {"MiscData Items", 51, 1.0f},
  };

  auto publish_field = [this](const StatField &field, int parsed) {
    float published = field.scale == 1.0f ? parsed : parsed / field.scale;
    ESP_LOGV(TAG, "STAT parsed battery=%u key='%s' value=%.3f", this->active_stat_address_, field.key, published);
    this->publish_stat_field_(field.field, published);
    if (std::strcmp(field.key, "CYCLE Times") == 0) {
      this->publish_stat_number_(&PylontechBattery::publish_stat_cycle_count, published);
    } else if (std::strcmp(field.key, "Pwr Coulomb") == 0) {
      this->publish_stat_number_(&PylontechBattery::publish_stat_coulomb, published);
    }
  };

  std::string exact_key;
  std::string exact_value;
  if (!this->extract_key_value_(cleaned, &exact_key, &exact_value) && cleaned.rfind("Device address", 0) == 0) {
    exact_key = "Device address";
    exact_value = cleaned.substr(std::strlen("Device address"));
  }
  if (!exact_key.empty()) {
    for (const auto &field : STAT_FIELDS) {
      if (exact_key == field.key) {
        publish_field(field, this->extract_int_(exact_value));
        return true;
      }
    }
  }

  bool parsed_any = false;
  for (const auto &field : STAT_FIELDS) {
    const size_t key_length = std::strlen(field.key);
    size_t search_from = 0;
    while (search_from < cleaned.length()) {
      size_t pos = cleaned.find(field.key, search_from);
      if (pos == std::string::npos) {
        break;
      }

      size_t value_pos = pos + key_length;
      while (value_pos < cleaned.length() && (cleaned[value_pos] == ' ' || cleaned[value_pos] == '\t')) {
        value_pos++;
      }
      if (value_pos < cleaned.length() && cleaned[value_pos] == ':') {
        value_pos++;
        while (value_pos < cleaned.length() && (cleaned[value_pos] == ' ' || cleaned[value_pos] == '\t')) {
          value_pos++;
        }
      }

      // Reject partial matches such as "SOH" inside "SOH Times".
      if (value_pos >= cleaned.length() ||
          (cleaned[value_pos] != '-' && (cleaned[value_pos] < '0' || cleaned[value_pos] > '9'))) {
        search_from = pos + key_length;
        continue;
      }

      publish_field(field, std::atoi(cleaned.c_str() + value_pos));
      parsed_any = true;
      break;
    }
  }

  if (parsed_any) {
    return true;
  }

  std::string key;
  std::string value;
  if (this->extract_key_value_(cleaned, &key, &value) && key == "Coulomb") {
    float coulomb = this->extract_int_(value) / 3600000.0f;
    ESP_LOGV(TAG, "STAT parsed battery=%u key='%s' value=%.3f", this->active_stat_address_, key.c_str(), coulomb);
    this->publish_stat_number_(&PylontechBattery::publish_stat_coulomb, coulomb);
    return true;
  }

  return true;
}

bool PylontechComponent::process_bat_line_(const std::string &line) {
  if (this->active_bat_address_ == 0) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  std::string cleaned = trim(line);
  if (cleaned.empty()) {
    return true;
  }

  if (this->wants_bat_response_(this->active_bat_address_)) {
    this->append_limited_response_(&this->bat_response_buffer_, &this->bat_response_truncated_, cleaned);
  }

  if (cleaned == "Command completed successfully") {
    uint8_t completed_address = this->active_bat_address_;
    this->complete_diagnostic_command_("BAT", completed_address);
    this->publish_bat_response_();
    ESP_LOGV(TAG, "Bat response complete for battery %u", completed_address);
    this->active_bat_address_ = 0;
    if (this->next_bat_address_ != 0) {
      uint8_t next_address = this->next_bat_address_;
      this->next_bat_address_ = this->next_battery_address_(next_address);
      ESP_LOGV(TAG, "BAT sequence next battery: completed=%u next=%u", completed_address, next_address);
      this->set_timeout("send_next_bat", 250, [this, next_address]() { this->send_bat_for_address_(next_address); });
    } else {
      this->cancel_timeout("clear_bat_context");
      this->bat_busy_until_ms_ = 0;
      this->clear_bat_context_();
    }
    return true;
  }

  if (cleaned == "@" || cleaned == "$$" || cleaned.rfind("bat", 0) == 0 ||
      cleaned.find("Battery") != std::string::npos || cleaned.find("Volt") != std::string::npos ||
      cleaned.find("pylon") != std::string::npos) {
    ESP_LOGV(TAG, "BAT ignored battery %u: %s", this->active_bat_address_, cleaned.c_str());
    return true;
  }

  int row_index = 0;
  int voltage_mv = 0;
  int current_ma = 0;
  int temperature_mc = 0;
  int soc_percent = 0;
  int coulomb_mah = 0;
  char base_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char voltage_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char current_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char temperature_state[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char unit_or_balance[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  char balance[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  int parsed = std::sscanf(cleaned.c_str(), "%d %d %d %d %15s %15s %15s %15s %d%% %d %15s %15s", &row_index,
                           &voltage_mv, &current_ma, &temperature_mc, base_state, voltage_state, current_state,
                           temperature_state, &soc_percent, &coulomb_mah, unit_or_balance, balance);
  if (parsed < 10) {
    ESP_LOGV(TAG, "BAT ignored short row battery %u: %s", this->active_bat_address_, cleaned.c_str());
    return true;
  }

  if (row_index < 0 || row_index >= PYLONTECH_MAX_CELLS) {
    return true;
  }

  std::string balance_state;
  if (parsed >= 12) {
    balance_state = balance;
  } else if (parsed >= 11 && std::strcmp(unit_or_balance, "mAH") != 0 &&
             std::strcmp(unit_or_balance, "mAh") != 0 && std::strcmp(unit_or_balance, "mah") != 0) {
    balance_state = unit_or_balance;
  }

  uint8_t cell = static_cast<uint8_t>(row_index + 1);
  ESP_LOGV(TAG,
           "Parsed bat cell: battery=%u cell=%u voltage=%.3fV current=%dmA temperature=%.1fC soc=%d%% "
           "coulomb=%dmAh states=%s/%s bal=%s",
           this->active_bat_address_, cell, voltage_mv / 1000.0f, current_ma, temperature_mc / 1000.0f,
           soc_percent, coulomb_mah, voltage_state, temperature_state, balance_state.c_str());
  this->publish_bat_cell_detail_(cell, voltage_mv, current_ma, temperature_mc, soc_percent, coulomb_mah,
                                 voltage_state, temperature_state, balance_state);
  return true;
}

bool PylontechComponent::process_soh_line_(const std::string &line) {
  if (this->active_soh_address_ == 0) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  std::string cleaned = trim(line);
  if (cleaned.empty()) {
    return true;
  }

  if (this->wants_soh_response_(this->active_soh_address_)) {
    this->append_limited_response_(&this->soh_response_buffer_, &this->soh_response_truncated_, cleaned);
  }

  int row_index = 0;
  int voltage_mv = 0;
  int soh_count = 0;
  char soh_status[PYLONTECH_TEXT_SENSOR_MAX_LEN] = {};
  if (std::sscanf(cleaned.c_str(), "%d %d %d %15s", &row_index, &voltage_mv, &soh_count, soh_status) == 4 &&
      row_index >= 0 &&
      row_index < PYLONTECH_MAX_CELLS && voltage_mv > 1000 && voltage_mv < 6000) {
    uint8_t cell = static_cast<uint8_t>(row_index + 1);
    ESP_LOGV(TAG, "Parsed soh cell: battery=%u cell=%u voltage=%.3fV count=%d status=%s",
             this->active_soh_address_, cell, voltage_mv / 1000.0f, soh_count, soh_status);
    this->publish_soh_cell_detail_(cell, voltage_mv, soh_count, soh_status);
  }

  if (cleaned == "Command completed successfully") {
    uint8_t completed_address = this->active_soh_address_;
    this->complete_diagnostic_command_("SOH", completed_address);
    this->publish_soh_response_();
    ESP_LOGV(TAG, "Soh response complete for battery %u", completed_address);
    this->active_soh_address_ = 0;
    if (this->next_soh_address_ != 0) {
      uint8_t next_address = this->next_soh_address_;
      this->next_soh_address_ = this->next_battery_address_(next_address);
      ESP_LOGV(TAG, "SOH sequence next battery: completed=%u next=%u", completed_address, next_address);
      this->set_timeout("send_next_soh", 250, [this, next_address]() { this->send_soh_for_address_(next_address); });
    } else {
      this->cancel_timeout("clear_soh_context");
      this->soh_busy_until_ms_ = 0;
      this->clear_soh_context_();
    }
  }
  return true;
}

bool PylontechComponent::process_slave_power_line_(const std::string &line) {
  std::string key;
  std::string value;
  if (!this->extract_key_value_(line, &key, &value)) {
    return false;
  }

  if (key == "Voltage") {
    float voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed slave pwr field: voltage=%.3fV", voltage);
    this->publish_slave_number_(&PylontechBattery::publish_voltage, voltage);
    return true;
  }
  if (key == "Current") {
    float current = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed slave pwr field: current=%.3fA", current);
    this->publish_slave_number_(&PylontechBattery::publish_current, current);
    return true;
  }
  if (key == "Temperature") {
    float temperature = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed slave pwr field: temperature=%.1fC", temperature);
    this->publish_slave_number_(&PylontechBattery::publish_temperature, temperature);
    return true;
  }
  if (key == "Coulomb") {
    float soc = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed slave pwr field: coulomb=%.0f%%", soc);
    this->publish_slave_number_(&PylontechBattery::publish_soc, soc);
    return true;
  }
  if (key == "Total Coulomb") {
    float total_coulomb = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed slave pwr field: total_coulomb=%.3fAh", total_coulomb);
    this->publish_slave_number_(&PylontechBattery::publish_total_coulomb, total_coulomb);
    return true;
  }
  if (key == "Max Voltage") {
    float max_voltage = this->extract_int_(value) / 1000.0f;
    ESP_LOGV(TAG, "Parsed slave pwr field: max_voltage=%.3fV", max_voltage);
    this->publish_slave_number_(&PylontechBattery::publish_max_voltage, max_voltage);
    return true;
  }
  if (key == "Charge Times") {
    float charge_times = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed slave pwr field: charge_times=%.0f", charge_times);
    this->publish_slave_number_(&PylontechBattery::publish_charge_times, charge_times);
    return true;
  }
  if (key == "Discharge Sec.") {
    float discharge_sec = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed slave pwr field: discharge_sec=%.0fs", discharge_sec);
    this->publish_slave_number_(&PylontechBattery::publish_discharge_sec, discharge_sec);
    return true;
  }
  if (key == "Charge Sec.") {
    float charge_sec = this->extract_int_(value);
    ESP_LOGV(TAG, "Parsed slave pwr field: charge_sec=%.0fs", charge_sec);
    this->publish_slave_number_(&PylontechBattery::publish_charge_sec, charge_sec);
    return true;
  }
  if (key == "Basic Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: basic_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_base_state, value);
    return true;
  }
  if (key == "Volt Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: voltage_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_voltage_state, value);
    return true;
  }
  if (key == "Current Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: current_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_current_state, value);
    return true;
  }
  if (key == "Tmpr. Status" || key == "Temp Status" || key == "Temperature Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: temperature_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_temperature_state, value);
    return true;
  }
  if (key == "Coul. Status" || key == "Coulomb Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: coulomb_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_coulomb_state, value);
    return true;
  }
  if (key == "Soh. Status" || key == "SOH Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: soh_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_soh_state, value);
    return true;
  }
  if (key == "Heater Status") {
    ESP_LOGV(TAG, "Parsed slave pwr field: heater_status=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_heater_state, value);
    return true;
  }
  if (key == "Protect ENA") {
    ESP_LOGV(TAG, "Parsed slave pwr field: protect_ena=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_protect_ena, value);
    return true;
  }
  if (key == "Bat Events") {
    ESP_LOGV(TAG, "Parsed slave pwr field: bat_events=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_bat_events, value);
    return true;
  }
  if (key == "Power Events") {
    ESP_LOGV(TAG, "Parsed slave pwr field: power_events=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_power_events, value);
    return true;
  }
  if (key == "System Fault") {
    ESP_LOGV(TAG, "Parsed slave pwr field: system_fault=%s", value.c_str());
    this->publish_slave_text_(&PylontechBattery::publish_system_fault, value);
    return true;
  }

  return false;
}

bool PylontechComponent::extract_key_value_(const std::string &line, std::string *key, std::string *value) const {
  std::string normalized = line;
  size_t end = normalized.find_first_of("\r\n");
  if (end != std::string::npos) {
    normalized = normalized.substr(0, end);
  }

  size_t separator = normalized.find(':');
  if (separator == std::string::npos) {
    return false;
  }

  auto trim = [](const std::string &text) {
    const char *whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
      return std::string();
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
  };

  *key = trim(normalized.substr(0, separator));
  *value = trim(normalized.substr(separator + 1));
  return !key->empty() && !value->empty();
}

int PylontechComponent::extract_int_(const std::string &value) const {
  const char *cursor = value.c_str();
  while (*cursor != '\0' && ((*cursor < '0' || *cursor > '9') && *cursor != '-')) {
    cursor++;
  }
  return std::atoi(cursor);
}

void PylontechComponent::publish_slave_number_(void (PylontechBattery::*publisher)(float) const, float value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == 1) {
      (battery->*publisher)(value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed slave field, but no sensor block is declared with battery: 1");
  }
}

void PylontechComponent::publish_slave_text_(void (PylontechBattery::*publisher)(const std::string &) const,
                                             const std::string &value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == 1) {
      (battery->*publisher)(value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed slave field, but no text_sensor block is declared with battery: 1");
  }
}

void PylontechComponent::publish_info_number_(void (PylontechBattery::*publisher)(float) const, float value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_info_address_) {
      (battery->*publisher)(value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed info field, but no sensor block is declared with battery: %u", this->active_info_address_);
  }
}

void PylontechComponent::publish_info_text_(void (PylontechBattery::*publisher)(const std::string &) const,
                                            const std::string &value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_info_address_) {
      (battery->*publisher)(value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed info field, but no text_sensor block is declared with battery: %u",
             this->active_info_address_);
  }
}

void PylontechComponent::publish_stat_number_(void (PylontechBattery::*publisher)(float) const, float value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_stat_address_) {
      (battery->*publisher)(value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed stat field, but no sensor block is declared with battery: %u", this->active_stat_address_);
  }
}

void PylontechComponent::publish_stat_field_(uint8_t field, float value) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_stat_address_) {
      battery->publish_stat_field(field, value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed stat field, but no sensor block is declared with battery: %u", this->active_stat_address_);
  }
}

void PylontechComponent::publish_getpwr_summary_(int voltage_mv, int current_ma, int temperature_mc, int residual_ma,
                                                 const std::string &base_state,
                                                 const std::string &voltage_state,
                                                 const std::string &current_state,
                                                 const std::string &temperature_state) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_getpwr_address_) {
      battery->publish_getpwr_summary(voltage_mv, current_ma, temperature_mc, residual_ma, base_state, voltage_state,
                                      current_state, temperature_state);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed getpwr summary, but no sensor block is declared with battery: %u",
             this->active_getpwr_address_);
  }
}

void PylontechComponent::publish_getpwr_tail_(int tail_1_value, int tail_2_value) {
  this->active_getpwr_tail_published_ = true;
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_getpwr_address_) {
      battery->publish_getpwr_tail(tail_1_value, tail_2_value);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed getpwr tail, but no sensor block is declared with battery: %u",
             this->active_getpwr_address_);
  }
}

void PylontechComponent::publish_getpwr_cell_detail_(uint8_t cell, int voltage_mv, int temperature_mc,
                                                    const std::string &voltage_state,
                                                    const std::string &temperature_state) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_getpwr_address_) {
      battery->publish_cell_detail(cell, voltage_mv, temperature_mc, voltage_state, temperature_state);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed getpwr cell, but no sensor block is declared with battery: %u",
             this->active_getpwr_address_);
  }
}

void PylontechComponent::publish_bat_cell_detail_(uint8_t cell, int voltage_mv, int current_ma, int temperature_mc,
                                                  int soc_percent, int coulomb_mah,
                                                  const std::string &voltage_state,
                                                  const std::string &temperature_state,
                                                  const std::string &balance_state) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_bat_address_) {
      battery->publish_bat_cell_detail(cell, voltage_mv, current_ma, temperature_mc, soc_percent, coulomb_mah,
                                       voltage_state, temperature_state, balance_state);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed bat cell, but no sensor block is declared with battery: %u", this->active_bat_address_);
  }
}

void PylontechComponent::publish_soh_cell_detail_(uint8_t cell, int voltage_mv, int soh_count,
                                                  const std::string &soh_status) {
  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_soh_address_) {
      battery->publish_soh_cell_detail(cell, voltage_mv, soh_count, soh_status);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed soh cell, but no sensor block is declared with battery: %u", this->active_soh_address_);
  }
}

bool PylontechComponent::append_limited_response_(PylontechBuffer *buffer, bool *truncated, const std::string &line) {
  if (buffer == nullptr || truncated == nullptr || line.empty()) {
    return false;
  }
  const size_t separator_len = buffer->empty() ? 0 : 3;
  if (buffer->length() + separator_len + line.length() >= PYLONTECH_MAX_INFO_RESPONSE_LENGTH) {
    *truncated = true;
    return false;
  }
  if (!buffer->empty()) {
    *buffer += " | ";
  }
  buffer->append(line.data(), line.size());
  return true;
}

bool PylontechComponent::wants_info_response_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address && battery->has_info_response_text_sensor())
      return true;
  }
  return false;
}

bool PylontechComponent::wants_getpwr_response_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address && battery->has_getpwr_response_text_sensor())
      return true;
  }
  return false;
}

bool PylontechComponent::wants_stat_response_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address && battery->has_stat_response_text_sensor())
      return true;
  }
  return false;
}

bool PylontechComponent::wants_bat_response_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address && battery->has_bat_response_text_sensor())
      return true;
  }
  return false;
}

bool PylontechComponent::wants_soh_response_(uint8_t address) const {
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == address && battery->has_soh_response_text_sensor())
      return true;
  }
  return false;
}

void PylontechComponent::publish_info_response_() {
  if (!this->wants_info_response_(this->active_info_address_)) {
    return;
  }
  std::string response;
  bool separator_pending = false;
  for (char c : this->info_full_response_buffer_) {
    if (c == '\r' || c == '\n') {
      if (!response.empty()) {
        separator_pending = true;
      }
      continue;
    }
    if (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) > 0x7E) {
      continue;
    }
    if (separator_pending) {
      response += " | ";
      separator_pending = false;
    }
    response += c;
  }
  if (this->info_response_truncated_) {
    response += " | [truncated]";
  }

  this->log_response_chunks_("INFO complete response", this->active_info_address_, response);

  std::string text_sensor_response = response;
  if (text_sensor_response.length() > PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH) {
    text_sensor_response.resize(PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH);
    text_sensor_response += " [truncated]";
  }
  this->publish_info_text_(&PylontechBattery::publish_info_response, text_sensor_response);
}

void PylontechComponent::publish_getpwr_response_() {
  if (!this->wants_getpwr_response_(this->active_getpwr_address_)) {
    return;
  }
  std::string response(this->getpwr_response_buffer_.data(), this->getpwr_response_buffer_.size());
  if (this->getpwr_response_truncated_) {
    response += " | [truncated]";
  }

  this->log_response_chunks_("GETPWR complete response", this->active_getpwr_address_, response);

  std::string text_sensor_response = response;
  if (text_sensor_response.length() > PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH) {
    text_sensor_response.resize(PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH);
    text_sensor_response += " [truncated]";
  }

  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_getpwr_address_) {
      battery->publish_getpwr_response(text_sensor_response);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed getpwr response, but no text_sensor block is declared with battery: %u",
             this->active_getpwr_address_);
  }
}

void PylontechComponent::publish_stat_response_() {
  if (!this->wants_stat_response_(this->active_stat_address_)) {
    return;
  }
  std::string response(this->stat_response_buffer_.data(), this->stat_response_buffer_.size());
  if (this->stat_response_truncated_) {
    response += " | [truncated]";
  }

  this->log_response_chunks_("STAT complete response", this->active_stat_address_, response);

  std::string text_sensor_response = response;
  if (text_sensor_response.length() > PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH) {
    text_sensor_response.resize(PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH);
    text_sensor_response += " [truncated]";
  }

  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_stat_address_) {
      battery->publish_stat_response(text_sensor_response);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed stat response, but no text_sensor block is declared with battery: %u",
             this->active_stat_address_);
  }
}

void PylontechComponent::publish_bat_response_() {
  if (!this->wants_bat_response_(this->active_bat_address_)) {
    return;
  }
  std::string response(this->bat_response_buffer_.data(), this->bat_response_buffer_.size());
  if (this->bat_response_truncated_) {
    response += " | [truncated]";
  }

  this->log_response_chunks_("BAT complete response", this->active_bat_address_, response);

  std::string text_sensor_response = response;
  if (text_sensor_response.length() > PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH) {
    text_sensor_response.resize(PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH);
    text_sensor_response += " [truncated]";
  }

  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_bat_address_) {
      battery->publish_bat_response(text_sensor_response);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed bat response, but no text_sensor block is declared with battery: %u",
             this->active_bat_address_);
  }
}

void PylontechComponent::publish_soh_response_() {
  if (!this->wants_soh_response_(this->active_soh_address_)) {
    return;
  }
  std::string response(this->soh_response_buffer_.data(), this->soh_response_buffer_.size());
  if (this->soh_response_truncated_) {
    response += " | [truncated]";
  }

  this->log_response_chunks_("SOH complete response", this->active_soh_address_, response);

  std::string text_sensor_response = response;
  if (text_sensor_response.length() > PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH) {
    text_sensor_response.resize(PYLONTECH_TEXT_SENSOR_RESPONSE_LENGTH);
    text_sensor_response += " [truncated]";
  }

  bool published = false;
  for (auto *battery : this->batteries_) {
    if (battery != nullptr && battery->address() == this->active_soh_address_) {
      battery->publish_soh_response(text_sensor_response);
      published = true;
    }
  }
  if (!published) {
    ESP_LOGV(TAG, "Parsed soh response, but no text_sensor block is declared with battery: %u",
             this->active_soh_address_);
  }
}

void PylontechComponent::publish_getpwr_tail_from_response_() {
  if (this->active_getpwr_tail_published_) {
    return;
  }

  int previous_single_numeric = 0;
  int last_single_numeric = 0;
  uint8_t single_numeric_count = 0;
  std::string segment;
  segment.reserve(96);
  auto local_trim = [](const std::string &text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) {
      begin++;
    }
    size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
      end--;
    }
    return text.substr(begin, end - begin);
  };
  auto handle_segment = [this, &previous_single_numeric, &last_single_numeric, &single_numeric_count,
                         &local_trim](const std::string &raw_segment) {
    std::string cleaned = local_trim(raw_segment);
    if (cleaned.empty() || cleaned == "Command completed successfully") {
      return;
    }

    std::string token;
    token.reserve(24);
    uint8_t token_count = 0;
    int only_value = 0;
    bool has_separator = false;
    for (char c : cleaned) {
      if (c == '#') {
        has_separator = true;
        std::string value = local_trim(token);
        if (!value.empty()) {
          token_count++;
          only_value = this->extract_int_(value);
        }
        token.clear();
      } else {
        token += c;
      }
    }
    if (!has_separator) {
      return;
    }

    std::string value = local_trim(token);
    if (!value.empty()) {
      token_count++;
      only_value = this->extract_int_(value);
    }
    if (token_count == 1) {
      previous_single_numeric = last_single_numeric;
      last_single_numeric = only_value;
      if (single_numeric_count < 2) {
        single_numeric_count++;
      }
    }
  };

  for (char c : this->getpwr_response_buffer_) {
    if (c == '|') {
      handle_segment(segment);
      segment.clear();
    } else {
      segment += c;
    }
  }
  handle_segment(segment);

  if (single_numeric_count < 2) {
    ESP_LOGV(TAG, "GETPWR tail not found for battery %u: single_numeric_fields=%u", this->active_getpwr_address_,
             static_cast<unsigned>(single_numeric_count));
    return;
  }

  int tail_1_value = previous_single_numeric;
  int tail_2_value = last_single_numeric;
  ESP_LOGV(TAG, "GETPWR parsed tail from complete response: battery=%u tail_1=%d tail_2=%d",
           this->active_getpwr_address_, tail_1_value, tail_2_value);
  this->publish_getpwr_tail_(tail_1_value, tail_2_value);
}

void PylontechComponent::clear_getpwr_context_() {
  ESP_LOGV(TAG, "Getpwr sequence complete");
  this->timeout_diagnostic_command_();
  this->active_getpwr_address_ = 0;
  this->next_getpwr_address_ = 0;
  this->active_getpwr_cell_ = 0;
  this->active_getpwr_tail_published_ = false;
  this->active_getpwr_tail_pending_ = false;
  this->active_getpwr_tail_1_value_ = 0;
  this->active_getpwr_summary_field_count_ = 0;
  this->active_getpwr_cell_field_ = 0;
  this->active_getpwr_cell_voltage_mv_ = 0;
  this->active_getpwr_cell_temperature_mc_ = 0;
  this->active_getpwr_cell_voltage_state_.clear();
  this->active_getpwr_cell_temperature_state_.clear();
  this->getpwr_response_buffer_.clear();
  this->getpwr_response_truncated_ = false;
  this->process_pending_requests_();
}

void PylontechComponent::clear_stat_context_() {
  ESP_LOGV(TAG, "Stat sequence complete");
  this->timeout_diagnostic_command_();
  this->active_stat_address_ = 0;
  this->next_stat_address_ = 0;
  this->stat_response_buffer_.clear();
  this->stat_response_truncated_ = false;
  if (this->pending_bat_after_stat_ && !this->command_busy_()) {
    this->pending_bat_after_stat_ = false;
    this->start_bat_sequence_();
    return;
  }
  if (this->pending_soh_after_stat_ && !this->command_busy_()) {
    this->pending_soh_after_stat_ = false;
    this->start_soh_sequence_();
    return;
  }
  this->pending_bat_after_stat_ = false;
  this->pending_soh_after_stat_ = false;
  if (this->active_slow_sequence_) {
    this->active_slow_sequence_ = false;
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->process_pending_requests_();
}

void PylontechComponent::clear_bat_context_() {
  ESP_LOGV(TAG, "Bat sequence complete");
  this->timeout_diagnostic_command_();
  this->active_bat_address_ = 0;
  this->next_bat_address_ = 0;
  this->bat_response_buffer_.clear();
  this->bat_response_truncated_ = false;
  if (this->pending_soh_after_bat_ && !this->command_busy_()) {
    this->pending_soh_after_bat_ = false;
    this->start_soh_sequence_();
    return;
  }
  this->pending_soh_after_bat_ = false;
  if (this->active_slow_sequence_) {
    this->active_slow_sequence_ = false;
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->process_pending_requests_();
}

void PylontechComponent::clear_soh_context_() {
  ESP_LOGV(TAG, "Soh sequence complete");
  this->timeout_diagnostic_command_();
  this->active_soh_address_ = 0;
  this->next_soh_address_ = 0;
  this->soh_response_buffer_.clear();
  this->soh_response_truncated_ = false;
  if (this->active_slow_sequence_) {
    this->active_slow_sequence_ = false;
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->process_pending_requests_();
}

void PylontechComponent::process_pending_requests_() {
  this->process_pending_console_request_();
  if (this->command_busy_()) {
    return;
  }
  this->process_pending_info_request_();
  if (!this->pending_info_request_) {
    this->process_pending_stat_request_();
  }
  if (!this->pending_info_request_ && !this->pending_stat_request_) {
    this->process_pending_bat_request_();
  }
  if (!this->pending_info_request_ && !this->pending_stat_request_ && !this->pending_bat_request_) {
    this->process_pending_soh_request_();
  }
  if (!this->pending_info_request_ && !this->pending_stat_request_ && !this->pending_bat_request_ &&
      !this->pending_soh_request_) {
    this->process_pending_slow_request_();
  }
}

void PylontechComponent::process_pending_console_request_() {
  if (this->command_busy_()) {
    return;
  }
  if (this->pending_us2000b_initialization_) {
    this->pending_us2000b_initialization_ = false;
    this->start_us2000b_initialization_();
  } else if (this->pending_login_debug_) {
    this->pending_login_debug_ = false;
    this->start_login_debug_();
  }
}

void PylontechComponent::process_pending_info_request_() {
  if (!this->pending_info_request_ || this->command_busy_()) {
    return;
  }
  ESP_LOGV(TAG, "Starting queued info request");
  this->pending_info_request_ = false;
  if (this->enable_info_) {
    this->schedule_next_automatic_info_(this->info_interval_);
  }
  this->start_info_sequence_();
}

void PylontechComponent::process_pending_stat_request_() {
  if (!this->pending_stat_request_ || this->command_busy_()) {
    return;
  }
  ESP_LOGV(TAG, "Starting queued stat request");
  this->pending_stat_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->start_stat_sequence_();
}

void PylontechComponent::process_pending_bat_request_() {
  if (!this->pending_bat_request_ || this->command_busy_()) {
    return;
  }
  ESP_LOGV(TAG, "Starting queued bat request");
  this->pending_bat_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->start_bat_sequence_();
}

void PylontechComponent::process_pending_soh_request_() {
  if (!this->pending_soh_request_ || this->command_busy_()) {
    return;
  }
  ESP_LOGV(TAG, "Starting queued soh request");
  this->pending_soh_request_ = false;
  if (this->enable_stat_ || this->enable_bat_ || this->enable_soh_) {
    this->schedule_next_automatic_slow_(this->slow_interval_);
  }
  this->start_soh_sequence_();
}

void PylontechComponent::process_pending_slow_request_() {
  if (!this->pending_slow_request_ || this->command_busy_()) {
    return;
  }
  ESP_LOGV(TAG, "Starting queued slow loop request");
  this->pending_slow_request_ = false;
  this->start_slow_sequence_();
}

bool PylontechComponent::command_busy_() const {
  const uint32_t now = millis();
  return deadline_pending(now, this->info_busy_until_ms_) || deadline_pending(now, this->getpwr_busy_until_ms_) ||
         deadline_pending(now, this->stat_busy_until_ms_) || deadline_pending(now, this->bat_busy_until_ms_) ||
         deadline_pending(now, this->soh_busy_until_ms_) || !this->response_capture_command_.empty() ||
         this->pending_pwr_after_pwrsys_ || this->pending_getpwr_after_pwr_ || this->login_debug_active_ ||
         this->us2000b_initialization_active_;
}

void PylontechComponent::log_response_chunks_(const char *prefix, uint8_t address,
                                              const std::string &response) const {
  if (response.empty()) {
    ESP_LOGV(TAG, "%s battery %u: <empty>", prefix, address);
    return;
  }

  uint16_t chunk = 1;
  for (size_t pos = 0; pos < response.length(); pos += PYLONTECH_RESPONSE_LOG_CHUNK_LENGTH) {
    std::string part = response.substr(pos, PYLONTECH_RESPONSE_LOG_CHUNK_LENGTH);
    if (address == 0) {
      ESP_LOGV(TAG, "%s [%u]: %s", prefix, chunk, part.c_str());
    } else {
      ESP_LOGV(TAG, "%s battery %u [%u]: %s", prefix, address, chunk, part.c_str());
    }
    chunk++;
  }
}

void PylontechComponent::start_response_capture_(const char *command) {
  this->cancel_timeout("response_capture_timeout");
  this->response_capture_command_ = command;
  this->begin_diagnostic_command_(std::strcmp(command, "pwrsys") == 0 ? "PWRSYS" : "PWR", 0);
  this->set_timeout("response_capture_timeout", 3000, [this]() {
    this->clear_response_capture_("timeout");
  });
}

void PylontechComponent::clear_response_capture_(const char *reason) {
  if (this->response_capture_command_.empty()) {
    return;
  }
  bool timed_out = std::strcmp(reason, "timeout") == 0;
  bool fast_command = this->response_capture_command_ == "pwrsys" || this->response_capture_command_ == "pwr";
  if (timed_out) {
    this->timeout_diagnostic_command_();
  } else {
    this->fail_diagnostic_command_(reason);
  }
  ESP_LOGW(TAG, "%s %s", this->response_capture_command_.c_str(), reason);
  if (timed_out && fast_command) {
    this->fast_command_failures_++;
    bool cooldown_elapsed = !this->login_debug_recovery_has_run_ ||
                            static_cast<uint32_t>(millis() - this->last_login_debug_recovery_ms_) >=
                                this->login_debug_recovery_interval_;
    if (this->enable_login_debug_recovery_ &&
        this->fast_command_failures_ >= this->login_debug_failure_threshold_ && cooldown_elapsed) {
      ESP_LOGW(TAG, "Fast communication failed %u times; queuing login debug recovery",
               this->fast_command_failures_);
      this->pending_login_debug_ = true;
    }
  }
  this->response_capture_command_.clear();
  this->pending_pwr_after_pwrsys_ = false;
  this->pending_getpwr_after_pwr_ = false;
  this->process_pending_requests_();
}

void PylontechComponent::append_response_capture_line_(const std::string &line) {
  if (this->response_capture_command_.empty()) {
    return;
  }

  if (line == "Command completed successfully") {
    const bool completed_pwrsys = this->response_capture_command_ == "pwrsys";
    const bool completed_pwr = this->response_capture_command_ == "pwr";
    this->complete_diagnostic_command_(completed_pwrsys ? "PWRSYS" : "PWR", 0);
    if (completed_pwr) {
      this->fast_command_failures_ = 0;
      this->on_fast_command_success_();
    }
    this->cancel_timeout("response_capture_timeout");
    this->response_capture_command_.clear();
    if (completed_pwrsys && this->pending_pwr_after_pwrsys_) {
      this->pending_pwr_after_pwrsys_ = false;
      this->set_timeout("send_pwr_after_pwrsys_complete", 250, [this]() {
        ESP_LOGV(TAG, "TX: pwr");
        this->start_response_capture_("pwr");
        this->write_str("pwr\n");
      });
    } else if (completed_pwr && this->pending_getpwr_after_pwr_) {
      this->pending_getpwr_after_pwr_ = false;
      this->set_timeout("send_getpwr_after_pwr_complete", 250, [this]() {
        if (!this->command_busy_()) {
          this->start_getpwr_sequence_();
        }
      });
    } else {
      this->process_pending_requests_();
    }
  }
}

void PylontechForceSlaveSwitch::setup() {
  if (this->parent_ != nullptr) {
    this->publish_state(this->parent_->is_slave_mode());
  }
}

void PylontechForceSlaveSwitch::dump_config() {
  LOG_SWITCH("", "Pylontech Force Slave Mode", this);
}

void PylontechForceSlaveSwitch::write_state(bool state) {
  if (this->parent_ != nullptr) {
    this->parent_->set_force_slave_mode(state);
  }
  this->publish_state(state);
}

void PylontechInfoButton::dump_config() {
  LOG_BUTTON("", "Pylontech Request Info", this);
}

void PylontechInfoButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_info_update();
  }
}

void PylontechGetPowerButton::dump_config() {
  LOG_BUTTON("", "Pylontech Request Getpwr", this);
}

void PylontechGetPowerButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_getpwr_update();
  }
}

void PylontechStatButton::dump_config() {
  LOG_BUTTON("", "Pylontech Request Stat", this);
}

void PylontechStatButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_stat_update();
  }
}

void PylontechBatButton::dump_config() {
  LOG_BUTTON("", "Pylontech Request Bat", this);
}

void PylontechBatButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_bat_update();
  }
}

void PylontechSohButton::dump_config() {
  LOG_BUTTON("", "Pylontech Request Soh", this);
}

void PylontechSohButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_soh_update();
  }
}

void PylontechLoginDebugButton::dump_config() {
  LOG_BUTTON("", "Pylontech Login Debug", this);
}

void PylontechLoginDebugButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_login_debug();
  }
}

void PylontechUS2000BInitializationButton::dump_config() {
  LOG_BUTTON("", "Pylontech US2000B Initialization", this);
}

void PylontechUS2000BInitializationButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_us2000b_initialization();
  }
}

}  // namespace pylontech
}  // namespace esphome
