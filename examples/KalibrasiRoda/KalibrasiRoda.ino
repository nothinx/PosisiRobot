// Mengukur diameter roda dan jarak antar roda yang sebenarnya.
// Ukuran di atas kertas hampir selalu meleset sedikit (ban karet tertekan,
// titik kontak ban tidak persis di tengah), dan galat kecil ini menumpuk
// jadi galat posisi yang besar.
//
// Sambungan encoder sama dengan contoh OdometriDasar.
// Buka Serial Monitor (115200), pilih "Newline", lalu ikuti langkahnya:
//
// 1. Letakkan robot di awal meteran. Ketik n (nol-kan).
// 2. Dorong robot LURUS tepat 100 cm dengan tangan. Ketik l.
//    -> diameter roda kiri & kanan dihitung.
// 3. Ketik n. Putar robot di tempat (poros di tengah kedua roda) tepat
//    10 putaran searah jarum jam, pakai tanda di lantai. Ketik p.
//    -> jarak antar roda dihitung.
// 4. Salin baris aturUkuran(...) yang dicetak ke program kamu.
#include <PosisiRobot.h>

#ifdef ESP32
const uint8_t KIRI_A = 18, KIRI_B = 19, KANAN_A = 16, KANAN_B = 17;
#else
const uint8_t KIRI_A = 2, KIRI_B = 4, KANAN_A = 3, KANAN_B = 5;
#endif

const float PULSA_PER_PUTARAN = 374;
const float JARAK_LURUS = 100; // cm, langkah 2
const float JUMLAH_PUTARAN = 10; // langkah 3

volatile long pulsaKiri = 0, pulsaKanan = 0;
long awalKiri = 0, awalKanan = 0;
float diameterKiri = 6.5, diameterKanan = 6.5, jarakRoda = 15; // perkiraan awal

void encoderKiri() {
  if (digitalRead(KIRI_B)) pulsaKiri = pulsaKiri + 1;
  else pulsaKiri = pulsaKiri - 1;
}

void encoderKanan() {
  if (digitalRead(KANAN_B)) pulsaKanan = pulsaKanan + 1;
  else pulsaKanan = pulsaKanan - 1;
}

void setup() {
  Serial.begin(115200);
  pinMode(KIRI_A, INPUT_PULLUP);
  pinMode(KIRI_B, INPUT_PULLUP);
  pinMode(KANAN_A, INPUT_PULLUP);
  pinMode(KANAN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KIRI_A), encoderKiri, RISING);
  attachInterrupt(digitalPinToInterrupt(KANAN_A), encoderKanan, RISING);
  Serial.println("Ketik n, l, atau p (lihat petunjuk di atas program).");
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();

  noInterrupts();
  long kiri = pulsaKiri - awalKiri, kanan = pulsaKanan - awalKanan;
  interrupts();

  if (c == 'n') {
    awalKiri += kiri;
    awalKanan += kanan;
    Serial.println("Hitungan dinolkan.");
    return;
  }
  if (c != 'l' && c != 'p') return;

  Serial.print("Pulsa kiri: ");
  Serial.print(kiri);
  Serial.print("  kanan: ");
  Serial.println(kanan);

  if (c == 'l') {
    if (kiri <= 0 || kanan <= 0) {
      Serial.println("Hitungan harus positif saat maju. Cek kabel encoder (lihat OdometriDasar).");
      return;
    }
    // Keliling roda = jarak / putaran roda = jarak * pulsaPerPutaran / pulsa.
    diameterKiri = JARAK_LURUS * PULSA_PER_PUTARAN / (PI * kiri);
    diameterKanan = JARAK_LURUS * PULSA_PER_PUTARAN / (PI * kanan);
  } else {
    // Saat berputar di tempat, tiap roda menempuh keliling lingkaran
    // berdiameter jarak antar roda, sebanyak JUMLAH_PUTARAN kali.
    float sKiri = labs(kiri) * PI * diameterKiri / PULSA_PER_PUTARAN;
    float sKanan = labs(kanan) * PI * diameterKanan / PULSA_PER_PUTARAN;
    jarakRoda = (sKiri + sKanan) / 2 / (PI * JUMLAH_PUTARAN);
  }

  Serial.print("posisi.aturUkuran(");
  Serial.print(diameterKiri, 3);
  Serial.print(", ");
  Serial.print(diameterKanan, 3);
  Serial.print(", ");
  Serial.print(jarakRoda, 3);
  Serial.println(");");
}
