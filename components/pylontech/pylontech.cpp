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

void PylontechComponent::update() { this->write_str("pwrsys\n"); }

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
  const int parsed = sscanf(                                                                                   // NOLINT
      buffer.c_str(), "System is %s", &l.value_systeme_is);  // NOLINT

  for (PylontechListener *listener : this->listeners_) {
    listener->on_line_read(&l);
  }
}
float PylontechComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pylontech
}  // namespace esphome
