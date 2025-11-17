#include <AFMotor.h>  // motor shield support library so pumps can run
#include <Wire.h>      // wire library required by the motor shield firmware


const byte STOP_BUTTON_PIN = A14;  // wired stop button that pauses everything
const byte START_BUTTON_PIN = A15; // wired start button that resumes the cycle

// Initialize motors and pumps
AF_DCMotor diaphragmPumpMotor(2); 
AF_DCMotor peristalticPumpMotor(1);
AF_DCMotor impellerMotor(4);
AF_DCMotor pressMotor(3);

const byte RELAY_PIN = 53;     // relay that toggles the submersible pump
const byte STEP_COUNT = 15;    // total number of steps in the system

// Durations for each numbered step (milliseconds) listed in execution order
const unsigned long stepTime[STEP_COUNT] = {
  10000, // Step 0  - diaphragm pump runs
  0,     // Step 1  - diaphragm pump releases
  2000,  // Step 2  - wait so fluid settles
  5000,  // Step 3  - peristaltic pump runs
  0,     // Step 4  - peristaltic pump releases
  2000,  // Step 5  - wait before impeller work
  10000, // Step 6  - impeller fast mix
  10000, // Step 7  - impeller slow mix
  0,     // Step 8  - impeller releases
  2000,  // Step 9  - wait so foam can drop
  20000, // Step 10 - press motor runs
  0,     // Step 11 - press motor releases
  2000,  // Step 12 - wait before relay action
  8000,  // Step 13 - relay energizes
  0      // Step 14 - relay turns off
};

byte currentStepIndex = 255;        // 255 means idle before the first run
bool processRunning = false;        // true while the drink sequence advances
unsigned long stepStartTime = 0;    // timestamp when the current step began
unsigned long pausedElapsedTime = 0;// time already spent in a paused step

// NEW: variables needed for edge-trigger button behavior
bool stopButtonPreviouslyPressed = false;
bool startButtonPreviouslyPressed = false;

void startSystem();        // starts or resumes the drink sequence
void stopSystem();         // pauses the sequence and releases hardware
void updateProcess();      // checks timers and advances steps
void enterStep(byte idx);  // sets the timer for a new step
void applyStep(byte idx);  // drives the hardware for a specific step
void stopAllActuators();   // turns every motor and relay off

void setup() {
  Serial.begin(9600);                         // open the USB serial log port
  Serial.println("Hi! I'm MARTY, hope you're thirsty");

  // Configure buttons as pull-ups so they read LOW when pressed.
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);

  // Prep the relay output so the accessory stays off until needed.
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Begin with every actuator released for safety.
  stopAllActuators();
}

void loop() {

  // === NEW EDGE-TRIGGER BUTTON LOGIC ===
  bool stopPressedNow = (digitalRead(STOP_BUTTON_PIN) == LOW);
  bool startPressedNow = (digitalRead(START_BUTTON_PIN) == LOW);

  if (stopPressedNow && !stopButtonPreviouslyPressed) {
    stopSystem();   // stop only once per press
  } 
  else if (startPressedNow && !startButtonPreviouslyPressed) {
    startSystem();  // start only once per press
  }

  // update stored states
  stopButtonPreviouslyPressed = stopPressedNow;
  startButtonPreviouslyPressed = startPressedNow;
  // === END NEW BUTTON LOGIC ===


  updateProcess();  // unchanged
}


// startSystem starts or resumes the drink cycle at the stored step index.
void startSystem() {
  if (currentStepIndex >= STEP_COUNT) {
    currentStepIndex = 0;                // restart from the first step
    pausedElapsedTime = 0;               // clear any stale pause timing
  }

  if (!processRunning) {
    processRunning = true;               // flag indicates timing should advance
    if (pausedElapsedTime == 0) {
      if (currentStepIndex == 255) {
        currentStepIndex = 0;            // move out of idle so first step runs
      }
      Serial.println("System starting.");
      enterStep(currentStepIndex);       // begin step timer and hardware actions
    } else {
      stepStartTime = millis() - pausedElapsedTime; 
      pausedElapsedTime = 0;             
      applyStep(currentStepIndex);     
      Serial.println("System resuming.");
    }
  }
}

void stopSystem() {
  if (processRunning) {
    pausedElapsedTime = millis() - stepStartTime;
  }
  processRunning = false;
  stopAllActuators();
  Serial.println("System stopped.");
}

void updateProcess() {
  if (!processRunning || currentStepIndex >= STEP_COUNT) {
    return;
  }

  unsigned long duration = stepTime[currentStepIndex];

  if (duration == 0 || millis() - stepStartTime >= duration) {
    currentStepIndex++;
    if (currentStepIndex >= STEP_COUNT) {
      processRunning = false;
      stopAllActuators();
      Serial.println("All done! Drink up :)");
    } else {
      enterStep(currentStepIndex);
    }
  }
}

void enterStep(byte idx) {
  currentStepIndex = idx;
  stepStartTime = millis();
  pausedElapsedTime = 0;
  applyStep(idx);
}

void applyStep(byte idx) {
  switch (idx) {
    case 0:
      diaphragmPumpMotor.setSpeed(255);
      diaphragmPumpMotor.run(FORWARD);
      Serial.println("Diaphragm pump on.");
      break;
    case 1:
      diaphragmPumpMotor.run(RELEASE);
      break;
    case 3:
      peristalticPumpMotor.setSpeed(255);
      peristalticPumpMotor.run(FORWARD);
      Serial.println("Peri pump on.");
      break;
    case 4:
      peristalticPumpMotor.run(RELEASE);
      break;
    case 6:
      impellerMotor.setSpeed(180);
      impellerMotor.run(FORWARD);
      Serial.println("Imp fast.");
      break;
    case 7:
      impellerMotor.setSpeed(100);
      impellerMotor.run(FORWARD);
      Serial.println("Imp slow.");
      break;
    case 8:
      impellerMotor.run(RELEASE);
      break;
    case 10:
      pressMotor.setSpeed(180);
      pressMotor.run(FORWARD);
      Serial.println("Press motor on.");
      break;
    case 11:
      pressMotor.run(RELEASE);
      break;
    case 13:
      digitalWrite(RELAY_PIN, HIGH);
      delay(8000);
      Serial.println("Relay on.");
      break;
    case 14:
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay off.");
      break;
    default:
      stopAllActuators();
      break;
  }
}

void stopAllActuators() {
  diaphragmPumpMotor.run(RELEASE);
  peristalticPumpMotor.run(RELEASE);
  impellerMotor.run(RELEASE);
  pressMotor.run(RELEASE);
  digitalWrite(RELAY_PIN, LOW);
}
