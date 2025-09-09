# Reaction Time Tester – STM32F429ZI (Lab 02)

## Overview
This project implements a **reaction time tester** on the STM32F429ZI Discovery board. The goal is to measure how quickly a user can press a button after an LED lights up, and to display their reaction time on the onboard LCD. The system uses a **finite state machine (FSM)** to manage its behavior, interrupt-driven hardware timers for precise timing, and both onboard and external pushbuttons for interaction.  

---

## Features
- **Reaction Time Measurement**  
  - Random delay (1–5 seconds) before the LED turns on.  
  - Measures the time (in milliseconds) between LED illumination and button press.  
  - Detects and rejects “cheating” (pressing the button before the LED lights).  

- **LCD Display**  
  - Displays the most recent reaction time.  
  - Displays the fastest recorded reaction time so far.  
  - Updates results in real time with millisecond accuracy.  

- **Reset Function**  
  - External pushbutton clears the LCD, resets stored fastest time, and restarts the test.  

- **FSM-Based Design**  
  - Implemented using the Garbini method for clarity and robustness.  
  - System behavior defined entirely by states and transitions.  

- **Interrupt-Driven Timing**  
  - All timing handled via hardware timers.  
  - No software wait-loops used.  

---

## Hardware
- **STM32F429ZI Discovery Board**  
  - Includes onboard green LED, blue user pushbutton, and 2.41" QVGA LCD.  

- **External Pushbutton**  
  - Connected to GPIO pin `PA6` with **internal pull-up resistor** enabled.  
  - Provides system reset functionality.  

---

## State Machine (FSM)
The system transitions between the following states:

1. **Idle / Ready State**  
   - Green LED flashes at ~10 Hz.  
   - Waits for onboard user button press.  

2. **Random Delay State**  
   - LED off for a random time (1–5 seconds).  
   - Ensures unpredictability of test.  

3. **Reaction Measurement State**  
   - LED turns on.  
   - Timer starts measuring reaction time.  
   - Detects premature button presses (cheating).  

4. **Result Display State**  
   - Reaction time displayed on LCD.  
   - Fastest recorded time tracked and displayed.  

5. **Reset State (external button)**  
   - Clears LCD and fastest time.  
   - Returns to Idle state.  

---

## Requirements
- **Keil Studio Cloud** with Mbed OS  
- **BSP and LCD libraries** imported into project  
- **C++ (Mbed OS)** for implementation  

---

## How to Run
1. Clone or import this project into **Keil Studio Cloud**.  
2. Set target to `DISCO-F429ZI`.  
3. Build and flash the program to the Discovery board.  
4. Connect an external pushbutton to pin `PA6` (with GND on the other side).  
5. Interact with the system:  
   - Onboard button starts reaction test.  
   - External button resets results.  

---

## Example Output (LCD)
