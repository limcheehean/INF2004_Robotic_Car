# INF2004_Robotic_Car
An intelligent robotic car, seamlessly integrating advanced capabilities including:
1. Maze Mapping
2. Fast Navigation with Shortest Path avoiding obstacle and wall
3. Accurate Barcode Scanning
4. Intuitive User-friendly Interface

All powered by Raspberry Pi Pico W.
## Equipment
1. Pico C/C++ SDK installed
2. Raspberry Pi Pico W
3. Micro-USB Cable
4. L298N Motor Driver Module
5. 2x DC Motor (left and right)
6. 2x WYC-H206 IR Rotary Speed Sensing Module (left and right)
7. 3x MH-Series Line Tracking Module (left, right and centre/barcode)
8. 1x HC-SR04 Ultrasonic Sensor
9. 1x GY-511 Magnetometer
## Pin Out for Raspberry Pi Pico W
![image](https://github.com/limcheehean/INF2004_Robotic_Car/assets/75230061/f9305d72-c14d-4dff-badd-0b419738d840)
## Set up of Pico SDK
For Windows OS only,
1. Download and install [this](https://github.com/raspberrypi/pico-setup-windows/releases/latest/download/pico-setup-windows-x64-standalone.exe) tool.
2. Visual Studio Code will ask if you want to configure the pico-examples project when it is first opened.
3. Click Yes on that prompt to proceed.
4. You will then be prompted to select a kit -- select the Pico ARM GCC - Pico SDK Toolchain with GCC arm-none-eabi entry.

**NOTE:** Please restart your laptop multiple times after installing the SDK.
## Required Libraries
Ensure that you install the necessary libraries before executing the project.
1. pico_cyw43_arch_lwip_threadsafe_background
2. pico_stdlib
3. pico_lwip_http
4. pico_lwip_iperf
5. hardware_adc
6. hardware_pwm
7. hardware_i2c
8. hardware_uart
9. hardware_timer
10. FreeRTOS-Kernel-Heap4
## Build, compile and run project

## Block Diagram
![image](https://github.com/limcheehean/INF2004_Robotic_Car/assets/35133370/d978c355-1fe3-474b-acdd-8aaa2dfe0434)
## Flow Chart
![image](https://github.com/limcheehean/INF2004_Robotic_Car/assets/35133370/764a51b6-fb15-4833-935e-1e46288d6947)

