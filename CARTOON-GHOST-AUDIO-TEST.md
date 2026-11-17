# Cartoon ghost sound test — Beat Saber 1.40.8

This branch supports two optional user-supplied sounds. Because the Pixabay Content License restricts standalone redistribution, the source repository and QMOD do not redistribute those sound files as individual archives.

- **Ghost Woo**, Shut_Up_Ghost — https://pixabay.com/sound-effects/horror-ghost-woo-135543/
- **Spooky Cartoon Ghost Wooo**, FadingEmbersAudio — https://pixabay.com/sound-effects/horror-spooky-cartoon-ghost-wooo-430234/

License: https://pixabay.com/service/terms/

## Test the uploaded Ghost Woo track

The user supplied the *Ghost Woo* MP3; an OGG conversion has been prepared as `ghost_woo.ogg`. Copy that converted file to:

`/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_woo.ogg`

If the user separately supplies/downloads the correctly licensed *Spooky Cartoon Ghost Wooo* recording, copy it as:

`/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_cartoon_wooo.mp3`

**Important:** A WAV upload labeled `cartoon_ghost_woooo(1).wav` was byte-for-byte identical to an earlier assistant-generated robotic demonstration; it is *not* the FadingEmbersAudio track and is deliberately not used.

The mod loads whichever of the two optional files are installed at startup. If neither exists, the three original ghost sounds are used. Fully quit/restart Beat Saber after copying sounds.

This feature is experimental; validate Quest 3 playback before releasing. Stable branches are not modified.
