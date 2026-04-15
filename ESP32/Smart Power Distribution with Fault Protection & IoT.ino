#include<WiFi.h>
const char* ssid="vivo";
const char* password="890890890";
HardwareSerial myserial(2);//UART declare
HardwareSerial gsm(1);
String data="";
String ov_fault="0";
int flag=0;

String api_key="OQ1C0ZPKC9MK4RO5";
const char* server="api.thingspeak.com";
WiFiClient client;
void sendAlert()
{
  Serial.println("Alert sending!");
  gsm.println("AT");
  delay(1000);
  gsm.println("AT+CMGF=1");
  gsm.println("AT+CMGS=\"+916380625951\"");
  delay(2000);
  gsm.println("Over Voltage Occurred!");
  delay(500);
  gsm.write(26);
  delay(5000);

  //call
  gsm.println("ATD+916380625951;");
  delay(10000);
  gsm.println("ATH");
}

void sendToThingSpeak(String data)
{
  int first = data.indexOf(',');
  int second = data.indexOf(',', first + 1);
  int third = data.indexOf(',', second + 1);

  if (first == -1 || second == -1 || third == -1)
  {
    Serial.println("Invalid data format!");
    return;
  }

  String voltage = data.substring(0, first);
  String current = data.substring(first + 1, second);
  String power = data.substring(second + 1, third);
   ov_fault = data.substring(third + 1);

  Serial.println("voltage: " + voltage);
  Serial.println("current: " + current);
  Serial.println("power: " + power);
  Serial.println("ov_fault: " + ov_fault);

  if (client.connect(server, 80))
  {
    String url = "/update?api_key=" + api_key;
    url += "&field1=" + voltage;
    url += "&field2=" + current;
    url += "&field3=" + power;
    url += "&field4=" + ov_fault;

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: api.thingspeak.com\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Data sent to ThingSpeak!");

    // Read response
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
  else
  {
    Serial.println("Connection failed!");
  }

  client.stop();
  if(ov_fault=="1" && flag==0)
  {
    sendAlert();
    flag=1;
  }
  if(ov_fault=="0")
  {
    flag=0;
  }
}

void setup() {
 Serial.begin(9600);
 myserial.begin(9600,SERIAL_8N1,16,17);//pin and data configure
 gsm.begin(9600,SERIAL_8N1,4,5);
 WiFi.begin(ssid, password);
 Serial.print("Connecting to Wifi");

while(WiFi.status()!=WL_CONNECTED)
{
  delay(500);
 Serial.print('.');
}
Serial.print("\nconnected");
}

void loop() {
 while(myserial.available())
 {
  char c=myserial.read();
  Serial.print(c);
  if(c=='\n')
  {
    Serial.println("\nReceived:"+data);
    sendToThingSpeak(data);
    data="";
    delay(15000);
  }
  else
  {
    data=data+c;
  }
 }

}
