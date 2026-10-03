"""Compile & jalankan simulasi.cpp (kode PosisiRobot asli), lalu render grafik ke ../gambar/.

Jalankan dari folder ini: python gambar.py   (butuh g++ dan matplotlib)
"""
import glob
import os
import subprocess
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

plt.rcParams.update({
    "figure.figsize": (8, 3.6), "figure.dpi": 100, "savefig.bbox": "tight", "savefig.pad_inches": 0.15,
    "figure.facecolor": "white", "axes.facecolor": "white", "savefig.facecolor": "white",
    "font.size": 10, "axes.titlesize": 11, "axes.titleweight": "bold", "axes.titlelocation": "left",
    "axes.spines.top": False, "axes.spines.right": False, "axes.edgecolor": "#9ca3af",
    "axes.grid": True, "grid.color": "#e5e7eb", "grid.linewidth": 0.8,
    "legend.frameon": False, "svg.fonttype": "path", "svg.hashsalt": "nothinx",
    "lines.linewidth": 1.8,
})
WARNA = {"utama": "#2563eb", "pembanding": "#dc2626", "ketiga": "#16a34a", "keempat": "#9333ea",
         "kelima": "#ea580c", "mentah": "#9ca3af", "target": "#111827"}
KELUAR = os.path.join("..", "gambar")


def jalankan():
    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, "sim.exe" if os.name == "nt" else "sim")
        subprocess.run(["g++", "-std=c++11", "-O2", "-I../test", "-I../../src", "simulasi.cpp",
                        *glob.glob("../../src/*.cpp"), "-o", exe], check=True)
        teks = subprocess.run([exe], check=True, capture_output=True, text=True).stdout
    data, nama = {}, None
    for baris in teks.splitlines():
        if baris.startswith("# "):
            nama = baris[2:]
            data[nama] = []
        elif baris.startswith("akhir,"):
            data[nama + "_akhir"] = [float(v) for v in baris.split(",")[1:]]
        elif baris:
            data[nama].append([float(v) for v in baris.split(",")])
    return {k: v if k.endswith("_akhir") else list(zip(*v)) for k, v in data.items()}  # per kolom


def koma(x, n=1):
    return f"{x:.{n}f}".replace(".", ",")


def simpan(fig, nama):
    fig.savefig(os.path.join(KELUAR, nama), format="svg", metadata={"Date": None})
    plt.close(fig)

def galat(d, k):
    xa, ya, xo, yo = d[k + "_akhir"]
    return ((xa - xo) ** 2 + (ya - yo) ** 2) ** 0.5


def kotak_target(ax):
    ax.plot([0, 0, 50, 50, 0], [0, 50, 50, 0, 0], color=WARNA["target"], ls="--", lw=1.2)


def lintasan(d):
    fig, (ax, bx) = plt.subplots(1, 2, figsize=(8, 4.2))
    for a, k, ket in [(ax, "kotak", "Kotak 50 × 50 cm (logika contoh KeTitikTujuan)"),
                      (bx, "lingkaran", "Lingkaran: PWM kiri 160, kanan 100")]:
        _, xa, ya, xo, yo = d[k]
        if k == "kotak":
            kotak_target(a)
        a.plot(xa, ya, color=WARNA["mentah"], lw=6, alpha=0.5, label="Lintasan sebenarnya")
        a.plot(xo, yo, color=WARNA["utama"], lw=1.4, label="Hasil odometri PosisiRobot")
        a.plot(0, 0, "o", color=WARNA["target"], ms=5)
        a.annotate("mulai", (0, 0), (6, -14), textcoords="offset points", fontsize=9)
        a.set_aspect("equal")
        a.set_xlabel("x (cm)")
        a.set_ylabel("y (cm)")
        a.text(0, 1.02, ket, transform=a.transAxes, fontsize=9, va="bottom")
    ax.set_xlim(-12, 62)
    ax.set_ylim(-12, 62)
    bx.legend(loc="upper center", bbox_to_anchor=(0.5, -0.2))
    fig.suptitle(f"Odometri berhimpit dengan lintasan sebenarnya (selisih akhir {koma(galat(d, 'kotak'), 2)} cm "
                 f"dan {koma(galat(d, 'lingkaran'), 2)} cm)", x=0.02, ha="left", fontsize=11, fontweight="bold")
    simpan(fig, "lintasan.svg")


def kalibrasi(d):
    fig, ax = plt.subplots(figsize=(8, 4.6))
    kotak_target(ax)
    sebelum, sesudah = galat(d, "kalibrasi_sebelum"), galat(d, "kalibrasi_sesudah")
    for k, label, w in [("kalibrasi_sebelum", f"Tanpa kalibrasi (galat {koma(sebelum)} cm)", "pembanding"),
                        ("kalibrasi_sesudah", f"Setelah aturUkuran() (galat {koma(sesudah)} cm)", "utama")]:
        _, xa, ya, _, _ = d[k]
        ax.plot(xa, ya, color=WARNA[w], lw=1.4, label=label)
        xe, ye = d[k + "_akhir"][:2]
        ax.plot(xe, ye, "o", color=WARNA[w], ms=5)
    ax.plot([], [], color=WARNA["target"], ls="--", lw=1.2, label="Kotak tujuan")
    ax.plot(0, 0, "o", color=WARNA["target"], ms=5)
    ax.annotate("mulai", (0, 0), (-32, -14), textcoords="offset points", fontsize=9)
    ax.set_aspect("equal")
    ax.set_xlim(-14, 62)
    ax.set_xlabel("x (cm)")
    ax.set_ylabel("y (cm)")
    ax.set_title(f"Roda kanan 1% lebih besar: galat {koma(sebelum)} cm setelah 3 putaran, "
                 f"{koma(sesudah)} cm setelah kalibrasi")
    ax.legend(loc="center left", bbox_to_anchor=(1.02, 0.5))
    simpan(fig, "kalibrasi-roda.svg")


if __name__ == "__main__":
    os.makedirs(KELUAR, exist_ok=True)
    d = jalankan()
    lintasan(d)
    kalibrasi(d)
