#include "jura_coffee.h"

namespace esphome {
namespace jura_coffee {

const std::string JuraCoffee::CMD_EEPROM = "RT:0000";
const std::string JuraCoffee::CMD_IC = "IC:";

void JuraCoffee::setup() {
  // Setup runs after UART is initialized by ESPHome
}

void JuraCoffee::loop() {
  static uint32_t last_update = 0;
  const uint32_t now = millis();
  
  // Only update every UPDATE_INTERVAL_MS to prevent unnecessary UART traffic
  if ((now - last_update) < UPDATE_INTERVAL_MS) {
    return;
  }
  last_update = now;
  
  // If UART is not available, skip this update
  if (!available()) {
    return;
  }
  
  update();
}

void JuraCoffee::update() {
  static uint32_t consecutive_failures = 0;
  
  // Get EEPROM data first - this contains critical state information
  std::string eeprom_data = fetchData(CMD_EEPROM);
  if (!eeprom_data.empty() && processEEPROMData(eeprom_data)) {
    // Only fetch IC data if EEPROM read was successful
    std::string ic_data = fetchData(CMD_IC);
    if (!ic_data.empty() && processICData(ic_data)) {
      consecutive_failures = 0;  // Reset failure counter on success
      publishSensorData();
      return;
    }
  }
  
  // Track consecutive failures
  consecutive_failures++;
  
  // Log errors with increasing delays to prevent log spam
  if (consecutive_failures == 1) {
    ESP_LOGE(TAG, "Failed to communicate with Jura coffee machine");
  } else if (consecutive_failures % 10 == 0) {  // Log every 10th failure (every 5 minutes at 30s interval)
    ESP_LOGE(TAG, "Still unable to communicate with Jura coffee machine after %u attempts", consecutive_failures);
  }
}

// The rest of your fetchData(), parseHexSubstring(), processEEPROMData(), processICData(), and publishSensorData()
// functions go here, unchanged from your original implementation.

void JuraCoffee::send_uart_byte(uint8_t byte) {
  this->write(byte);
  delay(UART_BYTE_DELAY_MS);
}

uint8_t JuraCoffee::receive_uart_byte() {
  return this->read();
}

bool JuraCoffee::wait_for_response(std::string &response) {
  uint32_t timeout = 0;
  char inbyte = 0;
  int bit_index = 0;
  bool got_any_data = false;
  
  while (!(response.size() >= 2 && response.substr(response.size() - 2) == "\r\n")) {
    if (this->available()) {
      got_any_data = true;
      uint8_t rawbyte = receive_uart_byte();
      bitWrite(inbyte, bit_index, bitRead(rawbyte, 2));
      bitWrite(inbyte, bit_index + 1, bitRead(rawbyte, 5));
      if ((bit_index += 2) >= 8) {
        response += inbyte;
        bit_index = 0;
      }
    } else {
      delay(UART_READ_DELAY_MS);
    }
    if (++timeout > UART_TIMEOUT_MS) {
      if (!got_any_data) {
        ESP_LOGE(TAG, "No response from Jura coffee machine - check UART connection");
      } else {
        ESP_LOGE(TAG, "Incomplete response from Jura coffee machine: %s", response.c_str());
      }
      return false;
    }
  }
  return true;
}

std::string JuraCoffee::fetchData(const std::string &command) {
  int timeout = 0;

  if (debug_mode_) {
    ESP_LOGI(TAG, "Sending command: %s", command.c_str());
  }

  // Clear buffer
  int cleared = 0;
  while (available()) {
    read();
    cleared++;
  }
  if (debug_mode_ && cleared > 0) {
    ESP_LOGI(TAG, "Cleared %d bytes from buffer", cleared);
  }

  // Send command
  std::string formatted_command = command + "\r\n";
  if (debug_mode_) {
    ESP_LOGI(TAG, "Sending formatted command (%d bytes): ", formatted_command.length());
  }

  for (char c : formatted_command) {
    for (int bit = 0; bit < 8; bit += 2) {
      char rawbyte = 255;
      bitWrite(rawbyte, 2, bitRead(c, bit));
      bitWrite(rawbyte, 5, bitRead(c, bit + 1));
      write(rawbyte);
      if (debug_mode_) {
        ESP_LOGV(TAG, "Sent byte: 0x%02X (bits %d,%d from char '%c')", 
                 rawbyte, bit, bit+1, c);
      }
    }
    delay(8);  // Critical: delay after each character
  }

  if (debug_mode_) {
    ESP_LOGI(TAG, "Waiting for response...");
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
      if (debug_mode_) {
        ESP_LOGV(TAG, "Received byte: 0x%02X (bits %d,%d)", 
                 rawbyte, bit_index, bit_index+1);
      }
      if ((bit_index += 2) >= 8) {
        inbytes += inbyte;
        if (debug_mode_) {
          ESP_LOGV(TAG, "Assembled char: '%c' (0x%02X)", inbyte, inbyte);
        }
        bit_index = 0;
      }
    } else {
      delay(10);
    }
    if (++timeout > 1500) {
      ESP_LOGE(TAG, "Timeout waiting for response to command: %s", command.c_str());
      return "";
    }
  }

  std::string result = inbytes.substr(0, inbytes.length() - 2);
  if (debug_mode_) {
    ESP_LOGI(TAG, "Received response: %s", result.c_str());
  }
  return result;
}

long JuraCoffee::parseHexSubstring(const std::string &data, int start, int end) {
  if (end > (int)data.length() || start < 0 || start >= end) {
    ESP_LOGE("jura_coffee", "Invalid substring range: start=%d, end=%d", start, end);
    return 0;
  }
  std::string substring = data.substr(start, end - start);
  if (!isHexadecimal(substring)) {
    ESP_LOGE("jura_coffee", "Invalid hex substring: %s", substring.c_str());
    return 0;
  }
  return strtol(substring.c_str(), NULL, 16);
}

bool JuraCoffee::isHexadecimal(const std::string &str) {
  for (char c : str) {
    if (!isxdigit((unsigned char)c)) return false;
  }
  return true;
}

bool JuraCoffee::processEEPROMData(const std::string &data) {
  if (data.empty()) return false;

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

  return true;
}

bool JuraCoffee::processICData(const std::string &data) {
  if (data.empty()) return false;

  if ((int)data.length() < 5) return false;
  std::string hex_string = data.substr(3, 2);
  if (!isHexadecimal(hex_string)) return false;

  uint8_t hex_to_byte = (uint8_t)strtol(hex_string.c_str(), NULL, 16);
  tray_status = bitRead(hex_to_byte, 4) ? "Present" : "Missing";
  tank_status = bitRead(hex_to_byte, 5) ? "Fill Tank" : "OK";
  return true;
}

void JuraCoffee::publishSensorData() {
  for (int i = 0; i < 11; i++) { // actual count[i] replace it when needed
    if (sensors[i] != nullptr) {
      sensors[i]->publish_state(counts[i]);
    }
  }

  if (text_sensors[0] != nullptr) text_sensors[0]->publish_state(tray_status.c_str());
  if (text_sensors[1] != nullptr) text_sensors[1]->publish_state(tank_status.c_str());
}

}  // namespace jura_coffee
}  // namespace esphome