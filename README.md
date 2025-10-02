# DLA Simulation with Organic Shaders ✨

A real-time generative art piece using Diffusion Limited Aggregation. Watch as particles randomly walk around and stick together, creating beautiful branching structures that look like coral, lightning, or neural networks.

Heavily inspired by [Sage Jenson's incredible 36 Points](https://www.sagejenson.com/36points/) - check out their work, it's amazing!

## What is this?

Particles spawn around a center seed and randomly wander until they bump into the growing structure and stick. It creates these gorgeous organic fractal patterns. Add some fancy lighting shaders and an animated wispy background, and you've got yourself some eye candy.

## Cool Features

- Real-time particle simulation (up to 8000+ particles)
- Beautiful beige/gold color palette with glass-like lighting
- Neural pulse waves that propagate from center to edges
- Wispy animated dark navy background
- Interactive controls - tweak everything in real-time
- GIF recording (press `G` for 3 seconds of footage)
- Spatial hashing for buttery smooth performance
- Deterministic mode if you want the same pattern twice

## How it Works

1. Start with one particle in the center
2. Spawn a bunch of "walker" particles around it
3. Walkers randomly stumble around
4. When they get close to the cluster, they might stick (it's probability-based)
5. Stuck particles become part of the structure
6. Particles that wander too far get respawned
7. Repeat forever (or until you hit the particle limit)

## Controls

### Keyboard

- `SPACE` - Pause/play
- `R` - Reset (start over)
- `G` - Record 3-second GIF
- `E` - Export PNG screenshot
- `H` - Toggle shader effects
- `L` - Toggle lines
- `P` - Toggle points (stuck particles)
- `W` - Toggle walkers (moving particles)
- `F` - Toggle fade trails
- `+`/`-` - Zoom
- `↑`/`↓` - More/fewer walkers

### Mouse

- **Scroll** - Zoom

## Parameters (GUI Panel)

Play around with these to get different vibes:

### Main Stuff
- **numWalkers**: How many particles are wandering around
- **stickRadius**: How close walkers need to be to stick
- **stepSize**: How big each random step is
- **stickProb**: Chance of actually sticking (0.0 = never, 1.0 = always)

### Spawn Settings
- **spawnMargin**: How far from the cluster walkers spawn
- **killMargin**: How far they can wander before respawning
- **maxStuck**: Max particles before simulation stops

### Visuals
- **drawLines**: Show the connections
- **drawPoints**: Show the stuck particles
- **drawWalkers**: Show the moving particles
- **fadeTrails**: Subtle fading effect

### Performance
- **perfSafeMode**: Turn this on if it's laggy
- **frameBudgetMs**: CPU time per frame (lower = faster but less smooth)
- **drawMaxNodes**: Start skipping particles after this many

## Visual Features

### Shaders
- **Phong lighting** with animated rotating light source
- **Glass-like beads** - each particle has specular highlights and depth
- **Organic flowing lines** with subtle waves and variable thickness
- **Neural pulse waves** - emanate from center every 2 seconds
- **Wispy background** - animated noise-based gradients in dark navy

### Colors
- Muted beige/gold particles (inspired by that physarum aesthetic)
- Dark navy blue background with atmospheric depth
- Subtle pulse highlights that travel outward

## Tech Stuff

**Performance tricks:**
- Spatial hashing for fast neighbor lookups
- Frame budgeting spreads work across frames
- Batched mesh rendering (not 20k individual draw calls!)
- Auto-decimation when things get huge

**Shader pipeline:**
- Vertex/fragment shaders for the DLA
- Separate background shader with simplex noise
- All GLSL 330

## Setup

**Requirements:**
- OpenFrameworks 0.12.1+
- C++11 compiler
- OpenGL 3.2+

**Building:**

macOS:
```bash
make && make RunRelease
```

Or just open `emptyExample.xcodeproj` in Xcode and hit run.

## Making GIFs

1. Press `G` to start recording
2. Wait 3 seconds (you'll see a red dot)
3. Frames save to `bin/gif_frames_TIMESTAMP/`
4. Convert with ffmpeg:

```bash
cd bin
ffmpeg -i gif_frames_TIMESTAMP/frame_%04d.png -vf "fps=30,scale=800:-1:flags=lanczos" output.gif
```

## Tips

- Start with defaults to see how it works
- More walkers = faster growth (but more CPU)
- Lower stickProb = sparser, more branchy structures
- Hide walkers (`W` key) for cleaner screenshots
- Turn off shaders (`H`) if it's too slow

## Inspiration & References

This project was heavily inspired by:
- **[36 Points by Sage Jenson](https://www.sagejenson.com/36points/)** - Seriously, go check this out. It's incredible.
- The [interactive-physarum](https://github.com/Bleuje/interactive-physarum) project by Bleuje
- Classic DLA simulations and fractal growth patterns

## License

Open source, do whatever you want with it. If you make something cool, let me know!

---

Built with [OpenFrameworks](https://openframeworks.cc/) 🎨