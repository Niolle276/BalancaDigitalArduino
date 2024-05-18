#include <WiFi.h>


#include <Servo.h>
#include <ESP32Servo.h>
#include "HX711.h"
#include <PubSubClient.h>

#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
#define DOUT  D4                      // HX711 DATA OUT = pino D3 do NodeMCU
#define CLK   D2
#define MIN_MICROS      800  //544 
#define MAX_MICROS      2450 

HX711 balanca;
#define SERVO D5 // Porta Digital 5 PWM
Servo s;

//Servo s; // Variável Servo
int pos = 0; // Posição Servo
float calibration_factor = 48000;

// WIFI
const char* ssid = "Aridu";
const char* password =  "22028262";
const char* mqttServer = "broker.mqttdashboard.com";
const int mqttPort = 1883;
const char* mqttUser = "dispensamedicamentos";
const char* mqttPassword = "123456";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  s.attach(SERVO);
  Serial.begin(9600);
  s.write(0); // Inicia motor posição zero
  //ISR_Servo.setupServo(D5, MIN_MICROS, MAX_MICROS); 
  balanca.begin(DOUT, CLK);                          // inicializa a balança
  Serial.println("Balança com HX711 - célula de carga 50 Kg");
  balanca.set_scale(calibration_factor);             // ajusta fator de calibração
  balanca.tare();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Iniciando conexão com a rede WiFi..");
  }
  Serial.println("Conectado");
}

void loop() {
  balanca.set_scale(calibration_factor);   // a balança está em função do fator de calibração
  int peso = 0;
     // Printa "Peso:" na COM
  
  peso = balanca.get_units();    // imprime peso
  if (peso <= 0) {    // se a unidade for menor que 0 será considerado 0
    peso = 0.00;
    //calibration_factor += 1;    // Para o caso do peso ser negativo, o valor apresentado será 0
  }else{
    Serial.print("Peso: "); 
    Serial.print(peso);    // Printa o peso na serial
    Serial.print(" kg");    // Printa "kg" na serial
    Serial.print(" Fator de calibração: ");    // Printa "Fator de calibração:" na serial
    Serial.print(calibration_factor);    // Printa o fator de calibração na serial
    Serial.println();
  }

  if (peso < 200) {
    for (pos = pos; pos < 180; pos++) {
      s.write(pos);
      //ISR_Servo.setPosition(0, pos);
      delay(15);
    }
    // reconectabroker();

    // //Envia a mensagem ao broker
    // bool publish = client.publish("DispensaMedicamentosNiolle", "registrar hora.");
    // if(publish){
    //   Serial.println("Mensagem enviada com sucesso...");
    // }else{
    //   Serial.println("Falha ao enviar a mensagem...");
    // }
    
  } else {
    for (pos = 90; pos >= 0; pos--) {
      s.write(pos);
      //ISR_Servo.setPosition(servoIndex1, pos);
      delay(15);
    }
  }
  delay(1000);
  if(peso != 215){
    calibration_factor += 1;
  }    // incrementa 1 no fator de calibração
}

void reconectabroker() {
  //Conexao ao broker MQTT
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("Conectado ao broker!");
    } else {
      Serial.print("Falha na conexão ao broker - Estado: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}