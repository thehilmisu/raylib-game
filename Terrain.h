// Terrain.h
#ifndef TERRAIN_H
#define TERRAIN_H

#include "raylib.h"
#include <stddef.h>
#include <stdbool.h>

// Macros and constants
#define CHUNK_SIZE 64          // Number of tiles per chunk side
#define TILE_SCALE 1.0f        // Size of each tile
#define MAX_CHUNKS 16          // Maximum number of terrain chunks
#define NOISE_FREQUENCY 0.01f  // Noise frequency
#define NOISE_AMPLITUDE 20.0f  // Noise amplitude

// Structures

typedef struct {
    Vector3 position; // Position offset of the chunk
    Mesh mesh;
    Model model;
} TerrainChunk;

typedef struct {
    TerrainChunk chunks[MAX_CHUNKS];
    int chunkCount;
} TerrainManager;

// Function prototypes

// Initialize the terrain manager
void InitTerrain(TerrainManager *terrain);

// Update terrain based on the plane's position
void UpdateTerrain(TerrainManager *terrain, Vector3 planePosition, Vector3 planeForward);

// Draw all terrain chunks
void DrawTerrain(TerrainManager *terrain);

// Unload all terrain resources
void UnloadTerrain(TerrainManager *terrain);

// Get noise value at a specific point (for collision detection)
float GetNoiseValue(float x, float z);

#endif // TERRAIN_H
