# Motor Reset Fix Guide - Lolin32 Lite with PWM DC Motor

## Problem
ESP32 (Lolin32 Lite) resets mid-run when PWM DC motor is running through MOSFET.

## Root Causes (Most Likely Hardware)

### 1. **Power Supply Issues** (MOST COMMON - 80% of cases)
- **Insufficient current capacity**: Motor draws more current than power supply can provide
- **Voltage drop**: When motor starts or changes speed, voltage drops below ESP32's minimum (2.8V)
- **Brown-out detection**: ESP32 has built-in brown-out detector that resets when voltage drops

**Solutions:**
- Use a **separate power supply** for the motor (don't power motor from ESP32's 5V pin)
- Use a power supply rated for **at least 2-3x the motor's stall current**
- Add a **large capacitor** (1000-4700µF) across motor power supply (close to motor)
- Use a **buck converter** or **LDO regulator** with good current capacity for ESP32
- Ensure power supply can handle **peak current** (motor startup draws 3-5x normal current)

### 2. **Electrical Noise / EMI** (15% of cases)
- Motor switching creates electrical noise that interferes with ESP32
- Fast PWM switching causes voltage spikes on power lines
- Noise couples into ESP32 through power or ground

**Solutions:**
- Add **100nF ceramic capacitor** close to ESP32's VCC and GND pins
- Add **10µF electrolytic capacitor** across ESP32 power supply
- Add **ferrite bead** on motor power line
- Use **twisted pair wires** for motor connections
- Keep motor wires **away from ESP32** and I2C/sensor wires
- Add **snubber circuit** (10Ω + 100nF) across motor terminals

### 3. **Ground Loops** (5% of cases)
- Improper grounding between motor power supply and ESP32
- Ground potential differences cause current spikes

**Solutions:**
- **Common ground**: Connect motor power supply ground to ESP32 ground at ONE point only
- Use **thick ground wire** (at least 18 AWG)
- Keep ground connections **short and direct**
- Don't daisy-chain grounds through multiple components

### 4. **MOSFET Issues**
- MOSFET switching transients cause voltage spikes
- Gate drive issues (too slow switching = more heat, too fast = more noise)
- MOSFET not fully on/off (linear region = more heat)

**Solutions:**
- Use **logic-level MOSFET** (IRLZ44N, IRL540N) - don't need gate driver
- Add **gate resistor** (100-470Ω) to limit switching speed and reduce noise
- Add **flyback diode** (1N4007 or Schottky) across motor (cathode to +V, anode to GND)
- Use **proper heatsink** if MOSFET gets hot
- Check MOSFET is fully on: Vgs should be >10V for N-channel MOSFETs

## Code Improvements (Already Added)

1. **Soft-start PWM ramping**: Gradually changes PWM instead of instant changes
   - Reduces current spikes
   - Prevents sudden voltage drops
   - Configurable ramp rate (currently 5 per 10ms)

2. **Brown-out detector notes**: Added comments about BOD configuration
   - ESP32 BOD can be configured via menuconfig/sdkconfig
   - Default 2.43V is too sensitive for motor applications
   - Recommend 2.5V or 2.67V threshold

## Hardware Setup Recommendations

### Recommended Circuit:
```
Motor Power Supply (12V, 2A+)
    |
    +--[1000µF cap]--+
    |                |
    +-- Motor --+    |
               |    |
    MOSFET Drain    |
    |                |
    +--[Flyback Diode]--+
    |                    |
    GND (common)         |
                         |
ESP32 Power (5V, 500mA+) |
    |                    |
    +--[10µF + 100nF]--+ |
    |                    |
    GND (common) --------+
    
MOSFET Gate --[100Ω]-- ESP32 Pin 12
MOSFET Source -- GND
```

### Component Recommendations:
- **Motor Power Supply**: 12V 2-3A (or match your motor requirements)
- **ESP32 Power**: 5V 500mA+ (separate supply or good quality USB)
- **MOSFET**: IRLZ44N or IRL540N (logic-level, low Rds(on))
- **Flyback Diode**: 1N4007 (1A) or Schottky 1N5819 (faster)
- **Capacitors**: 
  - 1000-4700µF electrolytic (motor power supply)
  - 10µF + 100nF ceramic (ESP32 power)
  - 100nF ceramic (close to ESP32 VCC/GND)

## Testing Steps

1. **Measure voltage during motor operation**:
   - Use multimeter on ESP32's 3.3V pin
   - Watch for voltage drops when motor starts/changes speed
   - Should stay above 3.0V at all times

2. **Check current draw**:
   - Measure motor current with multimeter
   - Ensure power supply can handle peak current
   - Motor startup current is 3-5x normal running current

3. **Monitor for resets**:
   - Add serial logging to detect reset cause
   - Check if resets happen during motor start or speed changes
   - Note if resets are random or consistent

## PlatformIO Configuration (Optional)

To adjust brown-out detector threshold, add to `platformio.ini`:

```ini
[env:esp32_controlls_ps4]
platform = espressif32
board = lolin32_lite
framework = arduino
board_build.arduino.memory_type = qio_opi
board_build.f_cpu = 240000000L
board_build.flash_size = 4MB
board_build.partitions = default_4MB.csv
build_flags = 
    -DCONFIG_ESP32_BROWNOUT_DET=1
    -DCONFIG_ESP32_BROWNOUT_DET_LVL=2  ; 2.5V threshold
    ; Options: 0=2.43V, 1=2.50V, 2=2.67V, 3=2.80V
```

## Quick Fix Checklist

- [ ] Use separate power supply for motor (not ESP32's 5V)
- [ ] Add 1000µF capacitor across motor power supply
- [ ] Add flyback diode across motor
- [ ] Add 100nF + 10µF capacitors on ESP32 power
- [ ] Use logic-level MOSFET (IRLZ44N)
- [ ] Add 100Ω gate resistor
- [ ] Ensure common ground connection
- [ ] Keep motor wires away from ESP32
- [ ] Test with multimeter: voltage should stay >3.0V
- [ ] Verify power supply can handle motor's peak current

## Most Likely Fix

**90% of reset issues are solved by:**
1. Using a **separate power supply** for the motor
2. Adding a **large capacitor** (1000µF+) across motor power
3. Adding a **flyback diode** across the motor
4. Ensuring **common ground** connection

If resets persist after these fixes, the issue is likely:
- Insufficient power supply current capacity
- MOSFET overheating
- Severe EMI requiring better filtering

