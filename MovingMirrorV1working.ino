// Define Constants

// Connections to A4988 (Swapped for LR and UD)
const int LRdirPin = 5;  // Direction for LR motor (was UD)
const int LRstepPin = 6; // Step for LR motor (was UD)

const int UDdirPin = 2;  // Direction for UD motor (was LR)
const int UDstepPin = 3; // Step for UD motor (was LR)

// Define the analog pins for the X and Y axis
const int X_PIN = A0;
const int Y_PIN = A1;

// Define the digital pin for the button 
const int BUTTON_PIN = 8;

// Variables to track the motor positions
int UDposition = 0;
int LRposition = 0;
const int maxPosition = 300;  // Maximum position (+300 or -300)

// Variables to store the target positions
int targetUDposition = 0;
int targetLRposition = 0;

// State variables
bool buttonPressed = false;
bool automaticMoveActive = false;  // Whether automatic movement is active
bool returningToZero = false;      // Whether the motors are returning to (0,0)
unsigned long lastMoveTime = 0;    // Track last automatic move time
const unsigned long moveInterval = 300000;  // 5 minutes between each move (300000 ms)

// Deadzone threshold
const int deadzone = 100;  // Deadzone range from center (0-1023)

void setup() {
  // Setup the pins as Outputs
  pinMode(LRstepPin, OUTPUT);  // Now LR uses pins 5 and 6
  pinMode(LRdirPin, OUTPUT);

  pinMode(UDstepPin, OUTPUT);  // Now UD uses pins 2 and 3
  pinMode(UDdirPin, OUTPUT);

  // Initialize the pushbutton pin as an input:
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  // Read the joystick positions
  int xValue = analogRead(X_PIN);
  int yValue = analogRead(Y_PIN);

  // Read the button state (LOW when pressed, HIGH when not pressed)
  int buttonState = digitalRead(BUTTON_PIN);

  // Apply deadzone to the joystick input
  int xDirection = 0;
  int yDirection = 0;

  if (xValue < (512 - deadzone)) {
    xDirection = -1;  // Move left
  } else if (xValue > (512 + deadzone)) {
    xDirection = 1;   // Move right
  }

  if (yValue < (512 - deadzone)) {
    yDirection = -1;  // Move down
  } else if (yValue > (512 + deadzone)) {
    yDirection = 1;   // Move up
  }

  // Check if the button is pressed to start/stop automatic movement
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;

    if (!automaticMoveActive) {
      // First press: lock the current position and start automatic movement
      automaticMoveActive = true;
      lastMoveTime = millis();
      Serial.println("Automatic movement started!");
    } else {
      // Stop automatic movement
      automaticMoveActive = false;
      returningToZero = true;
      targetLRposition = 0;
      targetUDposition = 0;
      Serial.println("Returning to zero position!");
    }
    
    delay(500);  // Debounce delay
  } else if (buttonState == HIGH) {
    buttonPressed = false;
  }

  // Handle joystick-controlled movement only when automatic movement is not active
  if (!automaticMoveActive && !returningToZero) {
    if (xDirection == 1 && LRposition < maxPosition) {
      targetLRposition++;
    } else if (xDirection == -1 && LRposition > -maxPosition) {
      targetLRposition--;
    }

    if (yDirection == 1 && UDposition < maxPosition) {
      targetUDposition++;
    } else if (yDirection == -1 && UDposition > -maxPosition) {
      targetUDposition--;
    }
  }

  // Automatic movement: move UP and RIGHT every 5 minutes (300,000 ms)
  if (automaticMoveActive && (millis() - lastMoveTime >= moveInterval)) {
    lastMoveTime = millis();  // Reset the timer

    if (LRposition < maxPosition && UDposition < maxPosition) {
      // Move UP 5 and RIGHT 5 until we reach the limit
      targetLRposition += 10;  // Increase LR for right movement
      targetUDposition += 10;  // Increase UD for upward movement
      Serial.println("Moving up and right by 5!");
    } else {
      // If we've hit the limit, return to zero
      automaticMoveActive = false;
      returningToZero = true;
      targetLRposition = 0;
      targetUDposition = 0;
      Serial.println("Reached limit. Returning to zero!");
    }
  }

  // Move LR motor towards the target position (adjusted for correct direction)
  if (LRposition != targetLRposition) {
    moveMotor(LRdirPin, LRstepPin, LRposition < targetLRposition);  // Moving right
    LRposition += (LRposition < targetLRposition) ? 1 : -1;
  }

  // Move UD motor towards the target position
  if (UDposition != targetUDposition) {
    moveMotor(UDdirPin, UDstepPin, UDposition > targetUDposition);  // Reverse direction for UP
    UDposition += (UDposition < targetUDposition) ? 1 : -1;
  }

  // Check if the motors have returned to (0, 0)
  if (LRposition == 0 && UDposition == 0 && returningToZero) {
    returningToZero = false;  // Reset the return-to-zero flag
    Serial.println("Returned to (0, 0). Waiting for joystick input.");
  }

  // Print current positions for debugging
  Serial.print("LR Position: ");
  Serial.print(LRposition);
  Serial.print("  |  UD Position: ");
  Serial.println(UDposition);

  delay(100);  // Small delay to slow down the loop
}

// Function to move the motor
void moveMotor(int dirPin, int stepPin, bool forward) {
  digitalWrite(dirPin, forward ? HIGH : LOW);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000);  // Slow movement
  digitalWrite(stepPin, LOW);
  delayMicroseconds(1000);  // Slow movement
}
