#include "raylib.h"

#define     BULLET_SPEED    200
#define     BULLET_RANGE    1000

typedef struct Bullet {
    Vector3 position;
    Vector3 direction;
    bool active;
} Bullet;