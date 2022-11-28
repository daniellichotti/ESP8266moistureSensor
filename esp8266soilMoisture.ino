#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
//Projeto baseado em https://www.youtube.com/watch?v=UkIt1wvq-Tk
//https://labdegaragem.com/forum/topics/high-e-low-no-esp8266
//Se o LED integrado na placa, estiver em current  a lógica de acionamento é invertida, mas se for source é logica direta (normalmente usada).?
#define HI LOW
#define LO HIGH

//#define relay 5  //GP2 ESP-01




//WiFi
const char* SSID = "Gladiator";                // SSID / nome da rede WiFi que deseja se conectar
const char* PASSWORD = "fab4954def";           // Senha da rede WiFi que deseja se conectar
WiFiClient wifiClient;                        
 
//MQTT Server
const char* BROKER_MQTT = "mqtt.eclipseprojects.io";  //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                               // Porta do Broker MQTT

//sensor umidade solo
int sense_Pin = 0; // sensor input at Analog pin A0
int value = 0;

//relay
const int relay = 5;

#define ID_MQTT  "WPSN9999"                           //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 
#define TOPIC_SUBSCRIBE "WPSN1111"                    //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
#define TOPIC_PUBLISH "WPSN0000"                      //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
PubSubClient MQTT(wifiClient);                        // Instancia o Cliente MQTT passando o objeto espClient

//Declaração das Funções
void mantemConexoes();                                //Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();                                   //Faz conexão com WiFi
void conectaMQTT();                                   //Faz conexão com Broker MQTT
void recebePacote(char* topic, byte* payload, unsigned int length);

void setup() {

  pinMode(relay, OUTPUT);

  Serial.begin(115200);
  

  conectaWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
  MQTT.setCallback(recebePacote); 
}

void loop() {
  mantemConexoes();
  MQTT.loop();
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }
    
    conectaWiFi();                                    //se não há conexão com o WiFI, a conexão é refeita
}

void conectaWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
     return;
  }
        
  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI  
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);  
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP()); 
}

void conectaMQTT() { 
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado ao Broker com sucesso!");
            MQTT.subscribe(TOPIC_SUBSCRIBE);
        } 
        else {
            Serial.println("Noo foi possivel se conectar ao broker.");
            Serial.println("Nova tentatica de conexao em 10s");
            delay(10000);
        }
    }
}

void recebePacote(char* topic, byte* payload, unsigned int length) 
{
    String msg;
    Serial.println(msg);

    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    
    if (msg == "0") {
        Serial.println("Desligado");
        MQTT.publish(TOPIC_PUBLISH, "Sem Leitura");
        digitalWrite(relay, LOW);
    }                     

    if (msg == "1") {
      Serial.println(analogRead(sense_Pin));
      Serial.println("Ligado");
      value =analogRead(sense_Pin);
      char str[8];
      MQTT.publish(TOPIC_PUBLISH, itoa( value, str, 10 ));
      /*
      if(analogRead(sense_Pin)>200){
        digitalWrite(relay, HIGH);
      }
      */
    }
}