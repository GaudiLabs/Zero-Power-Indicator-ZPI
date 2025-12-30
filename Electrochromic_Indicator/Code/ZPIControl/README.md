# Electrochromic Cell Active Driver for Seeed XIAO SAMD21

An Arduino implementation of active driving control for electrochromic displays, based on the Ynvisible active driving methodology. This driver enables precise state control and maintenance of electrochromic cells through open-circuit potential (OCP) monitoring and adaptive voltage pulsing.

## Features

- **Active Driving Implementation**: Full implementation of Ynvisible's active driving method (datasheet section 8.2)
- **OCP-Based State Control**: Measures open-circuit potential to determine and maintain cell state
- **Automatic State Maintenance**: Periodic refresh cycles prevent state degradation
- **Colored/Bleached Transitions**: Reliable switching between ON (colored) and OFF (bleached) states
- **Virtual Ground Architecture**: Uses VCC/2 (1.65V) reference for bipolar driving from single supply
- **Adaptive Pulse Control**: Continues pulsing until target voltage thresholds are reached

## Hardware Requirements

### Components
- **Microcontroller**: Seeed XIAO SAMD21 (or compatible SAMD21-based board)
- **Electrochromic Display**: Ynvisible electrochromic cell or compatible device
- **Resistors**: Two equal-value resistors for virtual ground voltage divider (e.g., 10kΩ each)

### Wiring Diagram
```
VCC (3.3V) ----[R1]----+----[R2]---- GND
                       |
                  Virtual GND (1.65V)
                       |
                   Electrode 1
                       
Pin A0 ----------- Electrode 2
```

## Installation

1. Install the Arduino IDE if you haven't already
2. Add Seeed SAMD board support:
   - Go to **File → Preferences**
   - Add to Additional Board Manager URLs: `https://files.seeedstudio.com/arduino/package_seeedstudio_index.json`
   - Go to **Tools → Board → Boards Manager**
   - Search for "Seeed SAMD" and install
3. Select **Tools → Board → Seeed XIAO M0**
4. Download this repository and open the `.ino` file
5. Upload to your XIAO SAMD21

## Usage

### Basic Operation

1. Connect your electrochromic cell as shown in the wiring diagram
2. Upload the code to your XIAO SAMD21
3. Open the Serial Monitor (**Tools → Serial Monitor**) at **115200 baud**
4. Watch the cell cycle between colored (ON) and bleached (OFF) states

### Demo Behavior

The default code demonstrates the driver by automatically cycling between states:
- **Every 5 seconds**: Switches target state (0.2 Hz cycle frequency)
- **Every 1 second**: Checks and refreshes current state if needed
- Real-time diagnostic information is printed to Serial Monitor

### Serial Monitor Output Example

```
Electrochromic Cell Active Driver
==================================
Setup complete. Starting test cycle at 0.2 Hz...

===========================================
Switching to: COLORED (ON)
===========================================

--- Coloring Segment ---
Iteration 1: OCP = -0.523V, Target: 1.400V
  Applying positive pulse...
Iteration 2: OCP = 0.847V, Target: 1.400V
  Applying positive pulse...
Iteration 3: OCP = 1.456V, Target: 1.400V
✓ Coloring complete!

[Refresh Colored] OCP = 1.234V (threshold: 0.700V)
State maintained ✓
```

## Configuration

### Voltage Thresholds

Adjust these constants in the code to match your specific electrochromic cell characteristics:

```cpp
const float V_LCRL = 0.7;   // Lower Colored Range Limit
const float V_UCRL = 1.4;   // Upper Colored Range Limit
const float V_LBRL = -1.4;  // Lower Bleached Range Limit
const float V_UBRL = -0.7;  // Upper Bleached Range Limit
```

### Driving Parameters

```cpp
const float DRIVE_VOLTAGE_POSITIVE = 1.8;  // Coloring voltage (V)
const float DRIVE_VOLTAGE_NEGATIVE = -1.8; // Bleaching voltage (V)
const int PULSE_DURATION_MS = 100;         // Pulse duration (ms)
const int MEASUREMENT_DELAY_MS = 50;       // Stabilization delay (ms)
const int REFRESH_INTERVAL_MS = 1000;      // Refresh check interval (ms)
```

### Test Cycle Frequency

```cpp
const unsigned long SWITCH_INTERVAL_MS = 5000; // Time between state changes (ms)
```

## How It Works

### Active Driving Method

The driver implements a feedback-controlled approach:

1. **Measure OCP**: Pin switches to high-impedance input mode to measure the cell's open-circuit potential
2. **Evaluate State**: Compares measured OCP against target voltage thresholds
3. **Apply Pulse**: If needed, applies a voltage pulse (positive for coloring, negative for bleaching)
4. **Iterate**: Repeats until target voltage is achieved
5. **Maintain**: Periodically checks and refreshes state to prevent degradation

### Virtual Ground

Since electrochromic cells require bipolar driving (both positive and negative voltages), but the microcontroller only provides 0-3.3V, a virtual ground at 1.65V is created using a voltage divider. This allows:
- **Positive voltage**: Drive pin HIGH (3.3V) → +1.65V relative to virtual ground
- **Negative voltage**: Drive pin LOW (0V) → -1.65V relative to virtual ground

### Pin Multiplexing

The same pin (A0) is used for both measurement and driving by switching between modes:
- **INPUT mode**: High-impedance, allows OCP measurement via ADC
- **OUTPUT mode**: Drives electrode with voltage pulses

## API Reference

### Main Functions

#### `colorSegment()`
Transitions the cell to the colored (ON) state by applying positive voltage pulses until OCP reaches the upper threshold.

#### `bleachSegment()`
Transitions the cell to the bleached (OFF) state by applying negative voltage pulses until OCP reaches the lower threshold.

#### `refreshColoredSegment()`
Checks if colored state is maintained; re-colors if OCP drops below lower colored threshold.

#### `refreshBleachedSegment()`
Checks if bleached state is maintained; re-bleaches if OCP rises above upper bleached threshold.

#### `measureOCP()`
Measures and returns the open-circuit potential across the cell (in volts, relative to virtual ground).

#### `applyPositivePulse()`
Applies a single positive voltage pulse for coloring.

#### `applyNegativePulse()`
Applies a single negative voltage pulse for bleaching.

## Troubleshooting

### Cell not changing state
- Check wiring connections
- Verify virtual ground voltage is at 1.65V
- Adjust voltage thresholds in code
- Increase pulse duration if cell responds slowly

### Inconsistent measurements
- Increase `MEASUREMENT_DELAY_MS` for better voltage stabilization
- Check for loose connections
- Verify ADC resolution is set to 12-bit

### State degradation
- Decrease `REFRESH_INTERVAL_MS` for more frequent refresh checks
- Adjust threshold ranges to be more conservative

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License

## References

- Ynvisible Electrochromic Display Active Driving Method (Datasheet Section 8.2)
- Seeed XIAO SAMD21 Documentation

## Acknowledgments

Based on electrochromic active driving principles for bistable display applications.

---

**Note**: This code is provided as-is for educational and experimental purposes. Always consult your electrochromic cell's datasheet for proper operating parameters.
