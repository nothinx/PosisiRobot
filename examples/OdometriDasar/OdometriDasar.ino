// Odometri dasar: posisi x, y dan arah robot dari dua encoder roda.
// Dorong atau jalankan robot, lalu lihat posisinya di Serial Monitor (115200).
//
// Sambungan encoder (motor DC dengan encoder kuadratur, mis. JGA25-370 / N20):
// - Roda kiri:  kanal A ke pin 2 (interrupt), kanal B ke pin 4.
// - Roda kanan: kanal A ke pin 3 (interrupt), kanal B ke pin 5.
// - ESP32 DevKit: lihat pin di bawah. ESP32-C3/S3: sesuaikan nomor GPIO.
// Saat robot maju, kedua hitungan harus NAIK. Jika ada yang turun (biasanya
// roda kanan karena motornya terpasang terbalik), tukar kabel A & B encoder
// itu, atau tukar ++ dan -- di fungsi interrupt-nya.
#include <PosisiRobot.h>

#ifdef ESP32
const uint8_t KIRI_A = 18, KIRI_B = 19, KANAN_A = 16, KANAN_B = 17;
#else
const uint8_t KIRI_A = 2, KIRI_B = 4, KANAN_A = 3, KANAN_B = 5;
#endif

// Diameter roda 6,5 cm, jarak antar roda 15 cm, 374 pulsa per putaran roda
// (encoder 11 pulsa x gearbox 34, hanya sisi naik kanal A yang dihitung).
// Ukur ulang dengan contoh KalibrasiRoda.
PosisiRobot posisi(6.5, 15.0, 374);

volatile long pulsaKiri = 0, pulsaKanan = 0;

void encoderKiri() {
  if (digitalRead(KIRI_B)) pulsaKiri++;
  else pulsaKiri--;
}

void encoderKanan() {
  if (digitalRead(KANAN_B)) pulsaKanan++;
  else pulsaKanan--;
}

void setup() {
  Serial.begin(115200);
  pinMode(KIRI_A, INPUT_PULLUP);
  pinMode(KIRI_B, INPUT_PULLUP);
  pinMode(KANAN_A, INPUT_PULLUP);
  pinMode(KANAN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KIRI_A), encoderKiri, RISING);
  attachInterrupt(digitalPinToInterrupt(KANAN_A), encoderKanan, RISING);
}

void loop() {
  static uint32_t terakhirHitung = 0, terakhirCetak = 0;

  if (millis() - terakhirHitung >= 20) { // perbarui 50 kali per detik
    terakhirHitung = millis();
    noInterrupts(); // long 4 byte: salin utuh tanpa disela interrupt
    long kiri = pulsaKiri, kanan = pulsaKanan;
    interrupts();
    posisi.perbarui(kiri, kanan);
  }

  if (millis() - terakhirCetak >= 500) {
    terakhirCetak = millis();
    Serial.print("x: ");
    Serial.print(posisi.x());
    Serial.print(" cm  y: ");
    Serial.print(posisi.y());
    Serial.print(" cm  arah: ");
    Serial.print(posisi.arah());
    Serial.print("  tempuh: ");
    Serial.print(posisi.jarakTempuh());
    Serial.println(" cm");
  }
}
