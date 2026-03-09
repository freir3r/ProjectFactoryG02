#include <FastLED.h>

// Configuration
#define LED_PIN       33
#define NUM_LEDS      20
#define BRIGHTNESS    20
#define LED_TYPE      WS2811
#define COLOR_ORDER   GRB
#define START_SWITCH  23 // Renamed for consistency

CRGB leds[NUM_LEDS];

void setup() {
  delay(2000); 

  // Initialize the switch with an internal pull-up resistor
  // This assumes the switch connects pin 23 to Ground when pressed
  pinMode(START_SWITCH, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Ensure LEDs start OFF
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // 1. Wait for the button to be pressed (Logic is LOW when pressed due to PULLUP)
  if (digitalRead(START_SWITCH) == LOW) {
    runColorCycle(10000); // Run for 10,000 milliseconds (10 seconds)
  }
}

void runColorCycle(unsigned long duration) {
  unsigned long startTime = millis();

  // 2. Loop until the current time exceeds the start time + duration
  while (millis() - startTime < duration) {
    
    // Example Animation: Rainbow Cycle (more dynamic than the static dot)
    static uint8_t hue = 0;
    fill_rainbow(leds, NUM_LEDS, hue++, 7);
    FastLED.show();
    
    // Small delay for smooth animation
    delay(20); 
  }

  // 3. Shut off LEDs after the 10 seconds are up
  FastLED.clear();
  FastLED.show();
}