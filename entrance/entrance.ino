#include <SoftwareSerial.h>

//communication globals
SoftwareSerial ArduinoUno(3,2);


//methods declarations
//counting
void checkIfWalkIn();
void checkIfWalkOut();
void countPeople();
//communication
void checkAndHandleResponseFromEsp();
void notifyEsp(int code);


//inputs
#define FIRST_LASER A0
#define SECOND_LASER A1

//outputs
#define RED 4
#define GREEN 5

//counting globals
bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;

#define LIGHT_THRESHOLD 100

//communication constants - replace with enum? check if it is available for arduino. move to shared .h? need to see if it is possible
#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0


void setup()
{
  // the setup routine runs once when you press reset:
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  //set pins for RGB LED
  pinMode(RED,  OUTPUT);
  pinMode(GREEN,  OUTPUT);
  //Init the serial communications with the ESP8266
  ArduinoUno.begin(4800);
  //Serial.println("Good Morning from Arduino Uno");
}

void loop() 
{
  checkAndHandleResponseFromEsp();
  countPeople(); 
  delay(10);
}


//implementations
//counting
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
    notifyEsp(ENTER);
    //Serial.println(counter);
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
    notifyEsp(EXIT);
    //Serial.println(counter);
  }
}

void countPeople()
{
  int firstLaserValue = analogRead(FIRST_LASER);
  int secondLaserValue = analogRead(SECOND_LASER);
  if(firstLaserValue > LIGHT_THRESHOLD)
  {
    firstIsCut = true;
  }
  else
  {
    firstIsCut = false;
  }
  if(secondLaserValue > LIGHT_THRESHOLD)
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

//communications
void checkAndHandleResponseFromEsp()
{
  while(ArduinoUno.available() > 0)
  {
    int val = ArduinoUno.parseInt();
    if(ArduinoUno.read()== '\n')
    {
      Serial.println(val);
      if(val == OK)
      {
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, HIGH);
      }
      else if(val == FULL)
      {
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
      }
      else //should now reach here
      {
        Serial.println("undefined message");
      }
   }
  }
}

//notifies the esp if someone enters/exists
//param code = 0 is for exit, 1 is for enter
void notifyEsp(int code)
{
    ArduinoUno.print(code);
    ArduinoUno.println("\n");
}
