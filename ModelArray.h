// ModelArray.h
#ifndef MODELARRAY_H
#define MODELARRAY_H

#include <stddef.h>
#include "raylib.h"

typedef struct {
    Model model;
    Texture2D texture;
    Vector3 position;
    float scale;
    Color color;
} ModelInstance;

typedef struct {
    size_t size;     // Number of models currently stored
    size_t capacity; // Allocated capacity
    ModelInstance *models;
} ModelArray;

// Function declarations
ModelArray *CreateModelArray(size_t initial_capacity);
void AppendModel(ModelArray *array, ModelInstance instance);
void UnloadModelArray(ModelArray *array);
void FreeModelArray(ModelArray *array);

#endif // MODELARRAY_H
