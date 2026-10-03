# PosisiRobot (English)

[Bahasa Indonesia](README.md)

An Arduino **odometry** (dead reckoning) library for two-wheeled differential-drive robots: x, y position in cm and heading from wheel encoders. Non-blocking. The API and examples are in Indonesian. This page maps every function to English.

```cpp
#include <PosisiRobot.h>

// Wheel diameter 6.5 cm, track width 15 cm, 374 encoder counts per wheel turn.
PosisiRobot pose(6.5, 15.0, 374);

volatile long leftCount = 0, rightCount = 0; // updated by encoder interrupts

void loop() {
  noInterrupts();
  long l = leftCount, r = rightCount;
  interrupts();
  pose.perbarui(l, r);             // update(leftTotal, rightTotal)

  float turn = pose.selisihArahKe(50, 100); // degrees to turn toward (50, 100)
}
```

## Coordinates

Map-like, with the robot at (0, 0) at start or after `reset()`: heading 0 = +Y (forward at start), 90 = +X (right at start). Heading is 0–360 degrees, increasing **clockwise**, the same convention as [ArahMPU6050](https://github.com/nothinx/ArahMPU6050), so a gyro heading can be passed straight to `aturArah()`.

## Why

- Takes **cumulative** encoder counts, so no pulses are lost if `perbarui()` runs late. Safe across `long` counter overflow.
- Exact arc integration: curved motion between updates is integrated as a circular arc, not a straight line.
- Go-to-point helpers: `jarakKe()` (distance), `arahKe()` (bearing), `selisihArahKe()` (signed turn, positive = right).
- Separate left/right wheel diameters for calibration, with a calibration example.
- 41 bytes of RAM, no interrupts inside the library, works with any encoder code.

Checked against the source of other odometry libraries in the Library Manager: DeadReckoning-library takes unsigned counts with a separate direction setting, reads `volatile unsigned long` counters without disabling interrupts, and its bundled example calls the constructor with 5 arguments where 6 are required (fails to compile on the `master` branch). Aerobotix_Arduino_nav blocks in `while` + `delay(10)` loops and estimates position in `go()` as total distance × cos/sin of the current heading instead of tracking x, y.

## Function reference

| Indonesian | English | Notes |
|---|---|---|
| `PosisiRobot(diameterRoda, jarakRoda, pulsaPerPutaran)` | constructor(wheelDiameter, trackWidth, countsPerRev) | cm |
| `perbarui(pulsaKiri, pulsaKanan)` | update(leftCount, rightCount) | cumulative counts, positive = forward; the first call only stores the baseline |
| `x()`, `y()` | x, y | cm |
| `arah()` | heading | 0–360, clockwise |
| `jarakTempuh()` | distance travelled | cm, forward and backward |
| `jarakKe(x, y)` | distance to | cm |
| `arahKe(x, y)` | bearing to | 0–360 |
| `selisihArahKe(x, y)` | heading error to | −180..180, positive = turn right |
| `aturArah(derajat)` | setHeading(degrees) | e.g. from an IMU |
| `reset()` | reset | position and heading to 0 |
| `aturUkuran(diameterKiri, diameterKanan, jarakRoda)` | setGeometry(leftDiameter, rightDiameter, trackWidth) | calibration |

## Examples

`OdometriDasar` (basic odometry with two interrupt-driven encoders), `KeTitikTujuan` (drive to a list of waypoints), `KalibrasiRoda` (measure the real wheel diameters and track width).

## Status

Version 1.0.0 passes automated logic tests and compiles on Uno, Mega, ESP32, ESP32-C3, ESP32-S3, STM32 Blackpill F411, and Bluepill F103. It has **not yet been tested on a real robot**.

## License

MIT © 2026 Amadeo Wisesa.
