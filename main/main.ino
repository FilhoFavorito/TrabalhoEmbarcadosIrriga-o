#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <iostream> 
#include <string> 

// Configurações da rede Wi-Fi
const char* ssid = "A15 de João Pedro";
const char* password = "opala.10";

WebServer server(80);

int umidade = 50;
int temperatura = 50;

class Perfil { 
  public:
    String nome; 
    int UmiMax; 
    int UmiMin;
}; 

Perfil alface("alface", 90, 60);
Perfil morango("morango", 30, 70); 

Perfil perfis[] = {alface, morango};

String geradorDePerfil(Perfil p){
  String nome = p.nome;
  String umiMaxStr = isnan(p.UmiMax) ? "Erro na leitura" : String(p.UmiMax) + "%";
  String umiMinStr = isnan(p.UmiMin) ? "Erro na leitura" : String(p.UmiMin) + "%";

  String perfilGenerico = R"rawliteral(
    <div>
      <p><strong>Perfil:</strong> )rawliteral" +  nome + R"rawliteral(</p>
      <p><strong>Umidade Máxima:</strong> )rawliteral" + umiMaxStr + R"rawliteral(</p>
      <p><strong>Umidade Minima:</strong> )rawliteral" + umiMinStr + R"rawliteral(</p>
    </div>
  )rawliteral";

  return perfilGenerico;
}

void handleRoot() {
  String umidadeStr = isnan(umidade) ? "Erro na leitura" : String(umidade) + "%";
  String temperaturaStr = isnan(temperatura) ? "Erro na leitura" : String(temperatura) + "°C";
  String html= R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
      <meta http-equiv='refresh' content='5'>
        <style>
          body { font-family: Arial, sans-serif; text-align: center; margin: 50px; }
          h1 { color: #333; }
          p { font-size: 18px; }
          .led { margin: 20px; }
        </style>
      </head>
    <body>

      <h1>Monitor de Ambiente</h1>)rawliteral";
    
  int arraySize = sizeof(perfis) / sizeof(perfis[0]);
  for( int i=0; i < arraySize; i++){
    html+= geradorDePerfil(perfis[i]);
  }

  html+= R"rawliteral(
      <div class="led">
        <a href="/ligar"><button>🔴 Ligar LED</button></a>
        <a href="/desligar"><button>⚪ Desligar LED</button></a>
      </div>
    </body>
    </html> )rawliteral";
  
  server.send(200, "text/html", html);
}
// Resposta para rotas não encontradas
void handleNotFound() {
  String message = "Página não encontrada\n\n";
  message += "URI: " + server.uri() + "\n";
  server.send(404, "text/plain", message);
}
void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);
  // Conexão com Wi-Fi
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
// Configuração do mDNS
  if (MDNS.begin("esp32")) {
    Serial.println("Servidor mDNS iniciado com sucesso!");
  }
// Rotas do servidor
  server.on("/", handleRoot);
  
//  server.on("/desligar", []() {
 //   desligarLed();
 //   server.send(200, "text/plain", "LED desligado");
 // });

  server.onNotFound(handleNotFound);
  // Inicia o servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}
void loop() {
  // Processa solicitações do cliente
  server.handleClient();
  delay(10);
}