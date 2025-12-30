/*
 * Electrochromic Cell Active Driver for Seeed XIAO SAMD21
 * 
 * Hardware Setup:
 * - Virtual ground at VCC/2 (1.65V) connected to one electrode
 * - Pin A0 connected to other electrode (work electrode)
 * - A0 switches between ADC input (for OCP measurement) and digital output (for driving)
 * 
 * Based on Ynvisible Active Driving method from datasheet section 8.2
 */

// Pin definitions
const int CELL_PIN = A0;

// Voltage thresholds (from datasheet section 8.2)
const float V_LCRL =0.7;// 0.9;  // Lower Colored Range Limit
const float V_UCRL =1.4;//1.4;  // Upper Colored Range Limit
const float V_LBRL = -1.4;//-1.4; // Lower Bleached Range Limit
const float V_UBRL = -0.7;//-1.6; // Upper Bleached Range Limit

// Driving parameters
const float DRIVE_VOLTAGE_POSITIVE = 1.8;  // Coloring voltage
const float DRIVE_VOLTAGE_NEGATIVE = -1.8; // Bleaching voltage
const int PULSE_DURATION_MS = 100;         // Duration of each voltage pulse
const int MEASUREMENT_DELAY_MS = 50;       // Delay before OCP measurement
const int REFRESH_INTERVAL_MS = 1000;      // Refresh interval (5 seconds)

// ADC reference voltage
const float V_REF = 3.3;
const float V_VIRTUAL_GND = V_REF / 2.0;  // 1.65V

// State variables
enum CellState { COLORED, BLEACHED };
CellState targetState = COLORED;
CellState currentState = BLEACHED;
unsigned long lastRefreshTime = 0;
unsigned long lastSwitchTime = 0;
const unsigned long SWITCH_INTERVAL_MS = 125000; // 0.2 Hz = 5 second period

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial or timeout
  
  Serial.println("Electrochromic Cell Active Driver");
  Serial.println("==================================");
  
  // Initialize ADC resolution (SAMD21 supports up to 12-bit)
  analogReadResolution(12);
  
  // Start with pin in high-impedance mode
  pinMode(CELL_PIN, INPUT);
  
  Serial.println("Setup complete. Starting test cycle at 0.2 Hz...\n");
  
  lastSwitchTime = millis();
}

void loop() {
  // Toggle target state every 5 seconds (0.2 Hz)
  if (millis() - lastSwitchTime >= SWITCH_INTERVAL_MS) {
    targetState = (targetState == COLORED) ? BLEACHED : COLORED;
    lastSwitchTime = millis();
    
    Serial.println("\n===========================================");
    Serial.print("Switching to: ");
    Serial.println(targetState == COLORED ? "COLORED (ON)" : "BLEACHED (OFF)");
    Serial.println("===========================================");
  }
  
  // Perform switching or refreshing based on target state
  if (targetState == COLORED && currentState != COLORED) {
    colorSegment();
  } else if (targetState == BLEACHED && currentState != BLEACHED) {
    bleachSegment();
  } else {
    // Refresh the current state
    if (millis() - lastRefreshTime >= REFRESH_INTERVAL_MS) {
      if (currentState == COLORED) {
        refreshColoredSegment();
      } else {
        refreshBleachedSegment();
      }
      lastRefreshTime = millis();
    }
  }
  
  delay(100); // Small delay between operations
}

// Measure Open Circuit Potential (OCP) voltage across the segment
float measureOCP() {
  // Set pin to high-impedance input mode
  pinMode(CELL_PIN, INPUT);
  
  // Wait for voltage to stabilize
  delay(MEASUREMENT_DELAY_MS);
  
  // Read ADC value
  int adcValue = analogRead(CELL_PIN);
  
  // Convert to voltage
  float voltage = (adcValue / 4095.0) * V_REF;
  
  // Calculate voltage relative to virtual ground
  float relativeVoltage = voltage - V_VIRTUAL_GND;
  
  return relativeVoltage;
}

// Apply positive voltage pulse for coloring
void applyPositivePulse() {
  // Calculate target voltage: Virtual_GND + 1.5V
  float targetVoltage = V_VIRTUAL_GND + DRIVE_VOLTAGE_POSITIVE;
  
  // Since we can't exceed VCC, clamp to VCC
  if (targetVoltage > V_REF) {
    // Drive pin HIGH (3.3V)
    pinMode(CELL_PIN, OUTPUT);

digitalWrite(CELL_PIN, HIGH);
  } else {
    // Use PWM to approximate the voltage
    pinMode(CELL_PIN, OUTPUT);
    int pwmValue = (int)((targetVoltage / V_REF) * 255);
    analogWrite(CELL_PIN, pwmValue);
  }
  
  delay(PULSE_DURATION_MS);
  
  // Return to high-impedance
  pinMode(CELL_PIN, INPUT);
}

// Apply negative voltage pulse for bleaching
void applyNegativePulse() {
  // Calculate target voltage: Virtual_GND - 1.5V
  // This means driving the pin LOW (0V) since virtual GND is at 1.65V
  pinMode(CELL_PIN, OUTPUT);
  digitalWrite(CELL_PIN, LOW);
  
  delay(PULSE_DURATION_MS);
  
  // Return to high-impedance
  pinMode(CELL_PIN, INPUT);
}

// Color one segment (turn ON) - Section 8.2.1
void colorSegment() {
  Serial.println("\n--- Coloring Segment ---");
  
  int iteration = 0;
  while (true) {
    float ocp = measureOCP();
    
    Serial.print("Iteration ");
    Serial.print(++iteration);
    Serial.print(": OCP = ");
    Serial.print(ocp, 3);
    Serial.print("V, Target: ");
    Serial.print(V_UCRL, 3);
    Serial.println("V");
    
    if (ocp >= V_UCRL) {
      Serial.println("✓ Coloring complete!");
      currentState = COLORED;
      lastRefreshTime = millis();
      break;
    }
    
    Serial.println("  Applying positive pulse...");
    applyPositivePulse();
  }
}

// Bleach one segment (turn OFF) - Section 8.2.2
void bleachSegment() {
  Serial.println("\n--- Bleaching Segment ---");
  
  int iteration = 0;
  while (true) {
    float ocp = measureOCP();
    
    Serial.print("Iteration ");
    Serial.print(++iteration);
    Serial.print(": OCP = ");
    Serial.print(ocp, 3);
    Serial.print("V, Target: ");
    Serial.print(V_LBRL, 3);
    Serial.println("V");
    
    if (ocp <= V_LBRL) {
      Serial.println("✓ Bleaching complete!");
      currentState = BLEACHED;
      lastRefreshTime = millis();
      break;
      
    }
    
    Serial.println("  Applying negative pulse...");
    applyNegativePulse();
  }
}

// Refresh colored segment - Section 8.2.3
void refreshColoredSegment() {
  float ocp = measureOCP();
  
  Serial.print("\n[Refresh Colored] OCP = ");
  Serial.print(ocp, 3);
  Serial.print("V (threshold: ");
  Serial.print(V_LCRL, 3);
  Serial.println("V)");
  
  if (ocp < V_LCRL) {
    Serial.println("OCP below threshold, recoloring...");
    colorSegment();
  } else {
    Serial.println("State maintained ✓");
  }
}

// Refresh bleached segment - Section 8.2.4
void refreshBleachedSegment() {
  float ocp = measureOCP();
  
  Serial.print("\n[Refresh Bleached] OCP = ");
  Serial.print(ocp, 3);
  Serial.print("V (threshold: ");
  Serial.print(V_UBRL, 3);
  Serial.println("V)");
  
  if (ocp > V_UBRL) {
    Serial.println("OCP above threshold, rebleaching...");
    bleachSegment();
  } else {
    Serial.println("State maintained ✓");
  }
}
