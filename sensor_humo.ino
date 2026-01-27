int Pin = A0;     
int ledPin = 13;    // led de alerta
int valorHumo = 0;  
int umbral = 300;   

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  valorHumo = analogRead(Pin);

  Serial.print("Nivel de humo: ");
  Serial.println(valorHumo);

  if (valorHumo > umbral) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Humo detectado");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("Aire limpio");
  }

  delay(1000);
}
