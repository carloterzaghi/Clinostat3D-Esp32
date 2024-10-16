# Clinostat3D-Esp32
This code was produced using four boards, one NodeMCU-Esp32 board, two Arduino Nano and one connection PCB board, so you need to use all the files to work this project, in this GitHub you should found the NodeMCU-Esp32 code part. I recommend to use the IDE PlatformIO to use this code, because was what I used to code this code, but if you want to use the Arduino IDE it is okay too. <br>
All the code produced in C++ is found in the src folder, while in the data folder you can find other content that was added through SPIFFS.h to create the web pages.</br>
If you want the see the <strong>Connection PCB</strong> part click here: [Clinostat3D-Connection-PCB](https://github.com/carloterzaghi/Clinostat3D-Connection-PCB)</br>
If you want the see the <strong>Arduino Nano 1</strong> part click here: [Clinostat3D-Arduino-Nano1](https://github.com/carloterzaghi/Clinostat3D-Arduino)</br>
If you want the see the <strong>Arduino Nano 2</strong> part click here: [Clinostat3D-Arduino-Nano2](https://github.com/carloterzaghi/Clinostat3D-Arduino2)</br>

# How to install
1. First you need to have PlatformIO install in your IDE, I recommend using VSCode in this case
2. Open this folder with PlatformIO
3. Connect your Eps32-Wroom on the computer
4. Use the "PlatformIO: Upload" to upload the src code with the C++ code
5. Go to the PlatformIO Icon > Platform folder > "Upload Filesystem Image" to upload the "data" folder to the esp32
6. Check if everything is ok with the serial input using "PlatformIO: Serial Monitor"
7. Connect with the connection that the esp32 is generating using the data you put in your code in ssid and password
8. In your internet connect to "192.168.4.1"

# How it should be assembled
![Alt text](https://github.com/carloterzaghi/Clinostat3D-Esp32/blob/main/Esp32_Main_PCB.png)

# PCB Exemple
![Alt text](https://github.com/carloterzaghi/Clinostat3D-Esp32/blob/main/NodeMCU-Esp32.png)
