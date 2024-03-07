#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>

// Define as portas Rx e Tx que serão usadas pelo Esp32 para se conectar ao Arduino
#define RXp2 16
#define TXp2 17

String receivedData = ""; // Dado recebido via WebSocket
String ReciveDataSerial = ""; // Dado recebido via Serial pelo Arduino

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

// Rede e Senha do ESP32
const char* ssid     = "ESP32-NSEE";  // Nome da Rede
const char* password = "12345678";  // Senha Precisa ter no mínimo 8 caracteres

// Init de intervalo de envio de dados do esp32 ao pagina web server
int interval = 1000;                                  // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; 

// Inputs e Outpouts do ESP32
const int ledPin = 5;

// Criação do Web Server na porta 80 e do envio de dados da placa na porta 81
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void setup() {
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2); // Inicia uma conexão via Serial com Tx e Rx ao Rx e Tx do Arduino na Serial 9600
  Serial.begin(115200);
  while (!Serial);

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

  Serial.println("GPIO configurado com sucesso!");

  Serial.println("Configurando servidor web...");
  
  // Configura o servidor
  server.begin();
  Serial.println("Servidor web configurado com sucesso!");

  // Testa se consegue pegar os arquivos na pasta /data
  if (!SPIFFS.begin()) {
    Serial.println("Mounting SPIFFS failed!");
    return;
  }

  // Rotas do servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/machine/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial2.println("on,"+String(receivedData)); // Envia via Serial para o Arduino ligar o led e envia o dado da gravidade desejada
    request->send(SPIFFS, "/pages/machineOn.html", "text/html");
  });

  server.on("/machine/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(ledPin, HIGH);
    Serial2.println("off"); // Envia via Serial para o Arduino desligar o led
    request->send(SPIFFS, "/pages/machineOff.html", "text/html");
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

 server.on("/csv/teste.csv", HTTP_GET, [](AsyncWebServerRequest *request){   // Teste CSV
    request->send(SPIFFS, "/csv/teste.csv", "text/csv");  
  });
    
  // Coemça o webSocket para poder ter o envio de dados
  webSocket.begin();                          
  webSocket.onEvent(webSocketEvent);


}

void loop() {
  webSocket.loop();                                   // Update function for the webSockets 

  // Se recebido dados do Arduino, leia e envie para o websocket
  if (Serial2.available() > 0) {
      ReciveDataSerial = Serial2.readStringUntil('\n');  // Recive data from Arduino -> buttonState, RandomNumber 
      String str = ReciveDataSerial+","+String(receivedData);          // Send data to server -> buttonState, RandomNumber, Gravidade (Informada no Site)
      int str_len = str.length() + 1;                   
      char char_array[str_len];
      str.toCharArray(char_array, str_len);             // convert to char array
      webSocket.broadcastTXT(char_array);               // send char_array to clients
  }
}
