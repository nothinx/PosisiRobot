// Uji logika PosisiRobot di PC:
//   g++ -std=c++11 -Wall -Wextra -I. -I../../src uji.cpp ../../src/PosisiRobot.cpp -o uji && ./uji
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "PosisiRobot.h"

static const double PI_D = 3.14159265358979323846;
static bool dekat(float a, float b, float tol = 0.01f) { return fabsf(a - b) <= tol; }
static bool dekatArah(float a, float b, float tol = 0.01f) {
  float e = fabsf(a - b);
  return e <= tol || fabsf(e - 360) <= tol;
}

// Roda 0.01 cm per pulsa (1000 pulsa per putaran, keliling 10 cm), jarak roda 15 cm.
static PosisiRobot robot() { return PosisiRobot(10 / 3.14159265f, 15, 1000); }
static const float PULSA_PUTAR_360 = 3.14159265f * 15 / 0.01f; // tiap roda menempuh pi x jarak roda

// Gerakkan kedua roda sejauh dKiri/dKanan pulsa, dalam n langkah.
static long kiri, kanan;
static void gerak(PosisiRobot &r, long dKiri, long dKanan, int n = 100) {
  long k0 = kiri, a0 = kanan;
  for (int i = 1; i <= n; i++) r.perbarui(k0 + dKiri * i / n, a0 + dKanan * i / n);
  kiri = k0 + dKiri;
  kanan = a0 + dKanan;
}

int main() {
  int kasus = 0;
  { // panggilan pertama hanya mencatat hitungan awal (counter tidak mulai dari 0)
    PosisiRobot r = robot();
    kiri = kanan = 5000;
    r.perbarui(kiri, kanan);
    assert(r.x() == 0 && r.y() == 0 && r.jarakTempuh() == 0);
    kasus++;
  }
  { // lurus maju 100 cm lalu mundur 30 cm
    PosisiRobot r = robot();
    kiri = kanan = 0;
    r.perbarui(0, 0);
    gerak(r, 10000, 10000);
    assert(dekat(r.x(), 0) && dekat(r.y(), 100) && dekat(r.arah(), 0));
    gerak(r, -3000, -3000);
    assert(dekat(r.y(), 70) && dekat(r.jarakTempuh(), 130));
    kasus++;
  }
  { // putar di tempat: 90 derajat ke kanan, lalu 360 penuh, posisi tetap
    PosisiRobot r = robot();
    kiri = kanan = 0;
    r.perbarui(0, 0);
    long q = lroundf(PULSA_PUTAR_360 / 4);
    gerak(r, q, -q);
    assert(dekat(r.arah(), 90, 0.1f) && dekat(r.x(), 0) && dekat(r.y(), 0));
    gerak(r, -q, q); // kembali ke kiri
    assert(dekatArah(r.arah(), 0, 0.1f));
    long penuh = lroundf(PULSA_PUTAR_360);
    gerak(r, -penuh, penuh, 360);
    assert(dekatArah(r.arah(), 0, 0.1f) && dekat(r.x(), 0) && dekat(r.y(), 0));
    kasus++;
  }
  { // konvensi tanda: hanya roda kiri maju -> belok kanan, x positif
    PosisiRobot r = robot();
    r.perbarui(0, 0);
    r.perbarui(1000, 0);
    assert(r.arah() > 0 && r.arah() < 90 && r.x() > 0 && r.y() > 0);
    PosisiRobot l = robot();
    l.perbarui(0, 0);
    l.perbarui(0, 1000);
    assert(l.arah() > 270 && l.x() < 0 && l.y() > 0);
    kasus++;
  }
  { // busur eksak: seperempat lingkaran radius 30 cm dalam SATU perbarui()
    // Belok kanan dari (0,0) menghadap +Y, pusat lingkaran di (30,0) -> berakhir di (30,30) arah 90.
    PosisiRobot r = robot();
    r.perbarui(0, 0);
    float sudut = 3.14159265f / 2;
    long dKiri = lroundf(sudut * (30 + 7.5f) / 0.01f), dKanan = lroundf(sudut * (30 - 7.5f) / 0.01f);
    r.perbarui(dKiri, dKanan);
    assert(dekat(r.x(), 30, 0.05f) && dekat(r.y(), 30, 0.05f) && dekat(r.arah(), 90, 0.1f));
    kasus++;
  }
  { // lingkaran penuh dalam banyak langkah kembali ke titik awal
    PosisiRobot r = robot();
    kiri = kanan = 0;
    r.perbarui(0, 0);
    float keliling = 2 * 3.14159265f;
    gerak(r, lroundf(keliling * 47.5f / 0.01f), lroundf(keliling * 32.5f / 0.01f), 1000);
    assert(dekat(r.x(), 0, 0.1f) && dekat(r.y(), 0, 0.1f) && dekatArah(r.arah(), 0, 0.1f));
    assert(dekat(r.jarakTempuh(), keliling * 40, 0.5f));
    kasus++;
  }
  { // kotak 100 cm: maju, putar kanan 90, x4 -> kembali ke awal
    PosisiRobot r = robot();
    kiri = kanan = 0;
    r.perbarui(0, 0);
    long q = lroundf(PULSA_PUTAR_360 / 4);
    for (int sisi = 0; sisi < 4; sisi++) {
      gerak(r, 10000, 10000);
      if (sisi == 0) assert(dekat(r.y(), 100) && dekat(r.x(), 0));
      if (sisi == 1) assert(dekat(r.x(), 100, 0.2f) && dekat(r.y(), 100, 0.2f));
      gerak(r, q, -q);
    }
    assert(dekat(r.x(), 0, 0.5f) && dekat(r.y(), 0, 0.5f) && dekatArah(r.arah(), 0, 0.5f));
    kasus++;
  }
  { // counter long meluap di tengah gerakan
    PosisiRobot r = robot();
    kiri = kanan = LONG_MAX - 400;
    r.perbarui(kiri, kanan);
    // Hitungan naik 10000 melewati LONG_MAX (seperti counter++ di ISR yang meluap).
    long k = (long)((unsigned long)kiri + 10000UL);
    assert(k < 0);
    r.perbarui(k, k);
    assert(dekat(r.y(), 100) && dekat(r.x(), 0));
    kasus++;
  }
  { // navigasi: arahKe, jarakKe, selisihArahKe
    PosisiRobot r = robot();
    assert(dekat(r.arahKe(0, 10), 0) && dekat(r.arahKe(10, 0), 90));
    assert(dekat(r.arahKe(0, -10), 180) && dekat(r.arahKe(-10, 0), 270));
    assert(dekat(r.jarakKe(30, 40), 50));
    assert(dekat(r.selisihArahKe(10, 0), 90) && dekat(r.selisihArahKe(-10, 0), -90));
    r.aturArah(350);
    assert(dekat(r.selisihArahKe(10, 10), 55)); // 45 - 350 -> belok kanan 55
    r.aturArah(-90);
    assert(dekat(r.arah(), 270));
    kasus++;
  }
  { // aturArah dari IMU lalu maju: gerak mengikuti arah baru; reset
    PosisiRobot r = robot();
    r.perbarui(0, 0);
    r.aturArah(90);
    r.perbarui(5000, 5000);
    assert(dekat(r.x(), 50) && dekat(r.y(), 0));
    r.reset();
    assert(r.x() == 0 && r.y() == 0 && r.arah() == 0 && r.jarakTempuh() == 0);
    r.perbarui(6000, 6000); // setelah reset, hitungan dari perbarui terakhir
    assert(dekat(r.y(), 10));
    kasus++;
  }
  { // aturPosisi: posisi ditimpa, arah & jarak tempuh tetap, gerak berlanjut dari titik baru
    PosisiRobot r = robot();
    r.perbarui(0, 0);
    r.aturArah(90);
    r.perbarui(1000, 1000);         // maju 10 cm ke +X
    r.aturPosisi(-20, 30);
    assert(r.x() == -20 && r.y() == 30 && dekatArah(r.arah(), 90) && dekat(r.jarakTempuh(), 10));
    assert(dekat(r.jarakKe(-20, 40), 10) && dekatArah(r.arahKe(-20, 40), 0));
    r.perbarui(2000, 2000);         // maju 10 cm lagi
    assert(dekat(r.x(), -10) && dekat(r.y(), 30));
    kasus++;
  }
  { // kalibrasi: roda kanan 1% lebih besar -> maju lurus pada pulsa yang sama terdeteksi belok kiri
    PosisiRobot r = robot();
    r.aturUkuran(10 / 3.14159265f, 10.1f / 3.14159265f, 15);
    r.perbarui(0, 0);
    r.perbarui(10000, 10000);
    assert(r.arah() > 270 && dekat(r.y(), 100.5f, 0.1f));
    kasus++;
  }
  { // jalur cepat (lurus, deret Taylor, putaran besar, lewat 0/360) = rumus busur apa adanya dalam double
    PosisiRobot r = robot();
    double x = 0, y = 0, sudut = 0;
    long k = 0, a = 0;
    r.perbarui(k, a);
    unsigned acak = 7;
    for (int i = 0; i < 3000; i++) {
      acak = acak * 1103515245u + 12345u;
      int jenis = acak >> 28; // 0..15
      long dk = (long)(acak >> 8 & 63) - 20, da = dk; // lurus
      if (jenis >= 4) da = dk - (long)(acak >> 16 & 15) + 7; // belok kecil: |h| < 0,1
      if (jenis >= 14) da = -dk * 40 - 900;                  // putar besar: |h| >= 0,1
      if (jenis == 15) da = dk = 0;                          // diam
      k += dk;
      a += da;
      r.perbarui(k, a);
      double sk = dk * 0.01, sa = da * 0.01, s = (sk + sa) / 2, putar = (sk - sa) / 15, h = putar / 2;
      double tali = h != 0 ? s * sin(h) / h : s;
      x += tali * sin(sudut + h);
      y += tali * cos(sudut + h);
      sudut = fmod(sudut + putar, 2 * PI_D);
      if (sudut < 0) sudut += 2 * PI_D;
      assert(dekat(r.x(), (float)x, 0.05f) && dekat(r.y(), (float)y, 0.05f));
      assert(dekatArah(r.arah(), (float)(sudut * 180 / PI_D), 0.01f));
    }
    kasus++;
  }
  printf("Semua uji lolos (%d kasus, sizeof = %u byte)\n", kasus, (unsigned)sizeof(PosisiRobot));
  return 0;
}
