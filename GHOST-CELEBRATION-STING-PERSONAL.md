# Personal ghost celebration sting (Beat Saber Quest 1.40.8)

The optional one-shot sound is **not redistributed** with the public QMOD.
A user-supplied, properly licensed copy should be placed at:

`/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_celebration_sting.ogg`

The mod uses `MediaAsyncLoader` to load it, plays the sting once
per A-or-higher results activation (A, S, SS, SSS), and stops it when
the results screen deactivates. The clip does not loop and doesn't
restart for each spawned ghost. Firework explosion sounds are suppressed
for eligible results; unqualified results retain the original behavior.

The user-provided source file is approximately 20.66 seconds of stereo
MP3 and was transcoded to Vorbis OGG for personal installation.
Keep the original creator/license attribution documentation with your
copy. Licensing and redistribution require checking the specific
license granted at download; do not push the recording to this public repo.

**Local installation without ADB**: in a personal QMOD package, add the
OGG file at the archive root and add the following fileCopy entry to
`mod.json`. QuestPatcher will place the OGG in the expected directory:

```json
{"name":"ghost_celebration_sting.ogg","destination":"/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_celebration_sting.ogg"}
```

If the optional file is absent, ghosts still work without the music.
