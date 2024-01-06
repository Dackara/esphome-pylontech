# esphome-pylontech
ESPHome component to monitor a pylontech batterie via RS232

VOLTAGE IS VOLTAGE, NOT TEMPERATURE (corrected for voltage_low and voltage_high)

Replace `voltage_low` to `cell_low:` and `voltage_high` to `cell_high`   #(is the voltage of one cellule)

 In your confuguration files and use that to your YAML :

```
external_components:
  - source: github://dackara/esphome-pylontech@official-patched
    refresh: 0s
sensor:
    cell_low:
      id: ${pylon_id}${pylon_number}_low_voltage_cell
      name: '${pylon_name} ${pylon_number} low voltage cell'
    cell_high:
      id: ${pylon_id}${pylon_number}_high_voltage_cell
      name: '${pylon_name} ${pylon_number} high voltage cell'
#    voltage_low:
#      id: bat1_low_voltage
#      name: "Battery1 low voltage" 
#    voltage_high:
#      id: bat1_high_voltage
#      name: "Battery1 high voltage"  
```
