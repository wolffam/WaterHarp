/*
  Strobe
  Turns on an LED on for one mS per second

  This simply pulses a pin (TRIG_PIN) active high.

  The pin should be wired to the XY-MOS driver board's "trig" pin.
  The rest of the board should be wired as follows:
    VIN+ = +12-24v (power supply)
    VIN- = GND (power supply)
    OUT+ = LED strip (+)
    OUT- = LED strip (-)

  by Rolf Widenfelt
*/


#define TRIG_PIN 5


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(TRIG_PIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(TRIG_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1);
  digitalWrite(TRIG_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(20);                     // wait for a second
}

