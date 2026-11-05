# Project 1 – Requirement Tracker

Source of truth: the DMET 502 Project 1 PDF. IDs are ours (the PDF has none).
Status values: `Not started` · `Partial` · `Done (verified)` · `Blocked (question)`.
Verification kinds: **logic test** = automated test program run against the real game.cpp (CP4: 90 checks, 0 failed);
**off-screen render** = scene drawn by the real drawing code into a hidden window and saved as an image;
**on-screen** = seen while playing the real game.

**How to read this file:** the section "Current rules" below describes the game as it is now. The status notes in the
requirement tables and every section from "Team decisions" down are **historical records**: they keep the evidence as
it was at that milestone, so they may quote values and rules that have since changed (old prices, permanent unlocks,
cooldowns, field limits). When they disagree with "Current rules", "Current rules" wins.

## Current rules (as of 2026-11-05; all values in config.h)
- **Coins:** start 30. Every defeated enemy drops one coin pickup worth 10, collected by mouse hover or when a player character touches it; pickups left at the end of a wave are collected automatically.
- **Prices:** Anab Man 10, Beram 20, Tayseer 30, paid only when the character is actually placed.
- **Unlocking (PDF p. 13):** Anab Man is unlocked from the start. Beram and Tayseer wait until the first Anab Man of the game is placed (once per game); after that a card is unlocked exactly while the balance is at least its threshold (Beram 20, Tayseer 30) and relocks below it. "Beram unlocked!" / "Tayseer unlocked!" appears when the balance rises to the threshold.
- **No placement limits:** the PDF sets no maximum, so any character can be placed as often as the player can afford, several in the same lane. Cards show the plain count ("On field 5"). There is no waiting time between placements.
- **Placement (lane first, then card, as the PDF says; confirmed by the TA, changed 2026-11-05):** a lane click toggles that lane (W/S jump and duck act on all selected lanes) and makes the lane clicked last the placement lane (white inner border); clicking the placement lane again deselects it. A card click places that character in the placement lane, then the player has to choose a lane again; with no lane chosen it shows "Select a lane first"; after a refusal the lane stays chosen. Anab Man and Beram appear at the left end of the lane and walk right; walkers of the same type can overlap while walking (accepted 2026-11-05, the PDF asks for no spacing). Tayseer stands where the lane was clicked, never on top of an enemy or another Tayseer: he steps just in front of it, or behind the Tayseers at the left end; with no free spot the placement is refused.
- **Refusals:** checks in order: game not over, unlocked, enough coins, valid lane, and for Tayseer a free spot. A refusal creates nothing and takes no coins. Locked and not-enough-coins show a message for 1.5 s; game over, invalid lane and no free spot show none (the screens never send the first two).
- **Waves:** 3 waves (easy / medium / hard: 3-0-0, 3-2-0, 4-3-3), one spawn every 6.5 s, a 5 s break between waves that clears the field; balance is kept.

## General
| ID | Requirement | Status |
|---|---|---|
| R-GEN-1 | Modern OpenGL 3.3+ | PASS – 3.3 core context (main.cpp window hints), `#version 330 core` shaders, VAO/VBO, textures; Debug + Release build |
| R-GEN-2 | Team-chosen creative theme, consistent across cutscene, story, environment, characters, weapons | Partial – theme approved (El-Ragol El-Enab, Egyptian street); environment done (CP2) |
| R-GEN-3 | Team of strictly 3–4 students | Team responsibility |

## Cutscene (Section 2)
| ID | Requirement | Status |
|---|---|---|
| R-CUT-1 | Fully animated 10–20 s cutscene before gameplay | PASS – 16 s cutscene (`cutscene.cpp`, CUTSCENE_LENGTH = 16; logic test: ends at 16 s, not before); visible window (M5C): played in full after Start |
| R-CUT-2 | At least one player and one enemy, visibly animated and interacting | PASS – Dr. Beram, Anab Man, Spider-Man, Abla Thalma, Sareeh, all animated (walking, slipper swings, web throw, duck, club hit with red flash); visible window (M5C): frames 1.7-16 s |
| R-CUT-3 | Establishes what the players defend and why it matters | PASS – the last anab bottle on the lab roof glows while Dr. Beram says it is the last one in Egypt and must be protected; visible window (M5C): first 3.5 s |
| R-CUT-4 | Short dialogue/interaction between players and enemies; subtitles allowed, not text-only | PASS – five subtitled lines (Arabic images) between heroes and villains plus action (web thrown, ducked, counter-hit); visible window (M5C): all five lines seen |
| R-CUT-5 | The cutscene leads directly into gameplay | PASS – fades to black and a fresh game starts with the "wave 1" banner; Enter/Space skips; visible window (M5C): both seen |

## Playfield (Section 3.1)
| ID | Requirement | Status |
|---|---|---|
| R-PF-1 | Side-view grid of horizontal lanes, at least 3 lanes | Done (verified M1) – 4 lanes |
| R-PF-2 | Home base at the left end, distinct and clearly visible | Implemented (CP2) – Dr. Beram's lab with formula bottle; seen in screenshot, needs team visual check |
| R-PF-3 | Enemies spawn at the right edge in a random lane and move left | Verified (CP3 screenshots: Spider-Man appears at the right edge in random lanes, walks left) |
| R-PF-4 | Non-defense players start at the leftmost position and move right until the right edge | Verified (CP3: players start at the lab, walk right, stop and stay at the right edge); M5C: they now stop 42 px before the edge (one enemy width) so an enemy appears in front of them, not on top (logic test `[edge]`) |
| R-PF-5 | Players and enemies attack automatically as soon as they appear | Verified (CP3: Beram fires and Anab Man swings with no enemy present) |
| R-PF-6 | On collision, player and enemy both stop and fight until one is defeated | Verified (CP3 screenshots: player and enemy stop in contact and fight) |
| R-PF-7 | No unit ever changes lane | Implemented (CP3) – lane stored once, never changed |
| R-PF-8 | No scene component is a single primitive; realistic, detailed models | Not started – rectangle, triangle, circle, ellipse tools ready (M2) |

## Players (Section 3.2)
| ID | Requirement | Status |
|---|---|---|
| R-PL-1 | Three types: Melee, Ranged, Defense; distinct models (not recolors) | Implemented (CP3) – Anab Man / Beram / Tayseer models; team visual check needed |
| R-PL-2 | Players are purchased with coins/collectibles | Done (CP4, logic test) – each successful placement costs its price exactly once; **M5B visible window:** Beram (40) and Tayseer (60) bought through their cards with normal clicks |
| R-PL-3 | At start only Melee is unlocked; starting coins buy at least 3 Melee | Done (CP4, logic test) – start 30 coins = 3 x Melee (10); Beram and Tayseer locked |
| R-PL-4 | Coins come from defeating enemies; other types unlock progressively | Done – **now:** every enemy drops 10; Beram unlocks at 20, Tayseer at 30, relocking below (see Current rules). Earlier (CP4, logic test): drops 10/20/30, Beram 100, Tayseer 150, permanent |
| R-PL-5 | Hovering highlights the entire lane under the cursor | Verified (CP2 screenshots show hovered lane tinted) |
| R-PL-6 | Left click toggles lane selection; multiple lanes can be selected | Implemented (M5), needs on-screen check; **M5A visible window (normal play):** all 4 lanes selected by left clicks; selection cleared at wave end |
| R-PL-7 | Select a card, choose a lane, player appears at the leftmost position | Implemented (CP4) – card first, then lane (**differs from the PDF's lane-first wording**, team decision); logic test + off-screen render; on-screen check by team; **M5A visible window (normal play):** card click, then lane click placed Anab Man (19 placements); refused click on locked Beram showed "LOCKED - UNLOCKS AT 100 COINS" |
| R-PL-8 | W/Up: players in all selected lanes jump. S/Down: they duck | Verified (logic test, CP4); on-screen check by team; **M5A visible window (normal play):** W/S presses made selected players jump over ballot boxes and duck under webs |
| R-PL-9 | Jump avoids low attacks; duck avoids high attacks | Verified (logic test, CP4): box passes under a jumping player, web over a ducking one; both hit otherwise |
| R-PL-10 | Defense player is stationary and ignores jump/duck | Verified (logic test, CP4) |
| R-PL-11 | Increasing red tint with damage (or scale down per hit) | Verified (CP3 screenshots: damaged units redder) |
| R-PL-12 | Defeated players are removed | Implemented (CP3) – player removal not yet observed |
| R-PL-13 | Players may overlap/pass each other in a lane | Verified (CP3: Anab Man walked past a stationary Tayseer) |
| R-PL-14 | Defense player protects players behind it; protection ends when they pass it or it dies | Verified (logic test, CP4): web and slipper hit Tayseer, not Beram behind him; Beram is hit after walking past |
| R-PL-15 | Non-defense players deal 1 damage per attack | Implemented (CP3) – PLAYER_DAMAGE = 1 |
| R-PL-16 | Health: Melee 5, Ranged 5, Defense 7 | Verified (CP3: bars show 5 / 5 / 7 segments) |
| R-PL-17 | Melee weapon animates in hand; Ranged fires a projectile that disappears on hit and never harms players | Partial – club swing and Beram projectile seen (CP3); team should confirm the animation reads well |
| R-PL-18 | Each player's remaining health always visible above/below it | Verified (CP3: bar + number above every player) |

## Enemies (Section 3.3)
| ID | Requirement | Status |
|---|---|---|
| R-EN-1 | Easy: upper-ranged, normal speed, attacks from above (duck), 5 HP | Verified (logic test, CP4): web starts high; ducking avoids it |
| R-EN-2 | Medium: lower-ranged, normal speed, attacks from below (jump), 5 HP | Verified (logic test, CP4): ballot box at ground height, jumping avoids it; Sareeh seen in off-screen render |
| R-EN-3 | Hard: melee, runs, 7 HP, 2 damage; all others 1 damage | Verified (logic test, CP4): faster than Sareeh, slipper does 2 damage; Thalma seen in a visible game window (charge, slipper swing, 5 -> 3 on Anab Man); **M5A visible window (normal play):** Thalma in wave 3, slipper swing visible, Anab Man 3 -> 1 in one hit (exactly 2) |
| R-EN-QUEUE | (Team decision, not in the PDF) Enemies in the same lane never overlap: an enemy stops 6 px behind the living enemy ahead and follows it at its pace; it resumes as soon as that enemy moves on or dies. Speeds and damage unchanged | Verified (logic test + visible game window); M5A visible window (placed scene): Thalma follows a walking Spider-Man at 41-42 px, waits behind a stopped one, resumes when he dies |
| R-EN-4 | Waves: W1 ≥3 easy; W2 ≥5 easy+medium; W3 ≥10 easy+medium+hard; random lanes | Verified (logic test, CP4): queues 3 / 3+2 / 4+3+3; wave 2 spawns exactly 5; **M5A visible window (normal play):** spawns read from the screen: W1 3 easy, W2 3 easy + 2 medium, W3 4 easy + 3 medium + 3 hard, 3 s apart |
| R-EN-5 | Wave starts at its first spawn, ends at its last spawn; clear the rest before the next wave | Verified (CP3 on-screen for wave 1; CP4 logic test for the transition) |
| R-EN-6 | Defeated enemies drop collectibles, collected when a player collides with them | Implemented – drops verified (logic test, once per enemy). Collected by **player-character collision** (PDF wording) **and** mouse hover; both go through one function that removes the coin, so it is credited once (logic test). A jumping character passes over a coin. Not yet seen live (no kill happened in the live runs); **M5A visible window (normal play):** Anab Man walked into a dropped coin, +10, counter 10 -> 20, cursor parked outside the play area and no click |
| R-EN-7 | Three distinct designs that communicate attack type and difficulty | Implemented – all three seen in off-screen renders (CP4); team visual check needed |
| R-EN-8 | Increasing red tint with damage (or scale down per hit) | Verified for Spider-Man (CP3) |
| R-EN-9 | Each enemy's remaining health always visible above/below it | Verified for Spider-Man (CP3) |

## Win / Lose (Section 3.4)
| ID | Requirement | Status |
|---|---|---|
| R-WL-1 | Win by defeating all enemies of all three waves | Verified (logic test + off-screen render, CP4); **M5A visible window (normal play):** won the game through waves 1-3 with normal controls ("YOU WIN!"), game froze |
| R-WL-2 | Lose when any enemy reaches the home base; everything stops | Verified (CP3 on-screen; CP4 logic test: no movement and no placement after the loss); **M5A visible window (normal play):** no input: Spider-Man reached the lab in wave 1, "THE LAB HAS FALLEN!", positions frozen for 14 s |
## Visuals and UI (Sections 4.1, 4.2)
| ID | Requirement | Status |
|---|---|---|
| R-VIS-1 | Everything player-related looks completely different from enemy-related | Implemented (CP3) – different models, green/gold player bars vs red/dark enemy bars |
| R-VIS-2 | At least 5 environmental elements that fit the theme | Implemented (CP2) – 8 elements: facades, balconies, water tanks, satellite dishes, clotheslines, streetlights, kiosk, election posters |
| R-UI-1 | Coin/collectible count shown at the top and updated correctly | Done (CP4, logic test + off-screen render); **M5B visible window:** Arabic counter "العملات" with Arabic-Indic digits, 30 -> 100 -> 60 -> 20 -> 150 -> 90 followed every purchase and grant |
| R-UI-2 | Player cards show whether each type is unlocked | Done (CP4, off-screen render); **M5B visible window:** Arabic cards; locked cards show "يُفتح عند ١٠٠ / ١٥٠" |
| R-UI-3 | Locked cards are grayscale and ignore hover/selection | Implemented (CP4) – grayscale + padlock + 'UNLOCKS AT n' (render); cannot be selected (logic test); no hover effect; **M5B visible window:** Beram/Tayseer gray with padlock while locked; clicking locked Beram refused with "مقفول - يُفتح عند ١٠٠ عملة", no selection |
| R-UI-4 | Unlocked cards are in color, react to hover, can be selected | Implemented (CP4) – hover/selection seen in off-screen render; on-screen check by team; **M5B visible window:** selected card: gold outline + "اختر حارة"; cooldown overlay + "انتظر ١٫٥ ث"; unaffordable: dimmed + red price + "تحتاج ٤٠" |
| R-UI-5 | Cards unlock automatically at the cost and relock automatically below it | Done – **now (since 2026-11-04):** cards relock below their threshold, as the PDF says (see Current rules). Earlier team decision (CP4): unlock was **permanent**, no relock, pending TA (Q1/Q2); **M5A visible window (normal play):** "BERAM UNLOCKED!" exactly when the counter reached 100; **M5B visible window:** Beram placed at 100 -> 60 and 60 -> 20: card stays unlocked (shows "تحتاج ٤٠", not "يُفتح عند"); Tayseer 150 -> 90 stays unlocked (coins granted with a test-only key, see Milestone 5B) |
## Menus (Section 4.3)
| ID | Requirement | Status |
|---|---|---|
| R-MENU-1 | Main menu at launch: Start (cutscene, then game), Exit | PASS – `screens.cpp`: shown at launch (before the cutscene) with Start (cutscene, then game) and Exit; logic test + visible window (M5C): Start played the cutscene, Exit closed the process (exit code 0) |
| R-MENU-2 | Esc opens pause menu: Continue, Restart, Main Menu; pausing stops all gameplay updates | PASS – Esc during play: Continue / Restart / Main Menu, Esc again resumes; the game is not updated while paused; logic test + visible window (M5C): positions identical after 2 s paused, Continue resumed, Restart and Main Menu worked |
| R-MENU-3 | Win/Lose menu: Restart, Main Menu | PASS – win and lose panels with Restart and Main Menu (they replace the temporary R key); logic test + visible window (M5C): lose menu -> Restart worked; win menu after a full play-through |

## Bonus: Sound (Section 5) – optional, kept separate
| ID | Requirement | Status |
|---|---|---|
| R-SND-1 | Background music | NOT IMPLEMENTED – optional bonus (PDF section 5), not started; not required |
| R-SND-2 | Sound effects for at least 4 interactions | NOT IMPLEMENTED – optional bonus (PDF section 5), not started; not required |
| R-SND-3 | Sound Settings menu from the Main Menu: volume up, volume down, mute; really changes sounds | NOT IMPLEMENTED – optional bonus (PDF section 5), not started; not required |

## Deliverables (Sections 6, 7)
| ID | Requirement | Status |
|---|---|---|
| R-DEL-1 | Compiles with no errors (otherwise zero) | Partial – M5A builds Debug + Release x64 with no compiler warnings; M5B: Debug + Release x64 build, no compiler warnings (LNK4098 from glfw3.lib in Debug only) |
| R-DEL-2 | Zip of source files only, named `[Team name]-[Member name].zip` | Not started – the zip must also contain `assets/ui/` (Arabic label PNGs) and `include/stb_image.h`; without the PNGs the game still runs with English text (see Q6) |
| R-DEL-3 | 2–5 min video with voiceover in the required order | Not started |
| R-DEL-4 | Drive link access granted; video link submitted through the form | Not started |

## Open questions (ask the TA, do not invent)
- **Q1:** Read the 2x / 3x numbers as unlock thresholds, separate from purchase prices. Then every rule can hold, e.g. Melee price 10, unlock thresholds 25 / 50 / 75, start 45 coins: three Melee can be bought before the Melee card relocks, and Ranged/Defense start locked. Confirm this reading with the TA.
- **Q2:** Is the unlock threshold the same number as the purchase price?
- **Q3:** What should a player do on reaching the right edge: stop there, or leave the screen?
- **Q4:** How many coins should each enemy type drop? The PDF does not say.
- **Q6:** The zip is "source files only". Do the Arabic UI PNGs in `assets/ui/` count (they are generated by `assets/ui/make_labels.ps1` from `labels.tsv`)? If they are left out the game falls back to English text.
- **Q5:** "Collected when a player collides with them": does "player" mean the person (mouse) or a player character? Resolved by doing both (team decision after CP4).

---
**Historical records from here down.** Each section keeps what was decided or verified at that time; for the rules
that apply now, see "Current rules" at the top.

## Team decisions (approved before CP2)
- Theme: "أزمة آخر قارورة عناب", inspired by El-Ragol El-Enab. Defend Dr. Beram's lab and the last El-Anab formula bottle.
- Players: El-Ragol El-Enab (melee), Beram (ranged; green T-shirt, gray vest), Tayseer Pasha (defense; police uniform).
- Enemies: Spider-Man (easy, high attack), Sareeh El-Feel (medium, low attack; follow the photo), Abla Thalma (hard, melee).
- Placement: card first, then lane. **Differs from the PDF wording** ("must first select a lane ... then selects a character").
- Right edge: moving players stop there and keep attacking. Damage: red tint. Health: bars (numbers optional).
- Arabic text: PNG images drawn as textures (stb_image). English UI text: stb_easy_font. Audio: miniaudio (zip contents to confirm).
- Coins (PROVISIONAL, configurable, waiting for TA): start 30; Melee 10; Ranged price 40 / unlock 100; Defense price 60 / unlock 150; drops 10 / 20 / 30.

## Checkpoint 4 rules (team decisions, all values in config.h)
- **Unlocking and buying are separate.** Unlocking: Anab Man from the start; Beram the first time the balance reaches 100; Tayseer the first time it reaches 150. Unlocks are stored separately and are **permanent** until restart: spending below the threshold never locks a card again. Buying: every successful placement costs 10 / 40 / 60. Unlocking places nothing. This permanent-unlock reading of the PDF is our interpretation; the PDF wording suggests relocking (R-UI-5).
- **Coins:** start 30. A defeated enemy drops one coin pickup (Easy 10, Medium 20, Hard 30), in the same step that removes it, so it can drop only once. A pickup is collected when the mouse hovers over it or a player character's body touches it (not while jumping over it); it then disappears and cannot be collected again. When a wave ends (and on the win), all remaining pickups are collected automatically. Restart resets the balance, unlocks and pickups.
- **Placement:** card first, then lane (differs from the PDF's lane-first wording). Second click on the selected card cancels it. With no card selected, clicking a lane still toggles it for jump/duck. Checks, in order: game not over, unlocked, enough coins, cooldown finished, field limit, valid lane, one Tayseer per lane. Any failure creates nothing, takes no coins and shows a message for 1.5 s. Coins are taken only after the character is created.
- **Limits and cooldown:** Anab Man 7, Beram 4, Tayseer 4 (max 1 per lane). 2 s cooldown per character type (not shared), shown as a shrinking dark overlay on the card.
- **Wave transition:** the moment a wave is cleared: "WAVE CLEARED!", all players and projectiles removed, lane and card selections cleared, cooldowns reset, remaining coins collected. Balance and unlocks are kept. Placing during the 3 s break is allowed; the next wave then starts with only those new characters.
- **Card layout:** the top bar stays 70 px; the 160 x 96 cards hang below it over the 4-floor buildings, because a 110 px bar would hide the formula bottle, the lab antenna and the roof props.

## Milestone 5A fixes
- **Game freeze (fixed):** the health-bar stacking loop compared `fabs(b.y - y) < 13` after moving a bar to `b.y + 13`; float rounding made some fractional heights (a jumping player's bar) still "overlap", so the loop never ended and the game stopped responding. Found by a full live play-through (the game froze in wave 2 twice). Now the bar is compared with the exact value it was moved to, so it always terminates. Logic test runs the real bar layout for 3000 fractional heights; with the old line the same test hangs.

## Milestone 5B – Arabic UI and visual polish

**Library added:** `include/stb_image.h` v2.30 (Sean Barrett, public domain, single header, PNG decoding only via `STBI_ONLY_PNG`). SHA-256 of the file: `594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3`.

**How the Arabic text works**
- Every label is a PNG in `assets/ui/`, generated by `assets/ui/make_labels.ps1` from `assets/ui/labels.tsv` (font: Dubai Bold, ships with Windows; white text on transparent, tinted in the shader). Re-run the script after editing the TSV.
- Seven size classes; all images of one class share one height (shared baseline). Numbers use Arabic-Indic digit images (٠–٩, `/`, `+`, `٫`), written left to right as in Arabic text.
- `arabic_text.cpp` lays lines out right to left (label + number pieces). It looks for `assets/ui/` in the working folder, then next to the .exe and up to two folders above it. If any image is missing it prints a console warning and draws the old English text instead.
- English kept: the keys R, W, S inside Arabic sentences. Character names are shown in Arabic (الرجل العناب، بيرم، تيسير) because they are the characters' own Arabic names.

**Labels translated:** coin counter, wave indicator, card names, HP / damage / "blocks", field count, every card status (unlocks at, limit, need, wait, choose a lane, ready), all placement and unlock messages, wave / wave-cleared / win / lose banners and the restart line, the hint line, floating "+N", health-bar numbers, and the signs (lab board, kiosk, election posters, Tayseer's shield).

**Visual-polish fixes**
- Card portraits are clipped to their picture panel (Beram's ray gun used to cover the card text). `setClipRect` uses the viewport size, so it also works on high-DPI framebuffers.
- Health bars and their numbers are kept fully on screen (an enemy that just spawned at the right edge had its bar cut off). Only the drawing position is clamped; unit positions are unchanged.
- Floating "+N" and health numbers get a dark 1 px outline (the gold "+10" was hard to read over a selected lane).
- English fallback is laid out left to right and at the old sizes.

**Results (visible window = a real 1280x720 game window captured from outside the game; logic = scratchpad test program, 151 checks, 0 failed)**

| Item | Result | Evidence |
|---|---|---|
| Beram unlock permanent after spending below 100 | PASS (logic + visible) | Visible: Beram card stays unlocked at 60 and 20 coins; at 20 it shows "تحتاج ٤٠", not "يُفتح عند". Coins were granted with a test-only key G in a test copy of `main.cpp`; every purchase used normal card + lane clicks. Logic: `[cards]` checks |
| Beram and Tayseer placed through their cards (unlock, cost, cooldown, limits) | PASS (logic + visible) | Visible: Beram 100 -> 60 -> 20, Tayseer 150 -> 90, second Tayseer in the same lane refused, Anab Man cooldown message. Logic: cooldown, limit 4 for both, one Tayseer per lane, unlocks kept over a wave transition |
| Arabic labels load from the project folder (VS / build_and_run.bat) | PASS (logic + visible) | Debug and Release started in the project folder show the Arabic UI |
| Arabic labels load when started from another folder (exe in `x64\Release`) | PASS (visible) | copy laid out like the project, started from C:\Windows\Temp |
| Missing assets -> English fallback, no crash | PASS (logic + visible) | exe alone, started from C:\Windows\Temp: English UI, correct word order |
| Every label/digit image matches its size class; labels.tsv matches the code | PASS (logic) | `[arabic]` checks |
| Cards, coin counter, wave indicator readable | PASS (visible) | zoomed real-size frames |
| Placement / unlock / cooldown / limit / lane messages readable | PASS (visible) | all 7 messages seen during the scripted card test |
| Wave banner, wave cleared, win, lose, restart line | PASS (visible) | normal play-through (win) and card test (loss) |
| Floating "+N" and health numbers readable | PASS (visible, after the outline fix) | harness run with early enemies |
| Normal play-through after the UI change (waves 3 / 5 / 10 spawns, 3 s apart, win) | PASS (visible) | unchanged Release build played by the screen-reading bot with normal controls |
| Health-bar 3000-height regression | PASS (logic) | `[bars]` |
| Hover brightening of cards with a real mouse | NOT TESTED | the bot only moves the cursor to click |
| Very high-DPI (framebuffer larger than 1280x720) | NOT TESTED | only tested at 1:1 |

## Milestone 5C – compliance audit against the official PDF

**Authority:** `GUC_312_70_70223_2026-10-04T13_33_53.pdf` (Downloads; the newest official copy). The 2026-10-01 copy has an error in the players table (Melee "advances towards the left"); `descrption - edited.pdf` on the Desktop is a re-saved copy with broken ligatures.
**AI policy:** the PDF (page 1) says LLM/AI tools are strictly prohibited. The user confirmed on 2026-10-14, and again on 2026-11-02 after being shown that sentence, that the team has permission that overrides it.

Levels: **logic** = scratchpad test program on the real code (190 checks, 0 failed); **visible** = real 1280x720 game window, captured from outside the game, normal mouse clicks / key presses only.

| # | Requirement (PDF section) | Status | Evidence |
|---|---|---|---|
| 1 | OpenGL 3.3+ (1) | PASS | 3.3 core profile, `#version 330 core`; Debug + Release x64 build |
| 2 | 10-20 s animated cutscene, a player and an enemy interacting, dialogue, not text only (2) | PASS (logic + visible) | 16 s; R-CUT-1..5 |
| 3 | Distinct home base on the left, more than one primitive, not a recolor (3.1, 4.1) | PASS (visible, every run) | Dr. Beram's lab: building, windows, doors, warning signs, name board, antenna, glowing formula bottle |
| 4 | Melee, Ranged, Defense with distinct models (3.2) | PASS (logic + visible) | Anab Man (club), Beram (ray gun, projectile), Tayseer (riot shield, stationary) |
| 5 | Easy attacks from above, Medium from below, Hard runs + close range + 2 damage (3.3) | PASS (logic + visible) | web (duck), ballot box (jump), Thalma's slipper: exactly 2 per hit (M5A) |
| 6 | At least 5 environment elements (4.2) | PASS (visible) | facades, balconies, water tanks, satellite dishes, clotheslines, streetlights, kiosk, election posters |
| 7 | Defeated enemies drop collectibles, collected when the player collides with them (3.3) | PASS (logic + visible) | character touch (M5A, cursor parked away), plus mouse hover as an extra (team decision, Q5) |
| 8 | Start coins buy at least 3 Melee (3.2) | PASS (logic + visible) | 30 coins, Melee costs 10 |
| 9 | Unlock / purchase rules (3.2 table, 4.2) | **FAIL against the PDF wording (team decision)** | Unlock is permanent, but PDF 4.2 says: "If the player's available points/collectibles later fall below the required unlock cost, the corresponding player card must automatically return to its locked state." Thresholds 100 / 150 are not "2x / 3x the collectibles required to unlock the Melee Player" (Melee is unlocked at 0 and costs 10). Kept as approved; see Q1, Q2 |
| 10 | Waves: 3 easy; 5 easy+medium; 10 easy+medium+hard; a wave runs from its first to its last spawn (3.3) | PASS (logic + visible) | 3 / 3+2 / 4+3+3, 3 s apart (M5A, M5B, M5C play-throughs) |
| 11 | Main menu (Start -> cutscene -> game, Exit), Esc pause (Continue, Restart, Main Menu), Win/Lose menu (Restart, Main Menu) (4.3) | PASS (logic + visible) | R-MENU-1..3 |
| 12 | Win = all waves defeated; Lose = any enemy reaches the base, everything stops (3.4) | PASS (logic + visible) | lose menu after an enemy reached the lab; win menu after a full play-through started from the main menu: 0 of 81,944 sampled play-area pixels changed between 80 s and 140 s |
| 13 | Lane hover/selection, W/Up jump, S/Down duck, jump avoids low / duck avoids high, Defense unaffected (3.2) | PASS (logic + visible) | the bot jumps/ducks in every play-through; logic tests [16] |
| 14 | Sound (5) | NOT IMPLEMENTED – **optional bonus**, not mandatory | no mandatory sound requirement exists |
| 15 | Red damage tint, defeated units removed, health always visible (3.2, 3.3, 4.1) | PASS (visible) | red tint + segmented bars + Arabic-Indic numbers |
| 16 | Locked cards grayscale and ignore hover/selection; unlocked cards colored and react to hover (4.2) | PASS (logic + visible); hover brightening with a real mouse NOT TESTED | M5B card test |
| 17 | Coin count at the top, updated correctly (4.2) | PASS (logic + visible) | counter followed every purchase and pickup |
| 18 | Video, 2-5 min, voice-over, given order (6) | NOT DONE – team task outside the code | |
| 19 | Zip `[Team name]-[Name].zip`, "only .h, .cpp, ... files", drive link, no compilation errors (7) | NOT DONE – package not built yet | must include `assets/ui/` + `include/stb_image.h` (Q6); builds with Visual Studio or `build_and_run.bat` |

**Fixes in this milestone**
- **Enemy spawning on a player at the right edge (reproduced, fixed):** players stopped at `SCREEN_WIDTH - half - 5`, which is where enemies spawn (`SCREEN_WIDTH - half - 2`), so a new enemy appeared 30-38 px inside a waiting player (the new logic test failed 6 of 6 before the fix). Players now stop `ENEMY_ENTRY_ZONE` = 42 px (widest enemy + spawn margin) before the edge. Enemy spawn position, speed and attacks are unchanged. Test `[edge]`: every player type against every enemy type now meets with a 0-2 px gap.
- **Zero coins readable:** still the Arabic-Indic zero; the label generator thickens its ink by 10 % of the font size, so 0 coins shows a clear round dot instead of a tiny one (seen in a visible window).
- **Temporary R restart removed:** restart is now in the pause and win/lose menus; the hint line says Esc pauses.
- Menu buttons use their own size class (`btn`), so the existing labels keep their exact layout.

**New ambiguity to raise with the TA**
- **Q7:** PDF 3.2: "To place a player on the grid, the player must first select a lane ... The player then selects a character from the active player card UI and chooses which lane to place that player on." We implemented card first, then lane; clicking a lane without a card selects it for jump/duck.

## Final technical audit – approved fixes (2026-11-03)

Unchanged on purpose: permanent unlock and thresholds (F1/F2), card-first placement (F3), all stats, prices, waves and visuals.

| # | Problem found in the audit | Fix (file / function) | Evidence |
|---|---|---|---|
| B1 | A coin dropped at the right edge (x 1256-1258) was out of reach of the player waiting there (reach ends at 1249) | `game.cpp` `removeDefeated`: drop x = `min(enemy x, SCREEN_WIDTH - ENEMY_ENTRY_ZONE)` (1238) | logic [fixB1] all 3 enemy types; visible harness: dropped at 1238, collected by touch 0.01-0.03 s later (cursor parked away) |
| B2 | With an enemy just entered in every lane, the next spawn was forced into a crowded lane (-33.6 px overlap) | `game.cpp` `spawnEnemy` picks a random lane among the free ones and returns false if none; `updateGame` retries every frame without popping the queue | logic [fixB2]: queue order kept, exactly 10 spawns in wave 3, no wave end while spawns are pending; visible harness: blocked 6 s, then Thalma spawned in the freed lane |
| B3 | An enemy killed by a melee hit still moved / shot / swung once in that frame | `game.cpp` `updateGame`: enemy loop skips `health <= 0` | logic [fixB3]: no web, no slipper damage, one coin; visible harness: same |
| B4 | `rand()` never seeded: same enemy lanes every launch | `main.cpp` `main`: `srand(time)` once at startup | logic [fixB4] fixed seed repeats; visible: 3 launches gave lanes 1-3-1, 0-0-0, 0-2-2 |
| R1 | Projectiles could jump over a body at 20 fps or less | `game.cpp` `updateGame`: collision against the whole path flown this frame (nearest enemy / front-most player on the path) | logic [fixR1] 0 misses at 60/30/20/15 fps, jump/duck still dodge; visible harness at 15 fps: 10 shots / 9 hits (1 in flight), 7 boxes / 7 hits, 0 pass-throughs |
| R2 | A click or W in the same frame as Esc was still handled | `main.cpp`: Esc during play pauses and nothing else is handled that frame | visible (production exe): Esc+card click -> paused, card not selected; Esc+W -> no jump; normal click / W afterwards work |
| R3 | On the loss frame, units killed earlier in that frame stayed on screen with 0 health | `game.cpp` `updateGame`: on loss, `removeDefeated()` then stop | logic [fixR3]; visible harness: loss screen shows no 0-health unit, frozen |

Logic tests: 244 checks, 0 failed (the 190 earlier checks + 54 new). Full normal play of the production Release exe from the main menu: cutscene, waves 1-3, win menu at 81.7 s, lanes unchanged afterwards.

## English translation (2026-11-03)

The whole game is now in English. This replaces the Arabic UI described in Milestone 5B (the 5B notes and older rows are kept as history). Gameplay code was not changed.

- **Text images:** `assets/ui/labels.tsv` now holds the English text; `make_labels.ps1` renders it left to right with Western digits (same Dubai Bold font, same size classes). Each class is padded to the old image height, so every label sits exactly where the Arabic one did. All 156 PNGs were regenerated.
- **Code:** `arabic_text.*` renamed to `text_labels.*` (functions `initTextLabels`, `textLabelsLoaded`, `textAssetFolder`); lines are laid out left to right. The fallback (if `assets/ui` is missing) draws the same English text with stb_easy_font.
- **Small layout changes:** win/lose panel 560 -> 760 px wide ("You win! The formula is safe"); "POLICE" on Tayseer's shield starts at y 8 instead of 16 so it ends below the hand. Window title: "The Last Anab Bottle Crisis".
- **Not changed (artwork):** the Arabic letter "ain" emblem on Anab Man's badge and on the coins (drawn with arcs, part of the models).
- **Tests:** logic 261 checks, 0 failed (244 earlier + 17 new `[english]` checks: left-to-right order, labels.tsv = code text, no Arabic in sources, every long label fits its space). Visible: main menu, whole cutscene, cards, all 10 messages, coin rewards, health numbers, wave banners, pause, lose, win (win screen through a test harness), Release exe started from C:\Windows\Temp loads the English images.
## Feature-folder reorganisation (2026-11-04)

Files moved into feature folders (see `documentation/README.md` for the map). Paths in older sections above refer to the old flat layout: `assets/ui/` is now `ui/assets/labels/`, and `characters.cpp` was split into `players/` (Anab Man, Beram + energy shot, Tayseer), `enemies/` (Spider-Man + web, Sareeh El-Feel + ballot box, Abla Thalma), `shared/character_parts` (legs and swing helpers used by both) and `game/collectibles/coin`. No game code changed: logic tests 292/0, all 13 test scenes pixel-identical before and after, Debug and Release build.
## Third-party libraries in one folder (2026-11-04)

`include/`, `External/`, `lib/` and `src/` were merged into `libraries/` (one folder per library: `GLFW/`, `GLAD/` incl. KHR, `GLM/`, `stb_image/`), with each library's inner folders unchanged, so no `#include` line changed. All 190 files were copied, checked byte-identical, built against, and only then removed from the old folders. The project's include and library folders point at `libraries/` in all four configurations; `glad.c` is still compiled as C. `.gitignore` keeps ignoring `*.lib` except `libraries/GLFW/lib/glfw3.lib`. Win32 Debug/Release still fail to link exactly as before the move (no 32-bit GLFW in the project; they never linked GLFW).

## History notes moved out of the code comments (2026-11-05)

These explanations used to sit in source comments. The rules themselves are unchanged and still described in the code; only the history was moved here.

- **Lane count (`config.h`, `LANE_COUNT`):** the PDF asks for at least 3 lanes; the team approved 4.
- **Enemy attack interval (`config.h`, `ENEMY_ATTACK_INTERVAL`):** raised from 2.0 s to 3.0 s to make the game easier.
- **Tuning values (`config.h`, `PLAYER_SPEED` to `JUMP_HEIGHT`):** these were not in the team's original tuning table; they were chosen during development by playing.
- **Economy values (`config.h`):** the section was marked as waiting for TA confirmation; the values are now settled (see Current rules).
- **Wave table (`game.cpp`, `WAVE_TABLE`):** 3-0-0, 3-2-0, 4-3-3 is the table the team approved.
- **Right-edge stop (`game.cpp`, `walkPlayer`):** players stop `ENEMY_ENTRY_ZONE` before the right edge because an enemy spawning on top of a waiting player used to overlap it by 30-38 px.
