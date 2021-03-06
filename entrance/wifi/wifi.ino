#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //v6

SoftwareSerial NodeMCU(D2,D3);

#define ENTRANCE_MESSAGE 2
#define TEMP_MESSAGE 3
#define INVALID_ENTRANCE 4
#define INVALID_TEMPERATUE 5
#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0

#define AWS_SERVER_IP "http://18.222.9.139:8080/"

const char *ssid =  "TP-LINK_RoEm2.4"; //enter credentials
const char *pass =  "Rmalal92M";

//methods declarations
void checkFromUno();
void connectToWifi();
void postHttp(int code);
void sendResponseToArduino(String message);
void readAndPostTemperature(bool invalid);
void readAndPostEntrance();
void checkForMsgFromAws();

int isOK = 1;


void setup()
{
  Serial.begin(9600);
  NodeMCU.begin(4800);
  connectToWifi();
  //loginServer();
}

void loop()
{
  checkForMsgFromAws();
  checkFromUno();
  delay(10);
}

//implementations
void checkFromUno()
{
  while (NodeMCU.available() > 0)
  {
    int code = NodeMCU.parseInt();
    if (NodeMCU.read() == '=')
    {
      if (code == TEMP_MESSAGE)
      {
        readAndPostTemperature(false);
      }
      else if (code == ENTRANCE_MESSAGE)
      {
        readAndPostEntrance();
      }
      else if (code == INVALID_ENTRANCE)
      {
         StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
         doc["invalid_entrance"] = "true";
         postHttp(doc, false); 
      }
      else if(code == INVALID_TEMPERATUE)
      {
         readAndPostTemperature(true);
      }
      else
      {
        Serial.print("ESP GOT A WEIRD CODE FROM ARDUINO! CHECK THIS : ");
        Serial.println(code);
      }
    }
  }
}
void postHttp(StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc, bool wait)
{
  String postMessage;
  HTTPClient http;
  http.begin(AWS_SERVER_IP);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Length", "");
  Serial.println("sending");
  serializeJsonPretty(doc, postMessage);
  serializeJsonPretty(doc, Serial);
  if(!wait)
  {
    http.POST(postMessage);
    return;
  }
  http.POST(postMessage);
  Serial.println("sent");
  Serial.println("waiting for response");
  String payload = http.getString();
  http.end();
  Serial.println("recieved response");
  Serial.println("gonna print payload now");
  Serial.println(payload);
  sendResponseToArduino(payload);
}

void connectToWifi()
{
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
}

void sendResponseToArduino(String message)
{
  int responseToUno = -1;
  if(message == "OK")
  {
    responseToUno = OK;
  }
  else if(message == "FULL")
  {
    responseToUno = FULL;
  }
  else if (message == "IP RECEIVED")
  {
    Serial.println("AWS REGISTERED THIS ESP");
    return;
  }
  else
  {
    Serial.print("undefined message: ");
    Serial.println(message);
    return;
  }
  if(responseToUno != -1)
  {
    if(responseToUno == isOK)
    {
      return;
    }
    isOK = responseToUno;
  }
  Serial.print("Sent to Arduino : ");
  Serial.println(responseToUno);
  NodeMCU.print(responseToUno);
  NodeMCU.println("\n");
}

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void readAndPostTemperature(bool invalid)
{
  String temperature;
  while (NodeMCU.available() > 0)
  {
    float temp_temp = NodeMCU.parseFloat();
    Serial.println(temp_temp);
    temperature =  String(temp_temp);
    if (NodeMCU.read() == '\n') break;
  }
  
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  if (invalid)
  {
    doc["invalid_temperature"] = temperature;
  }
  else
  {
    doc["temperature"] = temperature;
  }
  postHttp(doc, false);
}

void readAndPostEntrance()
{
  int code = 0;
  while (NodeMCU.available() > 0)
  {
    code =  NodeMCU.parseInt();
    if (NodeMCU.read() == '\n') break;
  }
  
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  if (code == EXIT)
  {
    doc["amount"] = "-1";
  }
  else if (code == ENTER)
  {
    doc["amount"] = "1";
  }
  else 
  {
    Serial.println("FOUND AN ILLEGAL CODE WHEN READING ENTRANCE!");
    return;
  }
  postHttp(doc, false);
}

void checkForMsgFromAws() 
{
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  doc["amount"] = "0";
  postHttp(doc, true);
}
