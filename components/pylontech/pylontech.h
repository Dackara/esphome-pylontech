#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace pylontech {

static const uint8_t NUM_BUFFERS = 20;
static const uint8_t TEXT_SENSOR_MAX_LEN = 8;

class PylontechListener {
 public:

  struct LineContents {
    int bat_num = 0, 
        volt, 
        curr, 
        tempr, 
        tlow, 
        thigh, 
        vlow, 
        vhigh, 
        coulomb, 
        year, month, day, hour, minute, second, 
        mostempr,

        //systeme_is, //char
        value_total_num,
        value_present_num,
        value_sleep_num,
        value_system_volt,
        value_system_curr,
        value_system_rc,
        value_system_fcc,
        value_system_soc,
        value_system_soh,
        value_highest_voltage,
        value_average_voltage,
        value_lowest_voltage,
        value_highest_temperature,
        value_average_temperature,
        value_lowest_temperature,
        value_recommend_chg_voltage,
        value_recommend_dsg_voltage,
        value_recommend_chg_current,
        value_recommend_dsg_current,
        value_system_recommend_chg_voltage,
        value_system_recommend_dsg_voltage,
        value_system_recommend_chg_current,
        value_system_recommend_dsg_current;
    char base_st[TEXT_SENSOR_MAX_LEN], 
         volt_st[TEXT_SENSOR_MAX_LEN], 
         curr_st[TEXT_SENSOR_MAX_LEN],
         temp_st[TEXT_SENSOR_MAX_LEN], 
         date[TEXT_SENSOR_MAX_LEN], 
         time[TEXT_SENSOR_MAX_LEN],
         bv_st[TEXT_SENSOR_MAX_LEN], 
         bt_st[TEXT_SENSOR_MAX_LEN], 
         mos_st[TEXT_SENSOR_MAX_LEN],
         value_systeme_is[TEXT_SENSOR_MAX_LEN]; //char
  };

  virtual void on_line_read(LineContents *line);
  virtual void dump_config();
};

class PylontechComponent : public PollingComponent, public uart::UARTDevice {
 public:
  PylontechComponent();


  enum ENUMPollingCommand {
    pwrsys = 0,
    pwr = 1,
  };

  struct PollingCommand {
    uint8_t *command;
    uint8_t length = 0;
    uint8_t errors;
    ENUMPollingCommand identifier;
};

  /// Schedule data readings.
  void update() override;
  /// Read data once available
  void loop() override;
  /// Setup the sensor and test for a connection.
  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override;

  void register_listener(PylontechListener *listener) { this->listeners_.push_back(listener); }

 protected:
  void process_line_(std::string &buffer);
  uint8_t send_next_command_();
  void send_next_poll_();
  void add_polling_command_(const char *command, ENUMPollingCommand polling_command);

  // ring buffer
  std::string buffer_[NUM_BUFFERS];
  int buffer_index_write_ = 0;
  int buffer_index_read_ = 0;

  std::vector<PylontechListener *> listeners_{};
};

}  // namespace pylontech
}  // namespace esphome
