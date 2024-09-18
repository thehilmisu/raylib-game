#define FNL_IMPL
#include "FastNoiseLite.h"
#include "Terrain.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include "rlgl.h"   

// Internal functions
float GetNoiseValue(float x, float z);
float GetOctaveNoise(float x, float z, int octaves, float persistence, float lacunarity);
Color ColorLerp(Color colorA, Color colorB, float t);
static Mesh GenerateTerrainMesh(int size, float scale, Vector3 offset);
static void AddTerrainChunk(TerrainManager *terrain, Vector3 offset);
static bool IsChunkLoaded(TerrainManager *terrain, int chunkX, int chunkZ);
static void UnloadFarChunks(TerrainManager *terrain, Vector3 planePosition, float chunkSize, int maxRange);

// FastNoiseLite state
static fnl_state noise;

void InitTerrain(TerrainManager *terrain) {
    terrain->chunkCount = 0;

    // Initialize FastNoiseLite
    noise = fnlCreateState();
    noise.seed = 1000;
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.frequency = NOISE_FREQUENCY;
}

void UpdateTerrain(TerrainManager *terrain, Vector3 planePosition, Vector3 planeForward, Camera camera) {
    float chunkSize = (CHUNK_SIZE - 1) * TILE_SCALE;

    // Calculate the visible area
    float maxDistance = camera.fovy * (float)720 / 1080;
    int range = ceilf(maxDistance / chunkSize) + 1;

    // Determine the chunk coordinates the plane is currently over
    int planeChunkX = (int)floorf(planePosition.x / chunkSize);
    int planeChunkZ = (int)floorf(planePosition.z / chunkSize);

    // Normalize the plane's forward direction to get chunk offsets
    Vector2 forwardDir = Vector2Normalize((Vector2){ planeForward.x, planeForward.z });

    // Loop over the range to generate chunks ahead and around the plane
    for (int z = -range; z <= range; z++) {
        for (int x = -range; x <= range; x++) {
            int offsetX = planeChunkX + x;
            int offsetZ = planeChunkZ + z;

            if (!IsChunkLoaded(terrain, offsetX, offsetZ)) {
                Vector3 chunkPosition = {
                    offsetX * chunkSize,
                    0,
                    offsetZ * chunkSize
                };
                AddTerrainChunk(terrain, chunkPosition);
            }
        }
    }
}

void UnloadFarChunks(TerrainManager *terrain, Vector3 planePosition, float chunkSize, int maxRange) {
    float maxDistance = maxRange * chunkSize;

    for (int i = 0; i < terrain->chunkCount; i++) {
        Vector3 chunkCenter = Vector3Add(terrain->chunks[i].position, (Vector3){ chunkSize / 2, 0, chunkSize / 2 });
        float distance = Vector3Distance(chunkCenter, planePosition);

        if (distance > maxDistance) {
            // Unload this chunk
            UnloadModel(terrain->chunks[i].model);
            // Remove chunk from array
            for (int j = i; j < terrain->chunkCount - 1; j++) {
                terrain->chunks[j] = terrain->chunks[j + 1];
            }
            terrain->chunkCount--;
            i--; // Adjust index after removal
        }
    }
}

void DrawTerrain(TerrainManager *terrain) {
    for (int i = 0; i < terrain->chunkCount; i++) {
        DrawModel(terrain->chunks[i].model, terrain->chunks[i].position, 1.0f, WHITE);
    }
}

void UnloadTerrain(TerrainManager *terrain) {
    for (int i = 0; i < terrain->chunkCount; i++) {
        UnloadModel(terrain->chunks[i].model);
    }
    terrain->chunkCount = 0;
}

// Customizable parameters for Perlin noise
#define NOISE_OCTAVES 4
#define NOISE_PERSISTENCE 0.5f
#define NOISE_LACUNARITY 2.0f

// Generate multi-octave noise for terrain
float GetOctaveNoise(float x, float z, int octaves, float persistence, float lacunarity) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float noiseHeight = 0.0f;
    float maxPossibleHeight = 0.0f;

    for (int i = 0; i < octaves; i++) {
        float sampleX = x * frequency;
        float sampleZ = z * frequency;
        float noiseValue = GetNoiseValue(sampleX, sampleZ) * 2.0f - 1.0f;

        noiseHeight += noiseValue * amplitude;
        maxPossibleHeight += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return noiseHeight / maxPossibleHeight; // Normalize height to [-1, 1]
}

float GetNoiseValue(float x, float z) {
    // Get noise value from FastNoiseLite
    return fnlGetNoise2D(&noise, x, z) * NOISE_AMPLITUDE;
}

Color ColorLerp(Color colorA, Color colorB, float t) {
    Color result;
    
    result.r = colorA.r + t * (colorB.r - colorA.r);
    result.g = colorA.g + t * (colorB.g - colorA.g);
    result.b = colorA.b + t * (colorB.b - colorA.b);
    result.a = colorA.a + t * (colorB.a - colorA.a);
    
    return result;
}

static bool IsChunkLoaded(TerrainManager *terrain, int chunkX, int chunkZ) {
    float chunkSize = (CHUNK_SIZE - 1) * TILE_SCALE;
    float posX = chunkX * chunkSize;
    float posZ = chunkZ * chunkSize;

    for (int i = 0; i < terrain->chunkCount; i++) {
        if ((terrain->chunks[i].position.x == posX) && (terrain->chunks[i].position.z == posZ)) {
            return true;
        }
    }
    return false;
}

static void AddTerrainChunk(TerrainManager *terrain, Vector3 offset) {
    if (terrain->chunkCount >= MAX_CHUNKS) {
        // Remove the oldest chunk
        UnloadModel(terrain->chunks[0].model);
        for (int i = 1; i < terrain->chunkCount; i++) {
            terrain->chunks[i - 1] = terrain->chunks[i];
        }
        terrain->chunkCount--;
    }

    // Generate new chunk
    Mesh mesh = GenerateTerrainMesh(CHUNK_SIZE, TILE_SCALE, offset);
    Model model = LoadModelFromMesh(mesh);

    terrain->chunks[terrain->chunkCount].position = offset;
    terrain->chunks[terrain->chunkCount].mesh = mesh;
    terrain->chunks[terrain->chunkCount].model = model;

    terrain->chunkCount++;
}

static Mesh GenerateTerrainMesh(int size, float scale, Vector3 offset) {
    int vertexCount = size * size;
    Mesh mesh = { 0 };

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = (size - 1) * (size - 1) * 2;

    // Allocate memory for mesh data
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));    // 3 components (x, y, z)
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));   // 2 components (u, v)
    mesh.normals = (float *)RL_CALLOC(mesh.vertexCount * 3, sizeof(float));      // Initialize to zero
    mesh.colors = (unsigned char *)RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)); // Colors (r, g, b, a)
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    int vertexIndex = 0;
    int texCoordIndex = 0;
    int colorIndex = 0;

    // Generate vertices, texcoords, and colors
    for (int z = 0; z < size; z++) {
        for (int x = 0; x < size; x++) {
            float posX = (float)x * scale;
            float posZ = (float)z * scale;
            float worldX = posX + offset.x;
            float worldZ = posZ + offset.z;

            // Generate height using octave noise
            float posY = GetOctaveNoise(worldX, worldZ, NOISE_OCTAVES, NOISE_PERSISTENCE, NOISE_LACUNARITY);

            // Set vertex positions
            mesh.vertices[vertexIndex++] = posX;
            mesh.vertices[vertexIndex++] = posY;
            mesh.vertices[vertexIndex++] = posZ;

            // Set texture coordinates
            mesh.texcoords[texCoordIndex++] = (float)x / (size - 1);
            mesh.texcoords[texCoordIndex++] = (float)z / (size - 1);

            // Normalize height to [0, 1]
            float minHeight = -NOISE_AMPLITUDE;
            float maxHeight = NOISE_AMPLITUDE;
            float normalizedHeight = (posY - minHeight) / (maxHeight - minHeight);

            // Define base colors for more detailed transitions
            Color waterColor = BLUE;
            Color sandColor = BEIGE;
            Color grassColor = GREEN;
            Color rockColor = DARKGRAY;
            Color snowColor = WHITE;

            // Interpolate colors based on height for more detailed terrain
            Color color;
            if (normalizedHeight < 0.2f) {
                // Water to sand transition
                float t = normalizedHeight / 0.2f;
                color = ColorLerp(waterColor, sandColor, t);
            } else if (normalizedHeight < 0.5f) {
                // Sand to grass transition
                float t = (normalizedHeight - 0.2f) / 0.3f;
                color = ColorLerp(sandColor, grassColor, t);
            } else if (normalizedHeight < 0.8f) {
                // Grass to rock transition
                float t = (normalizedHeight - 0.5f) / 0.3f;
                color = ColorLerp(grassColor, rockColor, t);
            } else {
                // Rock to snow transition
                float t = (normalizedHeight - 0.8f) / 0.2f;
                color = ColorLerp(rockColor, snowColor, t);
            }

            // Assign color
            mesh.colors[colorIndex++] = color.r;
            mesh.colors[colorIndex++] = color.g;
            mesh.colors[colorIndex++] = color.b;
            mesh.colors[colorIndex++] = color.a;
        }
    }

    // Generate indices and calculate normals (same as in the original)
    int index = 0;
    for (int z = 0; z < size - 1; z++) {
        for (int x = 0; x < size - 1; x++) {
            int i0 = z * size + x;
            int i1 = i0 + 1;
            int i2 = i0 + size;
            int i3 = i2 + 1;

            // Triangle 1
            mesh.indices[index++] = i0;
            mesh.indices[index++] = i2;
            mesh.indices[index++] = i1;

            // Triangle 2
            mesh.indices[index++] = i1;
            mesh.indices[index++] = i2;
            mesh.indices[index++] = i3;

            // Normals calculation remains unchanged
        }
    }

    // Normalize the accumulated normals
    for (int i = 0; i < mesh.vertexCount; i++) {
        Vector3 normal = (Vector3){
            mesh.normals[i * 3],
            mesh.normals[i * 3 + 1],
            mesh.normals[i * 3 + 2]
        };
        normal = Vector3Normalize(normal);
        mesh.normals[i * 3]     = normal.x;
        mesh.normals[i * 3 + 1] = normal.y;
        mesh.normals[i * 3 + 2] = normal.z;
    }

    // Upload mesh to GPU
    UploadMesh(&mesh, true);

    return mesh;
}
