# Ghost Celebration — 12-design visual test for Quest 1.40.8

This test branch adds eight independent original-style ghost sprites to the four
proven sprites in the 1.40.8 results celebration.

## Sprite inventory
- `ghost_single_0.png` ... `ghost_single_3.png`: original four, preserved
- `ghost_new_0.png`: tall reaching wailer
- `ghost_new_1.png`: smiling playful spirit
- `ghost_new_2.png`: two-arms-up wailer
- `ghost_new_3.png`: sideways grinning swooper
- `ghost_new_4.png`: worried curly-tailed ghost
- `ghost_new_5.png`: **shy/peekaboo ghost** specifically requested for retention
- `ghost_new_6.png`: crouching grinning ghost
- `ghost_new_7.png`: fierce flying wraith

## Consistent effects
All eight new PNGs have the same 256×256 transparent RGBA canvas as the
original four, with their main silhouettes separately cleaned. They enter the
same random pool; every spawned ghost remains its own object and inherits
the existing scale range, alpha fading, 7–9.5 second lifetime, gentle sway,
upward motion, left/right dispersion, world-space positioning, and teardown.

The number of simultaneously visible ghosts is **not increased** (eight cap);
the variety is in the art designs, not the visual density. A qualifying rank
of A, S, SS, or SSS is still required. The existing Ogrebane CC0 ghost sounds
are unchanged and the unlicensed Uppbeat entrance sting is not included.

## What to verify on Quest 3
- All 12 designs, including shy/peekaboo, eventually appear as separate ghosts
- New sprites do not display black/blue matte blocks or checkerboard
- New designs are roughly similar apparent sizes and softness to the originals
- Ghosts do not cluster and do not cover the score panel
- Effects and audio stop on Continue/Restart
- Rank B or lower does not produce ghosts

Build passing does not verify headset rendering; visual confirmation is needed.
