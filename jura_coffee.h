#include "esphome.h"
#include <array>  // for std::array

class JuraCoffee : public PollingComponent, public UARTDevice {
 private:
  std::array<Sensor *, 11> sensors;  // Number of sensors
  std::array<TextSensor *, 2> text_sensors; // Number of text sensors
  std::array<long, 11> counts;   // Number of sensors
  std::string tray_status, tank_status;

  const std::string CMD_EEPROM = "RT:0000";
  const std::string CMD_IC = "IC:";

 public:
  JuraCoffee(UARTComponent *parent, Sensor *sensor1, Sensor *sensor2, Sensor *sensor3, Sensor *sensor4, Sensor *sensor5, Sensor *sensor6, Sensor *sensor7, Sensor *sensor8, Sensor *sensor9, Sensor *sensor10, Sensor *sensor11,
             TextSensor *text_sensor1, TextSensor *text_sensor2)
      : UARTDevice(parent) {
    sensors = {sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7, sensor8, sensor9, sensor10};
    text_sensors = {text_sensor1, text_sensor2};
  }
 

  void setup() override {
    this->set_update_interval(60000);  // Update interval: 60 seconds
  }

  void update() override {
    // Fetch and process EEPROM data
    std::string eeprom_data = fetchData(CMD_EEPROM);
    if (!processEEPROMData(eeprom_data)) {
      ESP_LOGE("main", "Failed to process EEPROM data.");
      return;
    }

    // Fetch and process IC data
    std::string ic_data = fetchData(CMD_IC);
    if (!processICData(ic_data)) {
      ESP_LOGE("main", "Failed to process IC data.");
      return;
    }

    // Publish data to sensors
    publishSensorData();
  }

 private:
  // Function for reading data from the Jura coffee machine
  std::string fetchData(const std::string &command) {
    std::string result;
    int timeout = 0;

    // Clear buffer
    while (available()) read();

    // Send command
    std::string formatted_command = command + "\r\n";
    for (char c : formatted_command) {
      for (int bit = 0; bit < 8; bit += 2) {
        char rawbyte = 255;
        bitWrite(rawbyte, 2, bitRead(c, bit));
        bitWrite(rawbyte, 5, bitRead(c, bit + 1));
        write(rawbyte);
      }
      delay(8);
    }

    // Read response
    std::string inbytes;
    char inbyte = 0;
    int bit_index = 0;
    while (!(inbytes.size() >= 2 && inbytes.substr(inbytes.size() - 2) == "\r\n")) {
      if (available()) {
        byte rawbyte = read();
        bitWrite(inbyte, bit_index, bitRead(rawbyte, 2));
        bitWrite(inbyte, bit_index + 1, bitRead(rawbyte, 5));
        if ((bit_index += 2) >= 8) {
          inbytes += inbyte;
          bit_index = 0;
        }
      } else {
        delay(10);
      }
      if (++timeout > 1500) {
        ESP_LOGE("main", "Timeout waiting for response to command: %s", command.c_str());
        return "";
      }
    }
    return inbytes.substr(0, inbytes.length() - 2);
  }

  // Helper for parsing a hexadecimal substring
  long parseHexSubstring(const std::string &data, int start, int end) {
    if (end > data.length() || start < 0 || start >= end) {
      ESP_LOGE("main", "Invalid substring range: start=%d, end=%d", start, end);
      return 0;
    }
    std::string substring = data.substr(start, end - start);
    if (!isHexadecimal(substring)) {
      ESP_LOGE("main", "Invalid hex substring: %s", substring.c_str());
      return 0;
    }
    return strtol(substring.c_str(), NULL, 16);
  }

  // Check if a string is hexadecimal
  bool isHexadecimal(const std::string &str) {
    for (char c : str) {
      if (!isxdigit(c)) return false;
    }
    return true;
  }

// Process the EEPROM data to extract the coffee count values
bool processEEPROMData(const std::string &data) {
  if (data.empty()) return false;

  // Log the raw unparsed EEPROM data in one line

  //std::string log_msg = "Raw EEPROM Data: " + data;
  //ESP_LOGD("main", "%s", log_msg.c_str());
  

  // Log the raw unparsed EEPROM data in one line
  //ESP_LOGD("main", "Raw EEPROM Data: %s", data.c_str());

  // Known fields
  counts[0] = parseHexSubstring(data, 3, 7);   // Single espresso
  counts[1] = parseHexSubstring(data, 7, 11);  // Double espresso
  counts[2] = parseHexSubstring(data, 11, 15); // Single Coffee
  counts[3] = parseHexSubstring(data, 15, 19); // Double coffee
  counts[4] = parseHexSubstring(data, 19, 23); // Single Ristretto  
  counts[5] = parseHexSubstring(data, 23, 27); // Single Capuccino
  counts[6] = parseHexSubstring(data, 27, 31); // Double Ristretto   
  counts[7] = parseHexSubstring(data, 31, 35); // Brew-unit movements
  counts[8] = parseHexSubstring(data, 35, 39); // Cleanings
  counts[9] = parseHexSubstring(data, 39, 43); // descaling counter

  counts[10] = parseHexSubstring(data, 59, 63); // num of coffee grounds due to cleaning 
  
  /*
  // Log the known counter fields
  //  for (int i = 0; i < 10; ++i) { // actual counts[i] replace it when needed
  //  ESP_LOGD("main", "counters %d: %ld", i + 1, counts[i]);
  }
*/

  // Unknown fields 
  // long unknown_fields[5]; 
     
  //  unknown_fields[0] = parseHexSubstring(data, 43, 47); // Unknown field 
  //  unknown_fields[1] = parseHexSubstring(data, 47, 51); // Unknown field 
  //  unknown_fields[2] = parseHexSubstring(data, 51, 55); // Unknown field 
  //  unknown_fields[3] = parseHexSubstring(data, 55, 59); // Unknown field 
  //  unknown_fields[4] = parseHexSubstring(data, 63, 67); // brewings without filter?

  // //  // Log the unanalyzed fields
  //  for (int i = 0; i < 5; ++i) { // actual unknown_fields[i] replace it when needed 0 + [i]
  //    ESP_LOGD("main", "Unknown Field %d: %ld", i + 1, unknown_fields[i]);
  //  }

  return true;
}


  // Process IC data to determine the tray and tank status
  bool processICData(const std::string &data) {
    if (data.empty()) return false;

    std::string hex_string = data.substr(3, 5 - 3);
    if (!isHexadecimal(hex_string)) return false;

    byte hex_to_byte = strtol(hex_string.c_str(), NULL, 16);
    tray_status = bitRead(hex_to_byte, 4) ? "Present" : "Missing";
    tank_status = bitRead(hex_to_byte, 5) ? "Fill Tank" : "OK";
    return true;
  }

  // Publish data to sensors
  void publishSensorData() {
    for (auto sensor : sensors) {
      if (sensor != nullptr) {
        sensor->publish_state(counts[&sensor - &sensors[0]]); // Index calculation
      }
    }

    if (text_sensors[0] != nullptr) text_sensors[0]->publish_state(tray_status);
    if (text_sensors[1] != nullptr) text_sensors[1]->publish_state(tank_status);
  }
};