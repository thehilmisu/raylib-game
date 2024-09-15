#include "raylib.h"
#include "raymath.h"
#include "ModelArray.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1080;
    const int screenHeight = 720;
    Vector3 plane_position = { 0.0f, -8.0f, 0.0f };

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "FLIGHT MANIA");

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
    AppendModel(models, house_instance);
    AppendModel(models, cottage_instance);

    // Set models' textures
    for (size_t i = 0; i < models->size; ++i) {
        models->models[i].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = models->models[i].texture;
    }

    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    float speed = -50.0f; // Units per second

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Camera setup
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, -15.0f }; // Initial camera position (will be updated)
    camera.target = models->models[0].position;         // Camera looking at the plane
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera type

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Plane pitch (x-axis) controls
        if (IsKeyDown(KEY_DOWN)) pitch += 0.6f;
        else if (IsKeyDown(KEY_UP)) pitch -= 0.6f;
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

        // Plane roll (z-axis) controls
        if (IsKeyDown(KEY_LEFT)) roll -= 0.6f;
        else if (IsKeyDown(KEY_RIGHT)) roll += 0.6f;
        else
        {
            if (roll > 0.3f) roll -= 0.3f;
            else if (roll < -0.3f) roll += 0.3f;
        }

        // Transformation matrix for rotations
        models->models[0].model.transform = MatrixRotateXYZ((Vector3){ DEG2RAD * pitch, DEG2RAD * yaw, DEG2RAD * roll });

        // Compute the plane's forward vector
        Matrix rotation = models->models[0].model.transform;
        Vector3 forward = { -rotation.m8, -rotation.m9, -rotation.m10 };
        forward = Vector3Normalize(forward);

        // Update plane's position
        models->models[0].position = Vector3Add(models->models[0].position, Vector3Scale(forward, speed * GetFrameTime()));

        // Update camera to follow the plane
        Vector3 cameraOffset = { 0.0f, 5.0f, -125.0f };// Adjusted offset values
        Vector3 cameraPositionOffset = Vector3Transform(cameraOffset, rotation);
        camera.position = Vector3Add(models->models[0].position, cameraPositionOffset);
        camera.target = models->models[0].position;
        camera.up = Vector3Transform((Vector3){ 0.0f, 1.0f, 0.0f }, rotation); // Update camera up vector
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw 3D models
            BeginMode3D(camera);

                // Draw all models
                for (size_t i = 0; i < models->size; ++i) {
                    DrawModel(models->models[i].model, models->models[i].position, models->models[i].scale, models->models[i].color);
                }

                DrawGrid(10, 10.0f);

            EndMode3D();

            // Draw controls info
            DrawRectangle(30, 600, 340, 70, Fade(GREEN, 0.5f));
            DrawRectangleLines(30, 600, 340, 70, Fade(DARKGREEN, 0.5f));
            DrawText("Pitch controlled with: KEY_UP / KEY_DOWN", 40, 610, 10, DARKGRAY);
            DrawText("Roll controlled with: KEY_LEFT / KEY_RIGHT", 40, 620, 10, DARKGRAY);
            DrawText("Yaw controlled with: KEY_A / KEY_S", 40, 630, 10, DARKGRAY);

            DrawText("(c) HKN SoftCrafting", screenWidth - 200, screenHeight - 20, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload all models and textures
    UnloadModelArray(models);

    // Free the model array
    FreeModelArray(models);

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
