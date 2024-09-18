#ifndef TERRAIN_H
#define TERRAIN_H

#include "raylib.h"

#define CHUNK_SIZE 64          // Size of each terrain chunk
#define TILE_SCALE 3.0f        // Scaling for each tile
#define MAX_CHUNKS 100         // Maximum number of chunks loaded at once
#define NOISE_AMPLITUDE 10.0f  // Amplitude of noise for terrain generation
#define NOISE_FREQUENCY 0.01f  // Frequency of noise for terrain generation

// Terrain chunk structure
typedef struct TerrainChunk {
    Vector3 position;  // Position of the chunk in the world
    Mesh mesh;         // Terrain mesh for the chunk
    Model model;       // Model generated from the mesh
} TerrainChunk;

// Terrain manager structure
typedef struct TerrainManager {
    TerrainChunk chunks[MAX_CHUNKS];  // Array of terrain chunks
    int chunkCount;                   // Number of currently loaded chunks
} TerrainManager;

// Function declarations
void InitTerrain(TerrainManager *terrain);                                   // Initialize the terrain system
void UpdateTerrain(TerrainManager *terrain, Vector3 planePosition, Vector3 planeForward, Camera camera);  // Update terrain chunks based on the camera/plane position
void DrawTerrain(TerrainManager *terrain);                                   // Draw the loaded terrain chunks
void UnloadTerrain(TerrainManager *terrain);                                 // Unload all loaded terrain chunks

#endif // TERRAIN_H
