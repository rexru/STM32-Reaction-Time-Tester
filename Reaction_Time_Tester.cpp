/**
 * =====================================================
 * Reaction Time Tester – FSM (Finite State Machine)
 * =====================================================
 *
 * States:
 *
 *   [Idle / Ready]
 *       - Green LED blinks at ~10Hz
 *       - Waits for onboard button press
 *                |
 *                v
 *   [Random Delay / Reaction Prep]
 *       - LED off for random 1–5s delay
 *       - After delay, green LED turns on
 *       - Timer starts
 *                |
 *                v
 *   [Reaction Measurement]
 *       - Wait for user button press
 *       - If pressed too early → reset to Idle
 *       - If valid press:
 *            * Measure reaction time
 *            * Update personal best if faster
 *                |
 *                v
 *   [Test Complete]
 *       - Red LED blinks every 300ms
 *       - LCD displays:
 *            * Latest reaction time
 *            * Fastest reaction time
 *       - Onboard button → restart
 *       - External button → reset everything
 *                |
 *                v
 *   [Reset State]
 *       - Triggered by external button (PA6)
 *       - Clears LCD, resets fastest time
 *       - Returns to Idle
 *
 * =====================================================
 */

#include "LCD_DISCO_F429ZI.h" // LCD driver library
#include "mbed.h"             // Mbed OS hardware abstraction library

// -------------------- Hardware Setup --------------------
LCD_DISCO_F429ZI LCD;                   // LCD display object
InterruptIn userButton(BUTTON1);        // Onboard user button (blue button)
InterruptIn external_button(PA_6, PullUp); // External pushbutton with internal pull-up
DigitalOut green(PG_13);                // Onboard green LED
DigitalOut red(PG_14);                  // Onboard red LED
Ticker ticker;                          // Periodic timer (not actively used in this version)
Timeout timeout;                        // Timeout for scheduling events
Timer t;                                // Timer for reaction time measurement

// -------------------- Global Variables --------------------
int state = 0;                // Finite State Machine (FSM) state
bool reaction = false;        // Flag: true when waiting for a reaction
int pB = INT_MAX;             // Personal best reaction time (initialized to max value)
uint64_t elapsed = 0;         // Elapsed reaction time (ms)
char bufferElapsed[32];       // Buffer for latest reaction time string
char bufferpB[32];            // Buffer for personal best string

// -------------------- Function Declarations --------------------
void blinkRed();   // Red LED blinking after test completion
void reaction1();  // Start reaction phase
void tick();       // FSM tick handler
void user();       // Onboard user button ISR
void external();   // External reset button ISR

// -------------------- FSM Functions --------------------

/**
 * @brief Starts the reaction phase.
 * Turns on the green LED and starts the timer.
 */
void reaction1() {
    elapsed = 0;
    green = 1;    // LED on
    t.start();    // Start measuring reaction time
    reaction = true;
}

/**
 * @brief Handles FSM logic and blinking based on current state.
 */
void tick() {
    if (state == 0) {  
        // ---------------- Idle State ----------------
        // Blink green LED at ~10Hz to indicate readiness
        green = !green;
        timeout.attach(&tick, 100ms);

    } else if (state == 1) {  
        // ---------------- Reaction Phase ----------------
        // Timer is running, waiting for user input
        reaction = false;
        t.reset();

    } else if (state == 2) {  
        // ---------------- Test Complete ----------------
        // Blink red LED until reset
        blinkRed();
    }
}

/**
 * @brief Toggles red LED to indicate test completion.
 */
void blinkRed() {
    red = !red;
    timeout.attach(&blinkRed, 300ms); // Blink every 300ms
}

// -------------------- Interrupt Service Routines --------------------

/**
 * @brief Onboard button handler.
 * Handles state transitions:
 * - Start test
 * - Capture reaction time
 * - Restart or reset test depending on conditions
 */
void user() {
    if (state == 0) {
        // Start reaction test
        state = 1;
        green = 0; // LED off during random delay
        tick();

    } else if (state == 1 && reaction == true && elapsed >= 0) {
        // User pressed button during active reaction phase
        t.stop();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t.elapsed_time()).count();

        // Display latest time
        sprintf(bufferElapsed, "The time taken was %llu ms", elapsed);

        // Update personal best if faster
        if (elapsed < pB) {
            pB = elapsed;
        }
        sprintf(bufferpB, "Personal Best: %d ms", pB);

        green = 0;   // Turn off LED
        state = 2;   // Move to test completed
        tick();

    } else if (state == 2) {
        // Restart test after completion
        state = 0;
        red = 0; // Stop red blinking
        tick();

    } else {
        // Any invalid press → reset to idle
        state = 0;
        tick();
    }
}

/**
 * @brief External pushbutton handler.
 * Resets everything: LCD, fastest time, state machine.
 */
void external() {
    state = 0;
    pB = INT_MAX;
    elapsed = 0;
    reaction = false;

    // Clear LCD text buffers
    memset(bufferElapsed, 0, sizeof(bufferElapsed));
    memset(bufferpB, 0, sizeof(bufferpB));

    LCD.Clear(LCD_COLOR_WHITE); // Clear LCD screen
    red = 0; // Turn off red LED
    tick();  // Restart idle blinking
}

// -------------------- Main Program --------------------
int main() {
    // Initialize hardware
    green = 0;
    red = 0;

    // Attach interrupts
    userButton.fall(&user);
    external_button.fall(&external);
    __enable_irq();

    // Configure LCD
    LCD.SetFont(&Font12);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);

    // Start idle blinking
    tick();

    // Main loop updates LCD with results
    while (1) {
        LCD.DisplayStringAt(0, 40, (uint8_t *)bufferElapsed, LEFT_MODE);
        LCD.DisplayStringAt(0, 80, (uint8_t *)bufferpB, LEFT_MODE);
    }
}
