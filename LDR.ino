int ldr = A0;
int led = 8;

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int valorLuz = analogRead(ldr);
  Serial.println(valorLuz);

  if (valorLuz < 50) {
    digitalWrite(led, HIGH); 
  } else {
    digitalWrite(led, LOW); 
  }

  delay(500);
}
