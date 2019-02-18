#include "AstroMech.h"
#include "Adafruit_MCP3008.h"

#define SS 15
#define ANALOG_PIN 0
#define DEBUG true

Adafruit_MCP3008 adc;
AstroMech::AstroMech *astroMech;

int analogReadFP(uint8_t pin){
  return adc.readADC(pin);
}

void setup(){
  Serial.begin(115200);
  while (!Serial);

  astroMech = new AstroMech::AstroMech(analogReadFP, ANALOG_PIN, DEBUG);
  adc.begin(SS);
}

void loop(){
  astroMech->detect();
}
