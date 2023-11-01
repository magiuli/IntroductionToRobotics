/*
  Homework 3: Elevator simulator wannabe

  This homework assignment involves simulating a 3-floor elevator control system using LEDs, buttons, and a buzzer with an Arduino. Each LED and button represents one of the three floors, with the LED 
  corresponding to the current floor lighting up.

  Additionally, there is a 4th LED that signifies the operational state of the elevator: it blinks when the elevator is in motion and remains off when stationary. When the elevator begins moving, it 
  'closes the doors,' 'travels' to the desired floor, and announces 'the arrival.' Each of these quoted actions has its corresponding sound.

  While the elevator is stationary, pressing the button for the current floor results in no action. However, pressing a button while the elevator is in motion places the requested floor in a queue for following action.

  The circuit:
    a) Input:
      - button 1: 13
      - button 2: 10
      - button 3: 7

    b) Output
      - LED 1: 11
      - LED 2: 8
      - LED 3: 5
      - state LED: 4
      - buzzer: 3

  Created 1 November 2023
  By magiuli
*/

// Frequences for the sound of each action
#define CLOSING_DOORS_SOUND 100
#define MOVING_SOUND 262
#define ARRIVING_SOUND 650
// Values used for modifying  the current floor
#define UP 1
#define DOWN -1

const byte FLOORS_NUMBER = 3;

const byte FLOOR_INPUT_PIN[FLOORS_NUMBER] = { 13, 10, 7 };
const byte FLOOR_OUTPUT_PIN[FLOORS_NUMBER] = { 11, 8, 5 };

const byte BUZZER_PIN = 3;
const byte STATUS_LED = 4;

const unsigned short int FLOOR_TRANSITION_DELAY = 4000;
const unsigned short int LED_STAYS_ON = 2500;
const unsigned short int BLINKING_PERIOD = 1000;
const unsigned short int ANNOUNCEMENT_DURATION = 700;
const unsigned short int SILENCE_DURATION = 500;
const unsigned short int BLINKING_PAUSE = 500;
const unsigned short int SHORT_TONE = 100;
const unsigned short int DEBOUNCE_DELAY = 50;
const unsigned short int ARRIVING_ANNOUNCEMENT_DURATION = ANNOUNCEMENT_DURATION + SILENCE_DURATION;

// Arrays used to implement debouncing for each button
byte button_state[FLOORS_NUMBER];
byte led_state[FLOORS_NUMBER];
byte reading[FLOORS_NUMBER];
byte last_reading[FLOORS_NUMBER];
unsigned long int last_debounce[FLOORS_NUMBER];

// Remembers the last time the elevator changed the floors
unsigned long int last_floor_change = 0;

short int direction = 0;

byte current_floor = 0;
byte target_floor = 0;
//iterative  variable
byte thisPin = 0;

bool is_moving = false;

/*  
  While developing the elevator control system, I decided to implement a custom data structure for managing elevator requests efficiently. 
  In this design, I realized that a queue for storing elevator requests shouldn't contain redundant calls. For example, if a user presses 
  the buttons for floors 2, 3, 1, 3, 1, 3, and 1, it should translate into the elevator visiting these floors in the order 2 -> 3 -> 1. 
  Storing redundant calls is unnecessary.

  The elevatorQueue I designed includes both an integer array for storing elevator calls and a boolean array to track whether a floor is 
  present in the queue. By default, when there are no values in the queue, it stores -1.

  It has 4 methods:
  a) peak: returns true if there is at least one element in the queue, indicating that there are pending elevator requests.
  b) push: adds a floor value to the queue if it is not already present. If a floor is already in the queue or it matches the current target floor, it won't be added.
  c) pop: method retrieves and removes the top value (the first floor in the queue) from the queue.
  d) initialiseElevatorQueue: initializes the elevatorQueue object. It sets the insertIndex to 0, indicating an empty queue.
*/
const short int EMPTY = -1;

typedef struct elevatorQueue {
  int floors[FLOORS_NUMBER];
  int insertIndex;
  bool inQueue[FLOORS_NUMBER];
};

struct elevatorQueue decision_stack;

void setup() {
  // Initialize elevator control system
  for (thisPin = 0; thisPin < FLOORS_NUMBER; thisPin++) {
    pinMode(FLOOR_INPUT_PIN[thisPin], INPUT_PULLUP);
    pinMode(FLOOR_OUTPUT_PIN[thisPin], OUTPUT);
    button_state[thisPin] = HIGH; // Set the initial state of buttons to HIGH
    led_state[thisPin] = LOW; // Set the initial state of LEDs to LOW
  }
  initialiseElevatorQueue(decision_stack); // Initialize the elevator queue
}

void loop() {
  // Check if any button has been pressed
  aButtonWasPressed();
  // Update the state of floor LEDs
  updateFloorsLEDs();
  // Indicate the operational status of the elevator
  indicateStatus();

  // If the elevator is not at the target floor, move to the target
  // If the elevator reaches the target floor announce arrival
  // If the elevator is at the target floor 
  if (current_floor != target_floor) {
    changeFloors();
  } else if (is_moving) {
    announceArrival();
  } else {
    updateTargetFloor();
  }
}

// Function to check if any button has been pressed
void aButtonWasPressed() {
  for (thisPin = 0; thisPin < FLOORS_NUMBER; thisPin++) {
    // Debouncing logic
    reading[thisPin] = digitalRead(FLOOR_INPUT_PIN[thisPin]);

    if (reading[thisPin] != last_reading[thisPin]) {
      last_debounce[thisPin] = millis();
    }

    if (millis() - last_debounce[thisPin] > DEBOUNCE_DELAY) {
      if (button_state[thisPin] != reading[thisPin]) {
        button_state[thisPin] = reading[thisPin];
        if ((button_state[thisPin] == HIGH)) {
          // If a button is pressed (button_state is HIGH), push the floor to the elevator queue
          push(decision_stack, thisPin);
        }
      }
    }
    last_reading[thisPin] = reading[thisPin];
  }
}

// Function to update the state of floor LEDs
void updateFloorsLEDs() {
  for (thisPin = 0; thisPin < FLOORS_NUMBER; thisPin++) {
    if (thisPin == current_floor && is_moving == false) {
      digitalWrite(FLOOR_OUTPUT_PIN[thisPin], HIGH); // Turn on the LED for the current floor
    } else if (thisPin == current_floor && millis() - last_floor_change < LED_STAYS_ON) {
      digitalWrite(FLOOR_OUTPUT_PIN[thisPin], HIGH); // Keep the LED on for a duration
    } else {
      digitalWrite(FLOOR_OUTPUT_PIN[thisPin], LOW); // Turn off the LED for other floors and for the current floor if the elevator is "between" floors 
    }
  }
}

// Function to indicate the operational status of the elevator
void indicateStatus() {
  if (is_moving == false) {
    digitalWrite(STATUS_LED, LOW); // Turn off the status LED when the elevator is stationary
  } else if (millis() % BLINKING_PERIOD < BLINKING_PAUSE) {
    digitalWrite(STATUS_LED, LOW); // Turn off the status LED during the blinking pause
  } else {
    digitalWrite(STATUS_LED, HIGH); // Blink the status LED when the elevator is in motion
  }
}

// Function to handle elevator floor transitions
void changeFloors() {
  direction = target_floor > current_floor ? UP : DOWN; // Determine the direction of movement

  // Handle various actions during elevator floor transition
  if (millis() - last_floor_change < ANNOUNCEMENT_DURATION && is_moving == false) {
    // Play closing doors sound
    tone(BUZZER_PIN, CLOSING_DOORS_SOUND, SHORT_TONE); 
  } else if (millis() - last_floor_change < ANNOUNCEMENT_DURATION + SILENCE_DURATION && is_moving == false) {
    // Make a short pause
    noTone(BUZZER_PIN);
  } else if (millis() - last_floor_change < FLOOR_TRANSITION_DELAY) {
    // The elevator starts "moving" after the doors are "closed"
    is_moving = true;
    tone(BUZZER_PIN, MOVING_SOUND, SHORT_TONE);
  } else {
    // Update the current floor
    current_floor += direction;
    last_floor_change = millis();
  }
}

// Function to announce the arrival at a floor
void announceArrival() {
  // After a short pause the elevator announces its arrival and stops moving
  if (millis() - last_floor_change > SILENCE_DURATION) {
    is_moving = false;
    tone(BUZZER_PIN, ARRIVING_SOUND, ANNOUNCEMENT_DURATION);
  }
}

// Function to update the next floor in the elevator queue
void updateTargetFloor() {
  // After the arriving announcement was made the elevator pauses and then checks of there are requests in the queue
  if (millis() - last_floor_change > ARRIVING_ANNOUNCEMENT_DURATION + SILENCE_DURATION) {
    if (peak(decision_stack)) {
      target_floor = pop(decision_stack);
      last_floor_change = millis();
    }
  }
}

// Function to initialize the elevator queue
void initialiseElevatorQueue(struct elevatorQueue &q) {
  q.insertIndex = 0;
  for (int i = 0; i < FLOORS_NUMBER; i++) {
    q.floors[i] = EMPTY;
    q.inQueue[i] = false;
  }
}

// Function to check if the elevator queue is not empty
bool peak(struct elevatorQueue &q) {
  if (q.floors[0] == EMPTY)
    return false;
  else
    return true;
}

// Function to add a floor to the elevator queue
void push(struct elevatorQueue &q, int floor) {
  // If the target floor is, for example, 2 the elevator shoudln't add 2 in the queue
  if (q.inQueue[floor] == false && floor != target_floor) {
    q.floors[q.insertIndex++] = floor; // Add the floor to the queue
    q.inQueue[floor] = true; // Mark the floor as present in the queue
  }
}

// Function to retrieve and remove the top floor from the elevator queue
int pop(struct elevatorQueue &q) {
  int front = q.floors[0];
  for (int i = 0; i < q.insertIndex; i++) {
    q.floors[i] = q.floors[i + 1]; // Shift elements in the queue to remove the top floor
  }
  q.floors[--q.insertIndex] = EMPTY;
  q.inQueue[front] = false;

  return front;
}