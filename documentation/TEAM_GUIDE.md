# Team Guide – How the Game Works

A study guide for Team AAMM: what each part of the code does and which functions matter most, so every member can
explain the whole game. It describes the code as it is now. It does **not** say who wrote what: the four study areas
at the end are a suggested way to split the preparation, not a record of authorship.

Values below come from `config.h` and `game/game.cpp`. If a value there changes, this guide must change too.

---

## 1. The big picture

| Folder / file | Job |
|---|---|
| `main.cpp` | Window, game loop, keyboard input, which screen is active |
| `renderer/` | Everything that talks to OpenGL: shader, VAO/VBO, shapes, images, the transformation stack |
| `game/game.cpp` | All rules: placement, coins, waves, movement, attacks, projectiles, collisions, win/lose |
| `environment/lanes/` | Mouse clicks, lane hover and lane selection |
| `players/`, `enemies/`, `shared/` | The six character models and the body parts they share |
| `environment/` | Sky, street, buildings and the laboratory (home base) |
| `ui/` | Top bar and cards (`ui.cpp`), menus and screen changes (`screens.cpp`), text images (`text_labels.cpp`) |
| `cutscene/` | The 16-second introduction |

Coordinates everywhere are **screen pixels**: x 0..1280 from left to right, y 0..720 from **bottom** to top.
The four lanes lie between y = 30 and y = 470 (110 px each, lane 0 at the bottom). The laboratory covers x = 0..170.

---

## 2. The game loop (`main.cpp`)

`main()` creates an OpenGL 3.3 core window (1280×720, not resizable), loads GLAD, calls `initRenderer()` and
`initTextLabels()`, then `resetGame()`. The app starts on the main menu. Every frame then does four things:

1. **Measure time.** `deltaTime` = seconds since the last frame, capped at 0.05 s so that a pause (for example
   dragging the window) does not make everything jump forward. All movement is `speed * deltaTime`, so the game
   runs at the same speed whatever the frame rate.
2. **Read input** – `readInput()` builds a `FrameInput`:
   - The mouse y is flipped (`SCREEN_HEIGHT - cursorY`) because GLFW measures from the top.
   - Clicks and key presses are stored by GLFW callbacks and *taken* once (`takeLeftClick()`, `takeKey()`), so
     each press is handled exactly once. All keys are taken every frame, even on screens that ignore them, so a
     press never acts later on another screen.
3. **Update** – `updateScreen()` does something different on each screen:
   - **Cutscene:** advance it; Enter/Space skips it. When it ends, `finishCutscene()` resets the game and starts
     playing.
   - **Playing, Esc pressed:** pause. Nothing else is handled in that frame.
   - **Playing:**
     1. Find the hovered lane and card.
     2. Handle a click: a click on a card is checked first (`clickCard`), otherwise a click on a lane (`clickLane`).
     3. Collect coins under the mouse (`collectCoinsAt`).
     4. Apply W/Up (jump) and S/Down (duck).
     5. Call `updateGame(deltaTime)`.
   - **Any menu:** only the buttons and Esc react.
4. **Draw** – `drawScreen()` draws back to front: environment → laboratory → lane highlights → `drawGame()` → top
   bar → hint line → menus. Then `glfwSwapBuffers` shows the frame and `glfwPollEvents` collects new input.

**Screens** (`ui/screens.cpp`): main menu → (Start) cutscene → playing ⇄ paused (Esc). When the game is won or lost
the win/lose menu is drawn on top of the frozen game. Restart and Main Menu both call `resetGame()`.

---

## 3. Rendering (`renderer/renderer.cpp`)

**One shader program for everything.**
- **Vertex shader:** `gl_Position = uProjection * uModel * position`.
  - `uProjection` is `glm::ortho(0, 1280, 0, 720)`, so we can work in pixels.
  - `uModel` is the current transformation.
- **Fragment shader:** outputs `uColor`, optionally multiplied by a texture (for text images). It then applies two
  effects:
  - `uDamageTint` mixes the color toward red;
  - `uGrayscale` mixes it toward its gray brightness.

**Shapes.** Each shape function (`drawRectangle`, `drawTriangle`, `drawEllipse`, `drawArc`, …) works out its
corner points and calls `drawVertices()`. That function:
1. sends the color and the current model matrix to the shader;
2. uploads the points into the single shared VAO/VBO (`glBufferData`, `GL_DYNAMIC_DRAW`);
3. draws them with `glDrawArrays`.

Circles and ellipses are triangle fans. Images use a second VAO/VBO with texture coordinates (`drawImage`).

**Transformations (like Labs 3 and 4).** The renderer keeps a `currentModel` matrix and a stack:
- `pushMatrix()` saves the current matrix and `popMatrix()` restores it.
- `translate`, `rotate` and `scale` multiply onto it.
- Because glm multiplies new matrices on the right, the call written **last** acts on the shape **first**.

Characters use this all the time, for example in `drawEnemyUnit()`:

```cpp
pushMatrix();
translate(e.x, ground);    // move the model to its place in the lane
scale(-1, 1);              // enemies face left (mirror)
drawSpiderMan(...);        // the model is drawn around (0, 0)
popMatrix();
```

**Blending** is on (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`), so colors with a < 1 are see-through: shadows,
highlights, fading messages.

**Damage tint and grayscale** are shader switches. One call affects everything drawn after it:
- `setDamageTint()` turns a whole character (made of many shapes) red without changing its drawing code.
  `damageTintFor()` in `game.cpp` computes the amount: `(1 - health/maxHealth) * 0.6`, plus `0.35` during the
  0.25 s flash after a hit.
- `setGrayscale(1)` makes a locked card gray (`ui.cpp`).

Both are reset after use and at the start of every frame (`beginFrame`).

**Text** (`ui/text_labels.cpp`). OpenGL cannot draw text, so every label is a PNG made from `labels.tsv` by
`make_labels.ps1`, in white. `drawImage` multiplies the image by a color, so the same white image can be drawn in
any color.
- Numbers that change (coins, health) are built from per-digit images.
- `TextLine` joins labels and numbers in a row and can draw them aligned left, centered or right.

**Draw order inside `drawGame()`:**
1. Lanes from back (top) to front (bottom), so closer characters overlap farther ones. Each lane draws its
   players, enemies and projectiles.
2. Hit rings and coins.
3. Health bars last, so nothing hides them. `findFreeBarHeight` stacks bars that would overlap.
4. Floating "+10" texts, the message strip and the wave banners.

---

## 4. Characters (`players/`, `enemies/`, `shared/`)

Every model is drawn in its **own coordinates**: (0, 0) is between the feet, the character faces right, and it is
about 76 px tall. The game places it with `translate`, mirrors enemies with `scale(-1, 1)` and squashes a ducking
player with `scale(1.1, 0.55)`.

Each model has two steps:
1. a **pose function** (for example `anabManPose`) turns `walkTime`, `moving` and `attack` into joint angles
   (a `CharacterPose`);
2. the **draw function** draws the body parts rotated by those angles at the joints (`HIP_Y`, `SHOULDER_Y`, …).

Legs, the leg swing and the melee swing angle are shared in `shared/character_parts`.

| Character | Role | Health | Attack |
|---|---|---|---|
| Anab Man | Melee player, 10 coins | 5 | swing every 0.7 s, 1 damage, reach 12 px |
| Beram | Ranged player, 20 coins | 5 | energy shot every 1.0 s, 1 damage |
| Tayseer | Defense player, 30 coins | 7 | none; never walks; blocks enemies and all shots |
| Spider-Man | Easy enemy | 5 | web every 3 s, comes from above (duck under it) |
| Sareeh El-Feel | Medium enemy | 5 | ballot box every 3 s, along the ground (jump over it) |
| Abla Thalma | Hard enemy | 7 | slipper swing every 3 s, 2 damage, close range |

Body widths used for every collision: players ±16 px, Tayseer ±24 px, enemies ±16 px, Abla Thalma ±20 px
(`playerHalfWidth`, `enemyHalfWidth`).

---

## 5. Placement and economy

**Values** (`config.h`):

| Value | Setting |
|---|---|
| Starting coins | 30 |
| Prices | Anab Man 10, Beram 20, Tayseer 30 |
| Unlock thresholds | 0 / 20 / 30 |
| Reward per defeated enemy | 10 coins |

**Unlock rule** (`isUnlocked`):
- Before the first Anab Man of a game is placed, only Anab Man is available (`firstMeleePlaced`).
- After that, a card is unlocked **exactly while** the balance is at least its threshold. It locks again when a
  purchase takes the balance below it.
- Every coin goes through `addCoins()`, which shows "… unlocked!" when the balance climbs past a threshold.

**Placing a character – lane first, then card:**

1. The player clicks a lane. `clickLane()` makes it the *placement lane* (see section 6).
2. The player clicks a card. `clickCard(type)` runs:
   - `checkPurchase()` checks, in order: game over → locked → not enough coins. A refusal shows a message
     ("Place an Anab Man first", "Locked - unlocks at N coins", "Not enough coins - you need N") and changes
     nothing.
   - No placement lane → "Select a lane first".
   - Otherwise `tryPlacePlayer(type, placementLane, placementX)`.
3. `tryPlacePlayer()`:
   - Anab Man and Beram appear at the left end of the lane (x = 170 + half width + 4).
   - Tayseer stands where the lane was clicked. `defenseSpot()` keeps him inside the lane and steps him off any
     enemy or other Tayseer (`stepPastBodies`). If no spot is free, the result is `PLACE_NO_ROOM` and nothing
     happens.
   - Coins are subtracted **only after** the character exists, so a refused placement never costs anything.
4. After a successful placement `placementLane` becomes -1, so the next character needs a lane click again.

There is no limit on how many characters can be on the field, and no waiting time between placements.

**Coins on the field:**
- **Drop:** when an enemy is removed (`removeDefeated`), a coin worth 10 appears where it fell. It is kept left of
  the right-edge entry zone, so a waiting player can still reach it. It drops in the same step that removes the
  enemy, so it can only drop once.
- **Collect** with either method; both call `collectCoin()`, which credits the coin and deletes it in one step, so
  it can never count twice:
  - *mouse hover:* `collectCoinsAt()`, within 22 px of the cursor, every frame while playing;
  - *player touch:* `collectCoinsTouchedByPlayers()`, when a player's body overlaps the coin (a jumping player can
    pass over it).
- **End of a wave:** `collectAllCoins()` credits any coins still lying on the field.

---

## 6. Lane selection (`environment/lanes/lanes.cpp`)

There are two separate pieces of state:

| State | Meaning |
|---|---|
| `laneSelected[4]` | Lanes highlighted in gold. W/S make every non-defense player in **these** lanes jump/duck. Several can be selected. |
| `placementLane` / `placementX` | The one lane (and the x position) where the next card click places a character. White inner border. -1 = none. |

`clickLane(lane, x)`:
- **The current placement lane** → it is deselected and there is no placement lane.
- **Any other lane** → it becomes selected (if it was not already) and becomes the placement lane, in one click.
  Other selected lanes stay selected.

`getLaneAt(x, y)` converts the mouse position to a lane number: `(y - 30) / 110`, or -1 outside the lanes and over
the laboratory. `clearLaneSelection()` runs on a new game and when a wave is cleared.

**Jump and duck** (`jumpSelectedLanes`, `duckSelectedLanes`, `game.cpp`):
- A jump lasts 0.6 s and follows `sin(π · progress) · 60` px (`jumpOffset`). Starting a jump cancels a duck.
- A duck lasts 0.6 s and lowers the body from 75 to 38 px (`playerBodyHeight`). It is ignored while jumping.
- Tayseer never jumps or ducks.

---

## 7. One frame of gameplay (`updateGame`)

```text
updateGame(dt)
  messageTimer -= dt              (messages still fade after the game ends)
  if won or lost: return          (freeze)
  updateWaves(dt)                 -> may set gameWon
  updatePlayers(dt)               walk + attack
  updateEnemies(dt)               walk + attack, may set gameLost
  if lost: removeDefeated(); return
  updateProjectiles(dt)           move + hit
  collectCoinsTouchedByPlayers()
  cleanup                         removeDefeated, dead projectiles, effects, floating texts
```

Units hit in this frame are only *removed* in the cleanup at the end. That is why the loops skip anything with
`health <= 0`: a unit killed earlier in the frame must not move, attack or be hit again.

### Waves (`updateWaves`, `startWave`, `spawnEnemy`)

- **Contents:** `WAVE_TABLE` gives easy/medium/hard enemies per wave: {3,0,0}, {3,2,0}, {4,3,3}.
  `startWave()` shuffles them (Fisher–Yates), so the order is random.
- **Spawning:** one enemy every 6.5 s (`SPAWN_INTERVAL`) in a random lane, at the right edge. Lanes where an enemy
  is still near the edge (x > 1160) are skipped. If every lane is crowded, the spawn is retried next frame.
- **Wave cleared** (queue empty and no enemies left):
  - `clearFieldForNextWave()` credits the remaining coins, removes all players (they "return to the lab" with a
    small effect), removes projectiles and clears the lane selection. Coins are kept.
  - Then there is a 5 s break with the "cleared" banner. The player can already place characters during the break.
- **Win:** clearing wave 3 sets `gameWon`.

### Player movement and attacks (`updatePlayers`)

- `closestEnemyAhead()` finds the nearest living enemy in front in the same lane.
  - The gap between the two bodies decides everything.
  - `touching` (gap ≤ 2 px) stops the player.
- `walkPlayer()`: Anab Man and Beram walk right (`PLAYER_SPEED`) until touching an enemy or reaching the right
  edge. They stop 42 px early, so a new enemy can still appear in front of them.
- `playerAttack()`: attacks repeat on a timer even with no enemy present.
  - **Anab Man:** the damage lands in the middle of the 0.6 s swing, once per swing (`damageDone`), and only if the
    enemy is within 12 px.
  - **Beram:** fires a shot from gun height (44 px, or 25 px while ducking, plus the jump height).

### Enemy movement and attacks (`updateEnemies`)

- **Target:** `frontPlayerInReach()` returns the front-most player within 12 px. A Tayseer in front is reached
  first, so he protects the players behind him. `touching` stops the enemy.
- **Queueing:** `roomToEnemyAhead()` keeps 6 px between enemies in the same lane, so they queue instead of walking
  through each other. If two enemies have the same x, the one earlier in the list counts as ahead.
- **Movement:** the step is `min(speed · dt, room)`, never negative. Speeds are 51 / 64 / 77 px/s for
  easy / medium / hard.
- **Lose:** if an enemy's left edge reaches x = 170 (the lab), `gameLost` is set immediately.
- `enemyAttack()`: the first attack comes 0.5 s after spawning, then one every 3 s.
  - **Spider-Man:** a web starting at height 95 that sinks to 50 (head height); duck under it.
  - **Sareeh:** a ballot box at height 8 (the ground); jump over it.
  - Both shoot whether or not a player is there.
  - **Abla Thalma:** 2 damage in the middle of her swing, to the target found above.

---

## 8. Collision and cleanup

**Body collision** (walking, melee) is a 1-D test along the lane:
- Each body is `x ± halfWidth`. The gap between two bodies is `(right body's left edge) - (left body's right edge)`.
- Gap ≤ 2 px means touching (stop walking); gap ≤ 12 px means a melee hit lands.
- Only units in the **same lane** are compared.
- Enemies in a lane queue 6 px apart, so they never overlap. Walking players may overlap each other (team decision).

**Projectiles** (`updateProjectiles`):
- `moveProjectile()` moves the shot and returns the stretch it covered this frame (`pathLeft..pathRight`). The hit
  test checks that whole stretch, so a fast shot cannot skip over a body between two frames.
- **Player shots** (`firstEnemyOnPath`) hit the first living enemy on the path. They never test players, so friendly
  fire is impossible. They are removed on a hit or past x = 1300.
- **Enemy shots** (`frontPlayerInTheWay`) hit the front-most player whose body overlaps the shot's height (± 6 px).
  - This is what makes jumping and ducking work: a jump lifts the feet above the box; a duck lowers the head below
    the web.
  - Tayseer's shield blocks shots at any height.
  - A shot that reaches the lab wall splats there.
- A hit calls `hurtPlayer`/`hurtEnemy`. These subtract health, start the red flash and add a hit ring. The shot is
  marked `alive = false`.

**Cleanup** at the end of the frame:
- `removeDefeated()` drops coins for dead enemies, then erases dead enemies and players (erase–`remove_if`).
- Dead projectiles are erased.
- Hit rings and floating texts count down and are erased when their time runs out; coins only age (used for their
  bobbing animation).
- On a loss, only `removeDefeated()` runs before the freeze, so no unit with 0 health stays on the loss screen.

---

## 9. Suggested study areas for four members

Each member should be able to explain the whole loop (sections 2 and 7), and go deeper in one area:

1. **Rendering, environment and cutscene:**
   - `renderer/` (shader, VAO/VBO, `drawVertices`, matrix stack, ortho projection, blending, tint/grayscale)
   - `environment/environment.cpp`
   - `cutscene/`
2. **Characters:**
   - the six models in `players/` and `enemies/`
   - `shared/character_parts` (pose → draw, joints, leg swing, melee swing)
   - the coin model, and how `drawPlayerUnit`/`drawEnemyUnit` place the models
3. **Game mechanics:**
   - `game/game.cpp`: waves, movement, attacks, projectiles, collision, cleanup, win/lose
   - `config.h` values
4. **UI, input and integration:**
   - `main.cpp` (game loop, input, screens)
   - `environment/lanes/` (lane selection)
   - `ui/ui.cpp` (cards: locked / need coins / ready)
   - `ui/screens.cpp` (menus)
   - `ui/text_labels.cpp`
   - placement and economy (section 5)

### Questions worth practising

- Why is the mouse y flipped, and why is `deltaTime` capped?
- What do `pushMatrix`/`popMatrix` protect, and why does `scale(-1, 1)` make an enemy face left?
- Why are coins taken only after a character is created?
- What is the difference between a *selected* lane and the *placement* lane?
- How can a player avoid a web and a ballot box, and where in the code is that decided?
- Why can a fast projectile not pass through an enemy between two frames?
- Why do the update loops skip units with `health <= 0` instead of erasing them immediately?
- What exactly makes the game end in a loss, and in a win?
