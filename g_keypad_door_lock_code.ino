#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

#define BUZZER_PIN 13
#define PASSWORD_LENGTH 4 // 4-digit password

const char TARGET_PASSWORD[] = "4321";
char guessedPassword[PASSWORD_LENGTH + 1]; // +1 for null terminator '\0'
byte currentLength = 0;
byte cursorColumn = 6;
bool isDoorOpen = true; // Starts unlocked at servo angle 50

const byte ROWS = 4;
const byte COLS = 4;

// Note: Check Serial Monitor when pressing keys! 
// Standard 4x4 keypads usually start with '1','2','3','A' on row 0.
// Keep your layout below ONLY if your wiring reverses the rows/columns.
char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int rs = 12, en = 11, d4 = A0, d5 = A1, d6 = A2, d7 = A3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

Servo servo;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize LCD BEFORE servo to avoid startup voltage drop issues
  lcd.begin(16, 2);
  lcd.setCursor(3, 0);
  lcd.print("WELCOME TO");
  lcd.setCursor(0, 1);
  lcd.print("DOOR LOCK SYSTEM");

  // Initialize Servo
  servo.attach(10);
  servo.write(50); // Unlocked position

  delay(2500);
  showPrompt();
}

void loop() {
  char key = keypad.getKey();

  if (key != NO_KEY) {
    Serial.print("Key pressed: ");
    Serial.println(key); // Helpful for checking keypad orientation!

    // Short keypress feedback beep
    tone(BUZZER_PIN, 2000, 50);

    if (key == 'C') {
      resetInput();
      showPrompt();
    } else if (key == 'D') {
      checkPassword();
    } else {
      processKey(key);
    }
  }
}

void processKey(char key) {
  if (currentLength < PASSWORD_LENGTH) {
    lcd.setCursor(cursorColumn, 1);
    lcd.print("*");
    cursorColumn++;

    guessedPassword[currentLength] = key;
    currentLength++;
    guessedPassword[currentLength] = '\0'; // Keep string null-terminated
  }
}

void checkPassword() {
  lcd.clear();

  if (strcmp(guessedPassword, TARGET_PASSWORD) == 0) {
    // Success beep
    tone(BUZZER_PIN, 1500, 200);
    delay(250);

    if (isDoorOpen) {
      servo.write(110); // Lock door
      isDoorOpen = false;
      lcd.setCursor(0, 0);
      lcd.print("CORRECT PASSWORD");
      lcd.setCursor(2, 1);
      lcd.print("DOOR LOCKED");
    } else {
      servo.write(50);  // Open door
      isDoorOpen = true;
      lcd.setCursor(0, 0);
      lcd.print("CORRECT PASSWORD");
      lcd.setCursor(2, 1);
      lcd.print("DOOR OPENED");
    }
    delay(2000);
  } else {
    // Error alarm (3 low beeps)
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE TRY AGAIN");

    for (int i = 0; i < 3; i++) {
      tone(BUZZER_PIN, 400, 200);
      delay(400);
    }
    delay(1000);
  }

  resetInput();
  showPrompt();
}

void resetInput() {
  currentLength = 0;
  guessedPassword[0] = '\0';
  cursorColumn = 6;
}

void showPrompt() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");
}