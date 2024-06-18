#include <Stepper.h>
const int stepsRev=2048;
const int web=2;
const int potPin = A0; //Pin del potenciometro.
int lastDisplayValue = -1; //Variable para almacenar el ultimo valor mostrado.

Stepper Stepper(stepsRev, 8, 9, 10, 11);

void setup() {
  Serial.begin(9600);
  Stepper.setSpeed(5); //Fijar la velocidad del motor.
}

void loop() {

  int senVals = 0;
  Serial.println("Mueve el potenciometro...");
  delay(5000);

  for(int i = 0; i < 5; i++)
  {
    senVals += analogRead(potPin);
    delay(10);
  }

  int senVal = senVals / 5; //Leer valor del potenciometro
  int mapVal = map(senVal, 0, 1023, 100, 0); //Mapear valor del potenciometro a un rango de 1 a 100.

  if (mapVal != lastDisplayValue) {

    if(mapVal < 4)
      mapVal = 0;

    if(mapVal > 97)
      mapVal = 100;

    Serial.println(mapVal);

    const int steps= mapVal / 100.0 * stepsRev * web; //Con el if solo actualizamos la pantalla si el valor a cambiado.
    lastDisplayValue = mapVal; // Actualizar el último valor mostrado.
    Serial.println(steps);
    Stepper.step(steps);
    }
}