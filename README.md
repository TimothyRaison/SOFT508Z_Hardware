# SOFT508Z_Coursework
Controlling and using the hardware provided by Plymouth University for the SOFT508Z module

Discovered looking that the ESP32 board that the GPIO voltage only goes up to 3.6V and the arduino GPIO pins are 5V so I have purchaced a 3v3 to 5v bidirectional logic level converter (LLC) so I can get the ESP board to talk over SPI to the arduino board. 

Last: Connected ESP32 using UDP and sent packets from pc using a packets sender

To do:

- Setup ROS nodes
- Using LLC getting the ESP32 to talk to the Arduino mega
- Getting ros messages to control ouputs on hte buggy and return messages when done
