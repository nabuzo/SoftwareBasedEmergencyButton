#include <SoftwareSerial.h>
#include <DHT.h>
#include <ThingSpeak.h>;
#include "pitches.h" //add note library

#define RX 10
#define TX 11
#define DHTPIN 2

DHT dht(DHTPIN, DHT11);

String AP = "RPiNetwork";       // CHANGE ME
String PASS = "CPSC1234567890"; // CHANGE ME

String API = "FXMALLO639Y8Q46Q";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";

int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
int val;
int button = 2;

SoftwareSerial esp8266(RX,TX); 

//notes in melody
int melody[]{NOTE_C4, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
//digital pin 12 has a button attached to it. Give it a name
int buttonPin = 12;
// note durations. 4=quarter note / 8=eigth note
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

boolean start = true;                // stop void loop variable
volatile boolean e_stop = false;     // ISR variable

void setup() {
  pinMode(buttonPin, INPUT); // make the button's pin input
  Serial.begin(9600);
  esp8266.begin(115200);
  dht.begin();
  delay(100);
  attachInterrupt(0, e_stop_ISR ,RISING);   // call e_stop_ISR fucntion when D2 is switched
                                            // from low to high.. normally low due to pull
                                            // resistor attached to external button

  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");

  
  String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(val);
  sendCommand("AT+CIPMUX=1",5,"OK");  // CIPMUX=0: single connection, 1: multiple connection
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");  // CIPSTART = 0, id of connection
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");      // ID no. of transmit connection
  esp8266.println(getData);delay(1500);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
}

void loop() {
  val = digitalRead(e_stop); // read the input pin
  
  if(start == true){
    
    if(e_stop == false){
      Serial.println("All is well..");
      delay(300);
    }
    else{
      Serial.println("Emergency Stop!! Must press reset button to restart program.");
      start = false; // set value of start variable to false, makes void loop and halt the next time around

      // if the button is pressed, iterate over the notes of the melody
      for (int thisNote=0; thisNote<8; thisNote++){
        // to calculate the note duration, take one second divided by the note type
        int noteDuration = 1000 / noteDurations [thisNote];
        tone(8, melody [thisNote], noteDuration);
        // to distinguish the notes, set a minimum time between them
        // the note's duration +30% seems to work well
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing
        noTone(8);
        
        
      }
    }
  }
  
}

void e_stop_ISR(void){      // ISR function takes no parameter and returns no value
  detachInterrupt(0);       // makes button ebouncing unnecessary
  e_stop = !e_stop;         // toggle the current value of the e_stop variable
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }


  
