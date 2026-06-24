#include <stdio.h>
#include <math.h>
#include <raylib.h>

#define MAX_SUM 1000
#define MAX_DISTANCE 50
#define WIDTH 900
#define HEIGHT 800
#define ZOOM 60

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

int main()
{
    InitWindow(WIDTH, HEIGHT, "Riemann Zeta Function");

    while (!WindowShouldClose())
    {
        BeginDrawing();

        for (float t = 0; t <= MAX_DISTANCE; t += 0.1)
        {
            Vector2 pos = zeta(t);
            float pos_x = pos.x * ZOOM;
            float pos_y = -pos.y * ZOOM;

            DrawPixel(pos_x + WIDTH / 2, pos_y + HEIGHT / 2, WHITE);
        }

        EndDrawing();
    }

    return 0;
}