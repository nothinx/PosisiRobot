// Simulasi di PC yang menjalankan kode PosisiRobot asli (../../src), untuk grafik README.
// Dipanggil oleh gambar.py. Keluaran: bagian "# nama" diikuti baris CSV
// t, x sebenarnya, y sebenarnya, x odometri, y odometri (cm).
//
// Robot "sebenarnya" disimulasikan per 1 ms: dua roda, motor orde-1, encoder
// terkuantisasi 374 pulsa per putaran. PosisiRobot hanya melihat hitungan encoder.
#include <math.h>
#include <stdio.h>
#include "PosisiRobot.h"

const double PI_ = 3.14159265358979;
const double PULSA = 374, JARAK_RODA = 15;
const double PUTARAN_MAKS = 2; // putaran roda per detik pada PWM 255

struct Robot {
  double dKiri, dKanan;            // diameter roda sebenarnya (cm)
  double x = 0, y = 0, sudut = 0;  // sudut radian, searah jarum jam dari +Y
  double putarKiri = 0, putarKanan = 0, wKiri = 0, wKanan = 0; // putaran roda, putaran/s
  Robot(double kiri, double kanan) : dKiri(kiri), dKanan(kanan) {}
  long pulsaKiri() const { return (long)floor(putarKiri * PULSA); }
  long pulsaKanan() const { return (long)floor(putarKanan * PULSA); }
  void langkah(int pwmKiri, int pwmKanan, double dt) {
    wKiri += (pwmKiri / 255.0 * PUTARAN_MAKS - wKiri) * dt / 0.1; // konstanta waktu motor 0.1 s
    wKanan += (pwmKanan / 255.0 * PUTARAN_MAKS - wKanan) * dt / 0.1;
    putarKiri += wKiri * dt;
    putarKanan += wKanan * dt;
    double vKiri = wKiri * PI_ * dKiri, vKanan = wKanan * PI_ * dKanan;
    double v = (vKiri + vKanan) / 2, w = (vKiri - vKanan) / JARAK_RODA;
    double a = sudut + w * dt / 2; // titik tengah langkah
    x += v * sin(a) * dt;
    y += v * cos(a) * dt;
    sudut += w * dt;
  }
};

static int batasi(float v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : (int)v); }

// Logika examples/KeTitikTujuan: kotak 50 x 50 cm, perbarui() tiap 20 ms.
static void keTitik(Robot &r, PosisiRobot &posisi, int putaran) {
  const float TUJUAN[][2] = {{0, 50}, {50, 50}, {50, 0}, {0, 0}};
  const float SAMPAI = 3, KP_BELOK = 3;
  const int KECEPATAN = 120;
  int nomor = 0, kiri = 0, kanan = 0;
  for (long ms = 0; nomor < 4 * putaran && ms < 600000; ms++) {
    if (ms % 20 == 0) {
      posisi.perbarui(r.pulsaKiri(), r.pulsaKanan());
      printf("%.2f,%.3f,%.3f,%.3f,%.3f\n", ms / 1000.0, r.x, r.y, posisi.x(), posisi.y());
      float tx = TUJUAN[nomor % 4][0], ty = TUJUAN[nomor % 4][1];
      if (posisi.jarakKe(tx, ty) < SAMPAI) {
        nomor++;
      } else {
        float selisih = posisi.selisihArahKe(tx, ty);
        int belok = batasi(selisih * KP_BELOK, -150, 150);
        int maju = fabsf(selisih) > 30 ? 0 : KECEPATAN;
        kiri = batasi(maju + belok, -255, 255);
        kanan = batasi(maju - belok, -255, 255);
      }
    }
    r.langkah(kiri, kanan, 0.001);
  }
  posisi.perbarui(r.pulsaKiri(), r.pulsaKanan());
  printf("akhir,%.3f,%.3f,%.3f,%.3f\n", r.x, r.y, posisi.x(), posisi.y());
}

// PWM tetap (kiri 160, kanan 100): robot melingkar ke kanan satu putaran penuh.
static void lingkaran() {
  Robot r(6.5, 6.5);
  PosisiRobot posisi(6.5, 15.0, 374);
  for (long ms = 0; r.sudut < 2 * PI_; ms++) {
    if (ms % 20 == 0) {
      posisi.perbarui(r.pulsaKiri(), r.pulsaKanan());
      printf("%.2f,%.3f,%.3f,%.3f,%.3f\n", ms / 1000.0, r.x, r.y, posisi.x(), posisi.y());
    }
    r.langkah(160, 100, 0.001);
  }
  posisi.perbarui(r.pulsaKiri(), r.pulsaKanan());
  printf("akhir,%.3f,%.3f,%.3f,%.3f\n", r.x, r.y, posisi.x(), posisi.y());
}

int main() {
  { // ukuran roda tepat
    Robot r(6.5, 6.5);
    PosisiRobot posisi(6.5, 15.0, 374);
    puts("# kotak");
    keTitik(r, posisi, 1);
  }
  puts("# lingkaran");
  lingkaran();
  // Roda kanan sebenarnya 1% lebih besar, tiga putaran kotak.
  for (int kalibrasi = 0; kalibrasi < 2; kalibrasi++) {
    Robot r(6.5, 6.5 * 1.01);
    PosisiRobot posisi(6.5, 15.0, 374);
    if (kalibrasi) posisi.aturUkuran(6.5, 6.5 * 1.01, 15.0); // hasil KalibrasiRoda
    puts(kalibrasi ? "# kalibrasi_sesudah" : "# kalibrasi_sebelum");
    keTitik(r, posisi, 3);
  }
  return 0;
}
