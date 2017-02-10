//ARDUINO 1.0+ ONLY
//ARDUINO 1.0+ ONLY


#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;

////////////////////////////////////////////////////////////////////////
//CONFIGURE
////////////////////////////////////////////////////////////////////////
  byte ip[] = { 192, 168, 0, 2 };   //Manual setup only
  byte gateway[] = { 192, 168, 0, 1 }; //Manual setup only
  byte subnet[] = { 255, 255, 0, 0 }; //Manual setup only

  // if need to change the MAC address (Very Rare)
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

  EthernetServer server = EthernetServer(80); //port 80
////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(9600);

  //Pins 10,11,12 & 13 are used by the ethernet shield

  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  //Ethernet.begin(mac);
  Ethernet.begin(mac, ip, gateway, subnet); //for manual setup
  pinMode(LED_BUILTIN, OUTPUT);
  server.begin();
  
  Serial.println(Ethernet.localIP());

}

void loop(){

  // listen for incoming clients, and process qequest.
  checkForClient();

}

void checkForClient(){

  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;

    while (client.connected()) {
      if (client.available()) {

        if(!sentHeader){
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          sentHeader = true;
        }

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info

        if(reading){
          Serial.print(c);

           switch (c) {
            case 'r':
              Serial.println("Recieved msg r");
              makeColor(255,0,0);
              break;
            case 'p':
              //triggerPin(3,client);
              Serial.println("Recieved msg p");
              makeColor(255,255,0);
              break;
          }

        }

        if (c == '\n' && currentLineIsBlank)  break;

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }

      }
    }

    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:

  } 

}

void makeColor(int r, int b, int g){
  analogWrite(A3,r);
  analogWrite(A1,b);
  analogWrite(A2,g);
}

void triggerPin(int pin, EthernetClient client){
//blink a pin - Client needed just for HTML output purposes.  
  client.print("Recieved command ");
  client.println(pin);
  client.print("<br>");

  //Do circut stuff
  digitalWrite(pin, HIGH);
  delay(25);
  digitalWrite(pin, LOW);
  delay(25);
}
