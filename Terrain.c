// Terrain.c

#define FNL_IMPL
#include "FastNoiseLite.h"

#include "Terrain.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <string.h> 
#include "rlgl.h"   

// Internal functions
float GetNoiseValue(float x, float z);
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

float GetNoiseValue(float x, float y, float z) {
    // Get noise value from FastNoiseLite
    return fnlGetNoise3D(&noise, x, y, z) * NOISE_AMPLITUDE;
    //return fnlGetNoise2D(&noise, x, z) * NOISE_AMPLITUDE;
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
            float posY = GetNoiseValue(worldX, worldZ);

            // Set vertex positions
            mesh.vertices[vertexIndex++] = posX;
            mesh.vertices[vertexIndex++] = posY;
            mesh.vertices[vertexIndex++] = posZ;

            // Set texture coordinates (optional)
            mesh.texcoords[texCoordIndex++] = (float)x / (size - 1);
            mesh.texcoords[texCoordIndex++] = (float)z / (size - 1);

            // Normalize height to [0, 1]
            float minHeight = -NOISE_AMPLITUDE;
            float maxHeight = NOISE_AMPLITUDE;
            float normalizedHeight = (posY - minHeight) / (maxHeight - minHeight);

            // Define base colors
            Color waterColor = BLUE;
            Color landColor = GREEN;
            Color mountainColor = GRAY;
            Color snowColor = WHITE;

            // Interpolate colors based on height
            Color color;

            if (normalizedHeight < 0.3f) {
                // Water to land transition
                float t = normalizedHeight / 0.3f;
                color = ColorLerp(waterColor, landColor, t);
            } else if (normalizedHeight < 0.6f) {
                // Land to mountain transition
                float t = (normalizedHeight - 0.3f) / 0.3f;
                color = ColorLerp(landColor, mountainColor, t);
            } else {
                // Mountain to snow transition
                float t = (normalizedHeight - 0.6f) / 0.4f;
                color = ColorLerp(mountainColor, snowColor, t);
            }

            // Assign color
            mesh.colors[colorIndex++] = color.r;
            mesh.colors[colorIndex++] = color.g;
            mesh.colors[colorIndex++] = color.b;
            mesh.colors[colorIndex++] = color.a;
        }
    }

    // Generate indices and calculate normals
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

            // Calculate normals for Triangle 1
            Vector3 v0 = (Vector3){ mesh.vertices[i0 * 3], mesh.vertices[i0 * 3 + 1], mesh.vertices[i0 * 3 + 2] };
            Vector3 v1 = (Vector3){ mesh.vertices[i2 * 3], mesh.vertices[i2 * 3 + 1], mesh.vertices[i2 * 3 + 2] };
            Vector3 v2 = (Vector3){ mesh.vertices[i1 * 3], mesh.vertices[i1 * 3 + 1], mesh.vertices[i1 * 3 + 2] };

            Vector3 edge1 = Vector3Subtract(v1, v0);
            Vector3 edge2 = Vector3Subtract(v2, v0);
            Vector3 normal1 = Vector3CrossProduct(edge1, edge2);
            normal1 = Vector3Normalize(normal1);

            // Accumulate normals for vertices
            mesh.normals[i0 * 3]     += normal1.x;
            mesh.normals[i0 * 3 + 1] += normal1.y;
            mesh.normals[i0 * 3 + 2] += normal1.z;

            mesh.normals[i2 * 3]     += normal1.x;
            mesh.normals[i2 * 3 + 1] += normal1.y;
            mesh.normals[i2 * 3 + 2] += normal1.z;

            mesh.normals[i1 * 3]     += normal1.x;
            mesh.normals[i1 * 3 + 1] += normal1.y;
            mesh.normals[i1 * 3 + 2] += normal1.z;

            // Calculate normals for Triangle 2
            v0 = (Vector3){ mesh.vertices[i1 * 3], mesh.vertices[i1 * 3 + 1], mesh.vertices[i1 * 3 + 2] };
            v1 = (Vector3){ mesh.vertices[i2 * 3], mesh.vertices[i2 * 3 + 1], mesh.vertices[i2 * 3 + 2] };
            v2 = (Vector3){ mesh.vertices[i3 * 3], mesh.vertices[i3 * 3 + 1], mesh.vertices[i3 * 3 + 2] };

            edge1 = Vector3Subtract(v1, v0);
            edge2 = Vector3Subtract(v2, v0);
            Vector3 normal2 = Vector3CrossProduct(edge1, edge2);
            normal2 = Vector3Normalize(normal2);

            // Accumulate normals for vertices
            mesh.normals[i1 * 3]     += normal2.x;
            mesh.normals[i1 * 3 + 1] += normal2.y;
            mesh.normals[i1 * 3 + 2] += normal2.z;

            mesh.normals[i2 * 3]     += normal2.x;
            mesh.normals[i2 * 3 + 1] += normal2.y;
            mesh.normals[i2 * 3 + 2] += normal2.z;

            mesh.normals[i3 * 3]     += normal2.x;
            mesh.normals[i3 * 3 + 1] += normal2.y;
            mesh.normals[i3 * 3 + 2] += normal2.z;
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
