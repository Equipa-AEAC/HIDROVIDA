## PlatformIO-ready sensor sketches for ESP32-S3

This folder contains cleaned versions of the sensor files you already had, but adjusted for the `esp32-s3-devkitc-1` board that is configured in [platformio.ini](/C:/Users/luiss/Documents/PlatformIO/Projects/Hidroponia/platformio.ini).

### Files

- `full_sensor_code_esp32s3.cpp`
  Integrated version for DS18B20 + pH V2 + TDS V2 + Grove water level + OLED.
- `Sensor_DS18B20_esp32s3.cpp`
  Standalone DS18B20 test.
- `Sensor_Groove_waterlevel_percentage_esp32s3.cpp`
  Standalone Grove 10 cm water level test with direct I2C reads.
- `Sensor_PH_esp32s3.cpp`
  Standalone DFRobot pH V2 test.
- `Sensor_TDS_esp32s3.cpp`
  Standalone DFRobot TDS V2 test.

### Why these versions are safer

- Your project board is an ESP32-S3, so old ESP32 pins like `GPIO34` and `GPIO35` should not be reused blindly.
- The Grove water level library constructor calls `Wire.begin()` automatically, which can fight with custom I2C pin assignments on ESP32 boards. These versions read the sensor directly through `Wire`.
- The integrated sketch uses the DS18B20 temperature to compensate both pH and TDS readings.
- All files keep the same PlatformIO libraries you already have in the project.

### Default pin map used in these versions

- `DS18B20 data`: `GPIO7`
- `pH analog out`: `GPIO4`
- `TDS analog out`: `GPIO5`
- `I2C SDA`: `GPIO8`
- `I2C SCL`: `GPIO9`

### Important hardware notes

- DS18B20 needs a `4.7k` pull-up resistor between data and `3.3V`.
- pH and TDS sensor boards must share `GND` with the ESP32-S3.
- Before connecting any analog output to the ESP32-S3 ADC, make sure the signal does not exceed the ADC input range.
- Try not to power pumps, relays, and analog sensor boards from a noisy shared line without filtering. pH and TDS readings drift badly when the supply is noisy.

### Tank marks and detection sensors

The water level sketches include:

- `TANK_1_MARK_PERCENT`
- `TANK_2_MARK_PERCENT`

These are only software markers so you can label the percentage according to your tank marks.

The integrated sketch also includes:

- `TANK_1_DETECT_PIN`
- `TANK_2_DETECT_PIN`

Leave them as `-1` if the detection sensors are not wired yet. When you wire them later, change them to real GPIO numbers and the sketch will start reporting them over Serial.

### How to use these files with PlatformIO

These files are intentionally **not** inside `src/`, because each one has its own `setup()` and `loop()`.

Use one file at a time by copying the one you want to test into [src/main.cpp](/C:/Users/luiss/Documents/PlatformIO/Projects/Hidroponia/src/main.cpp), or by creating a dedicated temporary build entry for it.

### Recommended order

1. Test `Sensor_DS18B20_esp32s3.cpp`
2. Test `Sensor_PH_esp32s3.cpp`
3. Test `Sensor_TDS_esp32s3.cpp`
4. Test `Sensor_Groove_waterlevel_percentage_esp32s3.cpp`
5. Move to `full_sensor_code_esp32s3.cpp`
