/*
  AnalogReadSerial

  Reads an analog input on pin 0, prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogReadSerial
*/


/*
int currentLaserValue = 0;
int lastLaserValue = 0;
int lastDistanceValue = -1;
int currentDistanceValue = -1;
int counter = 0;
int laserWasCut = 0;
int distanceFlag = 0;
*/

bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;

#define MAX 5

//inputs
#define FIRST_LASER A0
#define SECOND_LASER A1

//outputs
#define RED 2
#define GREEN 4

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
}

void loop() {
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

  if(counter >= MAX)
  {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
  else
  {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
  }

  delay(10);
}
