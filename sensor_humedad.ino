#include <Servo.h>

Servo teleferico; // Servo que mueve polea

const int pinSensor = A0; // sensor de humedad / agua
const int pinServo = 9;

int valorSensor = 0;

// Ajustar valores según los datos recibidos por el sensor de humedad
int umbralLluvia = 500;   // menor = mojado

int velocidadNormal = 120;  // velocidad normal
int velocidadLluvia = 100;  // bajar velocidad por lluvia
int detenido = 90; // detener teleferico

void setup() {
  Serial.begin(9600);
  teleferico.attach(pinServo);
}

void loop() {
  valorSensor = analogRead(pinSensor);
  Serial.println(valorSensor);

  if (valorSensor < umbralLluvia) {
    // Está lloviendo
    teleferico.write(velocidadLluvia);
  } else {
    // No llueve
    teleferico.write(velocidadNormal);
  }

  delay(200);
}
