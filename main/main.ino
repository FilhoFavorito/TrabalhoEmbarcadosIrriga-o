#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <iostream>
#include <string>
#define pino_solo 4

// Configurações da rede Wi-Fi
const char* ssid = "Wifi";
const char* password = "";

WebServer server(80);

int buscaUmidade(int pino) {
  int leituraAnalogica = analogRead(pino);
  int umidadeCalculada = map(leituraAnalogica, 0, 4095, 0, 100);

  return umidadeCalculada;
}

int umidade = buscaUmidade(pino_solo);
int temperatura = 30;

class Perfil {
public:
  String nome;
  int UmiMax;
  int UmiMin;

  Perfil() {}

  Perfil(String nome, int UmiMax, int UmiMin) {
    this->nome = nome;
    this->UmiMax = UmiMax;
    this->UmiMin = UmiMin;
  }
};

Perfil perfis[10];
int numPerfis = 0;

String geradorDePerfil(Perfil p) {
  String perfilGenerico = R"rawliteral(
    <div>
      <p><strong>Perfil:</strong> )rawliteral"
                          + p.nome + R"rawliteral(</p>
      <p><strong>Umidade Maxima:</strong> )rawliteral"
                          + String(p.UmiMax) + R"rawliteral(%)</p>
      <p><strong>Umidade Minima:</strong> )rawliteral"
                          + String(p.UmiMin) + R"rawliteral(%)</p>
    </div>
  )rawliteral";

  return perfilGenerico;
}


String checaMin() {
  String alerta = "";

  for (int i = 0; i < numPerfis; i++) {
    if (umidade < perfis[i].UmiMin) {
      alerta += "Alerta: Umidade abaixo do minimo para o perfil '" + perfis[i].nome + "'!<br>";
    }
  }
  if (numPerfis == 0) {
    alerta += "Nao ha perfis para verificar";
  }

  if (alerta == "") {
    alerta = "Todos os perfis estao dentro do limite minimo de umidade.<br>";
  }
  return alerta;
}

void ligaIrrigacao() {
  bool precisaIrrigar = false;

  for (int i = 0; i < numPerfis; i++) {
    if (umidade < perfis[i].UmiMin) {
      precisaIrrigar = true;
      break;
    }
  }

  if (precisaIrrigar) {
    digitalWrite(LED_PIN, HIGH); 
  } else {
    digitalWrite(LED_PIN, LOW); 
  }
}

void handleRoot() {
  String umidadeStr = String(umidade) + "%";
  String temperaturaStr = String(temperatura) + "°C";
  String alerta = checaMin();
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
      <meta http-equiv='refresh' content='5'>
        <style>
          body { font-family: Arial, sans-serif; text-align: center; margin: 50px; }
          h1 { color: #333; }
          p { font-size: 18px; }
          .perfis { display: flex; flex-direction: row; justify-content: center; align-items: center; gap: 4px; padding: 10px}
        </style>
      </head>
    <body>
      <h1>Monitor de Ambiente</h1>
      <div class="perfis">
  )rawliteral";

  for (int i = 0; i < numPerfis; i++) {
    html += geradorDePerfil(perfis[i]);
  }

  html += R"rawliteral(
      </div>
      <p>Umidade atual: )rawliteral"
          + umidadeStr + R"rawliteral(</p>
      <p>Temperatura atual: )rawliteral"
          + temperaturaStr + R"rawliteral(</p>
      <div style="color: red; font-weight: bold;">)rawliteral" + alerta + R"rawliteral(</div>
      <a href="/adicionarPerfil">Adicionar Perfil</a> | <a href="/removerPerfil">Remover Perfil</a>
    </body>
    </html> )rawliteral";

  server.send(200, "text/html", html);
}

void handleAdicionarPerfil() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Adicionar Perfil</title>
      </head>
      <body>
        <h1>Adicionar Perfil</h1>
        <form action="/adicionarPerfil" method="POST">
          <label for="nome">Nome:</label><br>
          <input type="text" id="nome" name="nome"><br>
          <label for="UmiMax">Umidade Máxima:</label><br>
          <input type="number" id="UmiMax" name="UmiMax"><br>
          <label for="UmiMin">Umidade Mínima:</label><br>
          <input type="number" id="UmiMin" name="UmiMin"><br><br>
          <input type="submit" value="Adicionar">
        </form>
      </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleAdicionarPerfilPost() {
  if (server.hasArg("nome") && server.hasArg("UmiMax") && server.hasArg("UmiMin")) {
    if (numPerfis < 10) {
      String nome = server.arg("nome");
      int UmiMax = server.arg("UmiMax").toInt();
      int UmiMin = server.arg("UmiMin").toInt();
      perfis[numPerfis++] = Perfil(nome, UmiMax, UmiMin);
      server.send(200, "text/html", "<p>Perfil adicionado com sucesso! <a href='/'>Voltar</a></p>");
    } else {
      server.send(200, "text/html", "<p>Limite de perfis atingido! <a href='/'>Voltar</a></p>");
    }
  } else {
    server.send(400, "text/html", "<p>Dados inválidos! <a href='/adicionarPerfil'>Tentar novamente</a></p>");
  }
}

void handleRemoverPerfil() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Remover Perfil</title>
      </head>
      <body>
        <h1>Remover Perfil</h1>
        <form action="/removerPerfil" method="POST">
          <label for="nome">Nome do perfil para remover:</label><br>
          <input type="text" id="nome" name="nome"><br><br>
          <input type="submit" value="Remover">
        </form>
      </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleRemoverPerfilPost() {
  if (server.hasArg("nome")) {
    String nome = server.arg("nome");
    bool encontrado = false;

    for (int i = 0; i < numPerfis; i++) {
      if (perfis[i].nome == nome) {
        encontrado = true;
        for (int j = i; j < numPerfis - 1; j++) {
          perfis[j] = perfis[j + 1];
        }
        numPerfis--;
        break;
      }
    }

    if (encontrado) {
      server.send(200, "text/html", "<p>Perfil removido com sucesso! <a href='/'>Voltar</a></p>");
    } else {
      server.send(200, "text/html", "<p>Perfil não encontrado! <a href='/removerPerfil'>Tentar novamente</a></p>");
    }
  } else {
    server.send(400, "text/html", "<p>Nome inválido! <a href='/removerPerfil'>Tentar novamente</a></p>");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando-se à rede Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("Servidor mDNS iniciado com sucesso!");
  }

  server.on("/", handleRoot);
  server.on("/adicionarPerfil", HTTP_GET, handleAdicionarPerfil);
  server.on("/adicionarPerfil", HTTP_POST, handleAdicionarPerfilPost);
  server.on("/removerPerfil", HTTP_GET, handleRemoverPerfil);
  server.on("/removerPerfil", HTTP_POST, handleRemoverPerfilPost);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Página não encontrada");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}
