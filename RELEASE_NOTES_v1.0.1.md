# Halloween Theme v1.0.1 — Beat Saber Quest 1.40.8

## Changes
- The results-screen ghost celebration now triggers **only with rank S, SS or SSS**. Rank A or below does not produce celebration ghosts.
- Preserves twelve individually randomized newer ghost designs, including the shy/peekaboo ghost at the same selection weight as the rest.
- Balanced left/right spawn lanes, slow upward drift, occasional sideways motion, mild transparency, and automatic cleanup on leaving results.
- Ghost voice selection supports the three OpenGameArt recordings: Ogrebane's Ghost (CC0), Berklee's Scary Ghost 1 (CC BY 3.0), and qubodup's Ghost Breath (CC0). The appropriate audio credits are provided in `FileCopies/GHOST_AUDIO_CREDITS.txt`.
- An **optional personal** 5-sound testing variant additionally supports DRAGON-STUDIO Creepy Ghost Sound and Cemetery Wind. Those two Pixabay recordings are **not automatically bundled** in the GitHub CI release package.

## Audio and artwork notices
- The original Halloween menu music by Alex Morgan is CC BY 4.0, as explained in `THIRD_PARTY_NOTICES.md`.
- The three OpenGameArt SFX are separately licensed. Keep the bundled credits file, including the Berklee CC BY 3.0 attribution.
- The two Pixabay recordings require review for redistribution of extractable QMOD audio assets; they are omitted from automated public packages until cleared.
- The separately sourced Uppbeat cinematic music sting is **not** bundled.

## Release readiness checklist
- [x] Change results ghost threshold from rank A+ to S+.
- [x] Update QPM and QMOD package metadata to v1.0.1.
- [x] Compile and validate the 1.40.8 source through GitHub Actions.
- [ ] Commit the four approved `ghost_extra_0.png` to `ghost_extra_3.png` sprites as 256x256 RGBA PNGs under `FileCopies/`.
- [ ] Add their `fileCopies` entries to `mod.template.json` and assert their presence in the automated archive validation.
- [ ] Verify the resulting public QMOD and source archive are complete and match the published v1.0.1 tag.
- [ ] Merge the reviewed release branch and tag the final commit `v1.0.1` to activate the existing release workflow.

This branch is a **release candidate**, not an already-published GitHub release.
