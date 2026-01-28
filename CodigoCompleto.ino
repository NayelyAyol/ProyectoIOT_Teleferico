#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

SoftwareSerial esp8266(6, 7); 

String ssid = "CAMPUS_EPN";
String password = "politecnica**";
String apiKey = "OGU7QI6TPNMCSDR8";
String host = "api.thingspeak.com";

// -------- DHT --------
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------- SENSORES --------
#define MQ2 A0
#define LLUVIA A1
#define LDR A2

// -------- ACTUADORES --------
#define SERVO_PIN 8
#define LED1 3
#define LED2 4

Servo motor;

// -------- VELOCIDADES SERVO 360 --------
int VELOCIDAD_DETENIDO = 90;
int VELOCIDAD_LENTA   = 120;
int VELOCIDAD_NORMAL  = 125;
int VELOCIDAD_RAPIDA  = 180;

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600); // 

  dht.begin();

  // -------- SERVO Y LEDS --------
  motor.attach(SERVO_PIN);
  motor.write(VELOCIDAD_NORMAL);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  delay(2000);
  Serial.println("🚠 Iniciando Teleférico...");

  // -------- CONEXIÓN WIFI --------
  connectWiFi();
}

void loop() {
  // -------- LECTURA DE SENSORES --------
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  int humo = analogRead(MQ2);
  int lluvia = analogRead(LLUVIA);
  int luz = analogRead(LDR);

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("❌ Error leyendo DHT11");
    delay(2000);
    return;
  }

  // -------- MONITOR SERIAL --------
  Serial.println("\n=== Lectura de Sensores ===");
  Serial.print("🌡 Temperatura: "); 
  Serial.print(temperatura); 
  Serial.println(" °C");
  Serial.print("💧 Humedad: "); 
  Serial.print(humedad); 
  Serial.println(" %");
  Serial.print("💨 Humo (MQ2): "); 
  Serial.println(humo);
  Serial.print("🌧 Lluvia: "); 
  Serial.println(lluvia);
  Serial.print("💡 Luz (LDR): "); 
  Serial.println(luz);

  // -------- CONTROL DE LUCES --------
  if (luz <= 900) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    Serial.println("LEDs: ENCENDIDOS");
  } else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    Serial.println("LEDs: APAGADOS");
  }

  // -------- CONTROL SERVO --------
  if (temperatura < 20 && humedad < 80) {
    motor.write(VELOCIDAD_DETENIDO);
    Serial.println("MODO: DETENIDO (condiciones extremas)");
  }
  else if (humo > 120) {
    motor.write(VELOCIDAD_RAPIDA);
    Serial.println("MODO: VELOCIDAD RÁPIDA (humo detectado)");
  }
  else if (lluvia < 230) {
    motor.write(VELOCIDAD_LENTA);
    Serial.println("MODO: VELOCIDAD LENTA (lluvia detectada)");
  }
  else {
    motor.write(VELOCIDAD_NORMAL);
    Serial.println("MODO: VELOCIDAD NORMAL");
  }

  // -------- ENVIAR A THINGSPEAK --------
  enviarDatos(temperatura, humedad, humo, lluvia, luz);

  delay(25000);
}

// -------- CONEXIÓN WIFI --------
void connectWiFi() {
  Serial.println("Configurando ESP8266...");
  
  sendCommand("AT+RST", 2000);
  delay(2000);
  
  sendCommand("AT+CWMODE=1", 1000);
  sendCommand("AT+CIPMUX=0", 1000); 

  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  sendCommand(cmd, 15000);

  Serial.println("WiFi conectado");
}

// -------- ENVIAR DATOS --------
void enviarDatos(float temperatura, float humedad, int humo, int lluvia, int luz) {
  Serial.println("\n📡 Enviando datos a ThingSpeak...");

  sendCommand("AT+CIPCLOSE", 500);
  
  // 1. Abrir conexión
  String cmd = "AT+CIPSTART=\"TCP\",\"" + host + "\",80";
  sendCommand(cmd, 4000); 

  // 2. Preparar el mensaje
  String datos = "GET /update?api_key=" + apiKey + 
                 "&field1=" + String(temperatura) + 
                 "&field2=" + String(humedad) + 
                 "&field3=" + String(humo) + 
                 "&field4=" + String(luz) + 
                 "&field5=" + String(lluvia) + "\r\n\r\n";
  
  
  cmd = "AT+CIPSEND=" + String(datos.length());
  sendCommand(cmd, 2000); 

  esp8266.print(datos);
  
  delay(2000); 
  Serial.println("Proceso de envío finalizado");
}

// -------- COMANDOS AT --------
String sendCommand(String cmd, int timeout) {
  String response = "";
  
  // Limpiar buffer
  while (esp8266.available()) {
    esp8266.read();
  }
  
  esp8266.println(cmd);

  long int time = millis();
  while ((time + timeout) > millis()) {
    while (esp8266.available()) {
      response += char(esp8266.read());
    }
  }

  Serial.print("CMD: ");
  Serial.println(cmd);
  Serial.print("RSP: ");
  Serial.println(response);
  
  return response;
}