#include "pylontech.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

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
  if (this->available() > 0) {
  // pylontech sends a lot of data very suddenly
  // we need to quickly put it all into our own buffer, otherwise the uart's buffer will overflow
    uint8_t data;
    int recv = 0;
    while (this->available() > 0) {
      if (this->read_byte(&data)) {
        buffer_[buffer_index_write_] += (char) data;
        recv++;
        if (buffer_[buffer_index_write_].back() == static_cast<char>(ASCII_LF) ||
          buffer_[buffer_index_write_].length() >= MAX_DATA_LENGTH_BYTES) {
          // complete line received
          buffer_index_write_ = (buffer_index_write_ + 1) % NUM_BUFFERS;
        }
      }
    }
    ESP_LOGV(TAG, "received %d bytes", recv);
  } else {
    // only process one line per call of loop() to not block esphome for too long
    if (buffer_index_read_ != buffer_index_write_) {
      this->process_line_(buffer_[buffer_index_read_]);
      buffer_[buffer_index_read_].clear();
      buffer_index_read_ = (buffer_index_read_ + 1) % NUM_BUFFERS;
    }
  }
}

void PylontechComponent::process_line_(std::string &buffer) {
  ESP_LOGV(TAG, "Read from serial: %s", buffer.substr(0, buffer.size() - 2).c_str());
  // clang-format off
  // example line to parse:
  // Power       Volt     Curr     Tempr     Tlow     Thigh     Vlow     Vhigh     Base.St    Volt.St    Curr.St    Temp.St    Coulomb      Time                     B.V.St   B.T.St   MosTempr     M.T.St
  // %d          %d       %d       %d        %d       %d        %d       %d        %7s        %7s        %7s        %7s        %d%%         %4d-%2d-%2d %2d:%2d:%2d  %7s      %7s      %d           %7s
  // 1           50548    8910     25000     24200    25000     3368     3371      Charge     Normal     Normal     Normal     97%          2021-06-30 20:49:45      Normal   Normal   22700        Normal
  // &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st, l.curr_st, l.temp_st, &l.capacity, l.date,    l.time,       l.bv_st, l.bt_st, &l.mostempr, l.mos_st
  // clang-format on
 
  PylontechListener::LineContents l{};
  char mostempr_s[6];
  const int parsed = sscanf(                                                                                  // NOLINT
      buffer.c_str(), "%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %*d-%*d-%*d %*d:%*d:%*d %7s %7s %5s %7s", // NOLINT
      &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st,     // NOLINT
      l.curr_st, l.temp_st, &l.coulomb, l.bv_st, l.bt_st, mostempr_s, l.mos_st);                              // NOLINT
      //l.curr_st, l.temp_st, &l.ccoulomb, &l.year, &l.month, &l.day, &l.hour, &l.minute, &l.second, l.bv_st, // NOLINT
  
  if (l.bat_num <= 0) {
    ESP_LOGD(TAG, "invalid bat_num in line %s", buffer.substr(0, buffer.size() - 2).c_str());
    return;
  }
  
  if (parsed != 17) {
    ESP_LOGW(TAG, "invalid line: found only %d items in %s", parsed, buffer.substr(0, buffer.size() - 2).c_str());
    return;
  }
  auto mostempr_parsed = parse_number<int>(mostempr_s);
    if (mostempr_parsed.has_value()) {
      l.mostempr = mostempr_parsed.value();
    } else {
      l.mostempr = -300;
      l.mos_st == "NULL";
      ESP_LOGW(TAG, "bat_num %d: received no mostempr", l.bat_num);
    }
  
  for (PylontechListener *listener : this->listeners_) {
    listener->on_line_read(&l);
  }
//}

        std::string &str

        int sensorID = 0;
        float sensorTEMP = 0; 
        int intValue = 0;
        int highTemp_mC;
        int lowTemp_mC;
        float highestVoltage;
        float averageVoltage;
        float lowestVoltage;
        char stringValue[100];
  
        this->write_str("pwrsys\n");
  // -pwrsys- Section
        if (sscanf(str.c_str(), " System is %[^\r\n]", stringValue) == 1)                { id(systeme_is).publish_state(stringValue);                       return; }
        if (sscanf(str.c_str(), " Total Num : %d", &intValue) == 1)                      { id(pwrsys_total_num).publish_state(intValue);                    return; }
        if (sscanf(str.c_str(), " Present Num : %d", &intValue) == 1)                    { id(pwrsys_present_num).publish_state(intValue);                  return; }
        if (sscanf(str.c_str(), " Sleep Num : %d", &intValue) == 1)                      { id(pwrsys_sleep_num).publish_state(intValue);                    return; }
        if (sscanf(str.c_str(), " System Volt : %d mV", &intValue) == 1)                 { id(pwrsys_system_voltage).publish_state(intValue);               return; }
        if (sscanf(str.c_str(), " System Curr : %d mA", &intValue) == 1)                 { id(pwrsys_system_current).publish_state(intValue);               return; }
        if (sscanf(str.c_str(), " System RC : %d mAH", &intValue) == 1)                  { id(pwrsys_system_rc).publish_state(intValue);                    return; }
        if (sscanf(str.c_str(), " System FCC : %d mAH", &intValue) == 1)                 { id(pwrsys_system_fcc).publish_state(intValue);                   return; }
        if (sscanf(str.c_str(), " System SOC : %d %%", &intValue) == 1)                  { id(pwrsys_system_soc).publish_state(intValue);                   return; }
        if (sscanf(str.c_str(), " System SOH : %d %%", &intValue) == 1)                  { id(pwrsys_system_soh).publish_state(intValue);                   return; }
        if (sscanf(str.c_str(), " Highest voltage : %f", &highestVoltage) == 1)          { id(pwrsys_highest_voltage).publish_state(highestVoltage);        return; }
        if (sscanf(str.c_str(), " Average voltage : %d mV", &intValue) == 1)             { id(pwrsys_average_voltage).publish_state(intValue);              return; }
        if (sscanf(str.c_str(), " Lowest voltage : %f", &lowestVoltage) == 1)            { id(pwrsys_lowest_voltage).publish_state(lowestVoltage);          return; }
        if (sscanf(str.c_str(), " Highest temperature : %d mC", &intValue) == 1)         { id(pwrsys_high_Temp).publish_state(intValue);                    return; }
        if (sscanf(str.c_str(), " Average temperature : %d mC", &intValue) == 1)         { id(pwrsys_average_Temp).publish_state(intValue);                 return; }
        if (sscanf(str.c_str(), " Lowest temperature : %d mC", &intValue) == 1)          { id(pwrsys_low_Temp).publish_state(intValue);                     return; }
        if (sscanf(str.c_str(), " Recommend chg voltage : %d mV", &intValue) == 1)       { id(pwrsys_recommend_chg_voltage).publish_state(intValue);        return; }
        if (sscanf(str.c_str(), " Recommend dsg voltage : %d mV", &intValue) == 1)       { id(pwrsys_recommend_dsg_voltage).publish_state(intValue);        return; }
        if (sscanf(str.c_str(), " Recommend chg current : %d mA", &intValue) == 1)       { id(pwrsys_recommend_chg_current).publish_state(intValue);        return; }
        if (sscanf(str.c_str(), " Recommend dsg current : %d mA", &intValue) == 1)       { id(pwrsys_recommend_dsg_current).publish_state(intValue);        return; }
        if (sscanf(str.c_str(), " system Recommend chg voltage: %d mV", &intValue) == 1) { id(pwrsys_system_recommend_chg_voltage).publish_state(intValue); return; }
        if (sscanf(str.c_str(), " system Recommend dsg voltage: %d mV", &intValue) == 1) { id(pwrsys_system_recommend_dsg_voltage).publish_state(intValue); return; }
        if (sscanf(str.c_str(), " system Recommend chg current: %d mA", &intValue) == 1) { id(pwrsys_system_recommend_chg_current).publish_state(intValue); return; }
        if (sscanf(str.c_str(), " system Recommend dsg current: %d mA", &intValue) == 1) { id(pwrsys_system_recommend_dsg_current).publish_state(intValue); return; }
}

float PylontechComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pylontech
}  // namespace esphome
