# Digital Weather Clock README

## Overview

This repository contains the code and setup instructions for a Digital Weather Clock project. The clock displays the current time, date, and weather information using an ESP32 microcontroller and MAX7219 8x8 dot matrix display modules. The project leverages worldtimeapi.org to sync time once an hour, and an online weather API to fetch real-time weather data.

## Table of Contents

- [Overview](#overview)
- [Table of Contents](#table-of-contents)
- [Features](#features)
- [Components](#components)
- [Setup Instructions](#setup-instructions)
- [Web UI Control Interface](#web-ui-control-interface)
  - [Features](#features-1)
  - [Accessing the Web UI](#accessing-the-web-ui)
  - [Using the Web UI](#using-the-web-ui)
- [Libraries Used](#libraries-used)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Features

- Displays current time in HH:MM:ss format.
- Shows current date in DAY MM.DD format.
- Synchronizes the time once an hour using worldtimeapi.
- Fetches and displays real-time weather information.
- Web UI for Configuration: A web-based interface allows for configuration of clock settings, including mode of operation, timezone, brightness, weather units and location.
- Sticky settings using EEPROM
- Utilizes an ESP32 microcontroller for processing.
- Uses MAX7219 8x8 dot matrix display modules for output.

## Components

- ESP32 microcontroller
- MAX7219 8x8 dot matrix display modules (4 modules recommended)
- Jumper wires
- Breadboard or PCB for assembly
- Power supply for ESP32 (usbc cable)

## Setup Instructions

1. **Install PlatformIO:**

   - **VS Code Installation:**

     - Download and install Visual Studio Code from [here](https://code.visualstudio.com/).
     - Open Visual Studio Code and go to the Extensions view by clicking the square icon in the sidebar or pressing `Ctrl+Shift+X`.
     - Search for "PlatformIO IDE" and click `Install`.

   - **Command Line Installation:**
     - Install PlatformIO Core CLI by following the instructions [here](https://platformio.org/install/cli).

2. **Clone the Repository:**

   ```sh
   git clone https://github.com/AlexanderBiba/esp32-dotmatrix-wifi-clock
   cd esp32-dotmatrix-wifi-clock
   ```

3. **Open the Project:**

   - Open Visual Studio Code.
   - Click on `File` -> `Open Folder` and select the `esp32-dotmatrix-wifi-clock` directory.

4. **Install Dependencies:**

   - Open the PlatformIO Home by clicking on the PlatformIO icon in the sidebar.
   - Click on `Projects` -> `Open` and select the `esp32-dotmatrix-wifi-clock` project.
   - PlatformIO will automatically install the necessary libraries specified in the `platformio.ini` file.

5. **Wiring:**

   - Connect the MAX7219 modules to the ESP32 as follows:
     - VCC to 3.3V
     - GND to GND
     - DIN to GPIO 13
     - CS to GPIO 12
     - CLK to GPIO 14

6. **Upload the Code:**

   - Connect your ESP32 to your computer via USB.
   - In PlatformIO, click on the `PlatformIO: Upload` button in the status bar or use the command `PlatformIO: Upload` from the command palette (`Ctrl+Shift+P`).

7. **Monitor Serial Output (Optional):**
   - To view the serial output for debugging, click on the `PlatformIO: Serial Monitor` button or use the command `PlatformIO: Monitor` from the command palette.

Your Digital Weather Clock should now be running, displaying the current time, date, and weather information on the MAX7219 8x8 dot matrix display modules.

## Web UI Control Interface

The Digital Weather Clock includes a web-based user interface to control and configure various settings. This interface can be accessed through a browser by navigating to http://digiclk.local/ or the ESP32's IP address on your local network. Settings available through the Web UI:

### Features

- **General Settings:**

  - Timezone: Select the appropriate timezone from a dropdown list.
  - Brightness: Adjust the display brightness.

- **Weather Settings:**

  - Weather Location: Set the latitude and longitude for the weather data.
  - Weather Units: Choose between Celsius and Fahrenheit for temperature display.

### Accessing the Web UI

1. Connect the ESP32 to your local WiFi network.
2. Determine the IP address assigned to the ESP32 (this can typically be found in your router's connected devices list or through serial monitor output).
3. Open a web browser and navigate to `http://digiclk.local/`.
   - If using more than 1 clock, navigate to `http://<ESP32_IP_ADDRESS>``.

### Using the Web UI

- **General Settings:**

  - Select your timezone from the dropdown list and click `Set`.
  - Enter a brightness level (0-15) and click `Set` to adjust the display brightness.

- **Weather Settings:**

  - Enter the latitude and longitude for your location and click `Set`.
  - Select either Celsius or Fahrenheit for the temperature units and click `Set`.

This user-friendly interface simplifies the process of configuring and controlling the Digital Weather Clock, making it accessible even to users with minimal technical expertise.

## Libraries Used

- [MD_MAX72XX](https://platformio.org/lib/show/125/MD_MAX72XX) by MajicDesigns (version ^3.5.1)
- [ArduinoHttpClient](https://platformio.org/lib/show/1821/ArduinoHttpClient) by arduino-libraries (version ^0.6.0)
- [ArduinoJson](https://platformio.org/lib/show/64/ArduinoJson) by bblanchon (version ^7.0.4)
- [WiFiManager](https://platformio.org/lib/show/567/WiFiManager) by wnatth3 (version ^2.0.16-rc.2)

## Usage

1. **Power the ESP32:**
   Ensure your ESP32 is powered either via USB or an external power source.

2. **Observe the Display:**
   The MAX7219 modules will light up and display the current time, date, and weather information.

3. **Refresh Rate:**
   The time, date and weather information is updated every hour by default.

## Troubleshooting

- **No Display:** Check wiring connections between the ESP32 and MAX7219 modules.
- **Weather Data Not Updating:** Verify your API key and city ID. Check your internet connection.
- **Incorrect Time/Date:** Ensure NTP server is reachable and the correct timezone offset is set in the code.

## Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes. Ensure your code follows the style guidelines and includes appropriate documentation.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

Feel free to reach out if you have any questions or need further assistance. Enjoy your digital weather clock project!
