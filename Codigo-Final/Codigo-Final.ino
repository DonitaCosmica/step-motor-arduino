#include <SdFat.h>
#include <SPI.h>
#include <Ethernet.h>

#define IN1  11
#define IN2  10
#define IN3  9
#define IN4  8

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 100, 20 };
byte gateway[] = { 192, 168, 100, 1 };
byte subnet[] = { 255, 255, 255, 0 };

const int step [4][4] =
{
  {1, 0, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 1}
};
const int chipSelect = 4;
const int stepsRev = 2048;
const int potPin = A0;
int lastDisplayValue = -1;
SdFat SD;
File webFile;

EthernetServer server(80);

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Initializing SD card..."));

  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);

  if(!SD.begin(chipSelect, SD_SCK_MHZ(50)))
  {
    Serial.println(F("ERROR - SD card initialization failed"));
    return;
  }

  Serial.println("SUCCESS - SD card initialized...");
  if(!SD.exists("index.html"))
  {
    Serial.println(F("ERROR - Can't find index.html file!"));
    return;
  }

  Serial.println(F("Initializing Ethernet..."));
  Ethernet.begin(mac, ip, gateway, subnet);

  server.begin();
  Serial.println(F("Ethernet initialized"));

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println(F("No Ethernet hardware present"));
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("Ethernet cable is not connected."));
  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(potPin, INPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop()
{
  EthernetClient client = server.available();

  if (client) 
  {
    Serial.println(F("Conectado..."));
    boolean currentLineIsBlank = true;
    char httpRequest[256] = { 0 };
    int i = 0;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);

        if (i < sizeof(httpRequest) - 1)
          httpRequest[i++] = c;
          
        if (c == '\n' && currentLineIsBlank) 
        {
          delay(4000);
          int senVal = analogRead(potPin);
          int mapVal = map(senVal, 0, 1023, 100, 0);;
          Serial.println(httpRequest);

          if (strncmp(httpRequest, "GET ", 4) == 0)
          {            
            if (httpRequest[4] == '/' && httpRequest[5] == ' ')
              sendFile(client, "index.html", "text/html");
            else if (strncmp(httpRequest + 5, "stepMotor.gif", 13) == 0)
              sendFile(client, "stepMotor.gif", "image/gif");
            else if (strncmp(httpRequest + 5, "favicon.ico", 11) == 0)
              sendFile(client, "favicon.ico", "image/x-icon");
            else if (strncmp(httpRequest + 5, "per", 3) == 0)
            {
              if (mapVal < 4)
                mapVal = 0;
              if (mapVal > 96)
                mapVal = 100;
              sendData(client, mapVal);
            }
            else if (strncmp(httpRequest + 5, "tur", 3) == 0)
            {
              digitalWrite(chipSelect, HIGH);
              SPI.end();
              char *turnsStart = strstr(httpRequest, "turns=");
              if (turnsStart != NULL)
                turnsStart += 6;

              const int web = atoi(turnsStart);

              const int steps = mapVal / 100.0 * stepsRev * web;
              for (int i = 0; i < (steps / 4); i++)
              {
                for (int i = 0; i < 4; i++)
                {
                  digitalWrite(IN1, step[i][0]);
                  digitalWrite(IN2, step[i][1]);
                  digitalWrite(IN3, step[i][2]);
                  digitalWrite(IN4, step[i][3]);
                  delay(10);
                }
              }

              digitalWrite(IN1, LOW);
              digitalWrite(IN2, LOW);
              digitalWrite(IN3, LOW);
              digitalWrite(IN4, LOW);
            }
          }

          break;
        }

        if (c == '\n')
          currentLineIsBlank = true;
        else if (c != '\r')
          currentLineIsBlank = false;
      }
    }

    delay(100);
    client.stop();
    Serial.println(F("Desconectado..."));
  }
}

void sendFile(EthernetClient &client, const char* filename, const char* contentType)
{
  Serial.println(String("Sending file: ") + filename);
  File webFile = SD.open(filename);
  if(webFile)
  {
    Serial.println(String("File opened successfully: ") + filename);
    client.println(F("HTTP/1.1 200 OK"));
    client.print(F("Content-Type: "));
    client.println(contentType);
    client.println(F("Cache-Control: no-cache"));
    client.println(F("Connection: close"));
    client.println();

    byte buffer[64];
    int bytesRead = 0;
    while((bytesRead = webFile.read(buffer, sizeof(buffer))) > 0)
      client.write(buffer, bytesRead);
    webFile.close();
  }
  else
  {
    Serial.println(String("Error - Could not open file: ") + filename);
    send404(client);
  }
}

void send404(EthernetClient &client)
{
  client.println(F("HTTP/1.1 404 Not Found"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("<h1>404 Not Found</h1>"));
}

void sendData(EthernetClient &client, int mapVal)
{
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("connection: close"));
  client.println();
  client.print(F("{\"percentage\":"));
  client.print(mapVal);
  client.println(F("}"));
}

void sendMessage(EthernetClient &client)
{
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: application/json"));
  client.println(F("connection: close"));
  client.println();
  client.print(F("{\"percentage\":"));
  client.print(F("Data has been sent successfully"));
  client.println(F("}"));
}