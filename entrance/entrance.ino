/*
  AnalogReadSerial

  Reads an analog input on pin 0, prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogReadSerial
*/

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

/*
int currentLaserValue = 0;
int lastLaserValue = 0;
int lastDistanceValue = -1;
int currentDistanceValue = -1;
int counter = 0;
int laserWasCut = 0;
int distanceFlag = 0;
*/
#define DELAY 1500 //need to adjust

bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;
unsigned long _timestamp;


void checkIfWalkIn()
{
  if (firstIsCut != firstWasCut)
  {
    firstWasCut = firstIsCut;

    if(walkIn == false && firstIsCut == true)
    {
      walkIn = true;
      _timestamp = millis();
    }
  }

  if(millis() - _timestamp > DELAY)
  {
    walkIn = false;
  }

  if(walkIn && secondIsCut == true && firstIsCut == false)
  {
    walkIn = false;
    counter++;
  }
}

void checkIfWalkOut()
{
  if (secondIsCut != secondWasCut)
  {
    secondWasCut = secondIsCut;

    if(walkOut == false && secondIsCut == true)
    {
      walkOut = true;
      _timestamp = millis();
    }
  }

  if(millis() - _timestamp > DELAY)
  {
    walkOut = false;
  }

  if(walkOut && secondIsCut == false && firstIsCut == true)
  {
    walkIn = false;
    counter--;
  }
}


void loop() {
  int firstLaserValue = analogRead(A0);
  int secondLaserValue = analogRead(A1);
  if(firstLaserValue > 100)
  {
    firstIsCut = true;
  }
    if(secondLaserValue > 100)
  {
    secondIsCut = true;
  }

  checkIfWalkIn();
  checkIfWalkOut();


  Serial.println("people in:");
  Serial.println(counter);
  //delay(25);
}

/*
// the loop routine runs over and over again forever:
void loop() {
  lastLaserValue = currentLaserValue;
  currentLaserValue = analogRead(A0);
  int laserValue = currentLaserValue - lastLaserValue;
  if(laserValue > 150)
  {
    laserWasCut = 1;
    if (distanceFlag > 0) {
      //Somebody entered
      counter++;
      distanceFlag = 0;
      laserWasCut = 0;
    } else {
      laserWasCut = 1;
    }
  }
  else
  {
      if(laserWasCut > 0){
        laserWasCut -= 1;
      }
  }
  int distanceValue = analogRead(A1);
  lastDistanceValue = currentDistanceValue;
  currentDistanceValue = distanceValue;
  int diff = lastDistanceValue - currentDistanceValue;
  if (laserWasCut > 0)
  {
    if (diff < 50) counter++;
    else counter--;
  }
  if (diff < - 50 || diff > 50) 
  {
      if (laserWasCut > 0) {
        //Somebody exited
        counter--;
        distanceFlag = 0;
        laserWasCut = 0;
      } else {
        distanceFlag = 5;
      }
  }
  else if (distanceFlag > 0){
    distanceFlag = 0;
  }
  Serial.println("people in:");
  Serial.println(counter);
  
  delay(100);        // delay in between reads for stability
}
*/
