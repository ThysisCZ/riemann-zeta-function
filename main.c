#include <stdio.h>
#include <math.h>
#include <raylib.h>

#define MAX_SUM 1000
#define MAX_DISTANCE 85
#define MAX_POINT_COUNT 2000
#define WIDTH 900
#define HEIGHT 800
#define ZOOM 60
#define ZERO_POINT_RAD 5
#define STEP 0.05f
#define POINTS_PER_FRAME 4

int zero_count = 0;

Vector2 zeta(float t)
{
    float re_s = 0.5f;
    float im_s = t;

    float re_eta = 0.0f;
    float im_eta = 0.0f;

    // Dirichlet eta function
    for (int n = 1; n <= MAX_SUM; n++)
    {
        // odd - positive, even - negative
        float sign = (n % 2 == 1) ? 1.0f : -1.0f;

        float re_exp = exp(re_s * log(n)); // n^real

        // n^s
        float re_ns = re_exp * cos(im_s * log(n));
        float im_ns = re_exp * sin(im_s * log(n));

        // 1/n^s
        float denom = pow(re_ns, 2) + pow(im_ns, 2);
        float re_inv_eta = re_ns / denom;
        float im_inv_eta = -im_ns / denom;

        re_eta += sign * re_inv_eta;
        im_eta += sign * im_inv_eta;
    }

    // Convert from eta to zeta
    // 2^(1 - s)
    float re_exp = exp(re_s * log(2.0f));

    float re_two_pow = re_exp * cos(-t * log(2.0f));
    float im_two_pow = re_exp * sin(-t * log(2.0f));

    float re_div = 1.0f - re_two_pow;
    float im_div = -im_two_pow;

    // 1 / (1 - 2^(1-s))
    float denom = pow(re_div, 2) + pow(im_div, 2);
    float re_inv_zeta = re_div / denom;
    float im_inv_zeta = -im_div / denom;

    // Multiply the result of eta with zeta conversion inverse
    float re_zeta = re_eta * re_inv_zeta - im_eta * im_inv_zeta;
    float im_zeta = re_eta * im_inv_zeta + im_eta * re_inv_zeta;

    return (Vector2){re_zeta, im_zeta};
}

static bool point_near_center(float x, float y, float cx, float cy, float radius)
{
    return hypotf(x - cx, y - cy) <= radius;
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Riemann Zeta Function");
    SetTargetFPS(15);

    int point_count = (int)(MAX_DISTANCE / STEP) + 1;
    Vector2 points[MAX_POINT_COUNT];
    Vector2 screen_points[MAX_POINT_COUNT];
    bool crosses_center[MAX_POINT_COUNT] = {0};
    bool center_marker_drawn[MAX_POINT_COUNT] = {0};

    // Pre-calculate all points
    float t = 0;
    for (int i = 0; i < point_count; i++)
    {
        points[i] = zeta(t);

        screen_points[i].x = points[i].x * ZOOM + WIDTH / 2;
        screen_points[i].y = -points[i].y * ZOOM + HEIGHT / 2;

        t += STEP;
    }

    bool near_center = false;
    for (int i = 0; i < point_count; i++)
    {
        bool current_near_center = point_near_center(
            screen_points[i].x, screen_points[i].y,
            WIDTH / 2.0f, HEIGHT / 2.0f,
            ZERO_POINT_RAD);

        if (current_near_center && !near_center)
        {
            crosses_center[i] = true;
            zero_count++;
            near_center = true;
        }
        else if (!current_near_center)
        {
            near_center = false;
        }
    }

    int drawn_points = 1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw all calculated points
        for (int i = 0; i < drawn_points - 1; i++)
        {
            float x1 = screen_points[i].x;
            float y1 = screen_points[i].y;
            float x2 = screen_points[i + 1].x;
            float y2 = screen_points[i + 1].y;

            DrawLine(x1, y1, x2, y2, BLUE);

            if (crosses_center[i] && !center_marker_drawn[i])
            {
                DrawCircle(WIDTH / 2, HEIGHT / 2, ZERO_POINT_RAD, RED);
                center_marker_drawn[i] = true;
            }
        }

        EndDrawing();

        // Gradually reveal more points
        if (drawn_points < point_count)
        {
            drawn_points += POINTS_PER_FRAME;
            if (drawn_points > point_count)
            {
                drawn_points = point_count;
            }
        }
    }

    CloseWindow();

    return 0;
}