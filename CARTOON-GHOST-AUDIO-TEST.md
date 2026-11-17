# Cartoon ghost audio test (Beat Saber 1.40.8)

This private-development *branch* can use the two cartoon ghost voices selected for the results-screen celebration. The repository does **not** include the third-party MP3s; the mod reads user-installed audio from the headset. This is intentional to avoid publishing individually extractable Pixabay recordings as standalone file copies.

## Sound sources

1. **Spooky Cartoon Ghost Wooo** — FadingEmbersAudio — https://pixabay.com/sound-effects/horror-spooky-cartoon-ghost-wooo-430234/
2. **Ghost Woo** — Shut_Up_Ghost — https://pixabay.com/sound-effects/horror-ghost-woo-135543/

These are subject to the **Pixabay Content License**, not CC0; see https://pixabay.com/service/terms/. Download them from Pixabay yourself and observe its terms. Audio is credited here for clarity even though attribution is not required.

## How to test with your Quest 3

Download each MP3 directly from the corresponding Pixabay page. Rename the files (do not change their encoding or extension) to:

- `ghost_cartoon_wooo.mp3`
- `ghost_woo.mp3`

Copy both files to the headset directory:

`/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/`

Then **fully restart Beat Saber**. The mod selects both new sounds only if both MP3 files exist; otherwise it falls back to the three previously packaged OGG effects. It does not modify the existing Halloween menu music, ghost visuals, or any published release.

This is currently an *integration test*. If the runtime audio loader rejects MP3, we'll convert the two user-downloaded tracks to OGG and use the same approach for them. Confirm audio playback on Quest 3 before treating this as complete.
