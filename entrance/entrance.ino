#include <SoftwareSerial.h>
#include <Adafruit_MLX90614.h>


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
//temp
void readAndSendTemp();



//inputs
#define FIRST_LASER A0
#define SECOND_LASER A1

//outputs
#define RED 4
#define GREEN 5
#define BUZZER 10

//counting globals
bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;
bool isFull = false;

#define LIGHT_THRESHOLD 100

//communication constants - replace with enum? check if it is available for arduino. move to shared .h? need to see if it is possible
#define ENTRANCE_MESSAGE 2
#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0

//temperature
Adafruit_MLX90614 temperature_sensor = Adafruit_MLX90614();
#define TEMP_MESSAGE 3
#define TEMP_CLOCK A5
#define TEMP_DATA A4

void setup()
{
  // the setup routine runs once when you press reset:
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  //set pins for RGB LED
  pinMode(RED,  OUTPUT);
  pinMode(GREEN,  OUTPUT);
  pinMode(BUZZER, OUTPUT);
  //Init the serial communications with the ESP8266
  ArduinoUno.begin(4800);
  temperature_sensor.begin();
  //Serial.println("Good Morning from Arduino Uno");
}

void loop() 
{
  tone(BUZZER, 360, 2000);
  checkAndHandleResponseFromEsp();
  countPeople();
  readAndSendTemp();
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
    if (isFull) 
    {
        tone(BUZZER, 440, 2000);
    }
    notifyEsp(ENTRANCE_MESSAGE, ENTER);
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
    notifyEsp(ENTRANCE_MESSAGE, EXIT);
    Serial.println(counter);
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
        Serial.println("Got OK from ESP");
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, LOW);
        isFull = false;
      }
      else if(val == FULL)
      {
        Serial.println("Got FULL from ESP");
        tone(BUZZER, 440, 2000);
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
        isFull = true;
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
void notifyEsp(int messageType, int value)
{
    ArduinoUno.print(messageType);
    ArduinoUno.print('=');
    ArduinoUno.print(value);
    ArduinoUno.println("\n");
}

//temp
void readAndSendTemp()
{
  //Serial.print("Object: ");
  int object = temperature_sensor.readObjectTempC();
  //Serial.println(temperature_sensor.readObjectTempC());
  //Serial.print("Ambient: ");
  int ambient = temperature_sensor.readAmbientTempC();
  //Serial.println(ambient);
  if(object > 25 && object < 45)
  {
    if(object > 30)
    {
          notifyEsp(TEMP_MESSAGE, object);
          if (object >= 38) tone(BUZZER, 30, 200);
    }
    else
    {
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, HIGH);
        delay(500);
        digitalWrite(RED, LOW);
        digitalWrite(GREEN, LOW);
    }
  }

}
