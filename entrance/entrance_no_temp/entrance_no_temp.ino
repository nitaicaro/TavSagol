#include <SoftwareSerial.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


//counting globals
bool firstWasCut = false;
bool firstIsCut = false;
bool secondWasCut = false;
bool secondIsCut = false;
bool walkIn = false;
bool walkOut = false;
int counter = 0;
bool isFull = false;
bool tempIsOk = false;
int tempDelay = 0;
#define LIGHT_THRESHOLD 110

//display
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int lcdCurrentMessage = -1;
#define MSG_FULL 1
#define MSG_ENTER 2

//communication globals
SoftwareSerial ArduinoUno(3,2);
#define INVALID_ENTRANCE 4
#define ENTRANCE_MESSAGE 2
#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0


//inputs
#define FIRST_LASER A0
#define SECOND_LASER A1

//outputs
#define RED 4
#define GREEN 5
#define BUZZER 12

//methods declarations
//counting
void checkIfWalkIn();
void handleWalkIn();
void checkIfWalkOut();
void countPeople();
//communication
void checkAndHandleResponseFromEsp();
void notifyEsp(int code);
//display
void updateDisplay(int messageCode);


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
  // initialize the lcd 
  lcd.init();                      
  lcd.backlight();
  lcd.clear();
  updateDisplay(MSG_ENTER);
}

void loop() 
{
  checkAndHandleResponseFromEsp();
  countPeople();
  delay(10);
}

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
        updateDisplay(MSG_ENTER);
      }
      else if(val == FULL)
      {
        Serial.println("Got FULL from ESP");
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
        isFull = true;
        updateDisplay(MSG_FULL);
      }
      else //should now reach here
      {
        Serial.println("undefined message");
      }
   }
  }
}


void handleWalkIn()
{
  walkIn = false;
  tempDelay = 0;
  counter++;
  if (isFull)
  {
    tone(BUZZER, 440, 2000);
    updateDisplay(MSG_FULL);
  }
  else
  {
    updateDisplay(MSG_ENTER);
  }
  notifyEsp(ENTRANCE_MESSAGE, ENTER);
  Serial.println(counter);
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
    handleWalkIn();
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
  //Serial.println(firstLaserValue);
  int secondLaserValue = analogRead(SECOND_LASER);
  //Serial.println(secondLaserValue);

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

//notifies the esp if someone enters/exists
//param code = 0 is for exit, 1 is for enter
void notifyEsp(int messageType, int value)
{
    ArduinoUno.print(messageType);
    if(value < 0)
    {
      ArduinoUno.println('=');
      return;
    }
    ArduinoUno.print('=');
    ArduinoUno.print(value);
    ArduinoUno.println("\n");
}

void updateDisplay(int messageCode)
{
  if(messageCode == lcdCurrentMessage) return;
  lcd.clear();
  lcdCurrentMessage = messageCode;
  switch(messageCode)
  {
    case MSG_FULL:
        lcd.setCursor(0,0);
        lcd.print("Room is full");
        lcd.setCursor(0,1);
        lcd.print("Please wait.");
        break;
    case MSG_ENTER:
        lcd.setCursor(0,0);
        lcd.print("");
        lcd.setCursor(0,1);
        lcd.print("Please enter.");
        break;
     default:
        return;
  }
}
