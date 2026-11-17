# Cartoon Ghost Sound Test — Beat Saber 1.40.8

The user has now supplied **both** desired tracks:

| Quest filename | Source | Creator |
|---|---|---|
| `ghost_cartoon_wooo.ogg` | [Spooky Cartoon Ghost Wooo](https://pixabay.com/sound-effects/horror-spooky-cartoon-ghost-wooo-430234/) | FadingEmbersAudio |
| `ghost_woo.ogg` | [Ghost Woo](https://pixabay.com/sound-effects/horror-ghost-woo-135543/) | Shut_Up_Ghost |

The two user-uploaded MP3 files were converted locally to OGG Vorbis, 44.1 kHz mono, for Quest testing.

## Why files are separate from the QMOD

Both recordings use the [Pixabay Content License](https://pixabay.com/service/terms/). Although the license permits use in creative works, it restricts standalone redistribution. The repository and its public QMOD intentionally do **not** embed these individual third-party audio files or publish their original recordings.

The user can install the personally supplied recordings locally on their headset. This is **not** a packaged public release of those sound files. Keep your own copies and license provenance.

## Install for personal headset test

Extract the two OGG files from the separately provided personal test ZIP, then copy them to:

`/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/`

Do not copy the ZIP itself. The OGG file names must be exact. After installing the 1.40.8 test-branch QMOD, fully quit and restart Beat Saber. The mod randomly plays available cartoon tracks; if neither file is present, it uses the three original ghost audio clips.

The previous WAV named `cartoon_ghost_woooo(1).wav` was an old synthesized demonstration and is **not** used. Keep the stable Halloween release and next-version branch untouched until the Quest test confirms sound playback.
