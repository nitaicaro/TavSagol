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

int firstWasCut = 0;
int secondWasCut = 0;
bool walkIn = false;
bool walkOut = false;
int counter = 0;

void loop() {
  int firstLaserValue = analogRead(A0);
  int secondLaserValue = analogRead(A1);
  //Check walk in
  if (firstLaserValue > 100) {
    firstWasCut = 50;
    if (secondWasCut && !(firstLaserValue > 100 && secondLaserValue > 100)) {
      counter--;
      firstWasCut = 0;
      secondWasCut = 0;
    }
  } else {
    if (firstWasCut > 0) {
      firstWasCut--;
    }
  }
  if (secondLaserValue > 100) {
    secondWasCut = 50;
    if (firstWasCut && !(firstLaserValue > 100 && secondLaserValue > 100)) {
      counter++;
      firstWasCut = 0;
      secondWasCut = 0;
    }
  } else {
    if (secondWasCut > 0) {
      secondWasCut--;
    }
  }

  Serial.println("people in:");
  Serial.println(counter);
  delay(25);
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
