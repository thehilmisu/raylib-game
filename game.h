#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "ModelArray.h"

// Constants
#define     SCREEN_WIDTH                1080
#define     SCREEN_HEIGHT               720
#define     TARGET_FPS                  60

#define     PLANE_INITIAL_POSITION_X    0.0f
#define     PLANE_INITIAL_POSITION_Y    25.0f
#define     PLANE_INITIAL_POSITION_Z    -5.0f
#define     PLANE_INITIAL_SCALE         1.0f
#define     PLANE_INITIAL_SPEED         25.0f // Units per second

#define     CAMERA_INITIAL_POSITION_X    0.0f
#define     CAMERA_INITIAL_POSITION_Y    5.0f
#define     CAMERA_INITIAL_POSITION_Z    -15.0f
#define     CAMERA_FOVY                  60.0f        

#define     WINDOW_NAME                 "FLIGHT MANIA"

// Resource paths
#define     PLANE_MODEL     "resources/models/obj/plane.obj"
#define     PLANE_TEXTURE   "resources/models/obj/plane_diffuse.png"


// Function declarations
void LoadModels();
void LoadGame();
void GameLoop();
void Draw();
void UserInput();
void UnloadGame();
void ObjectLookAtMouse(Vector3 objectPosition, Camera3D camera, Matrix *outTransform);



#endif // GAME_H
