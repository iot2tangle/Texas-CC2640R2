
# CC2640R2 with I2T Sensors Stack  --  Bluetooth Low Energy - BLE

**CC2640R2** is one of the microcontrollers suitable for low power applications. Created and manufactured by *Texas Instruments*.

This Repository contains the source code and the steps to follow to be able to make ***CC2640R2 Microcontroller*** read sensor data and send it, in an organized way, to the **[Tangle](https://www.youtube.com/watch?v=ESF8UZM70wU) (DLT)** of the **[IOTA Network](https://www.iota.org/)** through the **[Streams](https://www.iota.org/solutions/streams)** layer.

# Setting up your CC2640R2 with the I2T Sensors Stack

This is the list of Sensors/Modules that you can connect and it will be recognized immediately.
- ***BME280*** (*Bosch*) - Temperature, Humidity and Pressure sensor. -> Connected by *I2C Bus* via: *DIO5/SDA* and *DIO4/SCL* --- Be careful with the supply of this sensor, the BM280 is powered with 3.3V, if your module does not have a voltage regulator (some modules do not have it) the sensor can be damaged if you supply 5V. *SDO* pin of the module should be tied to GND to have a fixed I2C address.
- ***MPU6050*** (*InvenSense-TDK*) - Acelerometer and Gyroscope 6-axis. -> Connected by *I2C Bus* via: *DIO5/SDA* and *DIO4/SCL*.
- ***BH1750*** (*ROHM*) - Ambient Light Sensor. -> Connected by *I2C Bus* via: *DIO5/SDA* and *DIO4/SCL*.
- ***Generic Adjustable Sound Sensor with digital output*** (like *KY038 Module*) - -> Digital Signal on *DIO13*.


## Connecting the sensors

The following diagram explains how each sensor of our stack must be connected to the CC2640R2 pins. 

![I2T Sensors Stack on CC2640R2](https://github.com/iot2tangle/Texas-CC2640R2/blob/main/BLE-sender/cc2640_bb.png)

**It is not neccesary to have all the sensors listed here**, the code is able to detect which sensors were connected. In case of not connecting MPU6050 for instance only the environmental service will be active.

# Download Firmware on CC2640R2

## 1) Install the official IDE:
This repository uses the ***Code Composer Studio***, the official IDE of Texas Instruments.

[Software available for macOS, Linux and Windows](https://www.ti.com/tool/CCSTUDIO)


## 2) Download the Iot2Tangle CC2640R2 Repository and go to the 'BLE-sender' folder:
You can download the repository directly from Github, or from shell or Command Prompt with the following command:
```
git clone https://github.com/iot2tangle/Texas-CC2640R2.git
cd Texas-CC2640R2/BLE-sender
```

## 3) Edit the file config.h (Optional)
The *config.h* file is in the directory *'Texas-CC2640R2/BLE-sender/ble5_project_zero_cc2640r2lp_app/Application'* of the repository.

This step is not as important as in projects that use WiFi, since in this case we must not connect to any network, the same device is a BLE server and the Gateway will have to READ the device data. 

In this file we can configure the NAME of the device with which the BLE server will be named, set the local data update interval, unlike HTTP or MQTT senders, this interval only updates the data locally (therefore it can be smaller than in the named cases), since the sending data interval to the Tangle is determined by the BLE Gateway, not by the CC2640R2).

The flag **INTERNAL_TEMPERATURE** if true will use the internal temperature sensor to evaluate the temperature. (This value could be not accurate)
```
#define UPDATE_SENSORS_TIMEOUT              500  // TIME IN MILLISECONDS TO UPDATE THE SENSOR DATA

uint8_t name_board[]="TexasInstruments2Tangle";  //NAME OF THE BOARD

#define INTERNAL_TEMPERATURE true
```
## 4) Compile and Download the Firmware:
You will import both the folders inside BLE-sender, the first one is the application project, the second one is the stack library of BLE.
Select File->Import Projects from file System or Archive
- Select the ble5_project_zero_cc2640r2lp_app folder
- Repeat the procedure and import ble5_project_zero_cc2640r2lp_stack_library folder

Press on **FLASH** and the code will be compiled, built and flashed on the board.


## Read BLE Services and Characteristics
You may also want to read the data directly from the *BLE Server*. For this there are Free OpenSource software.

We recommend ***nRF Connect*** (free OpenSource software) of *NordicSemiconductor* available in [Desktop](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Connect-for-desktop): on Windows, macOS and Linux. And in [Mobile](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Connect-for-mobile): on Android and iOS (The mobile version is very simple and more comfortable to debug).
It only needs to be connected to the *CC2640R2 BLE Server* and it will be able to read the data from the sensors found in the Characteristics Values

Note that when you have a device connected to CC2640R2 BLE the Gateway will not be able to read the data.


# Setting up the Streams BLE Gateway

## Preparation

Install Rust if you don't have it already. More info about Rust here https://www.rust-lang.org/tools/install

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Make sure you also have the build dependencies installed, if not run:

```
sudo apt update
sudo apt install build-essential pkg-config libssl-dev libdbus-glib-1-dev
```

## Installing the Streams BLE Gateway
### Clone the repository
Get the Streams BLE Gateway repository
https://github.com/iot2tangle/Streams-BLE-gateway
```
git clone https://github.com/iot2tangle/Streams-BLE-gateway
cd Streams-BLE-gateway
```
## Scan the BLE Server (CC2640R2 BLE Server in our case)
Here you will get the address of your *CC2640R2 BLE device* (similar to the MAC address). You should make a note of it, as we'll put that address in the *config.json* file later in the next step.

Run the following to scan and get the address of your CC2640.
```
cargo run --release --bin scan
```

## Edit *config.json* file
Navigate to the root of **Streams-BLE-gateway** directory and edit the **config.json** and copy the *address* obtained in the previous step.

Here you can also configure the time interval with which the GW will read the data to the BLE Devices and send the data to the Tangle, also the node, amoung other settings.
```
{
    "device_ids": [
        "XX:XX:XX:XX:XX:XX"
    ],
    "reading_interval": 30,
    "node": "https://nodes.iota.cafe:443",
    "mwm": 14,
    "local_pow": false
}
```
## Start the Streams BLE Server

### Sending messages to the Tangle

Run the Streams BLE Gateway:

```
cargo run --release --bin ble-gateway
```

This will compile and start the *Streams BLE Gateway*. Note that the compilation process may take from 3 to 25 minutes (Pi3 took us around 15/25 mins, Pi4 8 mins and VPS or desktop machines will generally compile under the 5 mins) depending on the device you are using as host.
You will only go through the compilation process once and any restart done later will take a few seconds to have the Gateway working.

Once started, the ***Channel Id*** will be displayed, and the gateway will be open waiting for data to send to the Tangle.

![Streams Gateway receiving data](https://i.postimg.cc/pVmbty9s/Screenshot-from-2020-10-16-19-05-09.png)
*The Channel Id that will allow subscribers to access the channel data.*
### Reading messages from the Tangle

You can read the received messages directly from the **I2T Explorer**: https://explorer.iot2tangle.io/ using the Channel Id printed by the Gateway in shell.   

![I2T Explorer](https://github.com/iot2tangle/Texas-CC2640R2/blob/main/BLE-sender/streams_cc.JPG)


*For inquiries, improvements or errors detected, please start an issue in this repository.*
