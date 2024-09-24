#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "ModelArray.h"


// Function declarations
void LoadGame();
void GameLoop();
void Draw();
void UserInput(ModelInstance *plane_instance);
void UnloadGame();


#endif // GAME_H
