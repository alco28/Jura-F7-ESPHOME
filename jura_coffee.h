#include "esphome.h"

class JuraCoffee : public PollingComponent, public UARTDevice {
 private:
  Sensor *sensors[9];  // Number of sensors
  TextSensor *text_sensors[2];
  long counts[9];  
  std::string tray_status, tank_status;

  const String CMD_EEPROM = "RT:0000";
  const String CMD_IC = "IC:";

 public:
  JuraCoffee(UARTComponent *parent, Sensor *sensor1, Sensor *sensor2, Sensor *sensor3, Sensor *sensor4, Sensor *sensor5, Sensor *sensor6, Sensor *sensor7, Sensor *sensor8, Sensor *sensor9, 
             TextSensor *text_sensor1, TextSensor *text_sensor2)
      : UARTDevice(parent) {
    sensors[0] = sensor1;  // Single espresso
    sensors[1] = sensor2;  // Double espresso
    sensors[2] = sensor3;  // Coffee
    sensors[3] = sensor4;  // Double coffee
    sensors[4] = sensor5;  // Cleanings
	  sensors[5] = sensor6;  // Single Ristretto
	  sensors[6] = sensor7;  // Double Ristretto
    sensors[7] = sensor8;  // Brew-unit movements
    sensors[8] = sensor9;  // # of coffee grounds due to cleaning 
         
    text_sensors[0] = text_sensor1;  // tank status 
    text_sensors[1] = text_sensor2;  // tray status
  }
 

  void setup() override {
    this->set_update_interval(60000);  // Update interval: 60 seconden
  }

  void update() override {
    // Fetch and process EEPROM data
    String eeprom_data = fetchData(CMD_EEPROM);
    if (!processEEPROMData(eeprom_data)) {
      ESP_LOGE("main", "Failed to process EEPROM data.");
      return;
    }

    // Fetch and process IC data
    String ic_data = fetchData(CMD_IC);
    if (!processICData(ic_data)) {
      ESP_LOGE("main", "Failed to process IC data.");
      return;
    }

    // Publish data to sensors
    publishSensorData();
  }

 private:
  // Functie om data op te halen van de koffiemachine
  String fetchData(const String &command) {
    String result;
    int timeout = 0;

    // Clear buffer
    while (available()) read();

    // Send command
    String formatted_command = command + "\r\n";
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
    String inbytes;
    char inbyte = 0;
    int bit_index = 0;
    while (!inbytes.endsWith("\r\n")) {
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
    return inbytes.substring(0, inbytes.length() - 2);
  }

  // Helper om hexadecimale substrings veilig te parsen
  long parseHexSubstring(const String &data, int start, int end) {
    if (end > data.length() || start < 0 || start >= end) {
      ESP_LOGE("main", "Invalid substring range: start=%d, end=%d", start, end);
      return 0;
    }
    String substring = data.substring(start, end);
    if (!isHexadecimal(substring)) {
      ESP_LOGE("main", "Invalid hex substring: %s", substring.c_str());
      return 0;
    }
    return strtol(substring.c_str(), NULL, 16);
  }

  // Check if a string is hexadecimal
  bool isHexadecimal(const String &str) {
    for (char c : str) {
      if (!isxdigit(c)) return false;
    }
    return true;
  }

// Process the EEPROM data to extract the coffee count values
bool processEEPROMData(const String &data) {
  if (data.isEmpty()) return false;

  // Log the raw unparsed EEPROM data in one line

  String log_msg = "Raw EEPROM Data: " + data;
  ESP_LOGD("main", "%s", log_msg.c_str());
  

  // Log the raw unparsed EEPROM data in one line
  //ESP_LOGD("main", "Raw EEPROM Data: %s", data.c_str());

  // Known fields
  counts[0] = parseHexSubstring(data, 3, 7);   // Single espresso
  counts[1] = parseHexSubstring(data, 7, 11);  // Double espresso
  counts[2] = parseHexSubstring(data, 11, 15); // Coffee
  counts[3] = parseHexSubstring(data, 15, 19); // Double coffee
  counts[4] = parseHexSubstring(data, 19, 23); // Single Ristretto
  counts[5] = parseHexSubstring(data, 27, 31); // Double Ristretto  
  counts[6] = parseHexSubstring(data, 31, 35); // Brew-unit movements
  counts[7] = parseHexSubstring(data, 35, 39); // Cleanings
  counts[8] = parseHexSubstring(data, 59, 63); // num of coffee grounds due to cleaning 
  
  /*
  // Log the known counter fields
    for (int i = 0; i < 9; ++i) { // actual counts[i] replace it when needed
    ESP_LOGD("main", "counters %d: %ld", i + 1, counts[i]);
  }
*/

  // Unknown fields 
//  long unknown_fields[7]; // number of unknown fields 47, 51)

//   unknown_fields[0] = parseHexSubstring(data, 23, 27); // Unknown field 
//   unknown_fields[1] = parseHexSubstring(data, 39, 43); // Unknown field 
//   unknown_fields[2] = parseHexSubstring(data, 43, 47); // Unknown field 
//   unknown_fields[3] = parseHexSubstring(data, 47, 51); // Unknown field 
//   unknown_fields[4] = parseHexSubstring(data, 51, 55); // Unknown field 
//   unknown_fields[5] = parseHexSubstring(data, 55, 59); // Unknown field 
//   unknown_fields[6] = parseHexSubstring(data, 63, 67); // Unknown field 

//   // Log the unanalyzed fields
//   for (int i = 0; i < 7; ++i) { // actual unknown_fields[i] replace it when needed
//     ESP_LOGD("main", "Unknown Field %d: %ld", i + 1, unknown_fields[i]);
//   }

  return true;
}


  // Process IC data to determine the tray and tank status
  bool processICData(const String &data) {
    if (data.isEmpty()) return false;

    String hex_string = data.substring(3, 5);
    if (!isHexadecimal(hex_string)) return false;

    byte hex_to_byte = strtol(hex_string.c_str(), NULL, 16);
    tray_status = bitRead(hex_to_byte, 4) ? "Present" : "Missing";
    tank_status = bitRead(hex_to_byte, 5) ? "Fill Tank" : "OK";
    return true;
  }

  // Publiceer data naar sensoren
  void publishSensorData() {
    for (int i = 0; i < 9; i++) { // actual count[i] replace it when needed
      if (sensors[i] != nullptr) {
        sensors[i]->publish_state(counts[i]);
      }
    }

    if (text_sensors[0] != nullptr) text_sensors[0]->publish_state(tray_status);
    if (text_sensors[1] != nullptr) text_sensors[1]->publish_state(tank_status);
  }
};
