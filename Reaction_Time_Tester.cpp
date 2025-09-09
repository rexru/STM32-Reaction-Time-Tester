#include "LCD_DISCO_F429ZI.h" // Include the LCD library for display functions
#include "mbed.h" // Include the mbed library for hardware abstraction
 
LCD_DISCO_F429ZI LCD; // Create an LCD object to interface with the display
 
// Declare hardware peripherals
InterruptIn userButton(BUTTON1); // Interrupt input for the user button
DigitalOut green(PG_13);         // Green LED output
DigitalOut red(PG_14);           // Red LED output
Ticker ticker;                   // Timer-based interrupt for periodic execution
Timeout timeout;                 // Timeout function for delayed execution
Timer t;                         // Timer for measuring reaction time
InterruptIn external_button(PA_6, PullUp);
 
int state = 0;         // State variable for tracking the game logic
bool reaction = false; // Flag to indicate if the reaction phase is active
int pB = INT_MAX;      // Personal best reaction time initialized to max value
uint64_t elapsed = 0;  // Elapsed time in milliseconds
char bufferElapsed[32]; // Buffer for displaying elapsed time
char bufferpB[32];      // Buffer for displaying personal best time
int r = std::rand() % 5 + 1;
 
void blinkRed(); // Forward declaration for blinking function
 
// Function to start reaction phase
void reaction1() {
    elapsed = 0;
    green = 1;  // Turn on the green LED
    t.start();  // Start the timer
    reaction = true;
}
 
// Function for handling state transitions and LED blinking
void tick() {
    if (state == 0) {  // Idle state
        green = !green; // Blink green LED
        timeout.attach(&tick, 100ms);
    } else if (state == 1) {  // Reaction phase
        reaction = false;
        t.reset();
        
    } else if (state == 2) {  // Test complete state (Blink red LED)
        blinkRed();
    }
}
 
// Function to handle red LED blinking after test completion
void blinkRed() {
    red = !red;
    timeout.attach(&blinkRed, 300ms); // Blink red LED every 300ms
}
 
// Function triggered by user button press
void user() {
    if (state == 0) { // Start reaction test
        state = 1;
        green = 0; // Turn off green LED
        tick();
    } else if (state == 1 && reaction == true && elapsed >= 0) { // Reaction detected
        t.stop();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t.elapsed_time()).count();
        sprintf(bufferElapsed, "The time taken was %llu ms", elapsed);
 
        if (elapsed < pB) { // Check if new personal best is achieved
            pB = elapsed;
        }
 
        sprintf(bufferpB, "Personal Best: %d ms", pB);
        green = 0; // Turn off the green LED
        state = 2; // Move to "test completed" state
        tick();    // Start red LED blinking
    } else if (state == 2) { // Restart test if button is pressed
        state = 0;
        red = 0; // Turn off red LED
        tick();
    } else { // Reset if button is pressed at the wrong time
        state = 0;
        tick();
    }
}
 
// Function to reset state on external button press
void external() {
    state = 0;
    pB = INT_MAX;
    elapsed = 0;
    reaction = false;
    memset(bufferElapsed, 0, sizeof(bufferElapsed));
    memset(bufferpB, 0, sizeof(bufferpB));
    LCD.Clear(LCD_COLOR_WHITE);
    red = 0; // Turn off red LED
    tick();
}
 
int main() {
    green = 0;
    red = 0;
    userButton.fall(&user);
    external_button.fall(&external);
    __enable_irq();
 
    LCD.SetFont(&Font12);
    LCD.SetTextColor(LCD_COLOR_DARKBLUE);
    tick(); // Start initial blinking sequence
 
    while (1) {
        LCD.DisplayStringAt(0, 40, (uint8_t *)bufferElapsed, LEFT_MODE);
        LCD.DisplayStringAt(0, 80, (uint8_t *)bufferpB, LEFT_MODE);
    }
}
