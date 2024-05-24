var Socket;
document.getElementById('BTN_SEND_BACK').addEventListener('click', button_send_back);
function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    processCommand(event);
  };
}
function button_send_back() {
  Socket.send('Sending back some random stuff');
}
function processCommand(event) {

  const dadosSeparados = event.data.split(',');

  // Obter os valores individuais
  const buttonState = dadosSeparados[1];
  const randomValue = dadosSeparados[2];

  // Atualizar os elementos da página
  document.getElementById('buttonState').innerHTML = buttonState;
  document.getElementById('rand').innerHTML = randomValue;

  console.log(event.data);
}
window.onload = function(event) {
    init();
}