# Fleetpix

Drone Landing Station with Verification and Data Transfer
Project Image <!-- Replace "project_image.jpg" with an image representing your project if available -->

This repository contains the source code for a Drone Landing Station project that verifies incoming drones and transfers their data securely to a Dock Station. The project utilizes an ESP32 microcontroller, an NFC (Near Field Communication) reader for drone verification, and various communication technologies to transfer and display data.

Features
Drone Verification: The station uses an NFC reader to verify if the approaching drone is the correct one allowed to land.
Data Transfer via Wi-Fi: When the drone is verified, the landing station sends all relevant information wirelessly to the Dock Station via Wi-Fi.
Single Pair Ethernet Communication: The Dock Station receives the data using Single Pair Ethernet technology, ensuring reliable and efficient data transfer.
OLED Display: The Dock Station has an OLED screen that shows the received data, providing real-time feedback to the user.
Data Logging: All received data is stored to an SD card, allowing for later analysis and review.
Components Used
ESP32 Microcontroller: The ESP32 serves as the central processing unit for the Drone Landing Station, responsible for verification and data transfer.
NFC Reader: An NFC reader is used to identify and verify the approaching drone using its unique identifier (UID).
Wi-Fi Module: The ESP32's built-in Wi-Fi capabilities enable wireless communication between the Landing and Dock Stations.
Single Pair Ethernet Transceiver: The Dock Station utilizes a Single Pair Ethernet transceiver for reliable data transfer from the Landing Station.
OLED Screen: The Dock Station has an OLED screen to display the received data to the user.
SD Card Module: An SD card module is used to store the received data for later analysis.
Installation and Setup
Clone this repository to your local machine.
Connect the ESP32, NFC reader, OLED screen, Single Pair Ethernet transceiver, and SD card module as per the wiring instructions provided in the code comments.
Install the required libraries (Wire, Adafruit_PN532, WiFi, SPI, SD, U8g2) using the Arduino Library Manager.
Upload the code to the ESP32 using the Arduino IDE.
Power up the Drone Landing Station and Dock Station.
Usage
Drone Verification:

When a drone approaches the landing station, it should have an NFC tag with a unique identifier (UID).
The landing station's NFC reader will verify if the drone is the correct one allowed to land based on the stored UID.
If verified, the landing station will proceed with data transfer.
Data Transfer and Display:

Once the drone is verified, the landing station will gather all relevant data.
The landing station will establish a Wi-Fi connection with the Dock Station and send the data securely.
The Dock Station will receive the data and display it on the OLED screen with a cool fade-in effect for an impressive visual display.
Data Logging:

The Dock Station will store all received data onto the connected SD card for future analysis and records.
Customize
You can customize the OLED display format and data processing in the code to suit specific requirements. Feel free to experiment with different visual effects for the OLED display to make it even more captivating!

License
This project is licensed under the MIT License.

Acknowledgments
The project was developed as part of the [Your Awesome Project Name] course at [University Name], with the guidance of [Instructor Name].
