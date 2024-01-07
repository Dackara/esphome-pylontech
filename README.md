![Static Badge](https://img.shields.io/badge/Work_In_Progress-Projet_en_cours_de_r%C3%A9alisation-red?logo=adblock&logoColor=red&style=plastic)

![Static Badge](https://img.shields.io/badge/Realease-Beta-blue?style=plastic)
[![Static Badge](https://img.shields.io/badge/License-Beerware-yellow?style=plastic)](https://fr.wikipedia.org/wiki/Beerware)
[![Static Badge](https://img.shields.io/badge/Donate-ko--fi_%E2%99%A5-pink?logo=kofi&style=plastic)](https://ko-fi.com/dackara)
[![Static Badge](https://img.shields.io/badge/Sponsor-On_Github-darkgreen?logo=github&logoColor=lightgrey&style=plastic)](https://github.com/sponsors/Dackara)

# esphome-pylontech
> [!NOTE]
> ESPHome component to monitor a pylontech batterie via RS232.
>
> Fork and rewite test of the **@functionpointer** Official integration for [![Static Badge](https://img.shields.io/badge/ESPHome-_-black?logo=esphome&style=social)](https://esphome.io)

## Corrected :
- Voltage unit for `voltage_low` and `voltage_high`

## Added :
- `B.V.St`  >> Etat BV ? BUS Voltage ?
- `B.T.St`  >> Etat BT ? BUS Temperature ?
- `M.T.St`  >> Etat Mos Temperature

## Allow possible config in your yaml :
- `voltage_low:` or `cell_low:`
- `voltage_high:` or `cell_high:`
- `coulomb:` or `capacity:`

## EXEMPLE :
```
external_components:
  - source: github://dackara/esphome-pylontech@official-patched
    refresh: 0s
sensor:
    cell_low:
      id: ${pylon_id}${pylon_number}_cell_low
      name: '${pylon_name}${pylon_number}_cell_low'
    cell_high:
      id: ${pylon_id}${pylon_number}_cell_high
      name: '${pylon_name}${pylon_number}_cell_high'
#    voltage_low:
#      id: bat1_low_voltage
#      name: "Battery1 low voltage" 
#    voltage_high:
#      id: bat1_high_voltage
#      name: "Battery1 high voltage"

text_sensor:
    bv_state:
      id: ${pylon_id}${pylon_number}_bv_state
      name: '${pylon_name} ${pylon_number}_bv_state'
    bt_state:
      id: ${pylon_id}${pylon_number}_bt_state
      name: '${pylon_name}${pylon_number}_bt_state'
    mos_state:
      id: ${pylon_id}${pylon_number}_mos_state
      name: '${pylon_name}${pylon_number}_mos_state' 
```
