# DLA Simulation with Organic Shaders

Real-time Diffusion Limited Aggregation simulation with custom lighting and visual effects.

Inspired by [Sage Jenson's 36 Points](https://www.sagejenson.com/36points/).

## Features

- Real-time particle simulation (up to 8000+ particles)
- Phong lighting with glass-like particle rendering
- Neural pulse waves propagating from center
- Animated noise-based background
- GIF recording
- Spatial hashing for performance optimization
- Deterministic mode for reproducible results

## Algorithm

1. Seed particle placed at center
2. Walker particles spawn in random distribution around cluster
3. Walkers perform random walk
4. When close to cluster, walkers stick with configurable probability
5. Stuck particles become part of cluster
6. Walkers beyond kill radius respawn

## Controls

**Keyboard:**
- `SPACE` - Pause/play
- `R` - Reset
- `G` - Record 3-second GIF
- `E` - Export PNG
- `H` - Toggle shaders
- `L` - Toggle lines
- `P` - Toggle points
- `W` - Toggle walkers
- `F` - Toggle fade trails
- `+`/`-` - Zoom
- `↑`/`↓` - Adjust walker count

**Mouse:**
- Scroll - Zoom

## Parameters

**Core:**
- numWalkers - Active walker particle count
- stickRadius - Distance threshold for sticking
- stepSize - Random walk step size
- stickProb - Sticking probability (0.0-1.0)

**Boundaries:**
- spawnMargin - Walker spawn distance from cluster
- killMargin - Walker respawn distance
- maxStuck - Maximum particle count

**Performance:**
- perfSafeMode - Enable optimizations
- frameBudgetMs - CPU time budget per frame
- drawMaxNodes - Node decimation threshold

## Technical Details

**Rendering:**
- Phong lighting with animated light source
- Glass-like particle rendering with specular highlights
- Organic curved lines with variable thickness
- Neural pulse wave propagation
- Simplex noise background shader
- Beige/gold color palette on dark navy background

**Performance:**
- Spatial hashing for neighbor queries
- Frame budgeting for distributed computation
- Batched mesh rendering
- Automatic decimation for large clusters
- GLSL 330 vertex/fragment shaders

## Building

**Requirements:**
- OpenFrameworks 0.12.1+
- C++11 compiler
- OpenGL 3.2+

**macOS:**
```bash
make && make RunRelease
```
Or open `emptyExample.xcodeproj` in Xcode.

## GIF Export

Press `G` to record 3 seconds (90 frames). Frames save to `bin/data/gif_frames_TIMESTAMP/`.

Convert to GIF:
```bash
cd bin/data
ffmpeg -i gif_frames_TIMESTAMP/frame_%04d.png -vf "fps=20,scale=400:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=64[p];[s1][p]paletteuse=dither=bayer" output.gif
```

## References

- [36 Points by Sage Jenson](https://www.sagejenson.com/36points/)
- [interactive-physarum by Bleuje](https://github.com/Bleuje/interactive-physarum)
- [OpenFrameworks](https://openframeworks.cc/)

## License

Open source.