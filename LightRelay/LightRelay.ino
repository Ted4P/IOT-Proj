
//Ethernet Library tutorial credit to 
//http://bildr.org/2011/06/arduino-ethernet-pin-control/
#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;
String lastCommand = "";
byte ip[] = {192, 168, 0, 2};  
byte gateway[] = {192, 168, 0, 1}; 
byte subnet[] = {255, 255, 0, 0}; 


boolean currentState=false;

//Placeholder MAC, can be replaced with value printed on ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(80); //80 = http communication

void setup(){
  pinMode(7, OUTPUT);  //Set the relay control pin to output mode

  Ethernet.begin(mac, ip, gateway, subnet);
  pinMode(LED_BUILTIN, OUTPUT);
  server.begin();
  
  Serial.println(Ethernet.localIP());

}

void loop(){

  // listen for incoming clients, and process request.
  checkForClient();
  executeCommand(lastCommand);
}

void checkForClient(){
  EthernetClient client = server.available();

  if (client) {

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean sentHeader = false;
    lastCommand="";
    while (client.connected()) {
      if (client.available()) {

        if(!sentHeader){
          //HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          //PUT HTML HERE
          sentHeader = true;
        }

        char c = client.read();

        if(reading && c == ' ') reading = false;
        if(c == '?') reading = true; //found the ?, begin reading the info
        
        if(reading){
          Serial.print(c);

          lastCommand += c;
        }

        if (c == '\n' && currentLineIsBlank)  break;  //If 2 /ns are recieved, end the reading phase

        if (c == '\n') {
          currentLineIsBlank = true;
        }

      }
    }
    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:

  } 

}

void executeCommand(String command){
  for(int i=0;i<command.length();i++){
      char c = command.charAt(i);
      switch(c){
            case 'o':
              makeOn();
              break;
            case 'f':
              makeOff();
              break;
            case 't':
              toggleRelay();
              break;
  }
}

void makeOn(){
  if(!currentState) toggleRelay();
}
void makeOff(){
  if(currentState) toggleRelay();
}

void toggleRelay(){    //Toggle relay on/off
  digitalWrite(7,currentState?LOW:HIGH);
  currentState=!currentState;
}
