#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "sd_fuctions.h"
#include <ThreeWire.h>
#include <RtcDS1302.h>

String receivedData = ""; // Dado recebido via WebSocket
String ReciveDataSerial = ""; // Dado recebido via Serial pelo Arduino
String ReciveTimer = ""; // Dado recebido via Parametro 

// Função para salvar os dados recebidos
void saveReceivedData(const char* data, size_t length) {
    // Limpa os dados recebidos anteriores
    receivedData = "";
    // Concatena os novos dados recebidos
    for (size_t i = 0; i < length; i++) {
        receivedData += data[i];
    }
    Serial.println("Recebendo dado: "+receivedData);
}

// Função informa sobre a coneção do Client no Serial
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {      // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
  switch (type) {                                     // switch on the type of information sent
    case WStype_DISCONNECTED:                         // if a client is disconnected, then type == WStype_DISCONNECTED
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:                            // if a client is connected, then type == WStype_CONNECTED
      Serial.println("Client " + String(num) + " connected");
      // optionally you can add code here what to do when connected
      break;
    case WStype_TEXT:                                 // if a client has sent data, then type == WStype_TEXT
      saveReceivedData((const char*)payload, length);
      
      // Imprime os dados recebidos
      Serial.print("Payload Recebido: ");
      Serial.println((const char*)payload);
      break;
  }
}

// Função que retorna o dia e a hora
String CSVDayTime(const RtcDateTime& dt) {
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u,%02u:%02u:%02u"),
             dt.Day(),
             dt.Month(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  return datestring;
}

// Rede e Senha do ESP32
const char* ssid     = "Teste-NSEE";  // Nome da Rede
const char* password = "12345678";  // Senha Precisa ter no mínimo 8 caracteres

// Init de intervalo de envio de dados do esp32 ao pagina web server
int interval = 1000;                                  // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; 
int collectDataSD = 0;    // Info if needs to store data or not
int sdExist = 0;   // If exist the SD card
int sdPage = 0;   // Enter in the SD card File Manager
String fileNameTXT = ""; // File Name Start

// Inputs e Outpouts do ESP32
const int ledPin = 4;
const int Enable =  27;  // Pin module RS485
const int IO = 25;    // DAT  DS-1302
const int SCLK = 14;  // CLK  DS-1302
const int CE = 26;    // RST  DS-1302

// ThreeWire para o módulo DS-1302 e Define o Counter
ThreeWire myWire(IO, SCLK, CE);
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))


// Criação do Web Server na porta 80 e do envio de dados da placa na porta 81
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void setup() {
  Serial2.begin(9600); // Start RS485 
  Serial.begin(9600);
  
  while (!Serial);
  // Inicia a configuração do Acess Point do ESP32
  Serial.println("Configurando AP...");

  WiFi.softAP(ssid, password);

  Serial.println("AP configurado com sucesso!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Senha: ");
  Serial.println(password);
  Serial.print("IP address = ");
  Serial.println(WiFi.softAPIP());
  
  Serial.println("Configurando GPIO...");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(Enable, OUTPUT);
  digitalWrite(Enable, LOW);

  Serial.println("GPIO configurado com sucesso!");

  Serial.println("Configurando servidor web...");
  
  // Configura o servidor
  server.begin();
  Serial.println("Servidor web configurado com sucesso!");
  // Começa o DS-1302
  Rtc.Begin(); 
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println(CSVDayTime(compiled));

  if (!Rtc.IsDateTimeValid()) {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  Serial.println(CSVDayTime(now));
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // Testa se consegue pegar os arquivos na pasta /data
  if (!SPIFFS.begin()) {
    Serial.println("Mounting SPIFFS failed!");
    return;
  }

  // Testa o cartão SD se consegue pegar os arquivos
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  // Rotas do servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(ledPin, LOW);
    digitalWrite(Enable, HIGH);   // Enable the RS485 module
    Serial2.println("off"); // Envia via Serial para o Arduino desligar o led
    Serial2.flush();   // Wait to send data
    digitalWrite(Enable, LOW);   // Disable the RS485 module
    receivedData = "";  // Set Gravity to None 
    collectDataSD = 0;     // Info that dont need to collect data
    sdPage = 0;   // Leave the SD Card Manager Page
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/machine/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    String ReciveParamater = request->getParam("filename")->value();   // Get Parameter (filename)
    ReciveTimer = request->getParam("timer")->value();   // Get Parameter (timer)
    Serial.println(ReciveTimer);
    fileNameTXT  = "/" + ReciveParamater + ".txt"; // Real File Name
    digitalWrite(Enable, HIGH);  // Enable the RS485 module
    Serial2.println("on,"+String(receivedData)); // Envia via Serial para o Arduino ligar o led e envia o dado da gravidade desejada
    Serial2.flush();  // Wait to send data
    digitalWrite(Enable, LOW);   // Disable the RS485 module
    if (existFile(SD, (char*) fileNameTXT.c_str()) == 0) writeFile(SD,(char*) fileNameTXT.c_str(), "buttonState, RandomNumber, Gravidade, Dia, Horário(BR)\n"); // Create de SD File if not exist
    collectDataSD = 1;   // Send to collect data
    sdPage = 0;   // Leave the SD Card Manager Page
    request->send(SPIFFS, "/pages/machineOn.html", "text/html");
  });

  server.on("/machine/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(Enable, HIGH);   // Enable the RS485 module
    Serial2.println("off"); // Envia via Serial para o Arduino desligar o led
    Serial2.flush();   // Wait to send data
    digitalWrite(Enable, LOW);   // Disable the RS485 module
    collectDataSD = 0;     // Info that dont need to collect data
    receivedData = "";  // Set Gravity to None 
    sdPage = 0;   // Leave the SD Card Manager Page
    request->send(SPIFFS, "/pages/machineOff.html", "text/html");
  });


  server.on("/sd/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    sdPage = 1;  // Enter in the SD Card Manager Page
    request->send(SPIFFS, "/pages/SDdata.html", "text/html");
  });

  // Carregando arquivo CSS e Script para as páginas
  server.on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){  // CSS das paginas pages
    request->send(SPIFFS, "/css/style.css", "text/css"); 
  });

  server.on("/css/indexStyle.css", HTTP_GET, [](AsyncWebServerRequest *request){   // CSS da pagina Index.html
    request->send(SPIFFS, "/css/indexStyle.css", "text/css"); 
  });

  server.on("/images/logo_nsee.png", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(SPIFFS, "/images/logo_nsee.png", "text/png"); 
  });

  server.on("/script/socket.js", HTTP_GET, [](AsyncWebServerRequest *request){  // Pagina Javascript para o Index.html
    request->send(SPIFFS, "/script/socket.js", "text/javascript"); 
  });

  server.on("/pages/sendWebSocketData.js", HTTP_GET, [](AsyncWebServerRequest *request){   // Pagina Javascript para as pages
    request->send(SPIFFS, "/pages/sendWebSocketData.js", "text/javascript"); 
  });

  server.on("/pages/SDdataScript.js", HTTP_GET, [](AsyncWebServerRequest *request){   // Pagina Javascript para as pages
    request->send(SPIFFS, "/pages/SDdataScript.js", "text/javascript"); 
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileName = request->getParam("filename")->value();   // Get Parameter (filename)
    String SD_Dowload_File  = "/" + fileName; // Create the Text to SD File
    request->send(SD, SD_Dowload_File.c_str(), "text");  // Dowload SD File from Web page
  });

  // Coemça o webSocket para poder ter o envio de dados
  webSocket.begin();                          
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();                                   // Update function for the webSockets 
  
  File fileTest = SD.open("/");
  if(!fileTest){
    sdExist = 0;
  }
  else sdExist = 1;
  fileTest.close();
 
  if (sdPage == 0){
    RtcDateTime now = Rtc.GetDateTime();

    // Se recebido dados do Arduino, leia e envie para o websocket
    if (Serial2.available() > 0) {

        ReciveDataSerial = Serial2.readStringUntil('\n');  // Recive data from Arduino -> buttonState, RandomNumber 
        
        // Remove the /n /r and /0
        ReciveDataSerial.replace("\n", ""); 
        ReciveDataSerial.replace("\r", ""); 
        ReciveDataSerial.replace("\0", ""); 

        String str = String(sdExist)+","+ReciveDataSerial+","+String(receivedData)+","+ReciveTimer;          // Send data to server -> buttonState, RandomNumber, Gravidade (Informada no Site), Timer (Informada no Site)
        Serial.println(str);
        webSocket.broadcastTXT((char*) str.c_str());               // send char_array to clients

        // String to add on .txt SD file
        String strSD = ReciveDataSerial+","+String(receivedData)+","+CSVDayTime(now)+"\n";          // Send data to SD -> buttonState, RandomNumber, Gravidade (Informada no Site), Dia, Horário 
        if (collectDataSD == 1) appendFile(SD,(char*) fileNameTXT.c_str(), (char*) strSD.c_str());   // Add data to SD file
    }
    // Se der problema com o DS-1302
    if (!now.IsValid()) {
      // Common Causes:
      //    1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }
  else if (sdPage == 1){
    String SD_Data = listDir(SD, "/", 0);  // <= Preciso pegar os dados ainda e transformar eles em um char* e mandar para o Web
    //Serial.println(SD_Data);
    Serial.println(receivedData);
    if (receivedData.startsWith("Baixar:")) { // Verifica se a string começa com "Baixar:"
      String receivedData2 = receivedData.substring(7); // Remove "Baixar:" da string
      Serial.println(receivedData2);
      receivedData = "";
    }
    else if (receivedData != ""){
      String path = "/"+receivedData;
      deleteFile(SD, (char*) path.c_str());
      receivedData = "";
    }
    else{
      delay(300);
      webSocket.broadcastTXT((char*) SD_Data.c_str());           // send char_array to clients
    }

  }
}