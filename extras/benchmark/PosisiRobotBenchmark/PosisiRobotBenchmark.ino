// Benchmark PosisiRobot di ATmega328P 16 MHz (simavr). Cara menjalankan dan
// angka hasilnya: README bagian "Kecepatan & memori".
// Siklus.h: Timer1 tanpa prescaler, UKUR(nama, ulang, kode) mencetak
// "BENCH nama siklus_per_panggilan".
#include <PosisiRobot.h>
#include "Siklus.h"

PosisiRobot posisi(6.5, 15, 360); // roda 6,5 cm, jarak roda 15 cm, 360 pulsa/putaran
long kiri, kanan;
volatile float hasil;

void setup() {
  Serial.begin(115200);
  posisi.perbarui(0, 0);
  Serial.print(F("BENCH sizeof "));
  Serial.println(sizeof(PosisiRobot));
  UKUR("perbarui_belok", 1000, { kiri += 3; kanan += 2; posisi.perbarui(kiri, kanan); });
  UKUR("perbarui_lurus", 1000, { kiri += 3; kanan += 3; posisi.perbarui(kiri, kanan); });
  UKUR("perbarui_diam", 1000, posisi.perbarui(kiri, kanan));
  // Penghalang memori: hasil tidak boleh dihitung sekali di luar loop.
  UKUR("arah", 1000, { asm volatile("" ::: "memory"); hasil = posisi.arah(); });
  UKUR("jarakKe", 1000, { asm volatile("" ::: "memory"); hasil = posisi.jarakKe(100, 50); });
  UKUR("selisihArahKe", 1000, { asm volatile("" ::: "memory"); hasil = posisi.selisihArahKe(100, 50); });
  selesai();
}

void loop() {}
