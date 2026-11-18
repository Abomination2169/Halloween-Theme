#!/usr/bin/env python3
"""Original, sample-free cartoon ghost voices. Reproducible synthesis only.

No external recordings, soundfonts or third-party audio are imported.
The three generated sounds are dedicated to freely distributable Halloween
Theme releases. See ORIGINAL-GHOST-SFX-LICENSE.md.
"""
from pathlib import Path
import math
import subprocess
import tempfile
import wave
import numpy as np

SAMPLE_RATE = 32000
NAMES = ("ghost_original_woo", "ghost_original_eww", "ghost_original_oooo")

def generate(index):
    rng = np.random.default_rng(4401 + index * 31)
    duration = (2.10, 1.85, 2.55)[index]
    t = np.arange(round(duration*SAMPLE_RATE)) / SAMPLE_RATE
    x = t/duration
    smooth = lambda z: np.clip(z,0,1)**2 * (3-2*np.clip(z,0,1))
    if index == 0:
        f0 = 205-70*smooth(x)+33*np.sin(2*np.pi*x*1.4)
        formants=(530,960,2170)
    elif index == 1:
        f0 = 160+67*np.sin(np.pi*x)-40*x
        formants=(385,1690,2600)
    else:
        f0 = 180-48*np.sin(np.pi*x)+45*x
        formants=(560,1110,2390)
    f0 = f0*(1+0.014*np.sin(2*np.pi*(5.1+index*0.35)*t+index))
    phase=2*np.pi*np.cumsum(f0)/SAMPLE_RATE
    voice=np.zeros_like(t)
    for harmonic in range(1,19):
        freq=f0.mean()*harmonic
        formant_gain=sum(w*np.exp(-0.5*((freq-f)/width)**2)
            for f,width,w in ((formants[0],280,1.0),(formants[1],400,.66),
                              (formants[2],550,.27)))
        voice+=(.23+formant_gain)/harmonic**1.18*np.sin(phase*harmonic+.13*harmonic)
    # Pure synthetic breath/noise, not a sample.
    breath=rng.normal(0,1,len(t))
    breath=np.convolve(breath,np.ones(9)/9,mode="same")
    breath/=max(np.max(np.abs(breath)),1e-9)
    voice+=.075*breath
    voice*=smooth(t/.28)*smooth((duration-t)/.48)
    voice*=.88+.12*np.sin(2*np.pi*(2.4+index*.43)*t)
    echo=np.zeros_like(voice)
    for delay,gain in ((.155,.22),(.295,.11),(.465,.055)):
        offset=int(delay*SAMPLE_RATE)
        echo[offset:]+=gain*voice[:-offset]
    voice=np.tanh(1.4*(voice+echo))
    return np.round(voice*(.78/max(np.max(np.abs(voice)),1e-9))*32767).astype("<i2")

def main():
    out=Path("FileCopies")
    out.mkdir(exist_ok=True)
    for i,name in enumerate(NAMES):
        with tempfile.TemporaryDirectory() as folder:
            wav=Path(folder)/(name+".wav")
            with wave.open(str(wav),"wb") as f:
                f.setnchannels(1)
                f.setsampwidth(2)
                f.setframerate(SAMPLE_RATE)
                f.writeframes(generate(i).tobytes())
            target=out/(name+".ogg")
            subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error",
                            "-y","-i",str(wav),"-c:a","libvorbis","-q:a","4",
                            str(target)],check=True)
            if target.stat().st_size < 3000:
                raise RuntimeError("Encoded sound is empty: "+str(target))
            print(target, target.stat().st_size)
if __name__=="__main__":
    main()
