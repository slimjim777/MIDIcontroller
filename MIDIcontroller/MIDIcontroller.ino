/*
 * Five button midi controller for the HX Stomp
 * 
 */
#include <Adafruit_NeoPixel.h>

// CC command on channel 1
const byte cmdCC01 = 0xB0;
#define SWITCH1 11
#define SWITCH2 12
#define SWITCH3 13
#define SWITCH4 14
#define SWITCH5 15

#define DEBOUNCE 10  // how many ms to debounce, 5+ ms is usually plenty
#define DOUBLECLICK 300  // how many ms for double-press detect

// How many NeoPixels are connected
#define LEDPIN    5
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

const uint32_t led1 = pixels.Color(200, 200, 200);
const uint32_t led2 = pixels.Color(255, 0, 0);
const uint32_t led3 = pixels.Color(200, 200, 200);
const uint32_t led4 = pixels.Color(255, 0, 0);
const uint32_t led5 = pixels.Color(200, 200, 200);
uint32_t leds[] = {led1, led2, led3, led4, led5};
 
//define the buttons and LEDs
byte buttons[] = {A0, A1, A2, A3, A4};
 
//determine how big the array up above is, by checking the size
#define NUMBUTTONS sizeof(buttons)
 
//track if a button is just pressed, just released, or 'currently pressed' 
byte pressed[NUMBUTTONS], justpressed[NUMBUTTONS], justreleased[NUMBUTTONS];
byte previous_keystate[NUMBUTTONS], current_keystate[NUMBUTTONS];
int stomp[NUMBUTTONS];
long lastpress[NUMBUTTONS];
 
void setup() {
  byte i;
  // Set up serial ports
  Serial.begin(9600);
  Serial1.begin(31250);

  Serial.print("Button checker with ");
  Serial.print(NUMBUTTONS, DEC);
  Serial.println(" buttons");

  // Initialise the LED
  pixels.begin();
  
  // Make input & enable pull-up resistors on switch pins
  for (i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    digitalWrite(buttons[i], LOW);
    lastpress[i] = 0;
    stomp[i] = 127;
  }

  // Initialize the LEDs§
  pixels.setBrightness(32);
  updateLEDs();

  // Initialize the Stomp
  updateStomp();
}
 
void loop() {
  byte thisSwitch=thisSwitch_justPressed();
  switch(thisSwitch)
  {  
  case 0: 
    Serial.println("switch 1 just pressed");
    buttonPress(thisSwitch, SWITCH1);
    break;
    
  case 1: 
    Serial.println("switch 2 just pressed");
    buttonPress(thisSwitch, SWITCH2);
    break;
  case 2: 
    Serial.println("switch 3 just pressed");
    buttonPress(thisSwitch, SWITCH3);
    break;
  case 3: 
    Serial.println("switch 4 just pressed");
    buttonPress(thisSwitch, SWITCH4);
    break;
  case 4: 
    Serial.println("switch 5 just pressed");
    buttonPress(thisSwitch, SWITCH5);
    break;   
  }
}

// Update the stomp state to match the controller
void updateStomp() {
  sendCommand(cmdCC01, SWITCH1, stomp[0]);
  sendCommand(cmdCC01, SWITCH2, stomp[1]);
  sendCommand(cmdCC01, SWITCH3, stomp[2]);
  sendCommand(cmdCC01, SWITCH4, stomp[3]);
  sendCommand(cmdCC01, SWITCH5, stomp[4]);
}
//64320
void updateLEDs() {
  if (stomp[0] == 127) {
    pixels.setPixelColor(0, leds[0]);
  } else {
    pixels.setPixelColor(0, 0);
  }
  if (stomp[1] == 127) {
    pixels.setPixelColor(2, leds[1]);
  } else {
    pixels.setPixelColor(2, 0);
  }  
  if (stomp[2] == 127) {
    pixels.setPixelColor(3, leds[2]);
  } else {
    pixels.setPixelColor(3, 0);
  }  
  if (stomp[3] == 127) {
    pixels.setPixelColor(4, leds[3]);
  } else {
    pixels.setPixelColor(4, 0);
  }  
  if (stomp[4] == 127) {
    pixels.setPixelColor(6, leds[4]);
  } else {
    pixels.setPixelColor(6, 0);
  }   

  pixels.show();
}

void buttonPress(byte thisSwitch, byte data1) {
    // Send the on/off command
    if (stomp[thisSwitch]==127) {
      stomp[thisSwitch] = 0;
      sendCommand(cmdCC01, data1, 0);
    } else {
      stomp[thisSwitch] = 127;
      sendCommand(cmdCC01, data1, 127);
    }

    // If double-press, send all buttons state
    if ( (lastpress[thisSwitch] + DOUBLECLICK) > millis() ) {
      Serial.println("Double press");
      updateStomp();
    }

    // Record the last press time
    lastpress[thisSwitch] = millis();
}

void sendCommand(byte cmd, byte data1, byte data2) {
    Serial1.write(cmd);
    Serial1.write(data1);
    Serial1.write(data2);
 
     //prints the values in the serial monitor so we can see what note we're playing
    Serial.print("cmd: ");
    Serial.print(cmd);
    Serial.print(", data1: ");
    Serial.print(data1);
    Serial.print(", data2: ");
    Serial.println(data2);

    updateLEDs();
}
 
void check_switches()
{
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime;
  byte index;
  if (millis() < lasttime) {
    // we wrapped around, lets just try again
    lasttime = millis();
  }
  if ((lasttime + DEBOUNCE) > millis()) {
    // not enough time has passed to debounce
    return; 
  }
  
  // ok we have waited DEBOUNCE milliseconds, lets reset the timer
  lasttime = millis();
  for (index = 0; index < NUMBUTTONS; index++) {
    justpressed[index] = 0;       //when we start, we clear out the "just" indicators
    justreleased[index] = 0;
    currentstate[index] = digitalRead(buttons[index]);   //read the button
    if (currentstate[index] == previousstate[index]) {
      if ((pressed[index] == LOW) && (currentstate[index] == LOW)) {
        // just pressed
        justpressed[index] = 1;
      }
      else if ((pressed[index] == HIGH) && (currentstate[index] == HIGH)) {
        justreleased[index] = 1; // just released
      }
      pressed[index] = !currentstate[index];  //remember, digital HIGH means NOT pressed
    }
    previousstate[index] = currentstate[index]; //keep a running tally of the buttons
  }
}
 
byte thisSwitch_justPressed() {
  byte thisSwitch = 255;
  check_switches();  //check the switches &amp; get the current state
  for (byte i = 0; i < NUMBUTTONS; i++) {
    current_keystate[i]=justpressed[i];
    if (current_keystate[i] != previous_keystate[i]) {
      if (current_keystate[i]) thisSwitch=i;
    }
    previous_keystate[i]=current_keystate[i];
  }  
  return thisSwitch;
}
