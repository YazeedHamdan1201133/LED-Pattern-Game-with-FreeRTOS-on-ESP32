/************************************************************  
  Blynk & ESP32 Game: "Mind Blink"
  Anas Hawamda - 1202121  
  Yazeed Hamdan - 1201133
  Mahmoud Hamdan - 1201134
************************************************************/

// Blynk App Template Information
#define BLYNK_TEMPLATE_ID "TMPL6DEK0XtXL"
#define BLYNK_TEMPLATE_NAME "MyGame"
#define BLYNK_AUTH_TOKEN "aBefrM3HgTqQYB2FNRPiCEBlZtdCk9jZ"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_PCF8574.h>
#include <Arduino.h>



// WiFi Credentials
char ssid[] = "AAAAAAA";
char pass[] = "qqqqqqqqqqq";

// LCD Setup
LiquidCrystal_PCF8574 lcd(0x27); // Initialize LCD at I2C address 0x27

//LED Pins 
const int ledPins[] = {13, 12, 14, 27};
const int totalLEDs = 4;

/*************** Global Game Variables ***************/
int patternArray[22];     
int playerArray[22];     

int currentLevel = 1;    
int playerMistakes = 0;  
int playerScore = 0;      
bool isGameRunning = false;      
bool isPatternShown = false;      
bool isGameOverShown = false;     
int playerIndex = 0;             
bool isStartOn = false;         

/* The debounceDelay is the minimal waiting time (in milliseconds) 
   used to ignore rapid repeated button presses.
   It prevents accidental double or multiple clicks from registering as separate inputs.*/
unsigned long lastInputTime = 0;
const unsigned long inputDebounceDelay = 500;  //  (0.5 second debounce)

/*************** Blynk Virtual Pins ***************/
#define V_START    V5
#define V_BUTTON_1 V1
#define V_BUTTON_2 V2
#define V_BUTTON_3 V3
#define V_BUTTON_4 V4
#define V_RESTART  V0

// FreeRTOS task handles
TaskHandle_t taskLEDHandle, taskLCDHandle;

/* This flag ensures that once we display the final 
   game result screen (like: "You Win!" or "Game Over"), 
   we don't overwrite it with new LCD updates. */
bool isFinalScreen = false;  

// Function Prototypes 
void restartFullGame();              
void createRandomPattern();         
void managePlayerInput(int pressedButton); 
void endGame(bool isManualEnd);   // Game Over   
void advanceLevel();   //  levelUp 

// FreeRTOS tasks
void taskPatternLED(void *parameter); 
void taskUpdateLCD(void *parameter);  

// SETUP & LOOP
void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize LED pins as OUTPUT
  for (int i = 0; i < totalLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Initialize I2C on GPIO21 (SDA) and GPIO22 (SCL) for ESP32
  Wire.begin(21, 22);

  // Start the LCD, set backlight, and clear
  lcd.begin(16, 2);
  lcd.setBacklight(1); 
  lcd.clear();
  lcd.print("Initializing...");

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize random seed with an analog input
  randomSeed(analogRead(0));

  // Create and pin tasks to Core 1 for concurrency
  xTaskCreatePinnedToCore(taskPatternLED, "taskPatternLED", 2048, NULL, 1, &taskLEDHandle, 1);
  xTaskCreatePinnedToCore(taskUpdateLCD, "taskUpdateLCD", 2048, NULL, 1, &taskLCDHandle, 1);

  Serial.println("Setup complete.");
}

// The main loop for Blynk.
void loop() {
  Blynk.run();
}

// BLYNK BUTTON HANDLERS

/* 
  BLYNK_WRITE(V_START):
    Called automatically whenever the value of Virtual Pin V5 changes,
    Toggles the game start/end based on the Blynk button state.
*/
BLYNK_WRITE(V_START) {
  int startState = param.asInt();
  if (startState == 1) {
    // If user turned the 'Start Game' switch ON
    isStartOn = true;
    Serial.println("Start button pressed");
    // show a "Starting" message on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting");
    lcd.setCursor(0, 1);
    lcd.print("The Game :)");

    // Suspend the LCD update task so it doesn't overwrite our message
    vTaskSuspend(taskLCDHandle); 
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
    // Resume the LCD update task
    vTaskResume(taskLCDHandle); 

    // Begin new game session
    restartFullGame();

  } else {
    // If user turned the 'Start Game' switch OFF
    isStartOn = false;
    Serial.println("End Game button pressed!");
    // End the game manually
    endGame(true);
  }
}

/* 
  BLYNK_WRITE(V_RESTART):
  Called whenever the user presses the 'Restart Game' button (Virtual Pin V0).
  Only restarts if the game is currently started (isStartOn = true).
*/
BLYNK_WRITE(V_RESTART) {
  int restartState = param.asInt();
  if (restartState == 1 && isStartOn) {
    // If the game is actually ON, allow restart
    Serial.println("Restart button pressed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Restarting");
    lcd.setCursor(0, 1);
    lcd.print("The Game :)");
    vTaskDelay(4000 / portTICK_PERIOD_MS); 
    restartFullGame();
  } 
  // If user pressed restart but the game isn't ON, 
  else if (restartState == 1 && !isStartOn) {
    // show a message on the LCD or log it
    Serial.println("Restart disabled: Start button is OFF.");
    lcd.clear();
    lcd.print("Can't Restart!");
    lcd.setCursor(0, 1);
    lcd.print("Start is OFF");
  }
}

/* 
  BLYNK_WRITE(V_BUTTON_X):
  Handles input from the 4 game buttons on the Blynk app.
  Each button corresponds to an LED in the sequence 
  "Yellow, Blue, Red, and Orange".
*/
BLYNK_WRITE(V_BUTTON_1) { managePlayerInput(1); }
BLYNK_WRITE(V_BUTTON_2) { managePlayerInput(2); }
BLYNK_WRITE(V_BUTTON_3) { managePlayerInput(3); }
BLYNK_WRITE(V_BUTTON_4) { managePlayerInput(4); }

//  GAME SETUP & UTILITY FUNCTIONS
/* 
  restartFullGame():
    Resets all game variables for a fresh start 
    and Generates a new random sequence.
*/
void restartFullGame() {
  Serial.println("Game reset!");
  currentLevel = 1;
  playerMistakes = 0;
  playerScore = 0;
  playerIndex = 0;
  isGameRunning = true;
  isPatternShown = false;
  isGameOverShown = false;
  isFinalScreen = false; 

  // Generate the initial random sequence
  createRandomPattern();
}

/* 
   Fills the 'patternArray' with random LED numbers.
   The length of the sequence is (currentLevel + 2). 
*/
void createRandomPattern() {
  Serial.print("Generated Sequence: ");
  for (int i = 0; i < currentLevel + 2; i++) {
    int newLED = random(1, totalLEDs + 1);
    patternArray[i] = newLED;
    Serial.print(patternArray[i]);
    Serial.print(" ");
  }
  Serial.println();
  // Reset the player input index
  playerIndex = 0;
  // Indicate we haven't yet displayed the sequence
  isPatternShown = false;
}

/* 
    Debounced handler for user input from Blynk buttons.
    Checks if the button pressed matches the expected sequence.
    Tracks mistakes and score, calls endGame or advanceLevel if needed.
*/
void managePlayerInput(int pressedButton) {
  unsigned long currentTime = millis();
  if (currentTime - lastInputTime < inputDebounceDelay) {
    // If called too quickly, ignore
    return;
  }
  lastInputTime = currentTime;

  // If game hasn't started or the sequence isn't fully displayed, ignore input
  if (!isGameRunning || !isPatternShown) return;

  Serial.print("Button Pressed: ");
  Serial.println(pressedButton);

  // Store the user's input
  playerArray[playerIndex] = pressedButton;

  // Compare with the expected pattern
  if (playerArray[playerIndex] != patternArray[playerIndex]) {
    // Incorrect input -> increment mistakes
    playerMistakes++;
    Serial.print("Mistake! Total mistakes: ");
    Serial.println(playerMistakes);
    // If user hits 3 mistakes, game over
    if (playerMistakes >= 3) {
      endGame(false); // Lost the game
      return;
    }
  } else {
    // Correct input -> increment score
    playerScore++;
    playerIndex++;

    // If we've matched the entire sequence (index surpasses 'currentLevel + 1'), 
    // then we advance to the next level
    if (playerIndex > currentLevel + 1) {
      advanceLevel();
    }
  }
}

// GAME PROGRESSION FUNCTIONS

/* 
    Terminates the current game session.
    If 'isManualEnd' is true, it means the user turned OFF the Start switch.
    Otherwise, it means the player lost due to mistakes.
*/
void endGame(bool isManualEnd) {
  // Mark game as stopped
  isGameRunning = false;
  lcd.clear();

  if (isManualEnd) {
    // User ended the game manually via the Start switch
    lcd.print("Game Ended");
    isFinalScreen = true;  // Stop further LCD updates

    // Wait 4 seconds to let the user read the message
    vTaskDelay(4000 / portTICK_PERIOD_MS);

    // Clear the LCD and remain blank
    lcd.clear();
    Serial.println("Game Ended!");
    isGameOverShown = true;
    
  } else {
    // Player lost because of 3 mistakes
    lcd.print("Game Over :(");
    lcd.setCursor(0, 1);
    lcd.print("3 Mistakes!");
    Serial.println("Game Over: 3 Mistakes!");
    isGameOverShown = true;
    // Immediately set isFinalScreen so "Game Over" text stays
    isFinalScreen = true;

    // Keep "Game Over" up for 3 seconds
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Then prompt to restart
    lcd.clear();
    lcd.print("To restart");
    lcd.setCursor(0, 1);
    lcd.print("Click Restart");
    // We remain on isFinalScreen = true
  }
}
/* 
    Increments the game level and generates a new sequence.
    If currentLevel passes a certain point (20 here), the player wins.
*/
void advanceLevel() {
  currentLevel++;
  // using "if (currentLevel > 20)" as the win condition
  if (currentLevel > 20) {
    lcd.clear();
    lcd.print("You Win");
    lcd.setCursor(0, 1);
    lcd.print("Congrats !!");
    Serial.println("You Win!");

    // Show "You Win!" screen until restart
    isFinalScreen = true;

    // Wait 3 seconds
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Prompt for user to restart
    lcd.clear();
    lcd.print("To restart");
    lcd.setCursor(0, 1);
    lcd.print("Click Restart");
    Serial.println("To restart, click the Restart button in Blynk.");

    // Stop the game
    isGameRunning = false;
    return;
  }

  // If game not won, proceed to next level
  playerMistakes = 0;  // Reset mistakes each new level
  Serial.print("Level Up! Now at level: ");
  Serial.println(currentLevel);

  // Briefly show which level we're at on the LCD
  vTaskDelay(1000 / portTICK_PERIOD_MS); 
  lcd.clear();
  lcd.print("Going to Level ");
  lcd.print(currentLevel);

  // Temporarily suspend LCD updates to let the user see this message
  vTaskSuspend(taskLCDHandle); 
  vTaskDelay(2000 / portTICK_PERIOD_MS); 
  vTaskResume(taskLCDHandle);

  // Generate new sequence for the new level
  createRandomPattern();
}

/************************   FreeRTOS TASKS    *****************************************/

/*  
    Runs in a separate task to display the LED sequence to the player.
    When a new sequence is generated, it lights LEDs in order with a delay.
    After displaying, sets 'isPatternShown' to true so user can start input.
*/
void taskPatternLED(void *parameter) {
  while (true) {
    // Check if the game is started but the pattern not yet shown
    if (isGameRunning && !isPatternShown) {
      Serial.println("Displaying LED sequence...");
      lcd.clear();
      lcd.print("Watch closely!");

      // Go through each element in the pattern and blink the respective LED
      for (int i = 0; i < currentLevel + 2; i++) {
        int ledIndex = patternArray[i] - 1; 
        if (ledIndex >= 0 && ledIndex < totalLEDs) {
          digitalWrite(ledPins[ledIndex], HIGH);
          vTaskDelay(500 / portTICK_PERIOD_MS);
          digitalWrite(ledPins[ledIndex], LOW);
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }
      }
      lcd.clear();
      Serial.println("Sequence displayed.");
      // Now the user can start pressing the buttons
      isPatternShown = true;
    }
    // Check again every 100 ms
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/*  
    Updates the LCD with the score and mistakes every second
    if the final screen isn't currently displayed.
    If the game hasn't started, it displays a "Welcome" message.
*/
void taskUpdateLCD(void *parameter) {
  while (true) {
    // If we've displayed a final message (Win/GameOver), skip updates
    if (isFinalScreen) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }
    // Otherwise, refresh the LCD
    lcd.clear();
    // If the game is not over yet 
    if (!isGameOverShown) {
      // the game is actually started, show Score and Mistakes
      if (isGameRunning) {
        lcd.setCursor(0, 0);
        lcd.print("Score: ");
        lcd.print(playerScore);
        lcd.setCursor(0, 1);
        lcd.print("Mistakes: ");
        lcd.print(playerMistakes);
      } 
      // If the game hasn't started, show a welcome screen
      else {
        lcd.setCursor(0, 0);
        lcd.print("Welcome to");
        lcd.setCursor(0, 1);
        lcd.print("Mind Blink!");
      }
    }
    // Wait 1 second before updating again
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
