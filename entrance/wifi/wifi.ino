#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //v6



SoftwareSerial NodeMCU(D2,D3);

#define OK 1
#define FULL 0
#define ENTER 1
#define EXIT 0

const char *ssid =  "****"; //enter credentials
const char *pass =  "****";

//methods declarations
void connectToWifi();
void postHttp(int code);
void sendResponseToArduino(String message);


void setup()
{
  Serial.begin(9600);
  NodeMCU.begin(4800);
  connectToWifi();
}


void loop()
{
  while(NodeMCU.available()>0)
  {
    int val = NodeMCU.parseInt();
    Serial.println("val: ");
    Serial.println(val);
    if(NodeMCU.read() == '\n')
    {
      postHttp(val);
      break;
    }
  }

  
 delay(100);
}

//implementations

void postHttp(int code)
{
  String postMessage;
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  if(code == EXIT)
  {
      doc["amount"] = "-1";
  }
  else if(code == ENTER)
  {
     doc["amount"] = "1";
  }
  HTTPClient http;
  http.begin("http://52.15.252.42:8080/");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Content-Length", "");
  Serial.println("sending");
  serializeJsonPretty(doc, postMessage);
  serializeJsonPretty(doc, Serial);
  http.POST(postMessage);
  Serial.println("sent");
  Serial.println("waiting for response");
  delay(2000);
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
  int responseToUno;
  if(message == "OK")
  {
    responseToUno = OK;
  }
  else if(message == "FULL")
  {
    responseToUno = FULL;  
  }
  else
  {
    Serial.println("undefined message");
  }
  
  NodeMCU.print(responseToUno);
  NodeMCU.println("\n");
}
