/*
 Homework 4: 7 segment display drawing


  This homework assignment involves controlling a 7-segment display using a joystick. The joystick allows you to move the active segment and "draw" on the display. The movement between segments is restricted to natural transitions, meaning the segment can only jump to neighboring positions without passing through "walls."

  Here are the key functionalities of the assignment:

   - The initial position of the segment is at the decimal point (DP).
   - The current segment always blinks, regardless of its state (on or off).
   - Use the joystick to move the current position to neighboring segments, based on a predefined table of corresponding movements.
   - A short press of the joystick button toggles the state of the current segment from ON to OFF or from OFF to ON.
   - A long press of the joystick button resets the entire display by turning off all the segments and moving the current position back to the decimal point (DP).

  The circuit:
    a) Input:
      - joystick x value: A0
      - joystick y value: A1
      - joystick button: 2

    b) Output
      - segment a: 7
      - segment b: 6
      - segment c: 12
      - segment d: 8
      - segment e: 9
      - segment f: 4
      - segment g: 5
      - decimal point: 10
      - buzzer: 11

  Created 8 November 2023
  By magiuli
*/

// Musical notes used for buzzer
#define RESET_NOTE 150
#define MOVEMENT_NOTE 400
#define TOGGLE_OFF_NOTE 600
#define TOGGLE_ON_NOTE 800

// Make it true if your 7 segments display has common annode
const bool commonAnnode = false;

// Pins used for the 7 segments display
const byte pinSegmentA = 7;
const byte pinSegmentB = 6;
const byte pinSegmentC = 12;
const byte pinSegmentD = 8;
const byte pinSegmentE = 9;
const byte pinSegmentF = 4;
const byte pinSegmentG = 5;
const byte pinDecimalPoint = 10;

// Pins used for joystick and buzzer
const byte buzzerPin = 11;
const byte joystickXPin = A0;
const byte joystickYPin = A1;
const byte joystickButtonPin = 2;

// Even 0 could be considered a magic number ;)
const byte arrayStartingIndex = 0;

// Mapping pins to segment numbers for ease of use
const byte segmentA = 0;
const byte segmentB = 1;
const byte segmentC = 2;
const byte segmentD = 3;
const byte segmentE = 4;
const byte segmentF = 5;
const byte segmentG = 6;
const byte decimalPoint = 7;

const byte numberOfSegments = 8;
const byte segmentsPins[numberOfSegments] = { pinSegmentA, pinSegmentB, pinSegmentC, pinSegmentD, pinSegmentE, pinSegmentF, pinSegmentG, pinDecimalPoint };


const byte numberOfDirections = 4;

// Suggestion made by chat gpt after asking it to review my coding style
// Before this I used a byte variable for each direction
enum JoystickDirection {
  up = 0,
  down = 1,
  left = 2,
  right = 3
} joystickDirection;

// Array the stores the state of each segment
bool segmentState[numberOfSegments];

// Array of possible moves as provided in the assignemnt
// Instead of using 'notAvailable' in the joystickMovement matrix, I used the index of the current segment for cleaner code.
// The matrix defines the allowed transitions between segments in response to joystick directions.
// Rows represent the current segment, and columns represent joystick directions.
const byte joystickMovement[numberOfSegments][numberOfDirections]{
  //  UP        DOWN      LEFT      RIGHT
  { segmentA, segmentG, segmentF, segmentB },             // segmentA
  { segmentA, segmentG, segmentF, segmentB },             // segmentB
  { segmentG, segmentD, segmentE, decimalPoint },         // segmentC
  { segmentG, segmentD, segmentE, segmentC },             // segmentD
  { segmentG, segmentD, segmentE, segmentC },             // segmentE
  { segmentA, segmentG, segmentF, segmentB },             // segmentF
  { segmentA, segmentD, segmentG, segmentG },             // segmentG
  { decimalPoint, decimalPoint, segmentC, decimalPoint }  // decimalPoint
};

byte joystickButtonState = HIGH;
bool segmentShouldMove = false;
bool joystickIsInResetRange = true;
// The segment will change when the joystick exceeds a certain threshold
const short int lowerLimit = 480;
const short int upperLimit = 544;
// The x and y values when the joystick is in rest position
const short int joystickXBaseValue = 520;
const short int joystickYBaseValue = 531;
short unsigned int joystickXValue;
short unsigned int joystickYValue;
// Values used for making the segment blink
short unsigned int blinkDuration = 1000;
short unsigned int blinkPause = 500;

byte currentSegment = decimalPoint;
short int thisSegment = 0;

// Since there are common annnodes and common catodes displays, wee cant use directly high
byte onState = HIGH;
volatile bool toggleSegmentState = false;

volatile bool displayShouldReset = false;

// Variables used for debouncing
volatile byte reading;
volatile byte lastReading;
volatile unsigned long int interruptTime;
volatile unsigned long int lastInterruptTime;
volatile unsigned long int lastResetTime = 0;

const unsigned long int debounceDelay = 50000;

// Values used for the audio feedback
const unsigned long int resetDelay = 2000000;
unsigned long int lastToggleTime = 10000000;  // if lastToggleTime was 0, the toggle sound would start at the beginning of the program. Any big enough number will to the trick
byte lastToggledValue;

const short unsigned int shortSoundDuration = 100;
const short unsigned int shortPause = 100;
const short unsigned int longPause = 500;
const short unsigned int soundDuration = 200;
const short unsigned int microsToMillisRatio = 10000;

void setup() {
  for (int i = arrayStartingIndex; i < numberOfSegments; i++) {
    pinMode(segmentsPins[i], OUTPUT);
    // At the beggining every led should be turned off
    segmentState[i] = !onState;
  }
  pinMode(joystickXPin, INPUT);
  pinMode(joystickYPin, INPUT);
  pinMode(joystickButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(joystickButtonPin), handleButtonPress, CHANGE);

  if (commonAnnode) {
    onState = !onState;
  }
}

void loop() {
  readJoystickMovement();
  updateCurrentSegment();
  blink();
  lightUpSegments();
  audioFeedback();
  if (displayShouldReset) {
    resetSevenSegmentDisplay();
  }
}

void readJoystickMovement() {
  joystickXValue = analogRead(joystickXPin);
  joystickYValue = analogRead(joystickYPin);

  // The segment should move after the joystick returns to the reset position
  // If the joystick's position is within the lower limit and upper limit, it is considered in the reset range both vertically and horizontally.
  joystickIsInResetRange = isInResetRange(joystickXValue) && isInResetRange(joystickYValue);
  if (joystickIsInResetRange) {
    segmentShouldMove = true;
  }

  // If the joystick is moved horizontally more than it is moved vertically, the x-axis is chosen
  if (abs(joystickXValue - joystickXBaseValue) > abs(joystickYValue - joystickYBaseValue)) {
    if (joystickXValue > upperLimit) {
      joystickDirection = right;

    } else if (joystickXValue < lowerLimit) {
      joystickDirection = left;
    }
  } else {
    if (joystickYValue > upperLimit) {
      joystickDirection = down;

    } else if (joystickYValue < lowerLimit) {
      joystickDirection = up;
    }
  }
}

bool isInResetRange(short unsigned int joystickAxis) {
  if (lowerLimit < joystickAxis && joystickAxis < upperLimit) {
    return true;
  }
  return false;
}

void updateCurrentSegment() {
  if (segmentShouldMove && !joystickIsInResetRange) {
    // The currentSegment remains unchanged if it cannot move in the specified joystickDirection due to matrix design
    currentSegment = joystickMovement[currentSegment][joystickDirection];
    segmentShouldMove = false;
    playMovementSound();
  }

  if (toggleSegmentState) {
    segmentState[currentSegment] = !segmentState[currentSegment];
    toggleSegmentState = false;
    // We need to save the last momment a value was toggles and the new value for the audioFeedback
    lastToggleTime = millis();
    lastToggledValue = segmentState[currentSegment];
  }
}

void blink() {
  // The current segment will stay on for blinkDuration - blinkPause seconds, and off for blinkPause seconds
  if (millis() % blinkDuration > blinkPause) {
    digitalWrite(segmentsPins[currentSegment], onState);
  } else {
    digitalWrite(segmentsPins[currentSegment], !onState);
  }
}

void lightUpSegments() {
  // Except for the current segment, which will blink, we want all the other segments to be updated
  if (thisSegment != currentSegment) {
    digitalWrite(segmentsPins[thisSegment], segmentState[thisSegment]);
  }
  // Looks cleaner than an if else statement
  thisSegment = thisSegment < numberOfSegments ? thisSegment + 1 : 0;
}

void handleButtonPress() {
  // Debounce logic
  reading = digitalRead(joystickButtonPin);
  if (reading != lastReading) {
    interruptTime = micros();
  }

  if (interruptTime - lastInterruptTime > debounceDelay) {
    if (joystickButtonState != reading) {
      joystickButtonState = reading;

      if (joystickButtonState == LOW) {
        toggleSegmentState = true;
      } else if (interruptTime - lastInterruptTime > resetDelay) {
        // If the button is pressed long enough the display will reset
        displayShouldReset = true;
        lastResetTime = micros();
      }
    }
  }
  lastReading = reading;
  lastInterruptTime = interruptTime;
}

void resetSevenSegmentDisplay() {
  // All segments are turned off and the current segment goes back to the starting position
  currentSegment = decimalPoint;
  for (int i = arrayStartingIndex; i < numberOfSegments; i++) {
    segmentState[i] = !onState;
  }
  displayShouldReset = false;
}

void audioFeedback() {
  // I'm well aware that this part kinda is a disaster and could be improved
  if (lastToggledValue == onState) {
    if (millis() - lastToggleTime < soundDuration) {
      tone(buzzerPin, TOGGLE_OFF_NOTE, shortSoundDuration);
    } else if (millis() - lastToggleTime < soundDuration + shortPause) {
      noTone(buzzerPin);
    } else if (millis() - lastToggleTime < soundDuration + shortPause + soundDuration) {
      tone(buzzerPin, TOGGLE_ON_NOTE, shortSoundDuration);
    }
  } else {
    if (millis() - lastToggleTime < soundDuration) {
      tone(buzzerPin, TOGGLE_ON_NOTE, shortSoundDuration);
    } else if (millis() - lastToggleTime < soundDuration + shortPause) {
      noTone(buzzerPin);
    } else if (millis() - lastToggleTime < soundDuration + shortPause + soundDuration) {
      tone(buzzerPin, TOGGLE_OFF_NOTE, shortSoundDuration);
    }
  }
  playResetSound();
}

void playMovementSound() {
  tone(buzzerPin, MOVEMENT_NOTE, soundDuration);
}

void playResetSound() {
  if (millis() - lastResetTime / microsToMillisRatio < longPause) {
    tone(buzzerPin, RESET_NOTE, shortSoundDuration);
  }
}