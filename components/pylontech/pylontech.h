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
        total_num,
        value_present_num,
        sleep_num,
        system_volt,
        system_curr,
        system_rc,
        system_fcc,
        system_soc,
        system_soh,
        highest_voltage,
        average_voltage,
        lowest_voltage,
        highest_temperature,
        average_temperature,
        lowest_temperature,
        recommend_chg_voltage,
        recommend_dsg_voltage,
        recommend_chg_current,
        recommend_dsg_current,
        system_recommend_chg_voltage,
        system_recommend_dsg_voltage,
        system_recommend_chg_current,
        system_recommend_dsg_current,
        ;
    char base_st[TEXT_SENSOR_MAX_LEN], 
         volt_st[TEXT_SENSOR_MAX_LEN], 
         curr_st[TEXT_SENSOR_MAX_LEN],
         temp_st[TEXT_SENSOR_MAX_LEN], 
         date[TEXT_SENSOR_MAX_LEN], 
         time[TEXT_SENSOR_MAX_LEN],
         bv_st[TEXT_SENSOR_MAX_LEN], 
         bt_st[TEXT_SENSOR_MAX_LEN], 
         mos_st[TEXT_SENSOR_MAX_LEN],
        systeme_is[TEXT_SENSOR_MAX_LEN], //char
        ;
  };

  virtual void on_line_read(LineContents *line);
  virtual void dump_config();
};

class PylontechComponent : public PollingComponent, public uart::UARTDevice {
 public:
  PylontechComponent();

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

  // ring buffer
  std::string buffer_[NUM_BUFFERS];
  int buffer_index_write_ = 0;
  int buffer_index_read_ = 0;

  std::vector<PylontechListener *> listeners_{};
};

}  // namespace pylontech
}  // namespace esphome
