#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include <array>

namespace esphome {
namespace jura_coffee {

using namespace esphome;
using namespace esphome::sensor;
using namespace esphome::text_sensor;
using namespace esphome::uart;
using namespace esphome::switch_;

static const char *const TAG = "jura_coffee";
static const uint32_t UPDATE_INTERVAL_MS = 30000;
static const uint32_t UART_TIMEOUT_MS = 1500;
static const uint8_t UART_BYTE_DELAY_MS = 8;
static const uint8_t UART_READ_DELAY_MS = 10;
static const uint8_t SENSOR_COUNT = 11;
static const uint8_t TEXT_SENSOR_COUNT = 2;

class JuraCoffee : public Component, public UARTDevice {
 private:
  std::array<Sensor *, SENSOR_COUNT> sensors{};  // Initialize to nullptr
  std::array<TextSensor *, TEXT_SENSOR_COUNT> text_sensors{};  // Initialize to nullptr
  Switch *debug_switch_{nullptr};
  std::array<long, SENSOR_COUNT> counts{};
  std::string tray_status, tank_status;

  static const std::string CMD_EEPROM;
  static const std::string CMD_IC;

 protected:  // Changed to protected for better extensibility
  void send_uart_byte(uint8_t byte);
  uint8_t receive_uart_byte();
  bool wait_for_response(std::string &response);

 public:
  JuraCoffee() : Component(), UARTDevice() {}

  void set_sensors(Sensor *sensor1, Sensor *sensor2, Sensor *sensor3, Sensor *sensor4, Sensor *sensor5,
                  Sensor *sensor6, Sensor *sensor7, Sensor *sensor8, Sensor *sensor9, Sensor *sensor10,
                  Sensor *sensor11) {
    sensors = {sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7, sensor8, sensor9, sensor10, sensor11};
  }

  void set_text_sensors(TextSensor *text_sensor1, TextSensor *text_sensor2) {
    text_sensors = {text_sensor1, text_sensor2};
  }

  void set_debug_switch(Switch *debug_switch) { debug_switch_ = debug_switch; }
  bool is_debug_enabled() const { return debug_switch_ != nullptr && debug_switch_->state; }

  void setup() override;
  void loop() override;
  void update();
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 private:
  std::string fetchData(const std::string &command);
  long parseHexSubstring(const std::string &data, int start, int end);
  bool isHexadecimal(const std::string &str);
  bool processEEPROMData(const std::string &data);
  bool processICData(const std::string &data);
  void publishSensorData();
};

}  // namespace jura_coffee
}  // namespace esphome
