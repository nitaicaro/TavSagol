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
#define LIGHT_THRESHOLD 100

//temperature
Adafruit_MLX90614 temperature_sensor = Adafruit_MLX90614();
#define IS_HUMAN_TEMP(value) (((value) < 45) && ((value) > 33))
#define IS_VALID_TEMP(value) (((value) < 38) && ((value) > 33))
#define TEMP_SENSOR_CORRECT_FACTOR 4
#define TEMP_MEASURE_ITERATIONS 10 

//display
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int lcdCurrentMessage = -1;
#define MSG_MES_TEMP 0
#define MSG_FULL 1
#define MSG_ENTER 2
#define MSG_INVALID_TEMP 3

//communication globals
SoftwareSerial ArduinoUno(3,2);
#define INVALID_ENTRANCE 4
#define TEMP_MESSAGE 3
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
//temp
void readAndSendTemp();
float measureTemp();
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
  //temperature init
  temperature_sensor.begin();
  // initialize the lcd 
  lcd.init();                      
  lcd.backlight();
  lcd.clear();
  updateDisplay(MSG_MES_TEMP);
}

void loop() 
{
  checkAndHandleResponseFromEsp();
  readAndSendTemp();
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
        updateDisplay(MSG_MES_TEMP);
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
  if (!tempIsOk)
  {
    // Someone entered when their temperature is high / didn't check temperature
    tone(BUZZER, 440, 2000);
    notifyEsp(INVALID_ENTRANCE, -1);
    // TODO: Maybe let the esp know that someone with high heat went inside
  }
  else
  {
    tempIsOk = false;
  }
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
    updateDisplay(MSG_MES_TEMP);
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

//notifies the esp if someone enters/exists
//param code = 0 is for exit, 1 is for enter
void notifyEsp(int messageType, int value)
{
    ArduinoUno.print(messageType);
    if(value < 0) return;
    ArduinoUno.print('=');
    ArduinoUno.print(value);
    ArduinoUno.println("\n");
}

//temp

float measureTemp()
{
  float sum = 0;
  for(int i = 0; i < TEMP_MEASURE_ITERATIONS; ++i)
  {
    sum += temperature_sensor.readObjectTempC();
    delay(10);
  }
  return sum/TEMP_MEASURE_ITERATIONS + TEMP_SENSOR_CORRECT_FACTOR;
}

void readAndSendTemp()
{
  if(isFull) return;
  if(tempDelay > 0)
  {
    //decrease timeout
    tempDelay -= 1;
    return;
  }
  else
  {
      //timeout, reset
      updateDisplay(MSG_MES_TEMP);
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      tempIsOk = false;
  }
  float temperature = temperature_sensor.readObjectTempC();
  if(IS_HUMAN_TEMP(temperature))
  {
    temperature = measureTemp();
    Serial.println(temperature);
    if(!(IS_VALID_TEMP(temperature)))
    {
       notifyEsp(TEMP_MESSAGE, temperature);
       tempIsOk = false;
       tempDelay = 600;
       tone(BUZZER, 30, 200);
       updateDisplay(MSG_INVALID_TEMP);
       digitalWrite(RED, HIGH);
       digitalWrite(GREEN, LOW);
    }
    else
    {
       tempIsOk = true;
       tempDelay = 600;
       updateDisplay(MSG_ENTER); 
       digitalWrite(RED, LOW);
       digitalWrite(GREEN, HIGH);
    }
  }
  // TODO: Need to do update tempIsOk only if the temperature is ok
}

void updateDisplay(int messageCode)
{
  if(messageCode == lcdCurrentMessage) return;
  lcd.clear();
  lcdCurrentMessage = messageCode;
  switch(messageCode)
  {
    case MSG_MES_TEMP:
        lcd.setCursor(0,0);
        lcd.print("Please measure");
        lcd.setCursor(0,1);
        lcd.print("your temperature");
        break;
    case MSG_FULL:
        lcd.setCursor(0,0);
        lcd.print("Room is full");
        lcd.setCursor(0,1);
        lcd.print("Please wait.");
        break;
    case MSG_ENTER:
        lcd.setCursor(0,0);
        lcd.print("Temperature OK");
        lcd.setCursor(0,1);
        lcd.print("Please enter.");
        break;
     case MSG_INVALID_TEMP:
        lcd.setCursor(0,0);
        lcd.print("Temperature High");
        lcd.setCursor(0,1);
        lcd.print("Don't enter.");
        break;
     default:
        return;
  }
}
