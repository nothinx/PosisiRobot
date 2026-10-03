#include "PosisiRobot.h"
#include <math.h>

static const float DUA_PI = 6.2831853f;
static const float KE_DERAJAT = 57.2957795f;

static float bungkus360(float d) {
  if (d >= 0 && d < 360.0f) return d; // paling sering: tanpa fmodf()
  d = fmodf(d, 360.0f);
  if (d < 0) d += 360.0f;
  return d >= 360.0f ? 0 : d; // -0.00001 + 360 bisa dibulatkan jadi 360
}

PosisiRobot::PosisiRobot(float diameterRoda, float jarakRoda, float pulsaPerPutaran)
    : _pulsa(pulsaPerPutaran) {
  aturUkuran(diameterRoda, diameterRoda, jarakRoda);
}

void PosisiRobot::aturUkuran(float diameterKiri, float diameterKanan, float jarakRoda) {
  _cmKiri = 3.14159265f * diameterKiri / _pulsa;
  _cmKanan = 3.14159265f * diameterKanan / _pulsa;
  _perJarak = 1 / jarakRoda;
}

void PosisiRobot::perbarui(long pulsaKiri, long pulsaKanan) {
  if (!_siap) {
    _siap = true;
  } else {
    // Selisih lewat unsigned agar benar walau counter meluap.
    long dKiri = (long)((unsigned long)pulsaKiri - (unsigned long)_kiriLalu);
    long dKanan = (long)((unsigned long)pulsaKanan - (unsigned long)_kananLalu);
    if (!dKiri && !dKanan) return; // diam: tidak ada yang berubah
    float sKiri = dKiri * _cmKiri, sKanan = dKanan * _cmKanan;
    float s = (sKiri + sKanan) / 2;
    float putar = (sKiri - sKanan) * _perJarak; // positif = searah jarum jam
    // Robot bergerak di busur lingkaran: perpindahannya adalah tali busur
    // sepanjang s * sin(h) / h, searah sudut tengah busur. Untuk |h| < 0,1 rad
    // (hampir selalu, karena perbarui() sering dipanggil) dipakai deret Taylor
    // 1 - h^2/6 + h^4/120: galatnya < 2e-10, jauh di bawah ketelitian float.
    float h = putar / 2;
    float h2 = h * h;
    float tali = h2 < 1e-8f   ? s // lurus
                 : h2 < 0.01f ? s * (1 - h2 * (1.0f / 6 - h2 * (1.0f / 120)))
                              : s * sinf(h) / h;
    float a = _sudut + h;
    _x += tali * sinf(a);
    _y += tali * cosf(a);
    _sudut += putar;
    if (_sudut < 0 || _sudut >= DUA_PI) { // jarang: fmodf() hanya saat melewati 0/360
      _sudut = fmodf(_sudut, DUA_PI);
      if (_sudut < 0) _sudut += DUA_PI;
    }
    _tempuh += fabsf(s);
  }
  _kiriLalu = pulsaKiri;
  _kananLalu = pulsaKanan;
}

float PosisiRobot::arah() const { return bungkus360(_sudut * KE_DERAJAT); }

void PosisiRobot::aturArah(float derajat) { _sudut = bungkus360(derajat) / KE_DERAJAT; }

void PosisiRobot::reset() {
  _x = _y = _sudut = _tempuh = 0;
}

float PosisiRobot::jarakKe(float x, float y) const {
  return sqrtf((x - _x) * (x - _x) + (y - _y) * (y - _y));
}

float PosisiRobot::arahKe(float x, float y) const {
  return bungkus360(atan2f(x - _x, y - _y) * KE_DERAJAT);
}

float PosisiRobot::selisihArahKe(float x, float y) const {
  float e = bungkus360(arahKe(x, y) - arah());
  return e > 180.0f ? e - 360.0f : e;
}
