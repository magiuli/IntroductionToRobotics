# Introduction to Robotics (2023 - 2024)

This repository is a *comprehensive collection* of my work completed during the **Introduction to Robotics** course, which I undertook in my third year at the **Faculty of Mathematics and Computer Science**, **University of Bucharest**. Here, you'll find *meticulously documented assignments*, each encompassing detailed requirements, thorough implementation insights, and comprehensive sets of code and accompanying image files. 

> Thee shalt find delecting insights within these medieval translation: *Hark, fair gentlefolk! This compendium doth contain a thorough assemblage of mine toils completed whilst I did engage in the Introduction to Robotics course during mine third year at the Faculty of Mathematics and Computer Science, University of Bucharest. Within these hallowed pages, thou shalt discover diligently chronicled tasks, each possessing detailed decrees, profound insights into their construction, and ample scrolls of code accompanied by likenesses captured in image files.*

## Homework 1

### Task assignment
The first homework involved creating the current repository, making it private, providing access to ten github users (nine teacher assistants and the course professor) and writing a succinct yet creative `README.md`. 

## Homework 2

### Task assignment
This assignment focused on controlling the intensity of *each color* of an **RGB LED** with an *individual potentiometer*. The control was achieved indirectly through an Arduino. The task involved reading three values from analog pins and **mapping** them to match the output capabilities of the **PWM** (*Pulse Width Modulation*) digital pins. </br>
**Additional information** about each input/output and its *respective port* can be found in the introductory comments of the `RGB-LED.ino` file in the `Homework\Code` folder.

### Setup
The setup included:
  - arduino uno board
  - medium breadboard
  - 1 x RGB LED
  - 3 x potentiometer
  - 1 x 330 Ω resistor (for the red channel)
  - 2 x 220 Ω resistor (one for the blue chanel, and one for the green chanel)
  - 14 x short/medium cable 
  - 1 x USB A to B cable

<img src="https://github.com/magiuli/IntroductionToRobotics/blob/main/Homework/Assets/rgb_led_setup.jpg" width="500px">

<a href="https://youtu.be/zIxEojEt65U?si=s0HmiYX0PCU4duPW">YouTube video</a>

## Homework 3

### Task assignment

This assignment involved simulating a 3-floor elevator control system using LEDs, buttons, and a buzzer with an Arduino. Each LED and button represents one of the three floors, with the LED corresponding to the current floor lighting up.Additionally, there is a 4th LED that signifies the operational state of the elevator: it blinks when the elevator is in motion and remains off when stationary. When the elevator begins moving, it 'closes the doors,' 'travels' to the desired floor, and announces 'the arrival.' Each of these quoted actions has its corresponding sound.

While the elevator is stationary, pressing the button for the current floor results in no action. However, pressing a button while the elevator is in motion places the requested floor in a queue for following action.

### Setup
The setup included:
  - arduino uno board
  - medium breadboard
  - 3 x blue LED
  - 1 x red LED
  - 1 x active buzzer
  - 3 x button
  - 2 x 220 Ω resistor (one for the red LED and one for the zctive buzzer)
  - 3 x 1000 Ω resistor (one for each blue led)
  - 13 x short cable 
  - 1 x USB A to B cable

<img src="https://github.com/magiuli/IntroductionToRobotics/blob/main/Homework/Assets/elevator_simulator.jpg" width="500px">

<a href="https://youtu.be/9KkHAXNGIgo?si=v4Pa6Egs84MHOEgk">YouTube video</a>

## Homework 4

### Task assignment

This homework assignment involves controlling a 7-segment display using a joystick. The joystick allows you to move the active segment and "draw" on the display. The movement between segments is restricted to natural transitions, meaning the segment can only jump to neighboring positions without passing through "walls."Here are the key functionalities of the assignment:
 
  - The initial position of the segment is at the decimal point (DP).
  - The current segment always blinks, regardless of its state (on or off).
  - Use the joystick to move the current position to neighboring segments, based on a predefined table of corresponding movements.
  - A short press of the joystick button toggles the state of the current segment from ON to OFF or from OFF to ON.
  - A long press of the joystick button resets the entire display by turning off all the segments and moving the current position back to the decimal point (DP).

### Setup
The setup included:
  - arduino uno board
  - medium breadboard
  - 1 x 7 segment display
  - 1 x joystick
  - 1 x active buzzer
  - 6 x 330 Ω resistor (for 6 of the segments. Dindn't have more)
  - 3 x 220 Ω resistor (2 for the remaining segments, 1 for the active buzzer)
  - 5 male to female wires
  - 20 male to male wires
  - 1 x USB A to B cable

<img src="https://github.com/magiuli/IntroductionToRobotics/blob/main/Homework/Assets/7_segment_display_drawing.jpg" width="500px">

<a href="https://youtu.be/WPqnlEvLR50?si=wX0zil5kxOrX1RIY">YouTube video</a>

## Homework 5

### Task assignment

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

### Setup
The setup included:
  - 1 x arduino uno board
  - 1 x medium breadboar
  - 1 x small breadboard
  - 1 x 4 digits 7 segment display
  - 1 x shift Register 74HC595
  - 3 x button
  - 8 x 220 Ω resistor (one for each segment)
  - 3 x blue LED
  - 3 X 2000 Ω resistor (one for each blue LED) 
  - many wires
  - 1 x USB A to B cable

<img src="https://github.com/magiuli/IntroductionToRobotics/blob/main/Homework/Assets/stopwatch_timer.jpg" width="500px">

<a href="https://youtu.be/kyaXbSkF6j8?si=5nbyd3CZajmmYpwQ">YouTube video</a>

# Homework 5

## Task assignment

This assignment involved utilizing multiple sensors to gather environmental data, adjusting sensor settings, logging these configurations into EEPROM, and offering visual feedback using an RGB LED. Additionally, user interaction is facilitated through a Start Menu. An ultrasonic sensor (HC-SR04) and a photoresistor are employed for environmental data collection. Specific alerts are displayed on the Serial Monitor when the readings fall below predefined thresholds. If the Automatic LED mode is enabled, the LED indicates an active alert by turning red; otherwise, it remains green.

Navigation through menus and submenus is accomplished via user input. It's important to note that error handling has been implemented to address invalid or out-of-bounds inputs, ensuring a smoother user experience.

### Menu structure and options
1. Sensor Settings
   - Sensor Sampling Interval
   - Ultrasonic Alert Threshold
   - LDR Alert Threshold
   - Back
2. Reset Logger Data
   - Yes
   - No
3. System status
   - Current Sensor Readings
   - Current Sensor Settings
   - Back
4. RGB LED Control
   - Manual Color Control
   - LED: Toggle Automatic ON/OFF
   - Back
