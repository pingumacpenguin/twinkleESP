#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC);
const char* ssid = "etv";
const char* password = "office.wifi";
bool shutdownPending = false;
int MOSFETPin = 4; //GPIO4
int ledPin = 2; // GPIO2


int ledRGBGreen = 12;
int ledRGBBlue = 13;
int ledRGBRed = 15;

#define numRGBLEDS 3 
char rgbLEDS[numRGBLEDS] = {ledRGBGreen,ledRGBBlue,ledRGBRed};

#define numRedLEDS 11
char redLEDS[numRedLEDS] = { 16, 14, 5, 4, 0, 2 , 0, 4, 5, 14, 16};

int count = 0;


int batteryVoltage = 4200; // Assume charged (4.2V) until we find out otherwise
//int lowBattery = 3200;     // Low battery at 3.2V
int lowBattery = 3421;     // Low battery at 3.420V for testing

WiFiServer server(80);

void setup() {
  enableLEDS();
  Serial.begin(115200);
  delay(10);
  // ESP.getVcc();
  // Start up, switch on the LED and set the MOSFET toggle pin HIGH
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(MOSFETPin, OUTPUT);
  digitalWrite(MOSFETPin, HIGH);

  //onSmallLEDs();

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  batteryVoltage = ESP.getVcc();

}

void loop() {
  knightRider();
  if (!(count %10)) {
   twinkleRGBLEDS();
  }
  count ++;
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    if (!( count % 1000) ) {
      checkBattery();
    }
    return;
  }

  // Wait until the client sends some data
  Serial.println("Client Connected");
  while (!client.available()) {
    if (!( count % 1000)) {
      checkBattery();
    }
    delay(1);

  }
  //shutDown=checkBattery(batteryVoltage,lowBattery);
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request

  int ledState = digitalRead(ledPin);


  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(ledPin, LOW);
    ledState = LOW;
    toggleMOSFET();
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(ledPin, HIGH);
    ledState = HIGH;
    toggleMOSFET();
  }

  // Set ledPin according to the request
  //digitalWrite(ledPin, ledState);
  if (!( count % 1000)) {
    checkBattery();
  }
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("MOSFET output toggled. ");
  client.print("Led is now: ");

  if (ledState == LOW) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br></br>");
  client.print("Battery Voltage: ");

  client.print(batteryVoltage / 1000);
  client.print(".");
  client.println(batteryVoltage % 1000);
  client.println("<br></br>");
  client.print("Shutdown pending: ");
  if (shutdownPending) {
    client.print(batteryVoltage);
    client.print("<");
    client.print(lowBattery);
    client.println(" Yes");
  } else {
    client.print(batteryVoltage);
    client.print(">");
    client.print(lowBattery);
    client.println(" No");
  }
  client.println("<br></br>");
  client.println("Click <a href=\"/LED=ON\">here</a> Toggle MOSFET and turn the LED on pin 2 ON<br>");
  client.println("Click <a href=\"/LED=OFF\">here</a> Toggle MOSFET and turn the LED on pin 2 OFF<br>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  //offSmallLEDs();
  delay(100);
}

void toggleMOSFET() {
  digitalWrite(MOSFETPin, LOW);
  delay(300);
  digitalWrite(MOSFETPin, HIGH);
  delay(300);
  digitalWrite(MOSFETPin, LOW);
  delay(300);
  digitalWrite(MOSFETPin, HIGH);
}

void checkBattery() {
  batteryVoltage = ESP.getVcc();
  if (batteryVoltage < lowBattery) {
    shutdownPending = true;
  } else {
    shutdownPending = false;
  }
  toggleLED();
  delay(10);
  toggleLED();
}

void toggleLED() {
  digitalWrite(ledPin, (digitalRead(ledPin)));
}

void enableLEDS() {
  
  int i = 0;
  for (i = 0; i < numRedLEDS; i++)
  {
    pinMode(redLEDS[i], OUTPUT);
  }
  for (i = 0; i <numRGBLEDS; i++)
  {
    pinMode(rgbLEDS[i], OUTPUT);
  }

}

void knightRider() {
  digitalWrite(redLEDS[count % numRedLEDS], !(digitalRead(redLEDS[count % numRedLEDS])));
  delay(30);
  digitalWrite(redLEDS[count % numRedLEDS], !(digitalRead(redLEDS[count % numRedLEDS])));
  delay(30);
}

void twinkleRGBLEDS() {
  int r = rand() % numRGBLEDS ;
  digitalWrite(rgbLEDS[r], !(digitalRead(rgbLEDS[r])));
}

