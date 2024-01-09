#include "pylontech.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pylontech {

static const char *const TAG = "pylontech";
static const int MAX_DATA_LENGTH_BYTES = 256;
static const uint8_t ASCII_LF = 0x0A;

PylontechComponent::PylontechComponent() {}

void PylontechComponent::dump_config() {
  this->check_uart_settings(115200, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  ESP_LOGCONFIG(TAG, "pylontech:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Connection with pylontech failed!");
  }

  for (PylontechListener *listener : this->listeners_) {
    listener->dump_config();
  }

  LOG_UPDATE_INTERVAL(this);
}

void PylontechComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up pylontech...");
  while (this->available() != 0) {
    this->read();
  }
}

void PylontechComponent::update() { this->write_str("pwr\n"); }

void PylontechComponent::loop() {
  uint8_t data;

  // pylontech sends a lot of data very suddenly
  // we need to quickly put it all into our own buffer, otherwise the uart's buffer will overflow
  while (this->available() > 0) {
    if (this->read_byte(&data)) {
      buffer_[buffer_index_write_] += (char) data;
      if (buffer_[buffer_index_write_].back() == static_cast<char>(ASCII_LF) ||
          buffer_[buffer_index_write_].length() >= MAX_DATA_LENGTH_BYTES) {
        // complete line received
        buffer_index_write_ = (buffer_index_write_ + 1) % NUM_BUFFERS;
      }
    }
  }

  // only process one line per call of loop() to not block esphome for too long
  if (buffer_index_read_ != buffer_index_write_) {
    this->process_line_(buffer_[buffer_index_read_]);
    buffer_[buffer_index_read_].clear();
    buffer_index_read_ = (buffer_index_read_ + 1) % NUM_BUFFERS;
  }
}

void PylontechComponent::process_line_(std::string &buffer) {
  ESP_LOGV(TAG, "Read from serial: %s", buffer.substr(0, buffer.size() - 2).c_str());

  PylontechListener::LineContents l{};

  switch(state_serie) {
    case pwrsys: 
      this->write_str("pwrsys\n"); 
  ESP_LOGV(TAG, "Read from serial: %s", buffer.substr(0, buffer.size() - 2).c_str());
        delay(10);
      if (buffer.find("pwrsys")) {
        if (sscanf(buffer.c_str(), "System is %s", &l.value_systeme_is))
        if (sscanf(buffer.c_str(), "Total Num : %d", &l.value_total_num))
        if (sscanf(buffer.c_str(), "Present Num : %d", &l.value_present_num))
        if (sscanf(buffer.c_str(), "Sleep Num : %d", &l.value_sleep_num))
        if (sscanf(buffer.c_str(), "System Volt : %d mV", &l.value_system_volt))
        if (sscanf(buffer.c_str(), "System Curr : %d mA", &l.value_system_curr))
        if (sscanf(buffer.c_str(), "System RC : %d mAH", &l.value_system_rc))
        if (sscanf(buffer.c_str(), "System FCC : %d mAH", &l.value_system_fcc))
        if (sscanf(buffer.c_str(), "System SOC : %d%%", &l.value_system_soc))
        if (sscanf(buffer.c_str(), "System SOH : %d%%", &l.value_system_soh))
        if (sscanf(buffer.c_str(), "Highest voltage : %d mV", &l.value_highest_voltage))
        if (sscanf(buffer.c_str(), "Average voltage : %d mV", &l.value_average_voltage))
        if (sscanf(buffer.c_str(), "Lowest voltage : %d mV", &l.value_lowest_voltage))
        if (sscanf(buffer.c_str(), "Highest temperature : %d mC", &l.value_highest_temperature))
        if (sscanf(buffer.c_str(), "Average temperature : %d mC", &l.value_average_temperature))
        if (sscanf(buffer.c_str(), "Lowest temperature : %d mC", &l.value_lowest_temperature))
        if (sscanf(buffer.c_str(), "Recommend chg voltage : %d mV", &l.value_recommend_chg_voltage))
        if (sscanf(buffer.c_str(), "Recommend dsg voltage : %d mV", &l.value_recommend_dsg_voltage))
        if (sscanf(buffer.c_str(), "Recommend chg current : %d mA", &l.value_recommend_chg_current))
        if (sscanf(buffer.c_str(), "Recommend dsg current : %d mA", &l.value_recommend_dsg_current))
        if (sscanf(buffer.c_str(), "system Recommend chg voltage : %d mV", &l.value_system_recommend_chg_voltage))
        if (sscanf(buffer.c_str(), "system Recommend dsg voltage : %d mV", &l.value_system_recommend_dsg_voltage))
        if (sscanf(buffer.c_str(), "system Recommend chg current : %d mA", &l.value_system_recommend_chg_current))
        if (sscanf(buffer.c_str(), "system Recommend dsg current : %d mA", &l.value_system_recommend_dsg_current))
      for (PylontechListener *listener : this->listeners_) {
        listener->on_line_read(&l);
      }
      }
      break;
    case pwr: 
      this->write_str("pwr\n");
  ESP_LOGV(TAG, "Read from serial: %s", buffer.substr(0, buffer.size() - 2).c_str());
  // clang-format off
  // example line to parse:
  // Power       Volt     Curr     Tempr     Tlow     Thigh     Vlow     Vhigh     Base.St    Volt.St    
  // %d          %d       %d       %d        %d       %d        %d       %d        %7s        %7s        
  // 1           50548    8910     25000     24200    25000     3368     3371      Charge     Normal     
  // &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st, //x10

  // Curr.St    Temp.St    Coulomb      Time                     B.V.St   B.T.St   MosTempr     M.T.St
  // %7s        %7s        %d%%         %4d-%2d-%2d %2d:%2d:%2d  %*s      %*s      %d           %*s
  // Normal     Normal     97%          2021-06-30 20:49:45      Normal   Normal   22700        Normal
  // l.curr_st, l.temp_st, &l.capacity, l.date,    l.time,       l.bv_st, l.bt_st, &l.mostempr, l.mos_st //x9

  // clang-format on
        delay(10);
      const int parsed = sscanf(                                                                                   // NOLINT
          //buffer.c_str(), "%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %d-%d-%d %d:%d:%d %7s %7s %d %7s",       // NOLINT
          //l.curr_st, l.temp_st, &l.ccoulomb, &l.year, &l.month, &l.day, &l.hour, &l.minute, &l.second, l.bv_st,  // NOLINT
          buffer.c_str(), "%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %*d-%*d-%*d %*d:%*d:%*d %7s %7s %d %7s",   // NOLINT
          &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st,      // NOLINT
          l.curr_st, l.temp_st, &l.coulomb, l.bv_st, l.bt_st, &l.mostempr, l.mos_st);                              // NOLINT
  
      if (l.bat_num <= 0) {
        ESP_LOGD(TAG, "invalid bat_num in line %s", buffer.substr(0, buffer.size() - 2).c_str());
        return;
      }
      if (parsed < 14) {
        ESP_LOGW(TAG, "invalid line: found only %d items in %s", parsed, buffer.substr(0, buffer.size() - 2).c_str());
        return;
      }

      for (PylontechListener *listener : this->listeners_) {
        listener->on_line_read(&l);
      }
      break;
  } //close switch
  return;
} //close process_line

float PylontechComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pylontech
}  // namespace esphome
