#include "raylib.h"
#include "raymath.h"
#include "ModelArray.h"
#include "rlgl.h"
#include "Terrain.h"
#include "Bullet.h"
#include <stdio.h>
#include <terraingeneration.h>


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int terraingeneration(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1080;
    const int screenHeight = 720;


    Vector3 plane_position = { 0.0f, 25.0f, -5.0f };

    Bullet bullet = { 0 };
    bullet.active = false;


    //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "FLIGHT MANIA");

    //to disable the raylib's debug messages.
    SetTraceLogLevel(LOG_NONE);
    //LOG_ALL: Show all messages.LOG_TRACE: Trace log messages.LOG_DEBUG: Debug log messages.
    //LOG_INFO: Information log messages.LOG_WARNING: Warning log messages.LOG_ERROR: Error log messages.LOG_FATAL: Fatal error log messages.
    
    ModelArray *models = CreateModelArray(0);

    // Load models and textures
    Model plane_model = LoadModel("resources/models/obj/plane.obj");
    Texture2D plane_texture = LoadTexture("resources/models/obj/plane_diffuse.png");

    // Model house_model = LoadModel("resources/models/obj/house.obj");
    // Texture2D house_texture = LoadTexture("resources/models/obj/house_diffuse.png");

    // Model cottage_model = LoadModel("resources/models/obj/cottage_obj.obj");
    // Texture2D cottage_texture = LoadTexture("resources/models/obj/cottage_diffuse.png");

    // Create model instances
    ModelInstance plane_instance = { plane_model, plane_texture, plane_position, 1.0f, WHITE };
    //ModelInstance house_instance = { house_model, house_texture, (Vector3){ 0.0f, 40.0f, 0.0f }, 1.0f, WHITE };
    //ModelInstance cottage_instance = { cottage_model, cottage_texture, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE };

    AppendModel(models, plane_instance);
    // AppendModel(models, house_instance);
    // AppendModel(models, cottage_instance);


    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    float speed = 25.0f; // Units per second

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------

    // Camera setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, -15.0f }; // Initial camera position (will be updated)
    camera.target = plane_instance.position;         // Camera looking at the plane
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera type

    //--------------------------------------------------------------------------------------

    // Terrain initialization
    TerrainManager terrain;
    InitTerrain(&terrain);

    //--------------------------------------------------------------------------------------


    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        ModelInstance *plane_instance = &(models->models[0]);

        float altitude = plane_instance->position.y;
        // Plane pitch (x-axis) controls
        if (IsKeyDown(KEY_DOWN)) { pitch += 0.6f; altitude -= 1.0f;}
        else if (IsKeyDown(KEY_UP)) { pitch -= 0.6f; altitude += 1.0f;}
        else
        {
            if (pitch > 0.3f) pitch -= 0.3f;
            else if (pitch < -0.3f) pitch += 0.3f;
        }

        // Plane yaw (y-axis) controls
        if (IsKeyDown(KEY_A)) yaw += 1.0f;
        else if (IsKeyDown(KEY_S)) yaw -= 1.0f;
        else
        {
            if (yaw > 0.0f) yaw -= 0.5f;
            else if (yaw < 0.0f) yaw += 0.5f;
        }

        float turning_value = 1.0f * GetFrameTime();
        // Plane roll (z-axis) controls and x-axis control
        if (IsKeyDown(KEY_LEFT)) {roll -= 1.0f; turning_value += 1.0f;} 
        else if (IsKeyDown(KEY_RIGHT)) {roll += 1.0f; turning_value -= 1.0f;}
        else
        {
            if (roll > 0.6f) roll -= 0.6f;
            else if (roll < -0.6f) roll += 0.6f;
        }

        // // Compute the plane's forward vector
        Matrix rotation = plane_instance->model.transform;
        Vector3 forward = { rotation.m8, rotation.m9, rotation.m10 };
        forward = Vector3Normalize(forward);

        // Plane shooting function
        if (IsKeyPressed(KEY_SPACE) && !bullet.active) {
            bullet.position = plane_instance->position;  // Set bullet position to plane's current position
            bullet.direction = forward;  // Set bullet direction to the plane's forward vector
            bullet.active = true;             // Activate the bullet
        }

        // Update plane's position
        //plane_instance->position.z += speed * GetFrameTime();//Vector3Add(models->models[0].position, Vector3Scale((Vector3){0.0f,0.0f,1.0f}, speed * GetFrameTime()));
        plane_instance->position.x += turning_value;//(models->models[0].position, Vector3Scale((Vector3){1.0f,0.0f,0.0f}, turning_value ));
        plane_instance->position.y = altitude;


        // Transformation matrix for rotations
        plane_instance->model.transform = MatrixRotateXYZ((Vector3){ DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll });

        // Collision detection with terrain
        // float terrainHeight = GetNoiseValue(models->models[0].position.x, models->models[0].position.z) + 5.0f;
        // if (models->models[0].position.y < terrainHeight) {
        //     models->models[0].position.y = terrainHeight;
        // }

        // Update camera to follow the plane
        Vector3 cameraOffset = { 0.0f, 100.0f, -300.0f };// Adjusted offset values
        //Vector3 cameraPositionOffset = Vector3Transform(cameraOffset, rotation);
        camera.position = Vector3Add(plane_instance->position, cameraOffset);
        camera.target = plane_instance->position;
        camera.up = Vector3Transform((Vector3){ 0.0f, 0.0f, 1.0f }, rotation); 
        //----------------------------------------------------------------------------------

        // Update terrain based on plane position
        UpdateTerrain(&terrain, plane_instance->position, forward, camera);

        // Update the bullet if it's active
        if (bullet.active) {
            // Move the bullet forward in its direction
            bullet.position = Vector3Add(bullet.position, Vector3Scale(bullet.direction, BULLET_SPEED * GetFrameTime()));
        
            // Deactivate the bullet if it goes out of bounds (for example, if it exceeds 1000 units from the origin)
            if (Vector3Length(bullet.position) > (plane_instance->position.z + BULLET_RANGE)) {
                bullet.active = false;
            }
        }

        // Draw
        //----------------------------------------------------------------------------------

        BeginDrawing();

            ClearBackground(BLANK);
      
            // Draw 3D models
            BeginMode3D(camera);

                // Enable wireframe mode
                rlEnableWireMode();
                
                // Draw terrain
                DrawTerrain(&terrain);
                
                // Disable wireframe mode
                rlDisableWireMode();
              
                // Draw all models
                for (size_t i = 0; i < models->size; ++i) {
                    DrawModel(models->models[i].model, models->models[i].position, models->models[i].scale, models->models[i].color);
                }

                if(bullet.active)
                    DrawCube(bullet.position, 7.5f, 7.5f, 15.0f, RED);  // Draw the bullet as a rectangle
                
            EndMode3D();

            DrawRectangle(5, 45, 250, 90, Fade(GREEN, 0.5f));
            DrawRectangleLines(5, 45, 250, 90, Fade(DARKGREEN, 0.5f));
            char info[128];
            sprintf(info, "Speed: %.2f units/s", speed);
            DrawText(info, 10, 50, 15, WHITE);
            sprintf(info, "Altitude: %.2f units", models->models[0].position.y);
            DrawText(info, 10, 70, 15, WHITE);
            sprintf(info, "Bullet Position: %f ", bullet.position.z);
            DrawText(info, 10, 90, 15, WHITE);
            sprintf(info, "Bullet Active: %d ", bullet.active);
            DrawText(info, 10, 110, 15, WHITE);

            DrawText("(c) HKN SoftCrafting", screenWidth - 200, screenHeight - 20, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload terrain
    UnloadTerrain(&terrain);
    
    // Unload all models and textures
    UnloadModelArray(models);

    // Free the model array
    FreeModelArray(models);

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
