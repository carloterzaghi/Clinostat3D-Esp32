var Socket;
document.getElementById('BTN_SEND_BACK').addEventListener('click', button_send_back);

let SD_Files;

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
  //console.log(gravityInput.value);
  //Socket.send(gravityInput.value);
}
function processCommand(event) {

  try {
    SD_Files = JSON.parse(event.data);
  } catch (error) {
    console.error('Sem dados sendo no cartão SD');
    SD_Files = "";
  }
  const container = document.getElementById('dados-container');

  container.innerHTML = '';

    // Cria uma nova tabela
    const table = document.createElement('table');
    table.border = 1;

    // Cria o cabeçalho da tabela
    const thead = document.createElement('thead');
    const headerRow = document.createElement('tr');
    const nameHeader = document.createElement('th');
    nameHeader.textContent = 'Name';
    const sizeHeader = document.createElement('th');
    sizeHeader.textContent = 'Size (KB)';
    const downloadHeader = document.createElement('th');
    downloadHeader.textContent = 'Download';
    const deleteHeader = document.createElement('th');
    deleteHeader.textContent = 'Delete';

    headerRow.appendChild(nameHeader);
    headerRow.appendChild(sizeHeader);
    headerRow.appendChild(downloadHeader);
    headerRow.appendChild(deleteHeader);
    thead.appendChild(headerRow);
    table.appendChild(thead);

    // Cria o corpo da tabela
    const tbody = document.createElement('tbody');

    if (SD_Files != ""){
    // Itera sobre cada elemento no array "dados"
    SD_Files.dados.forEach(dado => {
        // Cria uma linha para cada item
        const row = document.createElement('tr');
        const nameCell = document.createElement('td');
        nameCell.textContent = dado.nome;
        const sizeCell = document.createElement('td');
        sizeCell.textContent = (dado.size/1024.0).toFixed(2);


        // Cria as células para download e delete
        const downloadCell = document.createElement('td');
        const downloadButton = document.createElement('button');
        downloadButton.textContent = 'Download';
        downloadButton.onclick = function() {
            // Implementar a lógica de download aqui
            alert(`File Downloaded: ${dado.nome}`);
            Socket.send("Baixar:"+dado.nome);
            const a = document.createElement('a');
            a.href = `/download?filename=${encodeURIComponent(dado.nome)}`;
            a.download = dado.nome;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
        };
        downloadCell.appendChild(downloadButton);

        const deleteCell = document.createElement('td');
        const deleteButton = document.createElement('button');
        deleteButton.textContent = 'Delete';
        deleteButton.classList.add('delete-btn');
        deleteButton.onclick = function() {
            // Implementar a lógica de delete aqui
            alert(`File Deleted: ${dado.nome}`);
            Socket.send(dado.nome);
        };
        deleteCell.appendChild(deleteButton);

        row.appendChild(nameCell);
        row.appendChild(sizeCell);
        row.appendChild(downloadCell);
        row.appendChild(deleteCell);
        tbody.appendChild(row);
    }
  );}

    table.appendChild(tbody);
    container.appendChild(table);
  console.log(SD_Files);
}
window.onload = function(event) {
  initWebSocket();
}