#include <SoftwareSerial.h>

//communication globals
SoftwareSerial ArduinoUno(3,2);

//counting globals
bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;
void countPeople();
#define MAX 5

//inputs
#define FIRST_LASER A0
#define SECOND_LASER A1

//outputs
#define RED 4
#define GREEN 5

void checkIfWalkIn()
{
  if (firstIsCut != firstWasCut)
  {
    firstWasCut = firstIsCut;

    if(!walkIn && firstIsCut)
    {
      walkIn = true;
    }
  }

  if(walkIn && secondIsCut && !firstIsCut)
  {
    walkIn = false;
    counter++;
    ArduinoUno.print(1);
    ArduinoUno.println("\n");
    Serial.println(counter);
  }
}

void checkIfWalkOut()
{
  if (secondIsCut != secondWasCut)
  {
    secondWasCut = secondIsCut;

    if(!walkOut && secondIsCut)
    {
      walkOut = true;
    }
  }

  
  if(walkOut && !secondIsCut && firstIsCut)
  {
    walkOut = false;
    counter--;
    ArduinoUno.print(0);
    ArduinoUno.println("\n");
    Serial.println(counter);
  }
}


void setup()
{
  // the setup routine runs once when you press reset:
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(RED,  OUTPUT);
  pinMode(GREEN,  OUTPUT);
  ArduinoUno.begin(4800);
  Serial.println("Good Morning from Arduino Uno");
}

void loop() 
{
  while(ArduinoUno.available() > 0)
  {
    int val = ArduinoUno.parseInt();
    if(ArduinoUno.read()== '\n')
    {
      Serial.println(val);
      if(val == 1)
      {
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, HIGH);
      }
      else
      {
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
      }
   }
  }

  countPeople(); 
  delay(10);

}

void countPeople()
{
  int firstLaserValue = analogRead(FIRST_LASER);
  int secondLaserValue = analogRead(SECOND_LASER);
  if(firstLaserValue > 100)
  {
    firstIsCut = true;
  }
  else
  {
    firstIsCut = false;
  }
  if(secondLaserValue > 100)
  {
    secondIsCut = true;
  }
  else
  {
    secondIsCut = false;
  }

  if (!firstIsCut && !secondIsCut)
  {
    firstWasCut = false;
    secondWasCut = false;
    walkIn = false;
    walkOut = false;
  }

  checkIfWalkIn();
  checkIfWalkOut();

}
