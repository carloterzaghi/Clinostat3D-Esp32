var Socket;
document.getElementById('BTN_SEND_BACK').addEventListener('click', button_send_back);
const gravityInput = document.getElementById('gravity');
var fileNameToSend = "";
var startTimer = "";

function connectWebSocket() {
  try {
    var websocket = new WebSocket('ws://' + window.location.hostname + ':81/');
    websocket.onerror = function(event) {
      console.error("WebSocket error:", event);
    };

    websocket.onclose = function(event) {
      console.log("WebSocket closed. Reconnecting...");
      setTimeout(function() {
        initWebSocket(); // Tenta reconectar após 1 segundo
      }, 1000);
    };

    return websocket;
  } catch (error) {
    console.error("Error connecting to WebSocket:", error);
    reconnectWebSocket(); // Tenta reconectar em caso de erro
  }
}

function initWebSocket() {
  Socket = connectWebSocket()
  Socket.onmessage = function(event) {
    processCommand(event);
  };
}
function button_send_back() {
  console.log(gravityInput.value);
  Socket.send(gravityInput.value);
}

function processCommand(event) {

  const dadosSeparados = event.data.split(',');

  // Obter os valores individuais
  const sdExist = dadosSeparados[0];
  const buttonState = dadosSeparados[1];
  const randomValue = dadosSeparados[2];
  const gravity =  dadosSeparados[3];
  const timerValue = dadosSeparados[4];

  // Edita o Start se não tiver conectado o cartão SD no ESP32
  if (document.body.id === 'PageOff') {
    startTimer = "";
    const startButton = document.getElementById('BTN_SEND_BACK');
    const sdButton = document.getElementById('sdButtonID');
    const filenameInput = document.getElementById('filename');
    const timeInput = document.getElementById('time');

    console.log(sdExist);
    if (String(sdExist) === '1') {
      startButton.textContent = 'Start';
      sdButton.textContent = 'SD Card File Manager';
      sdButton.style.backgroundColor = "#007bff";
      sdButton.onclick= function(){
        location.href='/sd/data'
      }
      startButton.style.backgroundColor = "#50C878";
      if (gravityInput.value != "" && filenameInput.value != ""){
        // Se o timeInput não for mexido o dado não será relevado
        if (String(timeInput.value) == "00:00:00") TimeSet = "";
        else TimeSet = String(timeInput.value);

        // Verifica se o dado está em hh:mm, se sim adiciona no final um :00 para ficar hh:mm:ss
        if (/^([01]\d|2[0-3]):([0-5]\d)$/.test(TimeSet)) {
          TimeSet += ":00"; // Adiciona :00 para os segundos
        }

        // Pega o dado do nome do arquivo para ser enviado por via URI
        fileNameToSend = String(filenameInput.value);
        startButton.onclick= function(){
          location.href=`/machine/on?filename=${encodeURIComponent(fileNameToSend.replace(/ /g, "_"))}&timer=${encodeURIComponent(TimeSet)}`
        }
      }
      else{
        startButton.textContent = 'Please enter all Clinostat Configurations';
        startButton.style.backgroundColor = "lightgrey";
        startButton.onclick= function(){}
      }
    } else if (String(sdExist) === '0') {
      startButton.textContent = 'Cant Start if out SD Card';
      sdButton.textContent = 'SD Card not Found';
      sdButton.onclick= function(){
      }
      startButton.onclick= function(){
      }
      startButton.style.backgroundColor = "lightgrey";
      sdButton.style.backgroundColor = "lightgrey";
    }
  }
  else{
    // Verifica se existe o Timer setado, se existir um valor para timerValue da um valor ao startTimer e exibe o timer 
    if (timerValue != ""){
      if (startTimer == ""){
        document.getElementById('counterTitle').textContent = "Countdown Timer";
        startTimer = timerValue; // timerValue está em hh:mm:ss // 
        document.getElementById('countdown').innerHTML = startTimer;
      }
    }

    // Atualizar os elementos da página
    if (gravity != ""){
      document.getElementById('buttonState').innerHTML = buttonState;
      document.getElementById('rand').innerHTML = randomValue;
      document.getElementById('gravityData').innerHTML = gravity;
    }
    console.log(event.data);
  }
}
window.onload = function(event) {
  initWebSocket();
}