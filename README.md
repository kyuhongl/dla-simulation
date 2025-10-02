# Diffusion Limited Aggregation (DLA) Simulation

A real-time interactive simulation of Diffusion Limited Aggregation using OpenFrameworks. This project visualizes the growth of fractal-like structures through the random walk and clustering of particles.

## Overview

Diffusion Limited Aggregation (DLA) is a process where particles perform random walks and stick together when they come into contact with existing clusters. This creates beautiful, organic-looking fractal structures that resemble natural phenomena like coral growth, lightning, or mineral deposits.

## Features

- **Real-time simulation** with thousands of walker particles
- **Interactive GUI** with live parameter adjustment
- **Performance optimization** with spatial hashing and frame budgeting
- **Visual customization** with multiple rendering modes
- **Export functionality** for saving generated structures
- **Deterministic mode** for reproducible results

## Algorithm

The simulation works as follows:

1. **Seed**: A single particle is placed at the center as the initial cluster
2. **Walkers**: Multiple particles spawn at random positions on a ring around the cluster
3. **Random Walk**: Each walker takes random steps in 2D space
4. **Sticking**: When a walker gets close enough to the cluster, it has a probability of sticking
5. **Growth**: Stuck particles become part of the cluster and new walkers spawn
6. **Boundary**: Walkers that move too far from the cluster are respawned

## Controls

### Keyboard Shortcuts

- `SPACE` - Pause/resume simulation
- `R` - Reset simulation
- `E` - Export current state as PNG
- `S` - Toggle deterministic mode
- `L` - Toggle line drawing
- `P` - Toggle point drawing
- `F` - Toggle trail fading
- `+`/`=` - Zoom in
- `-`/`_` - Zoom out
- `↑`/`↓` - Increase/decrease number of walkers

### Mouse Controls

- **Scroll** - Zoom in/out

## Parameters

The GUI panel provides real-time control over simulation parameters:

### Core Parameters
- **numWalkers** (32-8192): Number of active walker particles
- **stickRadius** (0.5-12.0): Distance threshold for sticking
- **stepSize** (0.25-8.0): Size of each random walk step
- **stickProb** (0.0-1.0): Probability of sticking when in range

### Boundary Parameters
- **spawnMargin** (4.0-200.0): Distance from cluster edge to spawn walkers
- **killMargin** (20.0-400.0): Distance from cluster edge to respawn walkers
- **maxStuck** (100-200000): Maximum number of stuck particles

### Rendering Parameters
- **drawLines**: Show connections between cluster nodes
- **drawPoints**: Show individual cluster nodes
- **fadeTrails**: Enable trail fading effect
- **autoPauseOnMax**: Automatically pause when max particles reached

### Performance Parameters
- **perfSafeMode**: Enable performance optimizations
- **frameBudgetMs** (0-16): CPU time budget per frame in milliseconds
- **drawMaxNodes** (2000-60000): Maximum nodes to draw before decimation

### Randomization
- **seed**: Random seed for deterministic mode
- **deterministic**: Use fixed seed for reproducible results

## Technical Details

### Performance Optimizations

The simulation includes several performance optimizations to handle large numbers of particles:

1. **Spatial Hashing**: Efficient neighbor queries using a 2D grid
2. **Frame Budgeting**: Distributes computation across multiple frames
3. **Draw Decimation**: Reduces rendering load for large clusters
4. **Incremental Updates**: Only rebuilds spatial hash when parameters change

### Data Structures

- **Cluster**: Manages the growing structure with parent-child relationships
- **SpatialHash**: Provides O(1) neighbor queries for collision detection
- **Particle**: Represents individual walker particles

## Requirements

- **OpenFrameworks** 0.12.1 or later
- **C++11** compatible compiler
- **OpenGL** 3.2+ support
- **ofxGui** addon (included with OpenFrameworks)

## Building

1. Ensure you have OpenFrameworks installed
2. Open the project in your preferred IDE:
   - **Xcode** (macOS): Open `emptyExample.xcodeproj`
   - **Visual Studio** (Windows): Open the `.sln` file
   - **Code::Blocks** (Linux): Open the `.cbp` file
3. Build and run the project

### Command Line Build (macOS/Linux)

```bash
make
make RunRelease
```

## Usage Tips

1. **Start with default parameters** to see the basic behavior
2. **Increase numWalkers** for faster growth but higher CPU usage
3. **Adjust stickRadius and stepSize** to control structure density
4. **Use deterministic mode** for reproducible results
5. **Enable perfSafeMode** for better performance with large simulations
6. **Export images** to capture interesting structures

## Examples

Try these parameter combinations for different effects:

### Dense Growth
- numWalkers: 2048
- stickRadius: 2.0
- stepSize: 1.0
- stickProb: 1.0

### Sparse Branching
- numWalkers: 512
- stickRadius: 5.0
- stepSize: 3.0
- stickProb: 0.3

### Fast Growth
- numWalkers: 4096
- stickRadius: 4.0
- stepSize: 2.0
- stickProb: 0.8

## License

This project is open source. Feel free to use, modify, and distribute according to your needs.

## Contributing

Contributions are welcome! Areas for improvement include:

- Additional visualization modes
- 3D version of the simulation
- GPU acceleration using compute shaders
- Export to various formats (SVG, OBJ, etc.)
- Parameter presets and save/load functionality

## References

- [Diffusion Limited Aggregation on Wikipedia](https://en.wikipedia.org/wiki/Diffusion-limited_aggregation)
- [OpenFrameworks Documentation](https://openframeworks.cc/documentation/)
- [Fractal Geometry of Nature by Benoit Mandelbrot](https://en.wikipedia.org/wiki/The_Fractal_Geometry_of_Nature)