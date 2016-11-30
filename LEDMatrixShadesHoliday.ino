// LED Matrix Shades Holiday Edition
// Designed for 8MHz 3.3V ATMega328P
// Garrett Mace
// 2016 macetech LLC

// While turning on the Shades, hold down the buttons as below to enable different pattern sets:
// * Hold down both buttons: Normal mode
// * Hold down front button (SW1): New Year mode
// * Hold down rear button (SW2): Christmas mode
// Modes are saved in EEPROM across power cycles

#include <Wire.h>           // I2C library
#include <EEPROM.h>
#include "lib8tion.h"       // lib8tion adapted from FastLED https://github.com/FastLED/FastLED Copyright FastLED MIT License
#include <avr/pgmspace.h>   // Flash storage of long strings and fonts

// Initial settings
#define CYCLETIME 15000
#define startbrightness     153


// Time after changing settings before settings are saved to EEPROM
#define EEPROMDELAY 1500

// Global variables
unsigned long currentMillis = 0;
unsigned long cycleMillis = 0;
unsigned long effectMillis = 0;
unsigned long eepromMillis = 0;
boolean autoCycle = true;
byte currentEffect = 0;
byte currentBrightness = startbrightness;
byte runMode = 0;
boolean eepromOutdated = false;

// Data tables stored in flash memory
#include "messages.h"          // Message strings
#include "mathtables.h"        // Useful tables of values
#include "pcbiosfont.h"        // 8x8 bitmap font
#include "graphicframes.h"     // bitmap graphics

// Helper functions for AS1130 interfacing and graphics generation
#include "AS1130functions.h"   // Generic AS1130 functions
#include "glassesfunctions.h"  // Glasses-specifc functions
#include "FireworksXY.h"
#include "effects.h"   // Graphical patterns
#include "buttons.h"           // button handler


// List of effects that will be displayed

// Normal patterns
functionList effectListOne[] = {
                              beatingHearts,
                              sparkles,
                              dualPlasma,
                              fakeEQ,
                              blockyNoise,
                              Plasma,
                              scrollingHearts,
                              rain,
                              rider,
                              sines,
                              slantBars
};

// Christmas patterns
functionList effectListTwo[] = {
                              messageOne,
                              sparkles,
                              spinGrayscale,
                              scrollingHearts,
                              messageTwo,
                              dualPlasma,
                              blockyNoise,
                              Plasma,
                              rain,
                              messageThree,
                              rider,
                              sines,
                              slantBars
};

// New Year patterns
functionList effectListThree[] = {
                              messageFour,
                              sparkles,
                              rampStrober,
                              scrollingHearts,
                              vertimessageFive,
                              fireworks,
                              beatingHearts,
                              dualPlasma,
                              fakeEQ,
                              blockyNoise,
                              Plasma,
                              rain,
                              messageFive,
                              rider,
                              sines,
                              slantBars
};

byte numEffects;



// Begin program
void setup() {

  Wire.begin();                // Start I2C
  TWBR = 2;                    // Set to 400KHz bus speed (on 8MHz device)

  glassesInit();               // Preconfigure AS1130 chips

  pinMode(MODEBUTTON, INPUT_PULLUP);          // Prepare button input
  pinMode(BRIGHTNESSBUTTON, INPUT_PULLUP);    // Prepare button input

  // check to see if EEPROM has been used yet
  // if so, load the stored settings
  byte eepromWasWritten = EEPROM.read(0);
  if (eepromWasWritten == 99) {
    currentEffect = EEPROM.read(1);
    autoCycle = EEPROM.read(2);
    currentBrightness = EEPROM.read(3);
    runMode = EEPROM.read(4);
  }

  if (runMode > 3) {
    runMode = 0;
    saveEEPROMvals();
  }

  if (getStartupButtons() != 0b11) {
    runMode = getStartupButtons();
    saveEEPROMvals();
  }
  
  // initialize effect count
  switch(runMode) {
    case 0: // normal patterns
      numEffects = (sizeof(effectListOne)/sizeof(effectListOne[0]));
      break;
    
    case 1: // Christmas patterns
      numEffects = (sizeof(effectListTwo)/sizeof(effectListTwo[0]));
      break;
    
    case 2: // New Year patterns
      numEffects = (sizeof(effectListThree)/sizeof(effectListThree[0]));
      break;
  }
  
}



// Main loop
void loop() 
{
  currentMillis = millis();
  updateButtons();
  doButtons();

  checkEEPROM();            // update the EEPROM if necessary


  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > CYCLETIME && autoCycle == true) {
    cycleMillis = currentMillis; 
    if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
    effectInit = false; // trigger effect initialization when new effect is selected
  }
  
  // run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;

    switch(runMode) {
      case 0:
        effectListOne[currentEffect]();
        break;
        
      case 1:
        effectListTwo[currentEffect]();
        break;

      case 2:
        effectListThree[currentEffect]();
        break;
    }
    
    random16_add_entropy(1); // make the random values a bit more random-ish
  }
}
