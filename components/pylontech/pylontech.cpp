#include "pylontech.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace pylontech {

static const char *const TAG = "pylontech";



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
        char endMarker = '>';
        rc = '0';
  
  switch(state_serie) {
  // -pwrsys- Section
    case pwrsys: 
      this->write_str("pwrsys\n"); 
       // delay(10);
      if (sscanf(str.c_str(), "pwrsys") ==1)                                           { ESP_LOGD(TAG, "pwrsys command OK");                             // return; }
        while (rc != endMarker) {
          if (sscanf(str.c_str(), " System is %[^\r\n]", stringValue) == 1)                 { id(systeme_is).publish_state(stringValue);                       return; }
          if (sscanf(str.c_str(), " Total Num : %3d", &intValue) == 1)                      { id(pwrsys_total_num).publish_state(intValue);                    return; }
          if (sscanf(str.c_str(), " Present Num : %3d", &intValue) == 1)                    { id(pwrsys_present_num).publish_state(intValue);                  return; }
          if (sscanf(str.c_str(), " Sleep Num : %3d", &intValue) == 1)                      { id(pwrsys_sleep_num).publish_state(intValue);                    return; }
          if (sscanf(str.c_str(), " System Volt : %7d mV", &intValue) == 1)                 { id(pwrsys_system_voltage).publish_state(intValue);               return; }
          if (sscanf(str.c_str(), " System Curr : %7d mA", &intValue) == 1)                 { id(pwrsys_system_current).publish_state(intValue);               return; }
          if (sscanf(str.c_str(), " System RC : %7d mAH", &intValue) == 1)                  { id(pwrsys_system_rc).publish_state(intValue);                    return; }
          if (sscanf(str.c_str(), " System FCC : %7d mAH", &intValue) == 1)                 { id(pwrsys_system_fcc).publish_state(intValue);                   return; }
          if (sscanf(str.c_str(), " System SOC : %4d %%", &intValue) == 1)                  { id(pwrsys_system_soc).publish_state(intValue);                   return; }
          if (sscanf(str.c_str(), " System SOH : %4d %%", &intValue) == 1)                  { id(pwrsys_system_soh).publish_state(intValue);                   return; }
          if (sscanf(str.c_str(), " Highest voltage : %7d mV", &highestVoltage) == 1)       { id(pwrsys_highest_voltage).publish_state(highestVoltage);        return; }
          if (sscanf(str.c_str(), " Average voltage : %7d mV", &intValue) == 1)             { id(pwrsys_average_voltage).publish_state(intValue);              return; }
          if (sscanf(str.c_str(), " Lowest voltage : %7d mV", &lowestVoltage) == 1)         { id(pwrsys_lowest_voltage).publish_state(lowestVoltage);          return; }
          if (sscanf(str.c_str(), " Highest temperature : %6d mC", &intValue) == 1)         { id(pwrsys_high_Temp).publish_state(intValue);                    return; }
          if (sscanf(str.c_str(), " Average temperature : %6d mC", &intValue) == 1)         { id(pwrsys_average_Temp).publish_state(intValue);                 return; }
          if (sscanf(str.c_str(), " Lowest temperature : %6d mC", &intValue) == 1)          { id(pwrsys_low_Temp).publish_state(intValue);                     return; }
          if (sscanf(str.c_str(), " Recommend chg voltage : %7d mV", &intValue) == 1)       { id(pwrsys_recommend_chg_voltage).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), " Recommend dsg voltage : %8d mV", &intValue) == 1)       { id(pwrsys_recommend_dsg_voltage).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), " Recommend chg current : %7d mA", &intValue) == 1)       { id(pwrsys_recommend_chg_current).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), " Recommend dsg current : %8d mA", &intValue) == 1)       { id(pwrsys_recommend_dsg_current).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), " system Recommend chg voltage: %7d mV", &intValue) == 1) { id(pwrsys_system_recommend_chg_voltage).publish_state(intValue); return; }
          if (sscanf(str.c_str(), " system Recommend dsg voltage: %8d mV", &intValue) == 1) { id(pwrsys_system_recommend_dsg_voltage).publish_state(intValue); return; }
          if (sscanf(str.c_str(), " system Recommend chg current: %7d mA", &intValue) == 1) { id(pwrsys_system_recommend_chg_current).publish_state(intValue); return; }
          if (sscanf(str.c_str(), " system Recommend dsg current: %8d mA", &intValue) == 1) { id(pwrsys_system_recommend_dsg_current).publish_state(intValue); return; }
        }
      } //sscan
        delay(100);
      break;
  // -pwr- Section
    case pwr: 
      this->write_str("pwr\n"); 
       // delay(10);
      if (sscanf(str.c_str(), "pwr") ==1)      { ESP_LOGD(TAG, "pwr command OK");         // return; }
        char mostempr_s[6];
        const int parsed = sscanf(                                                                                                   // NOLINT
          buffer.c_str(), "%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %4d-%2d-%2d %2d:%2d:%2d %7s %7s %5s %7s",                    // NOLINT
          &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st, l.curr_st, l.temp_st,  // NOLINT
          &l.ccoulomb, &l.year, &l.month, &l.day, &l.hour, &l.minute, &l.second, l.bv_st, l.bt_st, mostempr_s, l.mos_st);            // NOLINT
// example line to parse:
// Power       Volt     Curr     Tempr     Tlow     Thigh     Vlow     Vhigh     Base.St    Volt.St    Curr.St    Temp.St    Coulomb     Time                    B.V.St   B.T.St   MosTempr     M.T.St
// %d          %d       %d       %d        %d       %d        %d       %d        %7s        %7s        %7s        %7s        %d%%        %4d-%2d-%2d %2d:%2d:%2d %7s      %7s      %d           %7s
// 1           50548    8910     25000     24200    25000     3368     3371      Charge     Normal     Normal     Normal     97%         2021-06-30  20:49:45    Normal   Normal   22700        Normal
// &l.bat_num, &l.volt, &l.curr, &l.tempr, &l.tlow, &l.thigh, &l.vlow, &l.vhigh, l.base_st, l.volt_st, l.curr_st, l.temp_st, &l.coulomb, l.date,     l.time,     l.bv_st, l.bt_st, &l.mostempr, l.mos_st   
        if (l.bat_num <= 0) {
          ESP_LOGD(TAG, "invalid bat_num in line %s", buffer.substr(0, buffer.size() - 2).c_str()); return; }
        if (parsed != 17) {
          ESP_LOGW(TAG, "invalid line: found only %d items in %s", parsed, buffer.substr(0, buffer.size() - 2).c_str()); return; }
        auto mostempr_parsed = parse_number<int>(mostempr_s);
          if (mostempr_parsed.has_value()) { l.mostempr = mostempr_parsed.value(); } 
          else {
            l.mostempr = -300;
            l.mos_st == "NULL";
            ESP_LOGW(TAG, "bat_num %d: received no mostempr", l.bat_num);
          }
        for (PylontechListener *listener : this->listeners_) { listener->on_line_read(&l); }
      } //sscan
        delay(100);
      break;
  // -info- Section
    case pwrsys: 
      this->write_str("info\n"); 
       // delay(10);
      if (sscanf(str.c_str(), "info") ==1)                                           { ESP_LOGD(TAG, "info command OK");                             // return; }
        while (rc != endMarker) {
          if (sscanf(str.c_str(), "Device address : %3d", &intValue) == 1)           { id(info_device_address).publish_state(intValue);         return; }
          if (sscanf(str.c_str(), "Manufacturer : %[^\r\n]", stringValue) == 1)      { id(info_manufacturer).publish_state(stringValue);        return; }
          if (sscanf(str.c_str(), "Device name : %[^\r\n]", stringValue) == 1)       { id(info_device_name).publish_state(stringValue);         return; }
          if (sscanf(str.c_str(), "Board version : %[^\r\n]", stringValue) == 1)     { id(info_board_version).publish_state(stringValue);       return; }
          if (sscanf(str.c_str(), "Board : %[^\r\n]", stringValue) == 1)             { id(info_board).publish_state(stringValue);               return; }
          if (sscanf(str.c_str(), "Main Soft version : %[^\r\n]", stringValue) == 1) { id(info_main_soft_version).publish_state(stringValue);   return; }
          if (sscanf(str.c_str(), "Soft  version : %[^\r\n]", stringValue) == 1)     { id(info_soft_version).publish_state(stringValue);        return; }
          if (sscanf(str.c_str(), "Boot  version : %[^\r\n]", stringValue) == 1)     { id(info_boot_version).publish_state(stringValue);        return; }
          if (sscanf(str.c_str(), "Comm version : %[^\r\n]", stringValue) == 1)      { id(info_comm_version).publish_state(stringValue);        return; }
          if (sscanf(str.c_str(), "Release Date : %[^\r\n]", stringValue) == 1)      { id(info_release_sate).publish_state(stringValue);        return; }
          if (sscanf(str.c_str(), "Barcode : %[^\r\n]", stringValue) == 1)           { id(info_barcode).publish_state(stringValue);             return; }
          if (sscanf(str.c_str(), "Specification : %[^\r\n]", stringValue) == 1)     { id(info_specification).publish_state(stringValue);       return; }
          if (sscanf(str.c_str(), "Cell Number : %2d", &intValue) == 1)              { id(info_cell_number).publish_state(intValue);            return; }
          if (sscanf(str.c_str(), "Max Dischg Curr : %8d mA", &intValue) == 1)       { id(info_max_dischg_curr).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), "Max Charge Curr : %8d mA", &intValue) == 1)       { id(info_max_charge_curr).publish_state(intValue);        return; }
          if (sscanf(str.c_str(), "EPONPort rate : %5d", &intValue) == 1)            { id(info_eponport_rate).publish_state(intValue);          return; }
          if (sscanf(str.c_str(), "Console Port rate : %7d", &intValue) == 1)        { id(info_console_port_rate).publish_state(intValue);      return; }
        }
      } //sscan
        delay(100);
      break;
    
} // switch state_serie

float PylontechComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pylontech
}  // namespace esphome
