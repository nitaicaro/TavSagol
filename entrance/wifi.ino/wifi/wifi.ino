#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //v6



SoftwareSerial NodeMCU(D2,D3);


const char *ssid =  "TP-LINK_RoEm2.4";
const char *pass =  "Rmalal92M";


void postHttp(int val)
{
  String postMessage;
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  if(val == 0)
  {
      doc["amount"] = "-1";
  }
  else
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
  int responseToUno;
  if(payload == "OK")
  {
    responseToUno = 1;
  }
  else
  {
    responseToUno = 0;  
  }
  NodeMCU.print(responseToUno);
  NodeMCU.println("\n");
}

void connectToWifi()
{
  Serial.println("esp, esp, show us the wifi");
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
