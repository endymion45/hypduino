/* 
   HypArdui pour télécomande, température et relai arduino
   V 0.5 - C. Alexis Salecroix  Décembre 2014
*/

#define WEBDUINO_FAIL_MESSAGE "<h1>Request Failed</h1>"
#include "SPI.h" // new include
#include "avr/pgmspace.h" // new include
#include "Ethernet.h"
#include "WebServer.h"
/*
  Include lié au ds18b20
*/
#include <OneWire.h>
#include <DallasTemperature.h>

/* IP STACK */
// Enter here the MAC adress from the Arduino Ethernet or Ethernet shield sticker.
static uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x5C, 0x94 };
IPAddress ip(192,168,1,47);
/* WEBROOT */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

/* IO PIN's Used */
#define UP_PIN 9
#define ST_PIN 8
#define DN_PIN 7
#define LD_PIN 6
#define RE1_PIN 3
#define RE2_PIN 5

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

/* This is the number of milliseconds for pulse delay */
int ButtonDelay = 100;
int volet=0;
int re1=0;
int re2=0;
/* store the HTML in program memory using the P macro */

  
// Button action
void ButtonAction (int pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(LD_PIN, HIGH);
  delay(ButtonDelay);
  digitalWrite(pin, LOW);
  digitalWrite(LD_PIN, LOW);
}

#define NAMELEN 32
#define VALUELEN 32

void parsedCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  int resultat = 0;
  int id=0;
  char cmd[VALUELEN];
  char obj[VALUELEN];
  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();
Serial.println("demande de la page cmd.html");
  if (strlen(url_tail))
    {
    while (strlen(url_tail))
      {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
       //Serial.println(name[0]);
       resultat=strcmp(name,"id");
       if(resultat == 0)
         {
           id=value[0];
           Serial.println(value[0]);
         }
        resultat=strcmp(name,"cmd");
        if(resultat == 0)
         {
           strcpy(cmd,value);
           Serial.println(cmd);
         }
        resultat=strcmp(name,"obj");
        if(resultat == 0)
         {
           strcpy(obj,value);
           Serial.println(name);
         }
      }
    }
switch (id){
  case '1':
  server.print(sensors.getTempCByIndex(0));
  break;
  case '2':
  
    if(strcmp(cmd,"up") == 0)
     {
       ButtonAction(UP_PIN);
        Serial.println(cmd);
       Serial.println(strcmp(cmd,"up"));
     }
    else if(strcmp(cmd,"stop")== 0)
      {
        ButtonAction(ST_PIN);
         Serial.println(cmd);
        Serial.println(strcmp(cmd,"stop"));
      }
    else if (strcmp(cmd,"down") == 0)
      {
        ButtonAction(DN_PIN);
        Serial.println(cmd);
        Serial.println(strcmp(cmd,"down"));
      }
     else
       {
         ButtonAction(ST_PIN);
         Serial.println("stop defaut");
       }
  break;
  case '3':
  if(strcmp(cmd,"on") == 0)
      {
       if(strcmp(obj,"1")== 0)
         {
           digitalWrite(RE1_PIN, LOW);
           digitalWrite(LD_PIN, HIGH);
           delay(ButtonDelay);
           digitalWrite(LD_PIN, LOW);
           re1=1; 
         }
         else if(strcmp(obj,"2") == 0)
           {
             digitalWrite(RE2_PIN, LOW);
             digitalWrite(LD_PIN, HIGH);
             delay(ButtonDelay);
             digitalWrite(LD_PIN, LOW);
             re2=1;  
           }
      }
    else if(strcmp(cmd,"off") == 0)
          {
       if(strcmp(obj,"1") == 0)
         {
           digitalWrite(RE1_PIN, HIGH);
           digitalWrite(LD_PIN, HIGH);
           delay(ButtonDelay);
           digitalWrite(LD_PIN, LOW);
           re1=0; 
         }
         else if(strcmp(obj,"2") == 0)
           {
             digitalWrite(RE2_PIN, HIGH);
             digitalWrite(LD_PIN, HIGH);
             delay(ButtonDelay);
             digitalWrite(LD_PIN, LOW);
             re2=0;  
           }
      }
    else if(strcmp(cmd,"etat") == 0)
          {
            if(strcmp(obj,"1") == 0)
               {
                 server.print(re1);
               }
              else if(strcmp(obj,"2") == 0)
                {
                 server.print(re2);
                }
            
          }
  }

}

void setup()
{
  //initialisation de la lib pour le ds18b20
  sensors.begin();
  // set the PIN's to output
  pinMode(UP_PIN, OUTPUT);
  pinMode(ST_PIN, OUTPUT);
  pinMode(DN_PIN, OUTPUT);
  pinMode(LD_PIN, OUTPUT);
  pinMode(RE1_PIN, OUTPUT);
  pinMode(RE2_PIN, OUTPUT);
digitalWrite(RE1_PIN, HIGH);
digitalWrite(RE2_PIN, HIGH);
  Serial.begin(115200);
  digitalWrite(LD_PIN,HIGH);
  Serial.println("HypArdui V0.4 - Alexis Salecroix - 2014");
  Serial.println("Starting IP stack and DHCP request...");
  
  // setup the Ehternet library to talk to the Wiznet board
  // DHCP
  Ethernet.begin(mac, ip);
  // print your local IP address:
  Serial.print("Le DHCP a attribué l'adresse ip suivante: ");
  Serial.println(Ethernet.localIP());
 
  /*This command  is called if you try to load /raw.html */
  webserver.addCommand("cmd.html", &parsedCmd);
  /* start the server to wait for connections */
  Serial.println("Démarrage du serveur WEB");
  webserver.begin();
  
  /* Signal startup OK  */
  for (int i=0; i<5; i++) {
    digitalWrite(LD_PIN, HIGH);
    delay(50);
    digitalWrite(LD_PIN, LOW);
    delay(50);
  }
  delay(1000);
  ButtonAction(ST_PIN);
  Serial.print("Requête pour la température");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("Ok");
  
  Serial.print("La température pour le capteur 1 (index 0) est: ");
  Serial.println(sensors.getTempCByIndex(0));
  Serial.println("Prêt.");
}

void loop()
{
 char buff[64];
  int len = 64;

  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);
}
