#!/usr/bin/env python3
"""Generate synthetic WAV sound assets for the Asteroids game.

All files written to assets/audio/ relative to the repo root.
Run from anywhere; uses the script's own location to find the root.
"""
import math
import os
import random
import struct
import wave

RATE = 44100
AMP  = 16000   # 16-bit signed; headroom below 32767

RNG  = random.Random(42)   # deterministic so assets are reproducible


# ── helpers ───────────────────────────────────────────────────────────────────

def _pack(samples):
    clamped = [max(-32767, min(32767, int(s))) for s in samples]
    return struct.pack(f'<{len(clamped)}h', *clamped)


def write_wav(out_dir, name, samples):
    os.makedirs(out_dir, exist_ok=True)
    with wave.open(os.path.join(out_dir, name), 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(_pack(samples))
    print(f'  {name}: {len(samples)/RATE:.3f}s  ({len(samples)} frames)')


def iir_lp(samples, alpha):
    """First-order IIR low-pass: y[i] = alpha*x[i] + (1-alpha)*y[i-1]"""
    out = [0.0] * len(samples)
    out[0] = samples[0]
    for i in range(1, len(samples)):
        out[i] = alpha * samples[i] + (1.0 - alpha) * out[i - 1]
    return out


def env(samples, attack_s=0.005, release_s=0.05):
    """Linear attack / release envelope (no sustain)."""
    n = len(samples)
    a = int(attack_s * RATE)
    r = int(release_s * RATE)
    result = list(samples)
    for i in range(min(a, n)):
        result[i] *= i / a
    for i in range(min(r, n)):
        result[n - 1 - i] *= i / r
    return result


# ── individual sound generators ───────────────────────────────────────────────

def make_thrust():
    """0.4 s loop — low sawtooth buzz + low-passed noise."""
    dur = 0.4
    n   = int(dur * RATE)
    saw = [AMP * 0.5 * (2.0 * ((i * 80.0 / RATE) % 1.0) - 1.0) for i in range(n)]
    nz  = iir_lp([RNG.uniform(-AMP * 0.6, AMP * 0.6) for _ in range(n)], 0.04)
    return [s + z for s, z in zip(saw, nz)]


def make_fire():
    """0.12 s — descending frequency chirp (laser pew)."""
    dur = 0.12
    n   = int(dur * RATE)
    phase = 0.0
    samp  = []
    for i in range(n):
        t     = i / RATE
        freq  = 900.0 * math.exp(-t * 18.0)
        phase += 2.0 * math.pi * freq / RATE
        samp.append(AMP * math.sin(phase))
    return env(samp, attack_s=0.001, release_s=0.04)


def make_explosion_large():
    """1.0 s — decaying low-rumble noise burst."""
    dur = 1.0
    n   = int(dur * RATE)
    raw = [RNG.uniform(-1.0, 1.0) for _ in range(n)]
    lp  = iir_lp(raw, 0.015)
    return [AMP * lp[i] * math.exp(-3.0 * i / n) for i in range(n)]


def make_explosion_small():
    """0.45 s — decaying mid-frequency noise burst."""
    dur = 0.45
    n   = int(dur * RATE)
    raw = [RNG.uniform(-1.0, 1.0) for _ in range(n)]
    lp  = iir_lp(raw, 0.06)
    samples = [AMP * lp[i] * math.exp(-5.5 * i / n) for i in range(n)]
    return env(samples, attack_s=0.001, release_s=0.08)


def make_saucer_large():
    """0.5 s loop — warbling square wave ~200 Hz."""
    dur = 0.5
    n   = int(dur * RATE)
    phase = 0.0
    samp  = []
    for i in range(n):
        t    = i / RATE
        freq = 200.0 + math.sin(2.0 * math.pi * 4.0 * t) * 25.0
        phase += 2.0 * math.pi * freq / RATE
        sq = 1.0 if math.sin(phase) >= 0.0 else -1.0
        samp.append(AMP * 0.45 * sq)
    return samp


def make_saucer_small():
    """0.35 s loop — warbling square wave ~620 Hz (higher, more urgent)."""
    dur = 0.35
    n   = int(dur * RATE)
    phase = 0.0
    samp  = []
    for i in range(n):
        t    = i / RATE
        freq = 620.0 + math.sin(2.0 * math.pi * 9.0 * t) * 70.0
        phase += 2.0 * math.pi * freq / RATE
        sq = 1.0 if math.sin(phase) >= 0.0 else -1.0
        samp.append(AMP * 0.45 * sq)
    return samp


def make_saucer_fire():
    """0.1 s — brief ascending chirp."""
    dur = 0.10
    n   = int(dur * RATE)
    phase = 0.0
    samp  = []
    for i in range(n):
        t     = i / RATE
        freq  = 350.0 + t * 2500.0
        phase += 2.0 * math.pi * freq / RATE
        samp.append(AMP * math.sin(phase))
    return env(samp, attack_s=0.002, release_s=0.04)


def make_beat_low():
    """0.08 s — short 220 Hz sine pulse (background beat, low tone)."""
    dur = 0.08
    n   = int(dur * RATE)
    samp = [AMP * math.sin(2.0 * math.pi * 220.0 * i / RATE) for i in range(n)]
    return env(samp, attack_s=0.004, release_s=0.04)


def make_beat_high():
    """0.08 s — short 440 Hz sine pulse (background beat, high tone)."""
    dur = 0.08
    n   = int(dur * RATE)
    samp = [AMP * math.sin(2.0 * math.pi * 440.0 * i / RATE) for i in range(n)]
    return env(samp, attack_s=0.004, release_s=0.04)


# ── main ──────────────────────────────────────────────────────────────────────

def main():
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    out_dir   = os.path.join(repo_root, 'assets', 'audio')

    print(f'Writing to {out_dir}')
    write_wav(out_dir, 'thrust.wav',          make_thrust())
    write_wav(out_dir, 'fire.wav',             make_fire())
    write_wav(out_dir, 'explosion_large.wav',  make_explosion_large())
    write_wav(out_dir, 'explosion_small.wav',  make_explosion_small())
    write_wav(out_dir, 'saucer_large.wav',     make_saucer_large())
    write_wav(out_dir, 'saucer_small.wav',     make_saucer_small())
    write_wav(out_dir, 'saucer_fire.wav',      make_saucer_fire())
    write_wav(out_dir, 'beat_low.wav',         make_beat_low())
    write_wav(out_dir, 'beat_high.wav',        make_beat_high())
    print('Done.')


if __name__ == '__main__':
    main()
