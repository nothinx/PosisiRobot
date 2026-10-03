// Robot bergerak ke beberapa titik tujuan berurutan, lalu berhenti.
// Koordinat dalam cm dari posisi awal: y = ke depan, x = ke kanan.
//
// Sambungan:
// - Encoder seperti contoh OdometriDasar (kiri: 2 & 4, kanan: 3 & 5).
// - Driver motor (L298N / TB6612): motor kiri PWM pin 6, arah pin 7 & 8;
//   motor kanan PWM pin 9, arah pin 10 & 11.
// - ESP32 DevKit: lihat pin di bawah. ESP32-C3/S3: sesuaikan nomor GPIO.
// Jalankan dulu OdometriDasar & KalibrasiRoda sampai posisi terbaca benar.
#include <PosisiRobot.h>

#ifdef ESP32 // GPIO 6..11 dipakai flash, jangan disentuh
const uint8_t KIRI_A = 18, KIRI_B = 19, KANAN_A = 16, KANAN_B = 17;
const uint8_t KIRI_PWM = 25, KIRI_IN1 = 26, KIRI_IN2 = 27, KANAN_PWM = 14, KANAN_IN1 = 12, KANAN_IN2 = 13;
#else
const uint8_t KIRI_A = 2, KIRI_B = 4, KANAN_A = 3, KANAN_B = 5;
const uint8_t KIRI_PWM = 6, KIRI_IN1 = 7, KIRI_IN2 = 8, KANAN_PWM = 9, KANAN_IN1 = 10, KANAN_IN2 = 11;
#endif

// Kotak 50 x 50 cm, kembali ke titik awal.
const float TUJUAN[][2] = {{0, 50}, {50, 50}, {50, 0}, {0, 0}};
const uint8_t JUMLAH_TUJUAN = sizeof(TUJUAN) / sizeof(TUJUAN[0]);

const float SAMPAI = 3;     // cm, dianggap sudah sampai
const int KECEPATAN = 120;  // PWM saat maju
const float KP_BELOK = 3;   // PWM per derajat selisih arah

PosisiRobot posisi(6.5, 15.0, 374);
volatile long pulsaKiri = 0, pulsaKanan = 0;
uint8_t nomor = 0; // tujuan yang sedang dituju

void encoderKiri() {
  if (digitalRead(KIRI_B)) pulsaKiri = pulsaKiri + 1;
  else pulsaKiri = pulsaKiri - 1;
}

void encoderKanan() {
  if (digitalRead(KANAN_B)) pulsaKanan = pulsaKanan + 1;
  else pulsaKanan = pulsaKanan - 1;
}

void motor(uint8_t pinPwm, uint8_t in1, uint8_t in2, int kecepatan) {
  kecepatan = constrain(kecepatan, -255, 255);
  digitalWrite(in1, kecepatan > 0);
  digitalWrite(in2, kecepatan < 0);
  analogWrite(pinPwm, abs(kecepatan));
}

void setup() {
  Serial.begin(115200);
  const uint8_t keluaran[] = {KIRI_PWM, KIRI_IN1, KIRI_IN2, KANAN_PWM, KANAN_IN1, KANAN_IN2};
  for (uint8_t i = 0; i < 6; i++) pinMode(keluaran[i], OUTPUT);
  pinMode(KIRI_A, INPUT_PULLUP);
  pinMode(KIRI_B, INPUT_PULLUP);
  pinMode(KANAN_A, INPUT_PULLUP);
  pinMode(KANAN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KIRI_A), encoderKiri, RISING);
  attachInterrupt(digitalPinToInterrupt(KANAN_A), encoderKanan, RISING);
}

void loop() {
  static uint32_t terakhir = 0;
  if (millis() - terakhir < 20) return;
  terakhir = millis();

  noInterrupts();
  long kiri = pulsaKiri, kanan = pulsaKanan;
  interrupts();
  posisi.perbarui(kiri, kanan);

  if (nomor >= JUMLAH_TUJUAN) { // semua tujuan selesai
    motor(KIRI_PWM, KIRI_IN1, KIRI_IN2, 0);
    motor(KANAN_PWM, KANAN_IN1, KANAN_IN2, 0);
    return;
  }

  float tx = TUJUAN[nomor][0], ty = TUJUAN[nomor][1];
  if (posisi.jarakKe(tx, ty) < SAMPAI) {
    Serial.print("Sampai di tujuan ");
    Serial.println(nomor + 1);
    nomor++;
    return;
  }

  // Selisih positif = tujuan di kanan: roda kiri lebih cepat.
  float selisih = posisi.selisihArahKe(tx, ty);
  int belok = constrain(selisih * KP_BELOK, -150, 150);
  int maju = fabs(selisih) > 30 ? 0 : KECEPATAN; // belok tajam: putar di tempat dulu
  motor(KIRI_PWM, KIRI_IN1, KIRI_IN2, maju + belok);
  motor(KANAN_PWM, KANAN_IN1, KANAN_IN2, maju - belok);
}
