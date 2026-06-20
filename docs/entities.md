# Entities Reference

This page follows the usual ESPHome documentation structure: first the YAML pattern, then the configuration variables and entity keys.

> Version: **v1.0.0**  
> Current validated setup: **Pylontech US2000C in Master mode**  
> Slave mode and other Pylontech models are still under validation.

## Component Structure

This component exposes one parent `pylontech:` component, then optional entities in the standard ESPHome domains:

- `sensor`
- `text_sensor`
- `binary_sensor`
- `switch`
- `button`

The parent component is declared once:

```yaml
pylontech:
  id: pylon
  uart_id: pylon_uart
  role: master
```

Battery-level entities require a `battery:` number:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    voltage:
      name: "Pylontech 1 Voltage"
```

System-level entities do **not** require `battery:`:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_voltage:
      name: "Pylontech System Voltage"
```

Valid battery indexes:

```yaml
battery: 1  # 1 to 16
```

---

## Parent Component Configuration

```yaml
pylontech:
  id: pylon
  uart_id: pylon_uart
  role: master

  enable_info: true
  enable_getpwr: true
  enable_stat: true
  enable_bat: true
  enable_soh: true

  slow_interval: 300s
  publish_only_changes: true

  memory_mode: auto
```

### Configuration Variables

| Option | Type | Default | Description |
|---|---:|---:|---|
| `id` | ID | Required | Parent component ID. |
| `uart_id` | ID | Required | UART bus used by the battery console link. |
| `role` | enum | `master` | `master` or `slave`. |
| `enable_info` | boolean | `false` | Enables automatic `INFO` polling. |
| `info_interval` | time | `24h` | Interval for automatic `INFO` refresh. |
| `info_startup_delay` | time | `0s` | Delay after the first validated fast response before the first automatic `INFO`. |
| `enable_getpwr` | boolean | `false` | Enables `GETPWR` after the fast loop. |
| `enable_stat` | boolean | `false` | Enables automatic `STAT` polling in the slow loop. |
| `enable_bat` | boolean | `false` | Enables automatic `BAT` polling in the slow loop. |
| `enable_soh` | boolean | `false` | Enables automatic `SOH` polling in the slow loop. |
| `slow_interval` | time | `300s` | Slow loop interval for `STAT`, `BAT` and `SOH`. |
| `stat_interval` | time | `300s` | Deprecated alias. Use `slow_interval`. |
| `publish_only_changes` | boolean | `false` | Only publishes entities when the value changes. |
| `enable_login_debug_recovery` | boolean | `false` | Enables automatic Login Debug recovery after repeated failures. |
| `login_debug_failure_threshold` | integer | `3` | Number of consecutive failures before recovery. |
| `login_debug_recovery_interval` | time | `24h` | Minimum delay between automatic Login Debug recovery attempts. |
| `enable_us2000b_initialization` | boolean | `false` | Enables the US2000B initialization helper. |
| `memory_mode` | enum | `internal` | `internal`, `auto` or `psram`. |

### Memory Modes

| Mode | Description |
|---|---|
| `internal` | Standard behavior. Component buffers use normal internal allocation. |
| `auto` | Allows the component to use PSRAM when available. |
| `psram` | Explicitly places large component-managed buffers in PSRAM when available. |

The ESPHome UART driver buffer itself remains managed by ESPHome. The `memory_mode` option only affects buffers owned by this component.

---

## Sensor Platform

### Base Configuration

Battery-level sensor:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    voltage:
      name: "Pylontech 1 Voltage"
```

System-level sensor:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_voltage:
      name: "Pylontech System Voltage"
```

### Sensor Configuration Variables

| Option | Scope | Source | Requires | Unit | Description |
|---|---|---|---|---|---|
| `voltage` | Battery | `PWR` | Fast loop | V | Pack voltage. |
| `current` | Battery | `PWR` | Fast loop | A | Pack current. |
| `temperature` | Battery | `PWR` | Fast loop | °C | Pack temperature. |
| `temperature_low` | Battery | `PWR` | Fast loop | °C | Lowest reported pack temperature. |
| `temperature_high` | Battery | `PWR` | Fast loop | °C | Highest reported pack temperature. |
| `cell_voltage_low` | Battery | `PWR` | Fast loop | V | Lowest cell voltage reported by the pack summary. |
| `cell_voltage_high` | Battery | `PWR` | Fast loop | V | Highest cell voltage reported by the pack summary. |
| `soc` | Battery | `PWR` | Fast loop | % | Pack state of charge. |
| `coulomb` | Battery | `PWR` | Fast loop | % | Compatibility alias mapped to SOC. |
| `total_coulomb` | Battery | `INFO` | `enable_info` | Ah | Total coulomb information. |
| `max_voltage` | Battery | `INFO` | `enable_info` | V | Maximum voltage information. |
| `charge_times` | Battery | `INFO` | `enable_info` | count | Charge count/time information. |
| `discharge_sec` | Battery | `INFO` | `enable_info` | s | Discharge time in seconds. |
| `charge_sec` | Battery | `INFO` | `enable_info` | s | Charge time in seconds. |
| `mos_temperature` | Battery | `PWR` | Fast loop | °C | MOS temperature when available. |
| `device_address` | Battery | `INFO` | `enable_info` | address | Battery device address. |
| `cell_number` | Battery | `INFO` | `enable_info` | count | Number of cells reported by the battery. |
| `max_charge_current` | Battery | `INFO` | `enable_info` | A | Maximum charge current. |
| `max_discharge_current` | Battery | `INFO` | `enable_info` | A | Maximum discharge current. |
| `getpwr_voltage` | Battery | `GETPWR` | `enable_getpwr` | V | GETPWR voltage. |
| `getpwr_current` | Battery | `GETPWR` | `enable_getpwr` | A | GETPWR current. |
| `getpwr_temperature` | Battery | `GETPWR` | `enable_getpwr` | °C | GETPWR temperature. |
| `getpwr_residual_capacity` | Battery | `GETPWR` | `enable_getpwr` | Ah | Residual capacity. |
| `getpwr_residual_ma` | Battery | `GETPWR` | `enable_getpwr` | mA | Residual current/capacity field in mA. |
| `getpwr_tail_1` | Battery | `GETPWR` | `enable_getpwr` | raw | First parsed tail value. |
| `getpwr_unknown` | Battery | `GETPWR` | `enable_getpwr` | raw | Compatibility alias for `getpwr_tail_1`. |
| `getpwr_tail_2` | Battery | `GETPWR` | `enable_getpwr` | raw | Second parsed tail value. |
| `getpwr_cycle` | Battery | `GETPWR` | `enable_getpwr` | raw | Compatibility alias for `getpwr_tail_2`. |
| `stat_cycle_count` | Battery | `STAT` | `enable_stat` | count | Cycle count from STAT. |
| `stat_coulomb` | Battery | `STAT` | `enable_stat` | Ah | Coulomb information from STAT. |

---

## Cell Sensor Lists

Cell lists accept up to 16 entries. Declare only the cells you want to expose.

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    cell_voltages:
      - name: "Pylontech 1 Cell 01 Voltage"
      - name: "Pylontech 1 Cell 02 Voltage"
      - name: "Pylontech 1 Cell 03 Voltage"
```

| Option | Scope | Source | Requires | Unit | Description |
|---|---|---|---|---|---|
| `cell_voltages` | Battery list | `GETPWR` | `enable_getpwr` | V | Individual cell voltages. |
| `cells` | Battery list | `GETPWR` | `enable_getpwr` | V | Compatibility alias for `cell_voltages`. |
| `cell_temperatures` | Battery list | `GETPWR` | `enable_getpwr` | °C | Individual cell temperatures. |
| `bat_cell_currents` | Battery list | `BAT` | `enable_bat` | mA | Per-cell current values from BAT. |
| `bat_cell_socs` | Battery list | `BAT` | `enable_bat` | % | Per-cell SOC values from BAT. |
| `bat_cell_coulombs` | Battery list | `BAT` | `enable_bat` | mAh | Per-cell coulomb values from BAT. |
| `soh_cell_counts` | Battery list | `SOH` | `enable_soh` | count | Per-cell SOH counters. |

---

## STAT sensors

The `STAT` command provides historical counters, warning counters, protection counters and global battery statistics.

Enable it with:

```yaml
pylontech:
  id: pylon
  enable_stat: true
```

STAT sensors are declared under a battery entry:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1

    stat_soh:
      name: "Pylontech Battery 1 STAT SOH"

    stat_pwr_percent:
      name: "Pylontech Battery 1 STAT Power Percent"

    stat_pwr_coulomb:
      name: "Pylontech Battery 1 STAT Power Coulomb"
```

STAT values are parsed by their protocol labels, not by fixed line position.

This matters because different Pylontech models or firmware versions may expose different STAT layouts.

| YAML key               |  Unit | Description                            |
| ---------------------- | ----: | -------------------------------------- |
| `stat_device_address`  |     - | Device address reported by STAT        |
| `stat_data_items`      | count | Number of data items                   |
| `stat_hisdata_items`   | count | Number of history data items           |
| `stat_miscdata_items`  | count | Number of miscellaneous data items     |
| `stat_charge_count`    | count | Charge count                           |
| `stat_discharge_count` | count | Discharge count                        |
| `stat_charge_times`    | count | Charge times                           |
| `stat_status_count`    | count | Status count                           |
| `stat_idle_times`      | count | Idle times                             |
| `stat_coc_times`       | count | Charge over-current times              |
| `stat_coc2_times`      | count | Secondary charge over-current times    |
| `stat_doc_times`       | count | Discharge over-current times           |
| `stat_doc2_times`      | count | Secondary discharge over-current times |
| `stat_coca_times`      | count | Charge over-current alarm times        |
| `stat_doca_times`      | count | Discharge over-current alarm times     |
| `stat_sc_times`        | count | Short-circuit times                    |
| `stat_bat_ov_times`    | count | Battery over-voltage times             |
| `stat_bat_hv_times`    | count | Battery high-voltage times             |
| `stat_bat_lv_times`    | count | Battery low-voltage times              |
| `stat_bat_uv_times`    | count | Battery under-voltage times            |
| `stat_bat_slp_times`   | count | Battery sleep times                    |
| `stat_pwr_ov_times`    | count | Power over-voltage times               |
| `stat_pwr_hv_times`    | count | Power high-voltage times               |
| `stat_pwr_lv_times`    | count | Power low-voltage times                |
| `stat_pwr_uv_times`    | count | Power under-voltage times              |
| `stat_pwr_slp_times`   | count | Power sleep times                      |
| `stat_cot_times`       | count | Charge over-temperature times          |
| `stat_cut_times`       | count | Charge under-temperature times         |
| `stat_dot_times`       | count | Discharge over-temperature times       |
| `stat_dut_times`       | count | Discharge under-temperature times      |
| `stat_cht_times`       | count | Charge high-temperature times          |
| `stat_clt_times`       | count | Charge low-temperature times           |
| `stat_dht_times`       | count | Discharge high-temperature times       |
| `stat_dlt_times`       | count | Discharge low-temperature times        |
| `stat_shut_times`      | count | Shutdown times                         |
| `stat_reset_times`     | count | Reset times                            |
| `stat_rv_times`        | count | Reverse voltage times                  |
| `stat_input_ov_times`  | count | Input over-voltage times               |
| `stat_soh_times`       | count | SOH warning/alarm times                |
| `stat_bmicerr_times`   | count | BMIC error times                       |
| `stat_cycle_times`     | count | Cycle times                            |
| `stat_soh`             |     % | State of Health reported by STAT       |
| `stat_pwr_percent`     |     % | Power percentage reported by STAT      |
| `stat_pwr_coulomb`     |    Ah | Power coulomb value                    |
| `stat_dsg_cap`         |    Ah | Discharge capacity                     |
| `stat_ht_half_c_count` | count | High-temperature 0.5C count            |
| `stat_lt_half_c_count` | count | Low-temperature 0.5C count             |
| `stat_ht_count`        | count | High-temperature count                 |
| `stat_lt_count`        | count | Low-temperature count                  |
| `stat_lv_count`        | count | Low-voltage count                      |
| `stat_lifewarn_times`  | count | Life warning times                     |
| `stat_lifealarm_times` | count | Life alarm times                       |

---

## System Sensors

System sensors do not require `battery:`.

Example:

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_voltage:
      name: "Pylontech System Voltage"
```

| Option | Source | Requires | Unit | Description |
|---|---|---|---|---|
| `total_num` | `PWRSYS` | Fast loop | count | Total battery count. |
| `present_num` | `PWRSYS` | Fast loop | count | Present battery count. |
| `sleep_num` | `PWRSYS` | Fast loop | count | Sleeping battery count. |
| `system_voltage` | `PWRSYS` | Fast loop | V | System voltage. |
| `system_current` | `PWRSYS` | Fast loop | A | System current. |
| `system_rc` | `PWRSYS` | Fast loop | Ah | Remaining capacity. |
| `system_fcc` | `PWRSYS` | Fast loop | Ah | Full charge capacity. |
| `system_soc` | `PWRSYS` | Fast loop | % | System state of charge. |
| `system_soh` | `PWRSYS` | Fast loop | % | System state of health. |
| `highest_voltage` | `PWRSYS` | Fast loop | V | Highest system cell/pack voltage. |
| `average_voltage` | `PWRSYS` | Fast loop | V | Average system cell/pack voltage. |
| `lowest_voltage` | `PWRSYS` | Fast loop | V | Lowest system cell/pack voltage. |
| `highest_temperature` | `PWRSYS` | Fast loop | °C | Highest system temperature. |
| `average_temperature` | `PWRSYS` | Fast loop | °C | Average system temperature. |
| `lowest_temperature` | `PWRSYS` | Fast loop | °C | Lowest system temperature. |
| `recommend_charge_voltage` | `PWRSYS` | Fast loop | V | Recommended charge voltage. |
| `recommend_discharge_voltage` | `PWRSYS` | Fast loop | V | Recommended discharge voltage. |
| `recommend_charge_current` | `PWRSYS` | Fast loop | A | Recommended charge current. |
| `recommend_discharge_current` | `PWRSYS` | Fast loop | A | Recommended discharge current. |
| `response_time` | Diagnostics | Optional | ms | Last command response time. |
| `consecutive_failures` | Diagnostics | Optional | count | Current consecutive failure count. |
| `timeout_count` | Diagnostics | Optional | count | Timeout counter. |
| `uart_queue_high_watermark` | Diagnostics | Optional | count | Highest internal UART queue depth. |
| `uart_queue_full_count` | Diagnostics | Optional | count | Number of UART queue full events. |

---

## Text Sensor Platform

### Base Configuration

Battery-level text sensor:

```yaml
text_sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    manufacturer:
      name: "Pylontech 1 Manufacturer"
```

System-level text sensor:

```yaml
text_sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_status:
      name: "Pylontech System Status"
```

### Battery Text Sensors

| Option | Source | Requires | Description |
|---|---|---|---|
| `base_state` | `PWR` / `GETPWR` | Fast loop | Base state. |
| `voltage_state` | `PWR` / `GETPWR` | Fast loop | Voltage state. |
| `current_state` | `PWR` / `GETPWR` | Fast loop | Current state. |
| `temperature_state` | `PWR` / `GETPWR` | Fast loop | Temperature state. |
| `coulomb_state` | `INFO` | `enable_info` | Coulomb state. |
| `soh_state` | `INFO` / `SOH` | `enable_info` or `enable_soh` | SOH state. |
| `heater_state` | `INFO` | `enable_info` | Heater state. |
| `protect_ena` | `INFO` | `enable_info` | Protection enable state. |
| `bat_events` | `INFO` / `BAT` | `enable_info` or `enable_bat` | Battery events. |
| `power_events` | `INFO` | `enable_info` | Power events. |
| `system_fault` | `INFO` | `enable_info` | System fault. |
| `manufacturer` | `INFO` | `enable_info` | Manufacturer string. |
| `device_name` | `INFO` | `enable_info` | Device name. |
| `board` | `INFO` | `enable_info` | Board name. |
| `board_version` | `INFO` | `enable_info` | Board version. |
| `main_soft_version` | `INFO` | `enable_info` | Main software version. |
| `soft_version` | `INFO` | `enable_info` | Software version. |
| `boot_version` | `INFO` | `enable_info` | Bootloader version. |
| `comm_version` | `INFO` | `enable_info` | Communication version. |
| `release_date` | `INFO` | `enable_info` | Firmware release date. |
| `barcode` | `INFO` | `enable_info` | Battery barcode / serial-like identifier. |
| `specification` | `INFO` | `enable_info` | Battery specification. |
| `epon_port_rate` | `INFO` | `enable_info` | EPON port rate. |
| `console_port_rate` | `INFO` | `enable_info` | Console port rate. |
| `info_response` | `INFO` | `enable_info` | Raw/truncated INFO response. |
| `getpwr_response` | `GETPWR` | `enable_getpwr` | Raw/truncated GETPWR response. |
| `stat_response` | `STAT` | `enable_stat` | Raw/truncated STAT response. |
| `bat_response` | `BAT` | `enable_bat` | Raw/truncated BAT response. |
| `soh_response` | `SOH` | `enable_soh` | Raw/truncated SOH response. |
| `battery_voltage_state` | `GETPWR` / `BAT` | Matching command | Battery voltage state. |
| `battery_temperature_state` | `GETPWR` / `BAT` | Matching command | Battery temperature state. |
| `mos_state` | `GETPWR` / `BAT` | Matching command | MOS state. |

### Cell Text Sensor Lists

| Option | Source | Requires | Description |
|---|---|---|---|
| `cell_voltage_states` | `GETPWR` | `enable_getpwr` | Per-cell voltage states. |
| `cell_temperature_states` | `GETPWR` | `enable_getpwr` | Per-cell temperature states. |
| `bat_cell_balance_states` | `BAT` | `enable_bat` | Per-cell balancing states. |
| `soh_cell_statuses` | `SOH` | `enable_soh` | Per-cell SOH status strings. |

Example:

```yaml
text_sensor:
  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    cell_voltage_states:
      - name: "Pylontech 1 Cell 01 Voltage State"
      - name: "Pylontech 1 Cell 02 Voltage State"
```

### System Text Sensors

| Option | Source | Scope | Description |
|---|---|---|---|
| `system_status` | `PWRSYS` | System | System status string. |
| `last_command` | Diagnostics | System | Last command sent or completed. |
| `last_error` | Diagnostics | System | Last protocol or communication error. |
| `configured_role` | Diagnostics | System | Role configured in YAML. |
| `observed_role` | Diagnostics | System | Role observed from protocol data. |
| `role_validation` | Diagnostics | System | Role consistency check. |

---

## Binary Sensor Platform

```yaml
binary_sensor:
  - platform: pylontech
    pylontech_id: pylon
    protocol_online:
      name: "Pylontech Protocol Online"
```

| Option | Description |
|---|---|
| `protocol_online` | Indicates whether recent protocol communication is valid. |

---

## Switch Platform

```yaml
switch:
  - platform: pylontech
    pylontech_id: pylon
    force_slave_mode:
      name: "Pylontech Force Slave Mode"
```

| Option | Description |
|---|---|
| `force_slave_mode` | Runtime helper to force slave mode. Mainly intended for testing and troubleshooting. |

---

## Button Platform

```yaml
button:
  - platform: pylontech
    pylontech_id: pylon
    request_info:
      name: "Pylontech Request INFO"
```

| Option | Action |
|---|---|
| `request_info` | Manually starts an `INFO` request. |
| `request_getpwr` | Manually starts a `GETPWR` request. |
| `request_stat` | Manually starts a `STAT` request. |
| `request_bat` | Manually starts a `BAT` request. |
| `request_soh` | Manually starts a `SOH` request. |
| `login_debug` | Manually starts `Login Debug` cmd. |
| `initialize_us2000b` | Manually starts the US2000B initialization helper. |

---

## Complete Declaration Example

This example shows the declaration pattern. Do not blindly enable everything on a production ESP if you do not need all entities.

```yaml
sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_voltage:
      name: "Pylontech System Voltage"
    system_current:
      name: "Pylontech System Current"
    system_soc:
      name: "Pylontech System SOC"

  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    voltage:
      name: "Pylontech 1 Voltage"
    current:
      name: "Pylontech 1 Current"
    soc:
      name: "Pylontech 1 SOC"
    cell_voltages:
      - name: "Pylontech 1 Cell 01 Voltage"
      - name: "Pylontech 1 Cell 02 Voltage"
      - name: "Pylontech 1 Cell 03 Voltage"

text_sensor:
  - platform: pylontech
    pylontech_id: pylon
    system_status:
      name: "Pylontech System Status"
    last_error:
      name: "Pylontech Last Error"

  - platform: pylontech
    pylontech_id: pylon
    battery: 1
    manufacturer:
      name: "Pylontech 1 Manufacturer"
    device_name:
      name: "Pylontech 1 Device Name"
    barcode:
      name: "Pylontech 1 Barcode"

binary_sensor:
  - platform: pylontech
    pylontech_id: pylon
    protocol_online:
      name: "Pylontech Protocol Online"

button:
  - platform: pylontech
    pylontech_id: pylon
    request_info:
      name: "Pylontech Request INFO"
    request_getpwr:
      name: "Pylontech Request GETPWR"
```
