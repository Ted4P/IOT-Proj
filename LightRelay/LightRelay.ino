
//Ethernet Library tutorial credit to 
//http://bildr.org/2011/06/arduino-ethernet-pin-control/
#include <Ethernet.h>
#include <SPI.h>
boolean reading = false;
String lastCommand = "";
byte ip[] = {192, 168, 0, 2};  
byte gateway[] = {192, 168, 0, 1}; 
byte subnet[] = {255, 255, 0, 0}; 
int RELAY_DELAY = 500;
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
  char lastC='';    //Store the last command to allow a "hold" suffix
  for(int i=0;i<command.length();i++){
      char c = command.charAt(i);
      switch(c){
            case 'r':
              makeColor(255,0,0);
              break;
            case 'p':
              makeColor(255,255,0);
              break;
            case 'b':
              makeColor(0,255,0);
              break;
            case 'g':
              makeColor(0,0,255);
              break;
            case 'y':
              makeColor(255,0,255);
              break;
            case 'c':
              makeColor(0,255,255);
              break;
            case 'w':
              makeColor(255,255,255);
              break;
      }
      if((int)c>=48 &&  (int)c<=57){  //If character code is numeric, convert to int and hold
          delay((lastC='h')?10:1)*      //If the last character is 'h', hold for 10x longer (e.g. gh5 hold green for 5s);
                  100*((int)c-48));
      }
      lastC=c;  //Update last character
  }
}

void toggleRelay(){    //Toggle relay on/off
  digitalWrite(7,currentState?LOW:HIGH);
  currentState=!currentState;
}
