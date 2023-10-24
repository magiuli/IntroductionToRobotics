/*
  Homework 2: RGB LED

  This homework required controlling each channel (Red, Green, Blue) of an RGB LED using individual potentiometers.

  The circuit:
    a) Input:
      - potentiometer 1: A0
      - potentiometer 2: A1
      - potentiometer 3: A2

    b) Output:
      - Red channel: 9
      - Blue channel: 10
      - Green channel: 11

  Created 22 October 2023
  By magiuli
  Edited 24 October 2023
  By magiuli
*/

const byte RED_INPUT_PIN = A0;
const byte GREEN_INPUT_PIN = A1;
const byte BLUE_INPUT_PIN = A2;

const byte RED_OUTPUT_PIN = 9;
const byte GREEN_OUTPUT_PIN = 10;
const byte BLUE_OUTPUT_PIN = 11;

const int MIN_INPUT_VALUE = 0;
const int MAX_INPUT_VALUE = 1023;

const int MIN_OUTPUT_VALUE = 0;
const int MAX_OUTPUT_VALUE = 255;

// Since blue and green were too bright I had to lower their brightness through code
// These values were chosen empirically
const float RED_COLOR_CORRECTION_FACTOR = 1.0;
const float GREEN_COLOR_CORRECTION_FACTOR = 0.3;
const float BLUE_COLOR_CORRECTION_FACTOR = 0.3;

// Because there are moments when the numbers read from the potentiometers flicker between 2 values I have decided to filter the numbers that will
// be stored using the instant_input variables and the FLICKERING_FILTER (it should be noted that some filtering is done automatically and 
// unintentionally by the map() function). Although there were moments when the difference between the oscillating values was 5 or 6, the biggest 
// filter value that doesn't affect our precision in 3, since the resolution of our input lowers 4 times through mapping
const int FLICKERING_FILTER = 3;

int red_instant_input;
int blue_instant_input;
int green_instant_input;

int red_input_value;
int green_input_value;
int blue_input_value;

int red_output_value;
int green_output_value;
int blue_output_value;

void setup() {
  pinMode(RED_INPUT_PIN, INPUT);
  pinMode(GREEN_INPUT_PIN, INPUT);
  pinMode(BLUE_INPUT_PIN, INPUT);

  pinMode(RED_OUTPUT_PIN, OUTPUT);
  pinMode(GREEN_OUTPUT_PIN, OUTPUT);
  pinMode(BLUE_OUTPUT_PIN, OUTPUT);
}

void loop() {
  red_instant_input = analogRead(RED_INPUT_PIN);
  green_instant_input = analogRead(GREEN_INPUT_PIN);
  blue_instant_input = analogRead(BLUE_INPUT_PIN);

  // The abs() function was needed because the flicker could happen both lower or higher than the base value 
  if(abs(red_input_value - red_instant_input) > FLICKERING_FILTER)
    red_input_value = red_instant_input;

  if(abs(green_input_value - green_instant_input) > FLICKERING_FILTER)
    green_input_value = green_instant_input;

  if(abs(blue_input_value - blue_instant_input) > FLICKERING_FILTER)
    blue_input_value = blue_instant_input;

  // Digital pins can output values only up to 255, but analog pins can read values between 0 and 1023. Therefore, we have to map our input to mach the output
  // Due to the color correction factors, the values for green and blue reach only up to 76 
  red_output_value = map(red_input_value, MIN_INPUT_VALUE, MAX_INPUT_VALUE, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE) * RED_COLOR_CORRECTION_FACTOR;
  green_output_value = map(green_input_value, MIN_INPUT_VALUE, MAX_INPUT_VALUE, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE) * GREEN_COLOR_CORRECTION_FACTOR;
  blue_output_value = map(blue_input_value, MIN_INPUT_VALUE, MAX_INPUT_VALUE, MIN_OUTPUT_VALUE, MAX_OUTPUT_VALUE) * BLUE_COLOR_CORRECTION_FACTOR;

  analogWrite(RED_OUTPUT_PIN, red_output_value);
  analogWrite(GREEN_OUTPUT_PIN, green_output_value);
  analogWrite(BLUE_OUTPUT_PIN, blue_output_value);
}
