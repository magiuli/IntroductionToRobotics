/*
  Homework 5: Stopwatch Timer

  This assignment involved the creation of a stopwatch timer utilizing a 4-digit 7-segment display along with three distinct buttons.
  The timer measures in tenths of seconds, indicated by a decimal point before the last digit, and includes a functionality to save laps. When initiated, 
  the timer displays all digits, showcasing "000.0" at the outset.

  It allows for the storage of up to four laps. In lap viewing mode, continously pressing of the Save Lap button cycles through the saved laps.

  Each button holds a unique functionality:

  - Button 1: Start/Pause
  - Button 2: Save Lap (in counting mode) / Cycle through saved laps (in lap viewing mode)
  - Button 3: Reset Time (in pause mode) / Reset saved laps (in lap viewing mode)

  Notably, not all buttons are available at all times. The reset button remains inaccessible during counting mode, while the Start/Pause button is unavailable 
  in lap viewing mode, only accessible post resetting the timer. Upon entering lap viewing mode, counting mode becomes accessible solely after resetting the saved laps.

  The circuit:
    a) Input:
      - Button 1: 2
      - Button 2: 3
      - Button 3: 4

    b) Output:
      - shift register clock pin: 8
      - shift register latch pin: 9
      - shift register data pin: 10
      - display digit 1: 11
      - display digit 2: 12
      - display digit 3: 13
      - display digit 4: 5

  Created 14 November 2023
  by magiuli
*/

// Define connections to the shift register
const byte dataPin = 10;  // Connects to DS (data pin) on the shift register
const byte latchPin = 9;  // Connects to STCP (latch pin) on the shift register
const byte clockPin = 8;  // Connects to SHCP (clock pin) on the shift register

// Define connections to the digit control pins for a 4-digit display
const byte displaySegment1 = 11;
const byte displaySegment2 = 12;
const byte displaySegment3 = 13;
const byte displaySegment4 = 5;

// Define connections to the buttons
const byte startPauseButtonPin = 2;
const byte lapButtonPin = 3;
const byte resetButtonPin = 4;

const byte buttonCount = 3;

// Define button identifiers 
const byte startPauseButton = 0;
const byte lapButton = 1;
const byte resetButton = 2;
// Mapping button pins to the button identifiers
const byte buttonPins[buttonCount] = { startPauseButtonPin, lapButtonPin, resetButtonPin };

// The starting index for arrays
const byte startingIndex = 0;

// Store the digits in an array for easy access
const byte displayDigits[] = { displaySegment1, displaySegment2, displaySegment3, displaySegment4 };
const byte displayCount = 4;  // Number of digits in the display

// Define the number of unique encodings (0-9)
const byte encodingsNumber = 10;
// Define byte encodings for the digits 0-9
byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110,  // 9
};

// Define the stopwatch modes
enum stopwatchModes {
  countingMode = 0,
  pauseMode = 1,
  resetCountingMode = 2,
  lapViewingMode = 3,
  resetLapViewingMode = 4
} currentStopwatchMode;

const byte stopwatchModesCount = 5;

// Inspired by the previous assignment, I've chosen to utilize a matrix to track accessible modes from the current mode upon pressing each button (utilising switch and if else statemnts whould have been boring). 
// If a button is currently unavailable, the current mode remains unchanged.
stopwatchModes modesTransitionMatrix[stopwatchModesCount][buttonCount] = {
//   Start/Pause        Lap       Reset
  { pauseMode, countingMode, countingMode },                 // countingMode
  { countingMode, pauseMode, resetCountingMode },            // pausMode
  { countingMode, lapViewingMode, resetCountingMode },       // resetCountingMode 
  { lapViewingMode, lapViewingMode, resetLapViewingMode },   // lapViewingMode
  { countingMode, resetLapViewingMode, resetLapViewingMode } // resetLapViewingMode
};

// Defining variables for manipulating the digits of the displayed number
const byte leastSignifiantDigit = 3;
const byte mostSignifiantDigit = 0;
const byte onesPlace = 2;
const byte radix = 10; // Synonym for base, but sounds cooler
const byte clearEncoding = B00000000;

// Iterative variable for button identifiers
byte thisButton;

// Defining variables for counting time
unsigned int time = 0;
unsigned long int lastTimeUpdate = 0;
unsigned long int timeUpdateDelay = 100;  // 100 milliseconds = 1 10'th of a seccond 

// Define variables for debounce logic and reading button inputs
volatile bool reading[buttonCount] = { HIGH };
volatile bool lastReading[buttonCount] = { HIGH };
volatile bool buttonState[buttonCount] = { HIGH };
volatile bool buttonWasPressed[buttonCount] = { false };
volatile unsigned long int debounceTime[buttonCount];
volatile unsigned long int lastDebounceTime[buttonCount];
const unsigned long int debounceDelay = 100000; // microseconds

// Define variables for cycling laos
unsigned long int lastLapChange;
const unsigned int lapChangeDelay = 400; // milliseconds

const byte maxQueueCount = 4;

// Define class used for saving laps
class LapQueue {
private:
  unsigned long int laps[maxQueueCount];
  byte front;
  byte rear;
  byte size;
  byte currentLap;
public:
  LapQueue() {
    front = 0; // first saved alp
    rear = 0; // last saved lap
    size = 0; // number of laps stored
    currentLap = 0; // lap to display
  }

  void addLap(int lap) {
    if (size == maxQueueCount) {
      front = (front + 1) % maxQueueCount;
    } else {
      size++;
    }

    laps[rear] = lap;
    rear = (rear + 1) % maxQueueCount;
  }

  unsigned long int getCurrentLap() {
    byte index = (front + currentLap) % maxQueueCount;
    return laps[index];
  }

// Cycle through laps
  void updateCurrentLap() {
    currentLap = (currentLap + 1) % size;
  }

// Delete laps
  void resetLaps() {
    for (int thisLap = startingIndex; thisLap < maxQueueCount; thisLap++) {
      laps[thisLap] = 0;
    }
  }
} lapsQueue;

// -----------------
byte ledStartPuasePin = 1;
byte ledLapPin = 6;
byte ledResetPin = 7;

byte ledPins[buttonCount] = { ledStartPuasePin, ledLapPin, ledResetPin };
// -----------------

void setup() {
  // Initialize the pins connected to the shift register as outputs
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // Initialize the pins connected to the buttons
  pinMode(startPauseButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(lapButtonPin, INPUT_PULLUP);

  // Attaching interrupts because CTI ;((((((((
  attachInterrupt(digitalPinToInterrupt(startPauseButtonPin), handdleStartPauseButtonPress, CHANGE);
  attachInterrupt(digitalPinToInterrupt(lapButtonPin), handdleLapButtonPress, CHANGE);

  // Initialize digit control pins and set them to LOW (off)
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  for (int i = startingIndex; i < buttonCount; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  currentStopwatchMode = pauseMode;
  // Begin serial communication for debugging purposes
  Serial.begin(9600);
}

void loop() {
  if (currentStopwatchMode == countingMode) {
    updateTime();
  }
  if (currentStopwatchMode == lapViewingMode) {
    cycleLaps();
  }
  updateDisplay();
  updateStopwatchMode();
  handdleResetButtonPress();
  showAvailableButton();
}

void updateTime() {
  // Every tenth of a second the time increments by 1
  if (millis() - lastTimeUpdate > timeUpdateDelay) {
    time++;
    lastTimeUpdate = millis();
  }
  // Wraping the value of time. Not really needed since the programm displays the last 4 digits of the variable 
  time %= 10000;
}

void updateDisplay() {
  if (currentStopwatchMode == lapViewingMode) {
    writeNumber(lapsQueue.getCurrentLap());
  } else {
    writeNumber(time);
  }
}

void updateStopwatchMode() {
  for (thisButton = startingIndex; thisButton < buttonCount; thisButton++) {
    if (buttonWasPressed[thisButton]) {
      currentStopwatchMode = modesTransitionMatrix[currentStopwatchMode][thisButton];
      buttonWasPressed[thisButton] = false;
    }
  }
}

void cycleLaps() {
  if (buttonState[lapButton] == LOW && millis() - lastLapChange > lapChangeDelay) {
    lapsQueue.updateCurrentLap();
    lastLapChange = millis();
  }
}

void writeNumber(int number) {
  int digitToDisplay;
  int encodingToDisplay;

  for (int thisDigit = leastSignifiantDigit; thisDigit >= mostSignifiantDigit; thisDigit--) {
    activateDisplay(thisDigit);
    digitToDisplay = number % radix;
    encodingToDisplay = byteEncodings[digitToDisplay];

    if (thisDigit == onesPlace) { encodingToDisplay++; }

    writeReg(encodingToDisplay);

    number /= radix;
    writeReg(clearEncoding);
  }
}

void activateDisplay(int displayNumber) {
  // Turn off all digit control pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  // Turn on the current digit control pin
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeReg(int digit) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

void handdleResetButtonPress() {
  debounceButton(resetButton);
  if (currentStopwatchMode == resetCountingMode) {
    time = 0;
    return;
  }
  if (currentStopwatchMode == resetLapViewingMode) {
    lapsQueue.resetLaps();
    return;
  }
}

void handdleStartPauseButtonPress() {
  debounceButton(startPauseButton);
}

void handdleLapButtonPress() {
  debounceButton(lapButton);

  if (currentStopwatchMode == countingMode && buttonWasPressed[lapButton]) {
    lapsQueue.addLap(time);
    buttonWasPressed[lapButton] = false;
  }
}

void debounceButton(byte thisButton) {
  reading[thisButton] = digitalRead(buttonPins[thisButton]);

  if (reading[thisButton] != lastReading[thisButton]) {
    debounceTime[thisButton] = micros();
  }

  if (debounceTime[thisButton] - lastDebounceTime[thisButton] > debounceDelay) {
    if (buttonState[thisButton] != reading[thisButton]) {

      if (buttonState[thisButton] == LOW) {
        buttonWasPressed[thisButton] = true;
      }
      buttonState[thisButton] = reading[thisButton];
    }
  }

  lastReading[thisButton] = reading[thisButton];
  lastDebounceTime[thisButton] = debounceTime[thisButton];
}

void showAvailableButton() {
  for (thisButton = startingIndex; thisButton < buttonCount; thisButton++) {
    if (modesTransitionMatrix[currentStopwatchMode][thisButton] != currentStopwatchMode) {
      digitalWrite(ledPins[thisButton], HIGH);
    } else {
      digitalWrite(ledPins[thisButton], LOW);
    }
    if (currentStopwatchMode == countingMode || currentStopwatchMode == lapViewingMode) {
      digitalWrite(ledPins[lapButton], HIGH);
    }
  }
}