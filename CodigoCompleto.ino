// ================== LIBRERÍAS ==================
// Librería para controlar el servomotor
#include <Servo.h>

// Librería para el sensor DHT11 (temperatura y humedad)
#include <DHT.h>

// Librería para comunicación serial con el ESP8266
#include <SoftwareSerial.h>

// ================== ESP8266 ==================
// Pines RX y TX para el módulo ESP8266
SoftwareSerial esp8266(6, 7); 

// Credenciales de la red WiFi
String ssid = "CAMPUS_EPN";
String password = "politecnica**";

// Datos necesarios para ThingSpeak
String apiKey = "OGU7QI6TPNMCSDR8";
String host = "api.thingspeak.com";

// ================== SENSOR DHT11 ==================
// Pin donde está conectado el DHT11
#define DHTPIN 2
#define DHTTYPE DHT11

// Se crea el objeto del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// ================== SENSORES ANALÓGICOS ==================
// Sensor de gas/humo MQ-2
#define MQ2 A0

// Sensor de lluvia
#define LLUVIA A1

// Sensor de luz (LDR)
#define LDR A2

// ================== ACTUADORES ==================
// Pin del servomotor
#define SERVO_PIN 8

// Pines de los LEDs
#define LED1 3
#define LED2 4

// Objeto del servomotor
Servo motor;

// ================== VELOCIDADES SERVO 360 ==================
// Valores que controlan la velocidad del servo
int VELOCIDAD_DETENIDO = 90;
int VELOCIDAD_LENTA   = 120;
int VELOCIDAD_NORMAL  = 125;
int VELOCIDAD_RAPIDA  = 180;

// ================== LECTURA ESTABLE MQ-2 ==================
// Función que promedia varias lecturas del MQ-2
// para evitar errores o lecturas inestables
int leerMQ2() {
  int suma = 0;

  // Se toman 10 lecturas del sensor
  for (int i = 0; i < 10; i++) {
    suma += analogRead(MQ2);
    delay(20);
  }

  // Se devuelve el promedio de las lecturas
  return suma / 10;
}

// ================== SETUP ==================
void setup() {
  // Comunicación con el monitor serial
  Serial.begin(9600);

  // Comunicación con el ESP8266
  esp8266.begin(9600);

  // Inicialización del sensor DHT11
  dht.begin();

  // Se conecta el servomotor al pin indicado
  motor.attach(SERVO_PIN);

  // El servo inicia en velocidad normal
  motor.write(VELOCIDAD_NORMAL);

  // Configuración de los LEDs como salidas
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  delay(2000);
  Serial.println("🚠 Iniciando Teleférico...");

  // Tiempo necesario para que el MQ-2 funcione correctamente
  Serial.println("🔥 Calentando sensor MQ-2...");
  delay(30000);
  Serial.println("MQ-2 listo");

  // Se realiza la conexión a la red WiFi
  connectWiFi();
}

// ================== LOOP PRINCIPAL ==================
void loop() {

  // Lectura de temperatura desde el DHT11
  float temperatura = dht.readTemperature();

  // Lectura de humedad desde el DHT11
  float humedad = dht.readHumidity();

  // Lectura del sensor MQ-2 usando promedio
  int humo = leerMQ2();

  // Lectura del sensor de lluvia
  int lluvia = analogRead(LLUVIA);

  // Lectura del sensor de luz
  int luz = analogRead(LDR);

  // Verificación de error en el DHT11
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("❌ Error leyendo DHT11");
    delay(2000);
    return;
  }

  // ================== MOSTRAR DATOS ==================
  // Se muestran todos los valores por el monitor serial
  Serial.println("\n=== Lectura de Sensores ===");

  // Muestra la temperatura
  Serial.print("🌡 Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  // Muestra la humedad
  Serial.print("💧 Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  // Muestra el nivel de humo
  Serial.print("💨 Humo: ");
  Serial.println(humo);

  // Muestra el valor del sensor de lluvia
  Serial.print("🌧 Lluvia: ");
  Serial.println(lluvia);

  // Muestra el nivel de luz
  Serial.print("💡 Luz: ");
  Serial.println(luz);

  // ================== CONTROL DE LEDs ==================
  // Si hay poca luz, se encienden los LEDs
  if (luz <= 900) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    Serial.println("LEDs: ENCENDIDOS");
  } 
  // Si hay suficiente luz, se apagan
  else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    Serial.println("LEDs: APAGADOS");
  }

  // ================== CONTROL DEL SERVOMOTOR ==================
  // Si la temperatura es baja y la humedad es baja
  // el sistema se detiene
  if (temperatura < 20 && humedad < 80) {
    motor.write(VELOCIDAD_DETENIDO);
    Serial.println("MODO: DETENIDO");
  }
  // Si hay mucho humo, el servo va rápido
  else if (humo > 120) {
    motor.write(VELOCIDAD_RAPIDA);
    Serial.println("MODO: VELOCIDAD RÁPIDA");
  }
  // Si se detecta lluvia, el servo va lento
  else if (lluvia < 230) {
    motor.write(VELOCIDAD_LENTA);
    Serial.println("MODO: VELOCIDAD LENTA");
  }
  // En condiciones normales
  else {
    motor.write(VELOCIDAD_NORMAL);
    Serial.println("MODO: VELOCIDAD NORMAL");
  }

  // ================== ENVÍO DE DATOS ==================
  // Se envían los datos a ThingSpeak
  enviarDatos(temperatura, humedad, humo, lluvia, luz);

  // Tiempo de espera antes de repetir el ciclo
  delay(25000);
}

// ================== CONEXIÓN WIFI ==================
void connectWiFi() {
  Serial.println("Configurando ESP8266...");

  // Reinicia el módulo
  sendCommand("AT+RST", 2000);

  // Modo estación
  sendCommand("AT+CWMODE=1", 1000);

  // Una sola conexión
  sendCommand("AT+CIPMUX=0", 1000);

  // Conexión a la red WiFi
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  sendCommand(cmd, 15000);

  Serial.println("WiFi conectado");
}

// ================== ENVÍO DE DATOS ==================
void enviarDatos(float temperatura, float humedad, int humo, int lluvia, int luz) {
  Serial.println("\n📡 Enviando datos a ThingSpeak...");

  // Cierra conexiones anteriores
  sendCommand("AT+CIPCLOSE", 500);

  // Abre conexión con ThingSpeak
  String cmd = "AT+CIPSTART=\"TCP\",\"" + host + "\",80";
  sendCommand(cmd, 4000);

  // Construcción del mensaje GET
  String datos = "GET /update?api_key=" + apiKey +
                 "&field1=" + String(temperatura) +
                 "&field2=" + String(humedad) +
                 "&field3=" + String(humo) +
                 "&field4=" + String(luz) +
                 "&field5=" + String(lluvia) + "\r\n\r\n";

  // Indica cuántos datos se van a enviar
  cmd = "AT+CIPSEND=" + String(datos.length());
  sendCommand(cmd, 2000);

  // Envío de los datos
  esp8266.print(datos);

  delay(2000);
  Serial.println("Datos enviados correctamente");
}

// ================== COMANDOS AT ==================
// Función que envía comandos AT al ESP8266
String sendCommand(String cmd, int timeout) {
  String response = "";

  // Limpia el buffer de comunicación
  while (esp8266.available()) {
    esp8266.read();
  }

  // Envía el comando
  esp8266.println(cmd);

  // Espera la respuesta
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (esp8266.available()) {
      response += char(esp8266.read());
    }
  }

  // Muestra el comando y la respuesta
  Serial.print("CMD: ");
  Serial.println(cmd);
  Serial.print("RSP: ");
  Serial.println(response);

  return response;
}
