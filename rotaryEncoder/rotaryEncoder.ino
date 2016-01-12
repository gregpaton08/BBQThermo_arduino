// usually the rotary encoders three pins have the ground pin in the middle
enum PinAssignments {
  encoderPinA = 2,   // rigth
  encoderPinB = 3,   // left
  clearButton = 8    // another two pins
};

volatile unsigned int newTemperature = 255;         // a counter for the dial
unsigned int currentTemperature = newTemperature;   // change management
static boolean rotating = false;                    // debounce management

// interrupt service routine vars
boolean A_set = false;              
boolean B_set = false;


void setup() {

  pinMode(encoderPinA, INPUT); 
  pinMode(encoderPinB, INPUT); 
  pinMode(clearButton, INPUT);
 // turn on pullup resistors
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);
  digitalWrite(clearButton, HIGH);

// encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);
// encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);

  Serial.begin(9600);  // output
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() { 
  rotating = true;  // reset the debouncer

  if (newTemperature != currentTemperature) {
    Serial.println(newTemperature, DEC);
    currentTemperature = newTemperature;
  }
  
  if (digitalRead(clearButton) == LOW)  {
    newTemperature = 5;
  }
}

// Interrupt on A changing state
void doEncoderA() {
  // debounce
  // wait a little until the bouncing is done
  if (rotating) {
    delay (1);  
  }

  // Test transition, did things really change? 
  if (digitalRead(encoderPinA) != A_set) {  // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if (A_set && !B_set)
      if (newTemperature < 500) {
        newTemperature += 5;
      }

    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB() {
  if ( rotating ) {
    delay (1);
  }
  
  if (digitalRead(encoderPinB) != B_set) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if (B_set && !A_set) {
      if (newTemperature > 5) {
        newTemperature -= 5;
      }
    }

    rotating = false;
  }
}
