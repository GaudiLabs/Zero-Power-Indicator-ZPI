/*
  Flip-Dot Control of Two Coils

  This sketch demonstrates how to drive a bistable flip-dot display
  using two separate coils:
  - One coil sets the dot (ON)
  - The other coil resets the dot (OFF)

  Each coil is pulsed briefly to avoid overheating, followed by
  a pause before the next action.

  Connections:
  - COIL_FLIP1_PIN   -> Driver for the "flip in one direction" coil
  - COIL_FLIP2_PIN   -> Driver for the "flip in the other direction" coil

  Note:
  Always use appropriate driver circuitry (transistors, MOSFETs,
  flyback diodes) when driving coils from an Arduino.

  Public domain.
*/

// =====================
// Pin Definitions
// =====================
const uint8_t COIL_FLIP1_PIN   = 5;
const uint8_t COIL_FLIP2_PIN = 6;

// =====================
// Setup
// =====================
void setup() {
  pinMode(COIL_FLIP1_PIN, OUTPUT);
  pinMode(COIL_FLIP2_PIN, OUTPUT);

  // Ensure both coils are off at startup
  digitalWrite(COIL_FLIP1_PIN, LOW);
  digitalWrite(COIL_FLIP2_PIN, LOW);
}

// =====================
// Main Loop
// =====================
void loop() {
  // --- Set flip-dot ---
  digitalWrite(COIL_FLIP1_PIN, HIGH);
  digitalWrite(COIL_FLIP2_PIN, LOW);
  delay(10);  // short pulse to energize coil

  // Turn both coils off
  digitalWrite(COIL_FLIP1_PIN, LOW);
  digitalWrite(COIL_FLIP2_PIN, LOW);
  delay(1000);

  // --- Reset flip-dot ---
  digitalWrite(COIL_FLIP1_PIN, LOW);
  digitalWrite(COIL_FLIP2_PIN, HIGH);
  delay(10);  // short pulse to energize coil

  // Turn both coils off
  digitalWrite(COIL_FLIP1_PIN, LOW);
  digitalWrite(COIL_FLIP2_PIN, LOW);
  delay(1000);
}
