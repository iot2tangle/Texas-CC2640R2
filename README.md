# Texas-CC2640R2 Launchpad

This LaunchPad speeds development with a Bluetooth Low Energy (BLE) connection using the CC2640R2F or CC2640R2L devices. A Bluetooth 4.2 and 5.0 stack is also available.
This board is suited for who already has experience with mcu with operating systems like R-TOS, or those who want to learn how to deal with an embedded operating system.
Operating system of Texas Instuments is **TI-RTOS**.

The real advantage of this microcontroller is the low power capability, the board is able to sink few uA in low power mode, making possible to run with a single charge even for years ([link](https://www.ti.com/tool/TIDA-00757)). 

In this repository you will find explanations and step by step for the development of a system that every certain time interval (which you will set) will collect data from 4 sensors, this data package will be sent to a special gateway designed by *IOT2TANGLE* that you will have on the local network. This gateway will be in charge of adding these packages to **Tangle Network of IOTA**, through *Streams*.

## I2T Sensors Stack

*IOT2TANGLE* provides with code to integrate the *CC2640R2* and a give set of generic sensors (I2T sensor's stack). These are just suggested sensors and you can always add some other peripherals required by your project.

This is the list of Sensors/Modules that you can connect and it will be recognized immediately:
- ***BME280*** (Bosch) - Temperature, Humidity and Pressure sensor.
- ***MPU6050*** (InvenSense-TDK) - Acelerometer and Gyroscope 6-axis.
- ***BH1750*** (ROHM) - Ambient Light Sensor.
- ***Generic Adjustable Sound Sensor with digital output*** (like *KY038 Module*)

## Available connectivity

- **[BLE-sender](https://github.com/iot2tangle/Texas-CC2640R2/tree/main/BLE-sender)** -- CC2640R2 will send the sensors data through ***Bluetooth Low Energy (BLE)*** to **[I2T BLE Gateway](https://github.com/iot2tangle/Streams-ble-gateway)**
