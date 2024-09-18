#include "raylib.h"
#include "raymath.h"
#include "ModelArray.h"
#include "rlgl.h"
#include "Terrain.h"
#include <stdio.h>

typedef struct Bullet {
    Vector3 position;
    Vector3 direction;
    bool active;
} Bullet;


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1080;
    const int screenHeight = 720;


    Vector3 plane_position = { 0.0f, 25.0f, -5.0f };

    Bullet bullet = { 0 };
    bullet.active = false;


    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "FLIGHT MANIA");

    //to disable the raylib's debug messages.
    SetTraceLogLevel(LOG_NONE);
    // LOG_ALL: Show all messages.LOG_TRACE: Trace log messages.LOG_DEBUG: Debug log messages.
    //LOG_INFO: Information log messages.LOG_WARNING: Warning log messages.LOG_ERROR: Error log messages.LOG_FATAL: Fatal error log messages.
    
    ModelArray *models = CreateModelArray(0);

    // Load models and textures
    Model plane_model = LoadModel("resources/models/obj/plane.obj");
    Texture2D plane_texture = LoadTexture("resources/models/obj/plane_diffuse.png");

    Model house_model = LoadModel("resources/models/obj/house.obj");
    Texture2D house_texture = LoadTexture("resources/models/obj/house_diffuse.png");

    Model cottage_model = LoadModel("resources/models/obj/cottage_obj.obj");
    Texture2D cottage_texture = LoadTexture("resources/models/obj/cottage_diffuse.png");

    // Create model instances
    ModelInstance plane_instance = { plane_model, plane_texture, plane_position, 1.0f, WHITE };
    ModelInstance house_instance = { house_model, house_texture, (Vector3){ 0.0f, 40.0f, 0.0f }, 1.0f, WHITE };
    ModelInstance cottage_instance = { cottage_model, cottage_texture, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE };

    AppendModel(models, plane_instance);
    // AppendModel(models, house_instance);
    // AppendModel(models, cottage_instance);


    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    float speed = 25.0f; // Units per second

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    // Create a render texture for drawing the mini-axis
    RenderTexture2D renderTexture = LoadRenderTexture(150, 150);
    //--------------------------------------------------------------------------------------

    // Camera setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, -15.0f }; // Initial camera position (will be updated)
    camera.target = models->models[0].position;         // Camera looking at the plane
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

        float altitude = models->models[0].position.y;
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
        Matrix rotation = models->models[0].model.transform;
        Vector3 forward = { rotation.m8, rotation.m9, rotation.m10 };
        forward = Vector3Normalize(forward);

        // Plane shooting function
        if (IsKeyPressed(KEY_SPACE) && !bullet.active) {
            bullet.position = models->models[0].position;  // Set bullet position to plane's current position
            bullet.direction = forward;  // Set bullet direction to the plane's forward vector
            bullet.active = true;             // Activate the bullet
        }

        // Update plane's position
        models->models[0].position.z += speed * GetFrameTime();//Vector3Add(models->models[0].position, Vector3Scale((Vector3){0.0f,0.0f,1.0f}, speed * GetFrameTime()));
        models->models[0].position.x += turning_value;//(models->models[0].position, Vector3Scale((Vector3){1.0f,0.0f,0.0f}, turning_value ));
        models->models[0].position.y = altitude;


        // Transformation matrix for rotations
        models->models[0].model.transform = MatrixRotateXYZ((Vector3){ DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll });

        // Collision detection with terrain
        // float terrainHeight = GetNoiseValue(models->models[0].position.x, models->models[0].position.z) + 5.0f;
        // if (models->models[0].position.y < terrainHeight) {
        //     models->models[0].position.y = terrainHeight;
        // }

        // Update camera to follow the plane
        Vector3 cameraOffset = { 0.0f, 100.0f, -300.0f };// Adjusted offset values
        //Vector3 cameraPositionOffset = Vector3Transform(cameraOffset, rotation);
        camera.position = Vector3Add(models->models[0].position, cameraOffset);
        camera.target = models->models[0].position;
        camera.up = Vector3Transform((Vector3){ 0.0f, 0.0f, 1.0f }, rotation); 
        //----------------------------------------------------------------------------------

        // Update terrain based on plane position
        UpdateTerrain(&terrain, models->models[0].position, forward, camera);

        // Update the bullet if it's active
        if (bullet.active) {
            // Move the bullet forward in its direction
            float bulletSpeed = 100.0f;  // Bullet speed
            //bullet.position = Vector3Add(bullet.position, Vector3Scale(bullet.direction, bulletSpeed * GetFrameTime()));
            bullet.position.z += bulletSpeed * GetFrameTime();

            // Deactivate the bullet if it goes out of bounds (for example, if it exceeds 1000 units from the origin)
            if (bullet.position.z > 1000.0f) {
                bullet.active = false;
                bullet.position.z = models->models->position.z;
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
                    DrawCube(bullet.position, 15.0f, 15.0f, 15.0f, RED);  // Draw the bullet as a rectangle
                
            EndMode3D();

            char info[128];
            sprintf(info, "Speed: %.2f units/s", speed);
            DrawText(info, 10, 50, 20, WHITE);
            sprintf(info, "Altitude: %.2f units", models->models[0].position.y);
            DrawText(info, 10, 80, 20, WHITE);
            sprintf(info, "Bullet Position: %f ", bullet.position.z);
            DrawText(info, 10, 110, 20, WHITE);
            sprintf(info, "Bullet Active: %d ", bullet.active);
            DrawText(info, 10, 140, 20, WHITE);

            // Draw controls info
            // DrawRectangle(30, 600, 340, 70, Fade(GREEN, 0.5f));
            // DrawRectangleLines(30, 600, 340, 70, Fade(DARKGREEN, 0.5f));
            // DrawText("Pitch controlled with: KEY_UP / KEY_DOWN", 40, 610, 10, DARKGRAY);
            // DrawText("Roll controlled with: KEY_LEFT / KEY_RIGHT", 40, 620, 10, DARKGRAY);
            // DrawText("Yaw controlled with: KEY_A / KEY_S", 40, 630, 10, DARKGRAY);

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
