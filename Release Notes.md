# Release Notes

## Version: 1.1.0  

**Release Date:** November 2, 2025  

---

### **Summary**

This release introduces a major update to the `jura-01.yaml` configuration file for the Jura coffee machine integration with ESPHome. The configuration has been restructured and enhanced to improve compatibility, stability, and maintainability. A custom function has been implemented to ensure compatibility with ESPHome (starting from version `2023.10.0`).

---

### **New Features**

- **Custom Component Integration**:
  - The `jura_coffee` component has been restructured as a custom function to ensure compatibility with ESPHome versions `2023.10.0` and later.
  - This change simplifies the integration process and reduces the need for manual adjustments.

- **Debug Mode**:
  - Added a debug switch (`jura_debug_mode`) to dynamically enable verbose logging for troubleshooting.
  - Allows for faster polling and detailed UART communication logs.

- **Enhanced Button Controls**:
  - Added buttons for navigating the Jura menu:
    - `jura_menu`
    - `jura_menu_ENTER`
    - `jura_menu_CW` (Clockwise)
    - `jura_menu_CCW` (Counterclockwise)
  - Added product-specific buttons:
    - `Make Espresso`
    - `Make Cappuccino`
    - `Make Steam`
    - `Make Hot Water`

---

### **Improvements**

- **Stability**:
  - Improved UART communication stability by refining the `turn_on_action` and `turn_off_action` commands for the power switch.
  - Added filters to sensor configurations to ignore invalid values (`nan`) and ensure consistent state updates.

- **Compatibility**:
  - Updated the configuration to align with ESPHome's latest framework recommendations.
  - Removed deprecated or unused components, such as `status_led` and `manual_ip`.

- **Logging**:
  - Default logging level set to `INFO` to reduce memory usage while maintaining sufficient diagnostic information.

- **WiFi Configuration**:
  - Updated fallback hotspot name to `${devicename} Fallback Hotspot` for better identification.
  - Added encryption support for the Home Assistant API using a secret key.

---

### **Bug Fixes**

- Resolved issues with inconsistent state restoration by setting `restore_mode: DISABLED` for switches and buttons.
- Fixed potential memory leaks by removing unused scripts and intervals.

---

### **Breaking Changes**

- **Custom Component**:
  - The `custom_component` section has been replaced with the `jura_coffee` custom function. This change requires ESPHome version `2023.10.0` or later.
- **Removed Features**:
  - The `dallas_temp` sensor for room temperature has been removed.
  - Diagnostic sensors like `uptime` and `wifi_signal` have been removed to optimize memory usage.

---

### **Known Issues**

- The configuration assumes the use of a `d1_mini` board. Compatibility with other ESP8266 boards has not been tested.
- The debug mode (`jura_debug_mode`) may increase memory usage when enabled for extended periods.

---

### **Upgrade Instructions**

1. Ensure you are using ESPHome version `2023.10.0` or later.
2. Replace your existing `jura-01.yaml` file with the updated version.
3. Validate the configuration using the following command:

   ```bash
   esphome config w:\1.Projecten\Jura-F7-ESPHOME\jura-01.yaml
   ```

4. Compile and upload the configuration to your ESP device:

   ```bash
   esphome run w:\1.Projecten\Jura-F7-ESPHOME\jura-01.yaml
   ```

---

### **Acknowledgments**

- Special thanks to [Ryan Alden Github][ryanalden] for the original implementation to the Jura coffee machine integration.
- Discuss and comment on [home assistant forum]

[ryanalden]: https://github.com/ryanalden/esphome-jura-component
[home assistant forum]: https://community.home-assistant.io/t/control-your-jura-coffee-machine/26604
