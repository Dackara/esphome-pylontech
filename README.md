# esphome-pylontech
ESPHome component to monitor a pylontech batterie via RS232

VOLTAGE IS VOLTAGE, NOT TEMPERATURE (corrected for voltage_low and voltage_hight)

Replace `voltage_low` to `cell_low:` and `voltage_hight` to `cell_hight`   #(is the voltage of one cellule)

 In your confuguration files and use that to your YAML :

```external_components:
  - source: github://dackara/esphome-pylontech@official-patched
    refresh: 0s```
