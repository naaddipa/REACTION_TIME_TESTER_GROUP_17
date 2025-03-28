//view simulation at: https://www.tinkercad.com/things/iZnFs5UJflk-reaction-time-project/editel?returnTo=%2Fthings%2FiZnFs5UJflk-reaction-tome-project
// Pin Connections:
// LCD (16x2) -> RS: 7, E: 6, D4: 5, D5: 4, D6: 3, D7: 2 (connected to Arduino)(Changed D7 to pin 13)
// LED -> Connected to pin 10
// Pushbutton -> Connected to pin 13 (active LOW with internal pull-up) Changed to pin 2 (supports interrupts)
// VCC & GND of LCD -> Connected to 5V & GND respectively
// LED Anode -> Connected to pin 10 via a current-limiting resistor
// LED Cathode -> Connected to GND
//BUZZER -> Connected to D12

#include <LiquidCrystal.h>

// Pin definitions
#define RS 7
#define EN 6
#define D4 5
#define D5 4
#define D6 3
#define D7 13
#define BTN_PIN 2  
#define LED_PIN 10
#define BUZZER_PIN 12 

// Timing constants
#define DISP_REFRESH_INTERVAL 150  // Refresh interval for LCD status (in ms)
#define DEBOUNCE_TIME 50           // Reduced debounce time for faster response (in ms)
#define TIMEOUT_DURATION 5000      // Timeout duration if player doesn't react (in ms)
#define MIN_WAIT_TIME 2000         // Minimum wait time before LED turns on (in ms)
#define MAX_WAIT_TIME 6000         // Maximum wait time before LED turns on (in ms)

// Sound constants 
#define NOTE_C5  523   // C5 note
#define NOTE_E5  659   // E5 note
#define NOTE_G5  784   // G5 note
#define NOTE_C6  1047  // C6 note
#define NOTE_A4  440   // A4 note
#define NOTE_B4  494   // B4 note
#define NOTE_F5  698   // F5 note

// Increased durations for better sound playback
#define TONE_DURATION_VERY_SHORT 100  // Very short beep (increased from 40ms)
#define TONE_DURATION_SHORT 150       // Short beep duration (increased from 80ms)
#define TONE_DURATION_MEDIUM 200      // Medium beep duration (increased from 150ms)

// Game state variables
volatile int mode = 0;             // 0 = idle, 1 = waiting for LED, 2 = LED on, 3 = game ended, 4 = timeout
volatile long gameStartTime = -1;  // Time when the game starts
volatile long randomWaitTime = -1; // Random wait time before turning on LED
volatile long buttonPressTime = -1;// Time when the button is pressed
volatile long lastInterrupt = -1;  // Last interrupt time for debouncing
volatile bool rushed = false;      // Flag for early button press

long highscore = 5000;             // Initial highscore (in milliseconds)
long reactionTime = -1;            // Reaction time of the current round
long lastDisplayRefresh = 0;       // Tracks the last display refresh time
int gamesPlayed = 0;               // Count of games played
long totalReactionTime = 0;        // Sum of all reaction times for average calculation

// Initialize the LCD
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  // Configure pins
  pinMode(BTN_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor for the button
  pinMode(LED_PIN, OUTPUT);        // Set LED pin as output
  pinMode(BUZZER_PIN, OUTPUT);     // Set buzzer pin as output

  // Initialize the LCD
  lcd.begin(16, 2);
  showWelcomeScreen();

  // Play a pleasant startup melody
  playStartupMelody();

  // Attach interrupt for button press (active LOW)
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonPressed, FALLING);

  // Initialize random seed with analog noise
  randomSeed(analogRead(A0));

  // Initialize variables
  lastInterrupt = millis();
  Serial.begin(9600);              // Enable serial debugging
}

void playNote(int note, int duration) {
  tone(BUZZER_PIN, note, duration);
  delay(duration + 20);  // Added extra delay to ensure tone completes
  noTone(BUZZER_PIN);    // Explicitly stop tone
}

void playStartupMelody() {
  playNote(NOTE_C5, TONE_DURATION_SHORT);
  delay(70);  // Increased delay between notes
  playNote(NOTE_E5, TONE_DURATION_SHORT);
  delay(70);  // Increased delay between notes
  playNote(NOTE_G5, TONE_DURATION_SHORT);
  delay(70);  // Increased delay between notes
  playNote(NOTE_C6, TONE_DURATION_MEDIUM);
  delay(120); // Increased final delay
}

void playSuccessMelody() {
  playNote(NOTE_C5, TONE_DURATION_VERY_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_E5, TONE_DURATION_VERY_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_G5, TONE_DURATION_SHORT);
  delay(100); // Added delay after final note
}

void playNewHighScoreMelody() {
  playNote(NOTE_C5, TONE_DURATION_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_E5, TONE_DURATION_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_G5, TONE_DURATION_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_C6, TONE_DURATION_SHORT);
  delay(60);  // Increased delay between notes
  playNote(NOTE_G5, TONE_DURATION_SHORT);
  delay(40);  // Increased delay between notes
  playNote(NOTE_C6, TONE_DURATION_MEDIUM);
  delay(100); // Added delay after final note
}

void playReadyTone() {
  // Play a gentle "ready" tone
  playNote(NOTE_G5, TONE_DURATION_SHORT);
  delay(50);  // Added delay after tone
}

void playFailTone() {
  // Play a gentle descending tone for failure
  playNote(NOTE_E5, TONE_DURATION_SHORT);
  delay(70);  // Increased delay between notes
  playNote(NOTE_C5, TONE_DURATION_MEDIUM);
  delay(100); // Added delay after final note
}

void playLEDOnTone() {
  // Distinct but gentle tone when LED turns on
  playNote(NOTE_A4, TONE_DURATION_VERY_SHORT);
  delay(30);  // Increased delay between notes
  playNote(NOTE_B4, TONE_DURATION_SHORT);
  delay(50);  // Added delay after final note
}

void showWelcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reaction Tester!");
  lcd.setCursor(0, 1);
  lcd.print("    Welcome!    ");
  delay(1500);
  lcd.clear();
}

void buttonPressed() {
  long currentTime = millis();
  
  // Simple debounce
  if (currentTime - lastInterrupt < DEBOUNCE_TIME) {
    return;
  }
  
  lastInterrupt = currentTime;
  Serial.println("Button Pressed!");
  
  switch (mode) {
    case 0: // Idle mode
      startGame();
      playReadyTone();
      break;
    case 1: // Waiting mode (LED off) - Rushing!
      rushed = true;
      mode = 4;
      playFailTone();
      break;
    case 2: // React mode (LED on)
      buttonPressTime = currentTime;
      playSuccessMelody();
      endGame();
      break;
    case 3: // Game ended mode
    case 4: // Timeout/Rush mode
      mode = 0; // Reset to idle
      playReadyTone();
      break;
  }
}

void startGame() {
  randomWaitTime = random(MIN_WAIT_TIME, MAX_WAIT_TIME); // 2-6 seconds wait
  gameStartTime = millis();         // Record the game start time
  digitalWrite(LED_PIN, LOW);       // Ensure the LED is off
  mode = 1;                         // Transition to "waiting for LED" mode
  rushed = false;                   // Reset Rush flag
  Serial.print("Wait time: ");
  Serial.println(randomWaitTime);
}

void endGame() {
  digitalWrite(LED_PIN, LOW);       // Turn off the LED
  mode = 3;                         // Transition to "game ended" mode
  
  // Calculate and update statistics
  reactionTime = buttonPressTime - (gameStartTime + randomWaitTime);
  
  // Play special melody for new high score
  if (reactionTime < highscore && reactionTime > 0) {
    highscore = reactionTime;
    delay(300);  // Small delay after the success sound
    playNewHighScoreMelody();
  }
  
  if (reactionTime > 0) {
    gamesPlayed++;
    totalReactionTime += reactionTime;
  }
}

void updateDisplay() {
  lcd.clear();

  switch (mode) {
    case 0: // Idle mode
      lcd.setCursor(0, 0);
      lcd.print("Press button to");
      lcd.setCursor(0, 1);
      lcd.print("start the game");
      break;
      
    case 1: // Waiting mode
      lcd.setCursor(0, 0);
      lcd.print("Get ready...");
      lcd.setCursor(0, 1);
      lcd.print("Wait for the LED");
      break;
      
    case 2: // React mode
      lcd.setCursor(0, 0);
      lcd.print("REACT NOW!");
      lcd.setCursor(0, 1);
      lcd.print("Press the button!");
      break;
      
    case 3: // Game ended mode
      lcd.setCursor(0, 0);
      lcd.print("Time: ");
      lcd.print(reactionTime);
      lcd.print(" ms");
      
      lcd.setCursor(0, 1);
      lcd.print("Best: ");
      lcd.print(highscore);
      lcd.print(" ms");
      break;
      
    case 4: // Timeout or Rushing mode
      lcd.setCursor(0, 0);
      if (rushed) {
        lcd.print("Too early!");
      } else {
        lcd.print("Too slow!");
      }
      lcd.setCursor(0, 1);
      lcd.print("Press to restart");
      break;
  }
}

void loop() {
  long current = millis();

  // Debug mode value
  if (current % 1000 == 0) {  // Print once per second to reduce serial spam
    Serial.print("Mode: ");
    Serial.println(mode);
  }

  // Refresh the display periodically
  if (current - lastDisplayRefresh > DISP_REFRESH_INTERVAL) {
    updateDisplay();
    lastDisplayRefresh = current;
  }

  // Handle game state transitions
  if (mode == 1) {
    if (current >= gameStartTime + randomWaitTime) {
      // Time to turn on LED
      digitalWrite(LED_PIN, HIGH);
      playLEDOnTone();  // Play a pleasant tone when LED turns on
      mode = 2;  // LED on mode
    }
  } else if (mode == 2) {
    if (current >= gameStartTime + randomWaitTime + TIMEOUT_DURATION) {
      // Player took too long to react
      digitalWrite(LED_PIN, LOW);
      playFailTone();  // Play gentle failure tone
      mode = 4;  // Timeout mode
      rushed = false;
    }
  }
}