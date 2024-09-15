// ModelArray.c
#include "ModelArray.h"
#include <stdlib.h>

ModelArray *CreateModelArray(size_t initial_capacity) {
    ModelArray *array = (ModelArray *)malloc(sizeof(ModelArray));
    if (!array) return NULL;

    array->size = 0;
    array->capacity = initial_capacity > 0 ? initial_capacity : 4;
    array->models = (ModelInstance *)malloc(array->capacity * sizeof(ModelInstance));
    if (!array->models) {
        free(array);
        return NULL;
    }
    return array;
}

void AppendModel(ModelArray *array, ModelInstance instance) {
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        ModelInstance *new_instances = (ModelInstance *)realloc(array->models, array->capacity * sizeof(ModelInstance));
        if (!new_instances) {
            // Handle realloc failure
            return;
        }
        array->models = new_instances;
    }
    
    instance.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = instance.texture;    
    array->models[array->size++] = instance;
}

void UnloadModelArray(ModelArray *array) {
    if (array) {
        for (size_t i = 0; i < array->size; ++i) {
            UnloadModel(array->models[i].model);
            UnloadTexture(array->models[i].texture);
        }
    }
}

void FreeModelArray(ModelArray *array) {
    if (array) {
        free(array->models);
        free(array);
    }
}
