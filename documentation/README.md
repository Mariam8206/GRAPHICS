# The Last Anab Bottle Crisis – Team AAMM

DMET 502 Computer Graphics, Project 1 (Winter 2026): a 2D side-view tower defense game in OpenGL 3.3.
Dr. Beram's laboratory holds the last bottle of the anab formula; Anab Man, Beram and Tayseer defend it
against Spider-Man, Sareeh El-Feel and Abla Thalma.

## Build and run

- Visual Studio: open `Template.sln`, choose **x64** and **Debug** or **Release**, press F5.
- Or double-click `build_and_run.bat` (Debug x64, Visual Studio 2026 Build Tools).
- The game needs the folder `ui/assets/labels/` next to the project files. It finds it from the project folder
  or from `x64/Debug` / `x64/Release`.

## Where to find things

Every folder holds one part of the game: its code, its headers and its own assets.

| Folder | What it contains |
|---|---|
| `main.cpp`, `config.h` | Window, main loop, input, screen switching; all game constants (prices, health, speeds, timings) |
| `cutscene/` | The 16-second animated introduction |
| `players/` | The three player models: `melee/anab_man`, `ranged/beram` (with his energy shot), `defense/tayseer`. `players.h` includes all three |
| `enemies/` | The three enemy models: `easy/spider_man` (with his web), `medium/sareeh_el_feel` (with his ballot box), `hard/abla_thalma`. `enemies.h` includes all three |
| `shared/` | Body parts used by BOTH players and enemies (legs, walking and swing animation, skin color) |
| `environment/` | Sky, Egyptian street, buildings, kiosk, election posters and Dr. Beram's laboratory; `lanes/` = the four lanes, lane hover and selection, mouse clicks |
| `ui/` | Top bar and cards (`ui`), menus (`screens`), the text system (`text_labels`) and its images in `assets/labels/` |
| `game/` | Game rules: placement and coins, waves, combat, projectiles, win/lose (`game.cpp`); `collectibles/` = the coin model |
| `renderer/` | OpenGL: shader, VAO/VBO, shapes, GLM transformations and orthographic projection, textures |
| `libraries/` | ALL third-party code, one folder per library: `GLFW/` (window and input), `GLAD/` (OpenGL loader, with KHR), `GLM/` (matrices), `stb_image/` (PNG loading). Licenses are inside the files (GLM: `copying.txt`); see `CREDITS.md` |
| `documentation/` | This file, `CREDITS.md`, `REQUIREMENTS.md` |

### Why all text images are in `ui/assets/labels/`

All on-screen text (menus, cards, messages, cutscene subtitles, signs, digits) is drawn from PNG images made by one
script (`make_labels.ps1`) from one list (`labels.tsv`) and loaded by one system (`ui/text_labels.cpp`). The digit
images are shared by every screen. Keeping them together means one generator, one loader and no duplicate files.

### Why `game.cpp` is one file

Players, enemies, projectiles and coins all act on the same lists in every frame (`updateGame` calls
`updateWaves`, `updatePlayers`, `updateEnemies`, `updateProjectiles`). Keeping them in one file avoids sharing those
lists between files through global variables. The character *drawings* are in `players/` and `enemies/`.
