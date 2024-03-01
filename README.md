# Clinostat3D-Esp32
This code was produced using two board, one Esp32 board and one Arduino, so you need to use both to work this project, in this GitHub you should found the Esp32 code part. I recommend to use the IDE PlatformIO to use this code, because was what I used to code this code. <br>
All the code produced in C++ is found in the src folder, while in the data folder you can find other content that was added through SPIFFS.h to create the web pages.<br>
If you want the see the Arduino part click here: [Clinostat3D-Arduino](https://github.com/carloterzaghi/Clinostat3D-Arduino)
<br><br>
# How to install
1. First you need to have PlatformIO install in your IDE, I recommend using VSCode in this case
2. Open this folder with PlatformIO
3. Connect your Eps32-Wroom on the computer
4. Use the "PlatformIO: Upload" to upload the src code with the C++ code
5. Go to the PlatformIO Icon > Platform folder > "Upload Filesystem Image" to upload the "data" folder to the esp32
6. Check if everything is ok with the serial input using "PlatformIO: Serial Monitor"
7. Connect with the connection that the esp32 is generating using the data you put in your code in ssid and password
8. In your internet connect to "192.168.4.1"
<br><br>

# How it should be assembled
![Alt text](https://github.com/carloterzaghi/Clinostat3D-Esp32/blob/main/image.png)
