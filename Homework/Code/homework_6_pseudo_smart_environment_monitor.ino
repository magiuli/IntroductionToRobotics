/*
  Homework 6: Pseudo-smart Environment and Logger

  
  This assignment involved utilizing multiple sensors to gather environmental data, adjusting sensor settings, logging these configurations into EEPROM, 
  and offering visual feedback using an RGB LED. Additionally, user interaction is facilitated through a Start Menu. An ultrasonic sensor (HC-SR04) and a 
  photoresistor are employed for environmental data collection. Specific alerts are displayed on the Serial Monitor when the readings fall below predefined 
  thresholds. If the Automatic LED mode is enabled, the LED indicates an active alert by turning red; otherwise, it remains green.

  Navigation through menus and submenus is accomplished via user input. It's important to note that error handling has been implemented to address invalid 
  or out-of-bounds inputs, ensuring a smoother user experience.

  Menu structure and options:
  1. Sensor Settings
    1.1 Sensor Sampling Interval
    1.2 Ultrasonic Alert Threshold
    1.3 LDR Alert Threshold
    1.4 Back
  2. Reset Logger Data
    1.1 Yes
    1.2 No
  3. System status
    1.1 Current Sensor Readings
    1.2 Current Sensor Settings
    1.3 Back
  4. RGB LED Control
    1.1 Manual Color Control
    1.2 LED: Toggle Automatic ON/OFF
    1.3 Back

  The circuit:
    a) Input:
      - Ultrasonic sensor(HC-SR04) echo: 5
      - LDR : A0

    b) Outputt
      - Ultrasonic sensor(HC-SR04) trigger: 3
      - RGB LED red channel: 11
      - RGB LED green channel: 10
      - RGB LED blue channel: 9

  Created 21 November 2023
  By magiuli
  Edited 26 November 2023
  By magiuli
*/

#include <EEPROM.h>

// Define connections to the sensors
const byte triggerPin = 3;
const byte echoPin = 5;
const byte ldrPin = A0;

// Define connections to the RGB LED
const byte redOutPutPin = 11;
const byte greenOutPutPin = 10;
const byte blueOutPutPin = 9;

// As the individual LEDs possess varying intensities, we must adjust them programmatically.
const float redColorCorrectionFactor = 1.0;
const float greenColorCorrectionFactor = 0.3;
const float blueColorCorrectionFactor = 0.3;

// Declare variables for LED control
bool colorPickingMode = false;
byte currentColor = 0;
const byte red = 0;
const byte green = 1;
const byte blue = 2;

byte colors[3];

// Define thresholds for LED values
const byte minimumIntensity = 0;
const byte maximumIntensity = 255;

// Define EEPROM address for different sensor or LED settings
const byte samplingRateAddress = 0;
const byte ldrSensorThresholdAddress = 4;
const byte ultrasonicSensorThresholdAddress = 8;
const byte redValueAddress = 12;
const byte blueValueAddress = 16;
const byte greenValueAddress = 20;

// Declare variables and tresholds for different settings
short int samplingRate = 1;
const short int lowerSamplaingLimit = 1;
const short int upperSamplingLimit = 10;

short int ultrasonicSensorThreshold = 0;
const short int lowerUltrasonicSensorLimit = 2;    //centimeters
const short int upperUltrasonicSensorLimit = 400;  //centimeters

short int ldrSensorThreshold = 0;
const short int lowerLdrSensorLimit = 0;
const short int upperLdrSensorLimit = 1023;

// Since alerts trigger when the readings fall below the threshold, it's crucial to initialize the sensor readings at their maximum values
int ultrasonicSensorReading = 400;
long duration;

short int ldrSenorReading = 1023;

// Decalre variables for timing
unsigned long int lastReadingTime = 0;
unsigned long int lastDisplayTime = 0;
unsigned short int millisecondsInASecond = 1000;

// Declare varaibles for handling the user input
int incomingNumber = 0;
bool inputWasntUsed = false;

// Declare varaibles for menu navigation
bool menuHasChanged = true;
bool optionWasSelected = false;
int currentOption = 0;

// Declare control variables for different options and alert handling
bool displayCurrentReadingsMode = false;
bool automaticLedMode = false;

bool lightLevelAlertWasDisplayed = false;
bool distanceAlertWasDisplayed = false;

bool lightLevelAlert = false;
bool distanceAlert = false;

// Declare constant for converting the ultrasonic reading to an actual distance
const float durationToDistanceFormula = 0.034 / 2;

// Defining menu options
enum Menus {
  mainMenu = 0,
  sensorSettingsSubMenu = 1,
  resetLoggerDataSubMenu = 2,
  systemStatusSubMenu = 3,
  rgbLedControlSubMenu = 4,
  colorSelectionMenu = 5
} currentMenu;

// Define the maximum number of saved logs
byte const maxReadingCount = 10;

class SensorReadingQueue {
private:
  int readings[maxReadingCount];  // Array to store readings
  int rear;                       // Index of the rear element
  int size;                       // Current size of the queue

public:
  // Constructor initializing rear and size
  SensorReadingQueue() {
    rear = -1;
    size = 0;
  }

  // Check if the queue is full
  bool isFull() {
    return size == maxReadingCount;
  }

  // Add a reading to the queue
  void addReading(int reading) {
    if (!isFull()) {
      size++;
    }

    rear = (rear + 1) % maxReadingCount;  // Circular queue: update rear index
    readings[rear] = reading;             // Add reading to the queue
  }

  // Print the queue
  void printQueue() {
    // Messages begin in lowercase as they are printed with the queue name in specific functions beforehand
    // Check if the queue is empty before printing
    if (size == 0) {
      Serial.println(F(" queue is empty!"));  // Print a message indicating an empty queue
      return;
    }

    // Print the number of readings and their values
    Serial.print(F(" last "));
    Serial.print(size);
    Serial.print(F(" readings: "));

    int startingIndex = (rear - size + 1 + maxReadingCount) % maxReadingCount;  // Calculate the starting index
    for (int i = 0; i < size; i++) {
      int index = (startingIndex + i) % maxReadingCount;  // Get the index of the reading
      Serial.print(readings[index]);                      // Print the reading value
      Serial.print(" ");
    }
    Serial.println();  // Print a newline character at the end
  }

  // Reset the queue by clearing readings and resetting rear and size
  void resetQueue() {
    for (int i = 0; i < maxReadingCount; i++) {
      readings[i] = 0;  // Clear all readings
    }
    rear = -1;  // Reset rear index
    size = 0;   // Reset size to zero
  }
};


SensorReadingQueue distanceReadings;
SensorReadingQueue lightLevelReadings;

const short int parseIntTimeoutValue = 5;  // milliseconds

void setup() {
  pinMode(triggerPin, OUTPUT);  // Set trigger pin for the ultrasonic sensor as OUTPUT
  pinMode(echoPin, INPUT);      // Set echo pin for the ultrasonic sensor as INPUT

  // Retrieve stored values from EEPROM for sensor settings and LED colors
  EEPROM.get(samplingRateAddress, samplingRate);
  EEPROM.get(ultrasonicSensorThresholdAddress, ultrasonicSensorThreshold);
  EEPROM.get(ldrSensorThresholdAddress, ldrSensorThreshold);
  EEPROM.get(redValueAddress, colors[red]);
  EEPROM.get(greenValueAddress, colors[green]);
  EEPROM.get(blueValueAddress, colors[blue]);

  currentMenu = mainMenu;                   // Set the initial menu to the main menu
  Serial.setTimeout(parseIntTimeoutValue);  // Set Serial timeout to custom timeout value
  Serial.begin(9600);                       // Initialize serial communication at 9600 baud rate
}


void loop() {
  readSerialInput();  // Check for input from the serial monitor
  updateRgbLed();     // Update RGB LED based on current settings
  if (displayCurrentReadingsMode) {
    displayCurrentReadings();  // Display current sensor readings if requested
  }
  updateMenu();         // Update the display menu based on user input
  handleMenuOptions();  // Handle user-selected menu options
  readSensorsData();    // Read data from connected sensors
  flagAlerts();         // Check for alert conditions
  displayAlerts();      // Display any triggered alerts
}

void readSerialInput() {
  if (Serial.available()) {
    incomingNumber = Serial.parseInt();  // Read incoming serial data and parse it to an integer

    if (incomingNumber == 0) {
      Serial.println(F("Error: Invalid input"));  // Notify about invalid input
    } else {
      inputWasntUsed = true;  // Mark input as not used
    }
  }
  /*
  IMPORTANT NOTE:
  
  This function exclusively handles keyboard inputs, adhering to the Single Responsibility Principle. The program implements a flag variable
  to track whether an input has been utilized, allowing only one usage per entered value. Throughout the program, you'll find instances
  such as 'inputWasntUsed = false;' or 'if (inputWasntUsed) {}' to maintain this constraint.
  */
}

void updateMenu() {
  if (menuHasChanged) {
    Serial.println();        // Print an empty line
    printCurrentMenu();      // Print the current menu
    menuHasChanged = false;  // Reset the menu change flag
  }
}

void printCurrentMenu() {
  // Switch statement to print the appropriate submenu or menu based on the current menu state
  switch (currentMenu) {
    case mainMenu:
      printMainMenu();
      break;
    case sensorSettingsSubMenu:
      printSensorSettingsSubMenu();
      break;
    case resetLoggerDataSubMenu:
      printResetLoggerDataSubMenu();
      break;
    case systemStatusSubMenu:
      printSystemStatusSubMenu();
      break;
    case rgbLedControlSubMenu:
      printRgbLedControlSubMenu();
      break;
    default:
      displayInvalidMenuError();
      break;
  }
}

void printMainMenu() {
  Serial.println(F("Main menu:"));
  Serial.println(F("1. Sensor Settings"));
  Serial.println(F("2. Reset Logger Data"));
  Serial.println(F("3. System Status"));
  Serial.println(F("4. RGB LED Control"));
}

void printSensorSettingsSubMenu() {
  Serial.println(F("1. Sensor Settings:"));
  Serial.println(F("\t1.1 Sensor Sampling Interval"));
  Serial.println(F("\t1.2 Ultrasonic Alert Threshold"));
  Serial.println(F("\t1.3 LDR Alert Threshold"));
  Serial.println(F("\t1.4 Back"));
}

void printResetLoggerDataSubMenu() {
  Serial.println(F("2. Reset Logger Data:"));
  Serial.println(F("\t2.1 Yes"));
  Serial.println(F("\t2.2 No"));
}

void printSystemStatusSubMenu() {
  Serial.println(F("3. System Status:"));
  Serial.println(F("\t3.1 Current Sensor Readings"));
  Serial.println(F("\t3.2 Current Sensor Settings"));
  Serial.println(F("\t3.3 Current Logged Data"));
  Serial.println(F("\t3.4 Back"));
}

void printRgbLedControlSubMenu() {
  Serial.println(F("4. RGB LED Control"));
  Serial.println(F("\t4.1 Manual Color Control"));
  Serial.println(F("\t4.2 LED Toggle Automatic ON/OFF"));
  Serial.println(F("\t4.3 Back"));
}

void handleMenuOptions() {
  if (inputWasntUsed) {
    // Switch statement to handle the current menu
    switch (currentMenu) {
      case mainMenu:
        handleMainMenu();
        break;
      case sensorSettingsSubMenu:
        handleSensorSettingsSubMenu();
        break;
      case resetLoggerDataSubMenu:
        handleResetLoggerDataSubMenu();
        break;
      case systemStatusSubMenu:
        handleSystemStatusSubMenu();
        break;
      case rgbLedControlSubMenu:
        handleRgbLedControlSubMenu();
        break;
      default:
        displayInvalidMenuError();
        break;
    }
  }
}

void handleMainMenu() {
  // Switch statement to handle the main menu aka navigate through submenus
  switch (incomingNumber) {
    case sensorSettingsSubMenu:
      changeMenuTo(sensorSettingsSubMenu);
      break;

    case resetLoggerDataSubMenu:
      changeMenuTo(resetLoggerDataSubMenu);
      break;

    case systemStatusSubMenu:
      changeMenuTo(systemStatusSubMenu);
      break;

    case rgbLedControlSubMenu:
      changeMenuTo(rgbLedControlSubMenu);
      break;

    default:
      displayInvalidOptionError();
      inputWasntUsed = false;
      break;
  }
}

void changeMenuTo(int newMenu) {
  currentMenu = newMenu;
  menuHasChanged = true;   // Flag that the menu has changed
  inputWasntUsed = false;  // Flag that the input was used
}

void handleSensorSettingsSubMenu() {
  // Saving the current option is necessary for situations where additional input might be needed later.
  if (!optionWasSelected) {
    currentOption = incomingNumber;
    optionWasSelected = true;
    inputWasntUsed = false;
  }

  switch (currentOption) {
    // Set the sampling rate
    case 1:
      updateSetting(samplingRate, lowerSamplaingLimit, upperSamplingLimit, F("Enter a sampling rate between 1 and 10 seconds."), samplingRateAddress);
      break;
    case 2:
      // Set the ultrsonic alert threshold
      updateSetting(ultrasonicSensorThreshold, lowerUltrasonicSensorLimit, upperUltrasonicSensorLimit, F("Enter a ultrasonic alarm treshold between 2 and 400 centimeters."), ultrasonicSensorThresholdAddress);
      break;
    // Set the ldr alert threshold
    case 3:
      updateSetting(ldrSensorThreshold, lowerLdrSensorLimit, upperLdrSensorLimit, F("Enter a LDR alarm treshold between 0 and 1023."), ldrSensorThresholdAddress);
      break;
    // Navigate back to the main menu
    case 4:
      optionWasSelected = false;
      changeMenuTo(mainMenu);
      break;
    default:
      displayInvalidOptionError();
      optionWasSelected = false;
      break;
  }
}

void updateSetting(short int &settingToUpdate, int lowerLimit, int upperLimit, const __FlashStringHelper *alert, int eepromAddress) {
  // If the selected setting falls within the given threshold, it updates the value. Otherwise, it displays a specific alert.
  if (inputWasntUsed && lowerLimit <= incomingNumber && incomingNumber <= upperLimit) {
    settingToUpdate = incomingNumber;
    EEPROM.put(eepromAddress, settingToUpdate);
    optionWasSelected = false;
    changeMenuTo(sensorSettingsSubMenu);
  } else {
    Serial.println(alert);
  }
  inputWasntUsed = false;
}

void handleResetLoggerDataSubMenu() {
  switch (incomingNumber) {
    // Yes
    case 1:
      distanceReadings.resetQueue();
      lightLevelReadings.resetQueue();
      changeMenuTo(mainMenu);
      break;
    // No
    case 2:
      changeMenuTo(mainMenu);
      break;
    default:
      displayInvalidOptionError();
      break;
  }
  inputWasntUsed = false;
}

void handleSystemStatusSubMenu() {
  switch (incomingNumber) {
    // Enter the display current sensor readings mode
    case 1:
      Serial.println(("Press any number to escape."));
      displayCurrentReadingsMode = true;
      break;
    // Display current settings
    case 2:
      displayCurrentSettings();
      changeMenuTo(systemStatusSubMenu);
      break;
    // Display the logged data (the last 10 readings for each sensor)
    case 3:
      displayLoggedData();
      changeMenuTo(systemStatusSubMenu);
      break;
    // Go back to the Main menu
    case 4:
      changeMenuTo(mainMenu);
      break;
    default:
      displayInvalidOptionError();
      break;
  }
  inputWasntUsed = false;
}

void displayCurrentReadings() {
  // Exit the displayCurrentReadingsMode when there is new viable input from the keyboard
  if (inputWasntUsed) {
    displayCurrentReadingsMode = false;
    inputWasntUsed = false;
    changeMenuTo(systemStatusSubMenu);
    return;
  }

  // Display the readings at the current sampling rate
  if (millis() - lastDisplayTime > samplingRate * millisecondsInASecond) {
    Serial.print(F("\nCurrent distance: "));
    Serial.println(ultrasonicSensorReading);
    Serial.print(F("Current light level: "));
    Serial.println(ldrSenorReading);

    lastDisplayTime = millis();
  }
}

void displayCurrentSettings() {
  Serial.print(F("Current sampling rate: "));
  Serial.println(samplingRate);
  Serial.print(F("LDR threshol: "));
  Serial.println(ldrSensorThreshold);
  Serial.print(F("Ultrasoni sensor threshold: "));
  Serial.println(ultrasonicSensorThreshold);
}

void displayLoggedData() {
  Serial.print(F("\nUltrasonic sensor"));
  distanceReadings.printQueue();
  Serial.print(F("\nLight sensor"));
  lightLevelReadings.printQueue();
}

void handleRgbLedControlSubMenu() {
  switch (incomingNumber) {
    // RGB LED manual control
    // Enter the color picking mode and
    case 1:
      colorPickingMode = true;
      Serial.println(F("Enter the value for red."));
      break;
    // Toggle automatic LED mode
    case 2:
      toggleAutomaticLedMode();
      changeMenuTo(rgbLedControlSubMenu);
      break;
    // Go back to the Main Meno
    case 3:
      changeMenuTo(mainMenu);
      break;
    default:
      optionWasSelected = false;
      displayInvalidOptionError();
      break;
  }
  inputWasntUsed = false;
}

void updateRgbLed() {
  updateLedColors();

  if (automaticLedMode) {
    if (lightLevelAlert || distanceAlert) {
      lightUpLed(maximumIntensity, minimumIntensity, minimumIntensity);  // Red
    } else {
      lightUpLed(minimumIntensity, maximumIntensity, minimumIntensity);  // Green
    }
  } else {
    lightUpLed(colors[red], colors[green], colors[blue]);
  }
}

void updateLedColors() {
  if (colorPickingMode && inputWasntUsed) {
    inputWasntUsed = false;

    // Don't continue until the inpute is viable
    if (incomingNumber < minimumIntensity || incomingNumber > maximumIntensity) {
      Serial.println(F("Color must be between 0 and 255!"));
      return;
    }

    switch (currentColor) {
      case red:
        updateColor(red, redValueAddress, F("Enter the value for green."));
        break;
      case green:
        updateColor(green, greenValueAddress, F("Enter the value for blue."));
        break;
      case blue:
        updateColor(blue, blueValueAddress, F(""));
        // Reset the iterative variable
        currentColor = 0;
        colorPickingMode = false;
        changeMenuTo(rgbLedControlSubMenu);
        break;
      default:
        displayInvalidOptionError();
        break;
    }
  }
}

void updateColor(byte color, byte colorEepromAddress, const __FlashStringHelper *message) {
  colors[color] = incomingNumber;
  EEPROM.put(colorEepromAddress, colors[color]);
  currentColor++;
  Serial.println(message);
}

void toggleAutomaticLedMode() {
  automaticLedMode = !automaticLedMode;
  if (automaticLedMode) {
    Serial.println("Automatic LED mode toggled ON!");
  } else {
    Serial.println("Automatic LED mode toggled OFF!");
  }
}

void lightUpLed(int red, int green, int blue) {
  analogWrite(redOutPutPin, red * redColorCorrectionFactor);
  analogWrite(greenOutPutPin, green * greenColorCorrectionFactor);
  analogWrite(blueOutPutPin, blue * blueColorCorrectionFactor);
}

void readSensorsData() {
  // Read sensors data at the current sampling rate
  if (millis() - lastReadingTime > samplingRate * millisecondsInASecond) {
    readUltrasonicSensorData();
    distanceReadings.addReading(ultrasonicSensorReading);

    readLdrSensorData();
    lightLevelReadings.addReading(ldrSenorReading);

    lastReadingTime = millis();
  }
}

void readUltrasonicSensorData() {
  // Code shamelessly stolen from the lab
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  ultrasonicSensorReading = duration * durationToDistanceFormula;
}

void readLdrSensorData() {
  ldrSenorReading = analogRead(ldrPin);
}

void displayAlerts() {
  // The alerts are displayed once per "event"
  if (distanceAlert && !distanceAlertWasDisplayed) {
    Serial.println(F("It's getting too close!"));
    distanceAlertWasDisplayed = true;
  }

  if (lightLevelAlert && !lightLevelAlertWasDisplayed) {
    Serial.println(F("Zzzz. Time for sleep!"));
    lightLevelAlertWasDisplayed = true;
  }
}

void flagAlerts() {
  flagAlert(ultrasonicSensorReading, ultrasonicSensorThreshold, distanceAlert, distanceAlertWasDisplayed);
  flagAlert(ldrSenorReading, ldrSensorThreshold, lightLevelAlert, lightLevelAlertWasDisplayed);
}

void flagAlert(int sensorReading, short int &sensorThreshold, bool &alertFlag, bool &alertWasDisplayes) {
  if (sensorReading < sensorThreshold) {
    alertFlag = true;
  } else {
    alertFlag = false;
    alertWasDisplayes = false;
  }
}

void displayInvalidOptionError() {
  Serial.println(F("Error: The selected option doesn't exist!"));
}

void displayInvalidMenuError() {
  Serial.println(F("Error: The selected menu doesn't exist"));
}