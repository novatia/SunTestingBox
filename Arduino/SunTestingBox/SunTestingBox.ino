/*
 * DayNightSimulator - coded by A. Novati on 28/02/2024 - property of N3 S.r.l.
 *
 * 
 *
 *
 *
 */
#include <EEPROM.h>
#include <Servo.h>

#include <Ethernet.h>
#include <SPI.h>

#define SERVO true
#define DEBUG_SERVO true
#define DEBUG_HTTP_REQUESTS false

#define SERVO_SIG_PIN 9
#define SUN_LED_PIN 3
#define EVENT_DET_LED_PIN 5 //Detection event pin
#define STARS_LED_PIN 2 // Define a pin for the non-dimmed LED, representing stars
#define EVENT_LED_PIN 6 // New LED for events

// Define sunrise and sunset times in seconds since midnight
#define DAY_SUNRISE_START     21600UL    // 6:00 AM
#define DAY_SUNRISE_STOP      25200UL    // 7:00 AM
#define DAY_SUNSET_START      64800UL    // 6:00 PM
#define DAY_SUNSET_STOP       68400UL    // 7:00 PM
#define END_OF_DAY            86400UL    // Define the end of the day in seconds

#define SERVO_START_POSITION  30          // default servo start position
#define SERVO_END_POSITION    150         // default servo end position

// Event handling
#define DEFAULT_EVENT_FALLOUT 100UL      // default event fallout

#define MAX_EVENT_BRIGHTNESS  50UL       // default event brightness
#define EVENT_DURATION        1000UL     // default event duration
#define EVENT_SPEED                 1.0f       // default speed

unsigned long currentMillis       = 0;
unsigned long event_duration      = 0;     // event default duration
unsigned long eventStartTime      = 0;     // Event timestamp
unsigned long eventDetStartTime   = 0;     // Detection event timestamp
unsigned long seconds             = 0;     // Current time in seconds
unsigned long previousMillis      = 0;     // Previous timestamp
unsigned long max_brightness      = 0;     //Maximum events brightness
float speed                       = 0.0f;  // Time progression speed

int brightness                    = 0;     // LED brightness level
bool eventTriggered               = false; // event mux
bool eventDetTriggered            = false; // detection event mux

// EEPROM addresses
const int addrSeconds = 0;                // Address for 'seconds'
const int addrSpeed = 4;                  // Address for 'speed', 4 bytes after 'seconds'
const int addrEventDuration = 8;          // Address for event_duration, assuming 'speed' takes 4 bytes at address 4
const int addrBrightness = 12;            // Address for addrBrightness

// Ethernet settings
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0xDE, 0x80 };  // MAC address for your Ethernet shield

IPAddress ip(192, 168, 26, 77);                       // IP address for the Ethernet shield
IPAddress dns(192, 168, 27, 254);
IPAddress gateway(192, 168, 27, 254);
IPAddress subnet(255, 255, 254, 0);

EthernetServer server(80);                            // Initialize the Ethernet server library with the port to listen on
EthernetClient client;

#if SERVO
Servo event_servo;                                   // Create a servo object

#endif 

void triggerEvent()
{
  if (!eventTriggered)
  {
    eventTriggered = true;
    eventStartTime = millis();
    Serial.println("Trigger event");
  }
}

void triggerDetEvent()
{
  if (!eventDetTriggered) 
  {
    eventDetTriggered = true;
    eventDetStartTime = millis();
    Serial.println("Trigger detection event");
  }else 
      Serial.println("Event discarded, another event is running.");

}

void handleEventLED()
{
  if (eventTriggered) {
    unsigned long  currentMillis = millis();
    unsigned long elapsedTime = currentMillis - eventStartTime;

    if (elapsedTime <= event_duration) {
      // Dim up the LED over the event_duration
      int event_brightness = map(elapsedTime, 0, event_duration, 0, max_brightness);
      analogWrite(EVENT_LED_PIN, event_brightness);
    } else if (elapsedTime <= event_duration + DEFAULT_EVENT_FALLOUT) {
      // Keep the LED on full brightness for the fallout duration
      analogWrite(EVENT_LED_PIN, max_brightness);
    } else {
      // Turn off the LED and reset eventTriggered flag
      analogWrite(EVENT_LED_PIN, 0);
      eventTriggered = false;
      Serial.println("Event end");
    }
  }
}

void handleDetEventLED()
{
  static int last_servo_pos = -1;
  
  if (eventDetTriggered)
  {
    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - eventDetStartTime;
    unsigned long full_duration = event_duration + DEFAULT_EVENT_FALLOUT;
    
    if (elapsedTime <= event_duration) {
      
      int event_brightness = map(elapsedTime, 0, event_duration, 0, max_brightness);
      analogWrite(EVENT_DET_LED_PIN, event_brightness);
      
      #if SERVO
      int servo_position = map(elapsedTime, 0, event_duration, SERVO_START_POSITION, SERVO_END_POSITION);
   
      if (servo_position != last_servo_pos){
        event_servo.write(servo_position);
        last_servo_pos = servo_position;
        if (DEBUG_SERVO)
          Serial.println(servo_position);
      }
      #endif
      
      
      
    } else if (elapsedTime <= full_duration) {
      // Keep the LED on full brightness for the fallout duration
       analogWrite(EVENT_DET_LED_PIN, max_brightness);

      #if SERVO
      //go to END position
      if (last_servo_pos != SERVO_END_POSITION){
        event_servo.write(SERVO_END_POSITION);
        last_servo_pos = SERVO_END_POSITION;
        if (DEBUG_SERVO)
          Serial.println(SERVO_END_POSITION);
      }
      #endif
       
    } else {
      // Turn off the LED and reset eventTriggered flag
      analogWrite(EVENT_DET_LED_PIN, 0);
   
     
      #if SERVO
       //go to START position
     
      if (last_servo_pos != SERVO_START_POSITION){
        event_servo.write(SERVO_START_POSITION);
        last_servo_pos = SERVO_START_POSITION;
        
        if (DEBUG_SERVO)
          Serial.println(SERVO_START_POSITION);
      }
    
      eventDetTriggered = false;
      
      #endif
      
      Serial.println("Detection event end");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial);  // Aspetta che la seriale sia pronta
  
  pinMode(SUN_LED_PIN, OUTPUT);// Initialize the stars LED pin as an output
  pinMode(EVENT_DET_LED_PIN, OUTPUT); // Initialize the stars LED pin as an output
  pinMode(STARS_LED_PIN, OUTPUT); // Initialize the stars LED pin as an output
  pinMode(EVENT_LED_PIN, OUTPUT); // Initialize the stars LED pin as an output
  

 
  // Read 'seconds' and 'speed' from EEPROM
  EEPROM.get(addrSeconds, seconds);
  EEPROM.get(addrSpeed, speed);
  EEPROM.get(addrEventDuration, event_duration);
  EEPROM.get(addrBrightness, max_brightness);

  //at first run from eeprom comes 0.

  if ( speed == 0UL || speed > 100000UL)
      setSpeed(EVENT_SPEED);

  if ( event_duration == 0UL || event_duration>25000UL )
      setEventDuration(EVENT_DURATION);
 
  if ( max_brightness == 0UL || max_brightness>255 )
      setEventDuration(MAX_EVENT_BRIGHTNESS);
  
  // Start the Ethernet connection and the server
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  server.begin();
 
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());

  #if SERVO
  event_servo.attach(SERVO_SIG_PIN); 
  #endif
}

void handleWebRequests ()
{
  // Listen for incoming clients
  client = server.available();
  if (client) {
    if (DEBUG_HTTP_REQUESTS)
      Serial.println("Client not null");
    String currentLine = "";
    while (client.connected()) {
      
      if (client.available()) {
        
        char c = client.read();
        if (DEBUG_HTTP_REQUESTS)
          Serial.write(c); // Print it to the Serial monitor
        if (c == '\n') { // New line character
          if (currentLine.length() == 0) {
            // HTTP request has ended, so send a response:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println(); // Required blank line before response body
            printStatusToClient();
            
            client.println("OK");
            break; // Exit the while loop
          } else {
            // Process the current line
            if (currentLine.startsWith("GET /R"))
                triggerEvent();
            else if (currentLine.startsWith("GET /D"))
                triggerDetEvent();
            else if (currentLine.startsWith("GET /S")) {
              int startPos = currentLine.indexOf("/S") + 2; // Find the start position of the speed value
              int endPos = currentLine.indexOf(" ", startPos); // Find the end of the speed value
              String speedStr = currentLine.substring(startPos, endPos); // Extract the speed value as a string
              float newSpeed = speedStr.toFloat(); // Convert the string to a float

              setSpeed(newSpeed);
            }
            else if (currentLine.startsWith("GET /E")) {
           
              int startPos = currentLine.indexOf("/E") + 2; // Find the start position of the event_duration value
              int endPos = currentLine.indexOf(" ", startPos); // Find the end of the event_duration value
              String durationStr = currentLine.substring(startPos, endPos); // Extract the event_duration value as a string
              unsigned long newEventDuration = durationStr.toInt(); // Convert the string to an unsigned long

              setEventDuration(newEventDuration);
            }
            else if (currentLine.startsWith("GET /I")) {
                int startPos = currentLine.indexOf("/I") + 2; // Find the start position of the seconds value
                int endPos = currentLine.indexOf(" ", startPos); // Find the end of the seconds value
                String secondsStr = currentLine.substring(startPos, endPos); // Extract the seconds value as a string
                unsigned long newSeconds = secondsStr.toInt(); // Convert the string to an unsigned long

                setTime(newSeconds);
            }else if (currentLine.startsWith("GET /B")) {
                int startPos = currentLine.indexOf("/B") + 2; // Find the start position of the seconds value
                int endPos = currentLine.indexOf(" ", startPos); // Find the end of the seconds value
                String secondsStr = currentLine.substring(startPos, endPos); // Extract the seconds value as a string
                unsigned long newValue = secondsStr.toInt(); // Convert the string to an unsigned long

                setMaxBrightness(newValue);
            }
            currentLine = ""; // Clear current line
          }
        } else if (c != '\r') {
          // Add character to current line
          currentLine += c;
        }
      }
    }
    // Close the connection:
    client.stop();
  }
}

void printStatusToClient()
{
  client.print("Time: ");
  client.println(seconds);
  client.print("Speed: ");
  client.println(speed);
  client.print("Event duration: ");
  client.println(event_duration);
  client.print("Event max brightness: ");
  client.println(max_brightness);

  if (eventTriggered)
    client.println("Event in progress");
    
  if (eventDetTriggered)
    client.println("Detection event in progress");
  
  if (Ethernet.linkStatus() == Unknown) {
    client.println("ETH Link status unknown. Link status detection is only available with W5200 and W5500.");
  }
  else if (Ethernet.linkStatus() == LinkON) {
    client.println("ETH Link status: On");
  }
  else if (Ethernet.linkStatus() == LinkOFF)
  {
    client.println("ETH Link status: Off");
  }
}

void printStatus()
{
  Serial.print("Time: ");
  Serial.println(seconds);
  Serial.print("Speed: ");
  Serial.println(speed);
  Serial.print("Event duration: ");
  Serial.println(event_duration);
  Serial.print("Event max brightness: ");
  Serial.println(max_brightness);

  if (eventTriggered)
    Serial.println("Event in progress");
    
  if (eventDetTriggered)
    Serial.println("Detection event in progress");
  
  if (Ethernet.linkStatus() == Unknown) {
    Serial.println("ETH Link status unknown. Link status detection is only available with W5200 and W5500.");
  }
  else if (Ethernet.linkStatus() == LinkON) {
    Serial.println("ETH Link status: On");
  }
  else if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("ETH Link status: Off");
  }
}
void setEventDuration (unsigned long newEventDuration){
  if (newEventDuration != event_duration) {
          event_duration = newEventDuration;
          EEPROM.put(addrEventDuration, event_duration); // Save new event_duration to EEPROM
          Serial.print("Event duration is set to:");
          Serial.println(event_duration);
        }else
          Serial.println("Event duration not changed");

}
void setSpeed(unsigned long newSpeed){
if (newSpeed != speed) { // Check if the speed has changed
          speed = newSpeed;
          EEPROM.put(addrSpeed, speed); // Save new speed to EEPROM
          Serial.print("New speed is set to:");
          Serial.println(speed);
        }else
          Serial.println("Speed not changed");

}

void setTime(unsigned long newSeconds){

if (newSeconds != seconds) { // Check if the seconds have changed
          seconds = newSeconds;
          EEPROM.put(addrSeconds, seconds); // Save new seconds to EEPROM
          Serial.print("Time is set to:");
          Serial.println(seconds);
        }else
          Serial.println("Time not changed");  
}

void setMaxBrightness(unsigned long newBrightness)
{
  if (newBrightness != max_brightness) 
  { // Check if the seconds have changed
    max_brightness = newBrightness;
    if (max_brightness>255)
      max_brightness = 255;

    if (max_brightness<0)
      max_brightness=0;

    EEPROM.put(addrBrightness, max_brightness); // Save new seconds to EEPROM
    Serial.print("Brightness is set to:");
    Serial.println(max_brightness);
  }else
    Serial.println("Brightness not changed");
}

void countSeconds()
{

  // Check if a second has passed, considering the speed factor
  if ( currentMillis - previousMillis >= (1000000.0f / speed )) {
    previousMillis = currentMillis;
    seconds++; // Increment the seconds counter based on the speed

    // Reset seconds to 0 after reaching the end of the day (END_OF_DAY seconds)
    if (seconds >= END_OF_DAY) {
      seconds = 0;
    }
  }
}

void handleSkyLeds()
{
  
  // LED dimming and control logic
  // Sunrise dimming logic
  if (seconds >= DAY_SUNRISE_START && seconds <= DAY_SUNRISE_STOP) {
    float progress = (float)(seconds - DAY_SUNRISE_START) / (DAY_SUNRISE_STOP - DAY_SUNRISE_START);
    brightness = (int)(progress * 255); // Scale progress to PWM range (0-255)
  }
  // Daytime logic
  else if (seconds > DAY_SUNRISE_STOP && seconds < DAY_SUNSET_START) {
    brightness = 255; // Full brightness
  }
  // Sunset dimming logic
  else if (seconds >= DAY_SUNSET_START && seconds <= DAY_SUNSET_STOP) {
    float progress = (float)(seconds - DAY_SUNSET_START) / (DAY_SUNSET_STOP - DAY_SUNSET_START);
    brightness = 255 - (int)(progress * 255); // Scale progress to PWM range (0-255)
  }
  // Nighttime logic
  else {
    brightness = 0; // LED off
  }

  // Additional LED control logic for the stars LED
  if ((seconds >= 0 && seconds <= DAY_SUNRISE_STOP) || (seconds >= DAY_SUNSET_START && seconds < END_OF_DAY)) {
    digitalWrite(STARS_LED_PIN, HIGH); // Turn the stars LED ON
  } else {
    digitalWrite(STARS_LED_PIN, LOW); // Turn the stars LED OFF
  }

  analogWrite(SUN_LED_PIN, brightness);
}

void handleSerialCommand()
{
   // Check for new speed value from Serial
  if (Serial.available() > 0)
  {
      String input = Serial.readStringUntil('\n');
      if (input.startsWith("R"))
      {
        triggerEvent();
      }
      else if (input.startsWith("D"))
      {
        triggerDetEvent();
      } 
      else if (input.startsWith("E"))
      {
        int newEventDuration = input.substring(1).toInt();
        setEventDuration(newEventDuration);
      }
      else if (input.startsWith("S"))
      {
        float newSpeed = input.substring(1).toFloat();
        setSpeed(newSpeed);
      } else if (input.startsWith("I"))
      {
        unsigned long newSeconds = input.substring(1).toInt();
        setTime(newSeconds);
      }
      else if (input.startsWith("H")) 
      {
        printStatus();
      }
      else if (input.startsWith("B")) 
      {
        unsigned long newBrightness = input.substring(1).toInt();
        setMaxBrightness(newBrightness);
      }
      else
      {
         Serial.println("Command not recognized");
      }
  }
}

#if SERVO
const int STEP = 10;
int currentPosition = SERVO_START_POSITION;  // Partiamo da posizione centrale

void handleServo(){
  if (Serial.available() > 0) {
    char comando = Serial.read();

    if (comando == '+') {
     
      currentPosition += STEP;
      if (currentPosition > 180) currentPosition = 180;
      event_servo.write(currentPosition);
      Serial.print("→ Posizione aumentata: ");
      Serial.println(currentPosition);
     
    }
    else if (comando == '-') {

      currentPosition -= STEP;
      if (currentPosition < 0) currentPosition = 0;
      event_servo.write(currentPosition);
      Serial.print("← Posizione diminuita: ");
      Serial.println(currentPosition);
    }
    else {
      Serial.print("Comando non valido: ");
      Serial.println(comando);
    }
  }
}
 #endif
void loop()
{
  currentMillis = millis();
  
  countSeconds();

  handleSerialCommand();
  handleWebRequests();
  

  handleSkyLeds();
  handleEventLED(); // Call the function to handle the event LED logic
  handleDetEventLED(); // Call the function to handle the event LED logic
  handleServo();
}
