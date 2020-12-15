#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //v6

SoftwareSerial NodeMCU(D2,D3);

#define ENTRANCE_MESSAGE 2
#define TEMP_MESSAGE 3
#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0

#define AWS_SERVER_IP "http://18.221.133.197:8080/"

const char *ssid =  "TP-LINK_RoEm2.4"; //enter credentials
const char *pass =  "Rmalal92M";

//methods declarations
void checkFromUno();
void connectToWifi();
void postHttp(int code);
void sendResponseToArduino(String message);
void loginServer();
void readAndPostTemperature();
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
   delay(100);
  checkFromUno();
   delay(100);

  checkFromUno();
   delay(100);

  checkFromUno();
   delay(100);

  checkFromUno();
 delay(100);
}

//implementations
void checkFromUno()
{
    if (NodeMCU.available() > 0)
  {
    int code = NodeMCU.parseInt();
    if (NodeMCU.read() == '=')
    {
      if (code == TEMP_MESSAGE)
      {
        readAndPostTemperature();
      }
      else if (code == ENTRANCE_MESSAGE)
      {
        readAndPostEntrance();
      }
      else
      {
        Serial.print("ESP GOT A WEIRD CODE FROM ARDUINO! CHECK THIS : ");
        Serial.println(code);
      }
    }
  }
}
void postHttp(StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc)
{
  String postMessage;
  HTTPClient http;
  http.begin(AWS_SERVER_IP);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Length", "");
  Serial.println("sending");
  serializeJsonPretty(doc, postMessage);
  serializeJsonPretty(doc, Serial);
  http.POST(postMessage);
  Serial.println("sent");
  Serial.println("waiting for response");
  String payload = "";
  delay(200);
  payload = http.getString();
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

void loginServer()
{
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  /*IPAddress ip = WiFi.gatewayIP();
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  char buffer[16];
  sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  Serial.print("Here is my ip : ");
  Serial.println(ip);
  Serial.print("Here is my buff : ");
  Serial.println(buffer);*/
  doc["ip"] = "Hi";
  postHttp(doc);  
}

void readAndPostTemperature()
{
  String temperature;
  while (NodeMCU.available() > 0)
  {
    temperature =  String(NodeMCU.parseFloat());
    if (NodeMCU.read() == '\n') break;
  }
  
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  doc["temperature"] = temperature;
  postHttp(doc);
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
  postHttp(doc);
}

void checkForMsgFromAws() 
{
  StaticJsonDocument<JSON_OBJECT_SIZE(2)> doc;
  doc["amount"] = "0";
  postHttp(doc);
}
