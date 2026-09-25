# OpenGL Digital Postcard — "Sunset by the Sea"

## Scene description

The postcard is based on a photo of a sunset over a rocky shore, redrawn as a
wide 1000×700 illustration. The sun is setting right on the sea horizon. Light
breaks through a gap in the clouds as a bright yellow band. The sky goes from warm
yellow at the horizon through peach and pink to grey-lilac. It is covered with
soft pink overcast and a long dark cloud band, and thin cloud streaks cross in
front of the sun. The sea is teal and turquoise. Rows of swells roll in, and
their crests break into white foam. A golden path of light shimmers on the water.
A small sailboat crosses the horizon and a flock of gulls flies past. Foam washes
around a row of boulders in the foreground. Press **Space** to fade the scene to
night over 2 seconds: the sun turns into a cratered moon, the reflection turns
silver, stars appear through the clouds and every colour cools down.

| Sunset | Night |
|---|---|
| ![Sunset](screenshot_sunset.png) | ![Night](screenshot_night.png) |

## Controls

| Key | Action |
|---|---|
| **Space** | Switch between sunset and night (smooth 2 s transition) |
| **← → ↑ ↓** | Move the sun / moon (the reflection fades out as it sinks below the horizon) |
| **W / S** | Increase / decrease global brightness |
| **P** | Save the current frame to `screenshot.tga` |
| **Esc** | Close the program |

## How each requirement is met

| # | Requirement | Implementation in `main.cpp` |
|---|---|---|
| 1 | One window, one finished scene | `main()` creates one 1000×700 GLFW window with an OpenGL 3.3 Core context. `render()` draws the whole postcard every frame. |
| 2 | ≥ 5 different visible objects | Sky, sun/moon, clouds (12 clusters), sea, sun path, ripples, swells, sailboat, foam, rocks, gulls, stars (each has its own `build*()` function and `Mesh`). |
| 3 | All three primitives | `GL_POINTS`: `stars`, `bigStars`. `GL_LINES`: `sunPath`, `wavesA/B` (ripples), `foam` (lace), `rockCracks`, `boatRig` (mast + flag), `birdsUp/Down`. `GL_TRIANGLES`: everything else. |
| 4 | Complex objects from simple primitives | `Mesh::circle()` builds the sun, glow and craters as a triangle fan. `Mesh::softEllipse()` builds soft-edged ellipses (a solid core plus a fading ring of triangles); `makeCloud()` stacks 7–12 of them into each cloud. `addRock()` builds each boulder as a triangle fan with a bumpy radius, plus a highlight and crack lines. `buildSwells()` builds each wave row from strips of quads (shadow, face, spilling foam, ragged foam cap). `buildBirds()` builds each gull from 4 line segments. |
| 5 | Vertices in VAO/VBO | `struct Mesh` has `point`, `line`, `triangle`, `quad`, `circle`, `ellipse` and `upload()`. `upload()` creates one VAO and one VBO per mesh with 3 attributes (position, sunset colour, night colour). |
| 6 | ≥ 4 RGB colours | `namespace pal` and the `build*()` functions use yellow, peach, pink, lilac, teal, turquoise, white, beige-grey, red and more. |
| 7 | Colour interpolation | The sky (`buildSky`) and sea (`buildSea`) use bands with different top/bottom vertex colours. The sun disk has a white centre and an orange edge (`circle` radial gradient). Each wave face goes from dark teal to light turquoise (`buildSwells`). Each rock goes from a lit top to a dark bottom (`addRock`). The sails go from a light top to a pink base (`buildBoat`). |
| 8 | Vertex & fragment shaders | `VERTEX_SHADER` converts scene pixels to NDC and mixes colours. `FRAGMENT_SHADER` outputs the colour and makes star points round and soft. Both are compiled in `createProgram()`. |
| 9 | Uniforms | **`uNight`**: `mix(aDayColor, aNightColor, uNight)`, because every vertex has two colour attributes. **`uOffset`**: moves the sun, boat, clouds, gulls and foam, and each sun-path row and swell row separately. **`uBrightness`**: multiplies RGB (W/S keys). **`uAlpha`**: fades the stars in at night (with twinkle), fades the glow and the reflection as the sun sets. Extra: `uPointSize` and `uRoundPoints` for the stars. |
| 10 | Controls | `keyCallback()` handles Space (toggles `nightTarget`), Esc and P. `update()` handles the held keys (arrows, W/S) and moves `g.night` smoothly toward the target. The reflection alpha is `smooth01((sunY − (HORIZON − R)) / 1.5R)`, so it goes to 0 once the sun is below the horizon. The sea is drawn after the sun, so the sea hides it. |

**Animation.** Everything is animated in `render()` from the time `t`. The boat moves with `wrapX` and bobs with a sine. Every cloud cluster drifts at its own speed and wraps around the screen. The gulls fly across and switch between the `birdsUp` and `birdsDown` meshes to flap. Each swell row rolls sideways and rises and falls with its own phase. Each sun-path row gets its own sine jitter, so the reflection shimmers. The ripples sway, the foam washes around the rocks, and the stars twinkle.

**Other techniques.** Core profile only guarantees 1-pixel lines, so `Mesh::line()` draws thick lines as several parallel 1-px lines. Alpha blending (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`) and 4× MSAA are enabled. Soft clouds and foam come from ellipses whose edge vertices have alpha 0. The sun glow is drawn a second time, faintly, after the clouds, so they look lit from behind. The moon's craters have alpha 0 in the sunset colour and are visible only at night.

## Build and run

Requirements: a C++17 compiler, CMake ≥ 3.16 and GLFW 3.3+. GLAD is already
generated in `glad/` (`python -m glad --profile core --api gl=3.3 --generator c --out-path glad`).

```bash
# macOS: brew install glfw cmake    Ubuntu: sudo apt install libglfw3-dev cmake
cmake -S . -B build
cmake --build build
./build/postcard                 # interactive
./build/postcard --screenshots   # writes screenshot_sunset.tga / screenshot_night.tga
```
