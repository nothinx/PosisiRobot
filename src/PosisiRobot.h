// PosisiRobot - odometri robot dua roda (diferensial) dari encoder roda.
// Copyright (c) 2026 Amadeo Wisesa. Lisensi MIT.
//
// Satuan panjang: cm. Arah: derajat 0..360, naik searah jarum jam
// (sama dengan ArahMPU6050). Posisi seperti peta:
//   arah 0  = sumbu +Y (depan robot saat mulai/reset)
//   arah 90 = sumbu +X (kanan robot saat mulai/reset)
//
// - Menerima hitungan encoder kumulatif, aman saat counter long meluap.
// - Integrasi busur eksak: tidak ada galat tambahan walau perbarui()
//   jarang dipanggil, selama robot bergerak dengan lengkung tetap.
#pragma once
#include <Arduino.h>

class PosisiRobot {
public:
  // Ukuran roda & jarak antar roda (tengah tapak ke tengah tapak) dalam cm.
  // pulsaPerPutaran: hitungan encoder per satu putaran roda (setelah gearbox).
  PosisiRobot(float diameterRoda, float jarakRoda, float pulsaPerPutaran);

  // Panggil berkala dengan hitungan encoder kumulatif, positif = roda maju.
  // Panggilan pertama hanya mencatat hitungan awal.
  void perbarui(long pulsaKiri, long pulsaKanan);

  // --- Posisi ---
  float x() const { return _x; }          // cm, positif = kanan posisi awal
  float y() const { return _y; }          // cm, positif = depan posisi awal
  float arah() const;                     // 0..360, naik searah jarum jam
  float jarakTempuh() const { return _tempuh; } // total cm, maju & mundur

  // --- Navigasi ke titik (x, y) ---
  float jarakKe(float x, float y) const;  // cm
  float arahKe(float x, float y) const;   // 0..360
  // Selisih terpendek ke arah titik, -180..180.
  // Positif = belok kanan (searah jarum jam), negatif = belok kiri.
  float selisihArahKe(float x, float y) const;

  // --- Koreksi ---
  // Timpa arah, misalnya dari IMU: posisi.aturArah(imu.arah()).
  void aturArah(float derajat);
  // Posisi & arah kembali ke 0. Counter encoder tidak perlu di-nol-kan.
  void reset();
  // Hasil kalibrasi: diameter roda kiri & kanan boleh sedikit berbeda.
  void aturUkuran(float diameterKiri, float diameterKanan, float jarakRoda);

private:
  float _cmKiri, _cmKanan, _jarak, _pulsa;  // cm per pulsa, cm, pulsa per putaran
  float _x = 0, _y = 0, _sudut = 0, _tempuh = 0; // _sudut dalam radian, 0..2pi
  long _kiriLalu = 0, _kananLalu = 0;
  bool _siap = false; // hitungan awal sudah dicatat
};
