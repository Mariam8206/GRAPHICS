# Credits – The Last Anab Bottle Crisis (Team AAMM)

DMET 502 Computer Graphics, Project 1, Winter 2026.

## Libraries included in the project

| Library | Version | Files in this project | Author | License |
|---|---|---|---|---|
| GLFW (window, keyboard, mouse) | 3.5 | `External/include/GLFW/`, `lib/glfw3.lib` | Marcus Geelnard, Camilla Löwy | zlib/libpng license (full notice at the top of `glfw3.h`) |
| GLAD (OpenGL function loader) | generated with glad 0.1.36 | `src/glad.c`, `include/glad/glad.h` | generated code (glad by David Herberth, https://glad.dav1d.de) | generator output; see the glad project |
| Khronos platform header | – | `include/KHR/khrplatform.h` | The Khronos Group Inc. | MIT-style license (full notice at the top of the file) |
| stb_image (PNG loading) | 2.30 | `include/stb_image.h` | Sean Barrett | public domain or MIT, your choice (notice at the end of the file) |
| GLM (matrix math) | 1.0.3 | `include/glm/` – only the 182 headers the game uses, taken from the course's Lab 4 copy | G-Truc Creation | MIT or Happy Bunny license (`include/glm/copying.txt`) |

## Fonts and images

- The UI text images in `assets/ui/` were made by our script `assets/ui/make_labels.ps1`, using the **Dubai** font
  that ships with Windows. The font file itself is not part of the project.
- All characters, the laboratory, the street and the coins are drawn by our own code from OpenGL shapes
  (`characters.cpp`, `environment.cpp`). No image files are used for them.

## Course material and references

- Shader compilation, VAO/VBO setup and the GLM transformations and orthographic projection follow the
  DMET 502 lab examples (Lab 1, Lab 3, Lab 4).
- LearnOpenGL (https://learnopengl.com, Joey de Vries) was used as a reading reference; no code was copied from it.

## Characters and theme

- The theme is inspired by the Egyptian comedy TV series *El-Ragol El-Enab*.
- Spider-Man is a trademark of Marvel. He appears here as a parody character in a non-commercial student project.

## AI assistance

Parts of the code, the test programs and the documentation were written with the help of an AI coding assistant
(Claude, by Anthropic). The team chose the game design, story and characters, directed the work and reviewed
each milestone.
