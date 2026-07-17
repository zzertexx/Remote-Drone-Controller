# Remote Drone Controller
Remote Controller for my [Drone](https://github.com/zzertexx/drone) with FPV viewer with Phone and Control with Joystick

# How it works
My drone uses ELRS channel to Receive commands and this controller acts as Transmitter on ELRS using SX1280 module while getting joysticks commands from ESP32-C3 that sends them to ESP32-S3.

# How to use it
1. Order PCB
2. Solder all components as stated in Schematics
3. Flash [ELRS firmware](https://github.com/expresslrs/expresslrs) for the main ESP32-S3
4. Flash the companion firmware @ /firmware to the ESP32-C3
5. Power on, bind the controller to drone's ELRS receiver
6. Calibrate joystick min/center/max values.

# Why I designed it?
I designed drone recently, but I don't have controller for it, so I built one

# Bill of Materials
| Name | Quantity |
| --- | --- |
| E28-2G4M27S SX1280 module | 1 pc | 
| ESP32-S3 | 1 pc |
| ESP32-C3 | 1 pc |
| 2.4 GHz antenna  | 1 pc |
| Joystick | 2 pcs |
| TP4056 chip | 1 pc |
| FS8205A chip | 1 pc |
| DW01A chip | 1 pc |
| MT3608 5V converter | 1 pc |
| AMS1117 3.3V converter | 1 pc |
| WS2812 | 2 pcs |
| Tactile Button | 2 pcs |
| Resistors | 1K x3, 1.2K, 100, 4.7K x2, 0, 110K, 15K, 5.1K |
| Inductor 4.7uH | 1 pc |
| Capacitors | 0.1uF x2, 22uF x3, 47uF |
| LED THT D3mm | 2 pcs |
| 2400mAh Battery 18650 | 1 pc |
| Battery Holder | 1 pc |

# Next feautures
3D Enclosure
VTX Video Receiver for Phone

# Photos
<img width="1472" height="635" alt="image" src="https://github.com/user-attachments/assets/9c5682e8-398e-4df3-8acb-70189224a620" />
<img width="1491" height="511" alt="image" src="https://github.com/user-attachments/assets/a18f0ae0-ad75-4a7c-b250-e7b243003250" />
<img width="721" height="861" alt="image" src="https://github.com/user-attachments/assets/6682884c-286d-490c-b996-b06030a8383c" />
<img width="1307" height="822" alt="image" src="https://github.com/user-attachments/assets/e88922bd-9cfc-49d3-b998-f9fb47e39c6d" />
<img width="1477" height="574" alt="image" src="https://github.com/user-attachments/assets/794fa1bb-4329-434b-836f-6819488f60bb" />
<img width="1110" height="739" alt="image" src="https://github.com/user-attachments/assets/b0726d85-a3a5-4e7e-87b3-292ef052395b" />

# AI DISCLOSURE
AI was used for researching various components, tradeofs and their compatibility
