#include <stdio.h>
#include <math.h>
#include <raylib.h>

#define MAX_SUM 1000
#define MAX_DISTANCE 85
#define MAX_POINT_COUNT 2000
#define WIDTH 900
#define HEIGHT 800
#define ZOOM 100
#define ZERO_POINT_RAD 5
#define STEP 0.05
#define POINTS_PER_FRAME 2
#define UNIT_LENGTH 8
#define MAX_UNITS 47

float t = 0;
int zero_count = 0;
float current_re = 0;
float current_im = 0;
float zero_t = 0;
bool zero_detected = false;

int point_count = (MAX_DISTANCE / STEP) + 1;
int drawn_points = 0;

bool crosses_center[MAX_POINT_COUNT] = {false};
bool center_marker_drawn[MAX_POINT_COUNT] = {false};

typedef struct
{
    float pos_x;
    float pos_y;
    float re;
    float im;
    float t;
} Point;

Vector2 zeta(float t)
{
    float re_s = 0.5;
    float im_s = t;

    float re_eta = 0;
    float im_eta = 0;

    // Dirichlet eta function
    for (int n = 1; n <= MAX_SUM; n++)
    {
        // odd - positive, even - negative
        float sign = (n % 2 == 1) ? 1 : -1;

        float re_s_n_pow = pow(n, re_s);
        float im_s_n_pow_log = log(pow(n, im_s));

        // n^s
        float re_eta_n_pow = re_s_n_pow * cos(im_s_n_pow_log);
        float im_eta_n_pow = re_s_n_pow * sin(im_s_n_pow_log);

        // 1/n^s
        float denom = pow(re_eta_n_pow, 2) + pow(im_eta_n_pow, 2);
        float re_inv_eta = re_eta_n_pow / denom;
        float im_inv_eta = -im_eta_n_pow / denom;

        re_eta += sign * re_inv_eta;
        im_eta += sign * im_inv_eta;
    }

    // Convert from eta to zeta
    float re_s_two_pow = pow(2, re_s);
    float im_s_two_pow_log = log(pow(2, -im_s));

    // 2^(1 - s)
    float re_two_pow = re_s_two_pow * cos(im_s_two_pow_log);
    float im_two_pow = re_s_two_pow * sin(im_s_two_pow_log);

    float re_denom = 1 - re_two_pow;
    float im_denom = -im_two_pow;

    // 1 / (1 - 2^(1-s))
    float denom = pow(re_denom, 2) + pow(im_denom, 2);
    float re_inv_zeta = re_denom / denom;
    float im_inv_zeta = -im_denom / denom;

    // Multiply the result of eta with zeta conversion inverse
    float re_zeta = re_eta * re_inv_zeta - im_eta * im_inv_zeta;
    float im_zeta = -(re_eta * im_inv_zeta + im_eta * re_inv_zeta);

    return (Vector2){re_zeta, im_zeta};
}

void draw_plane()
{
    Vector2 re_start = {0, HEIGHT / 2};
    Vector2 re_end = {WIDTH, HEIGHT / 2};

    Vector2 im_start = {WIDTH / 2, HEIGHT};
    Vector2 im_end = {WIDTH / 2, 0};

    float axis_thickness = 3;

    DrawLineEx(re_start, re_end, axis_thickness, WHITE);
    DrawLineEx(im_start, im_end, axis_thickness, WHITE);

    float d = 9.75;
    float line_offset = 0;
    float value = 0;
    float value_x_offset = 17;
    float value_y_offset = 11.7;

    const char *text;
    int font_size = 10;
    float unit_thickness = 1;

    Vector2 unit_x_start;
    Vector2 unit_x_end;

    // Zero
    text = TextFormat("%d", (int)value);
    DrawText(text, WIDTH / 2 - value_x_offset, HEIGHT / 2 + value_y_offset, font_size, WHITE);

    value_x_offset = 20;
    value_y_offset = 15;

    // Negative real axis
    for (int i = 1; i < MAX_UNITS + 1; i++)
    {
        if (i % 5 == 0)
        {
            line_offset = 5;
            value -= 0.5;

            if (i % 2 == 0)
            {
                text = TextFormat("%d", (int)value);
                value_x_offset = 15;
            }
            else
            {
                text = TextFormat("%.1f", value);
                value_x_offset = 20;
            }

            DrawText(text, unit_x_end.x - value_x_offset, unit_x_end.y + value_y_offset, font_size, WHITE);
        }
        else
        {
            line_offset = 0;
        }

        unit_x_start = (Vector2){WIDTH / 2.0f - i * d, HEIGHT / 2 + UNIT_LENGTH / 2 + line_offset};
        unit_x_end = (Vector2){WIDTH / 2.0f - i * d, HEIGHT / 2 - UNIT_LENGTH / 2 - line_offset};

        DrawLineEx(unit_x_start, unit_x_end, unit_thickness, WHITE);
    }

    value = 0;

    // Positive real axis
    for (int i = 1; i < MAX_UNITS + 1; i++)
    {
        if (i % 5 == 0)
        {
            line_offset = 5;
            value += 0.5;

            if (i % 2 == 0)
            {
                text = TextFormat("%d", (int)value);
                value_x_offset = 8;
            }
            else
            {
                text = TextFormat("%.1f", value);
                value_x_offset = 4;
            }

            DrawText(text, unit_x_end.x + value_x_offset, unit_x_end.y + value_y_offset, font_size, WHITE);
        }
        else
        {
            line_offset = 0;
        }

        unit_x_start = (Vector2){WIDTH / 2.0f + i * d, HEIGHT / 2 + UNIT_LENGTH / 2 + line_offset};
        unit_x_end = (Vector2){WIDTH / 2.0f + i * d, HEIGHT / 2 - UNIT_LENGTH / 2 - line_offset};

        DrawLineEx(unit_x_start, unit_x_end, unit_thickness, WHITE);
    }

    value = 0;
    value_y_offset = 5;

    // Negative imaginary axis
    for (int i = 1; i < MAX_UNITS + 1; i++)
    {
        if (i % 5 == 0)
        {
            line_offset = 5;
            value -= 0.5;

            if (i % 2 == 0)
            {
                text = TextFormat("%di", (int)value);
                value_x_offset = 20;
            }
            else
            {
                text = TextFormat("%.1fi", value);
                value_x_offset = 28;
            }

            DrawText(text, unit_x_start.x - value_x_offset, unit_x_start.y + value_y_offset, font_size, WHITE);
        }
        else
        {
            line_offset = 0;
        }

        unit_x_start = (Vector2){WIDTH / 2 - UNIT_LENGTH / 2 - line_offset, HEIGHT / 2.0f + i * d};
        unit_x_end = (Vector2){WIDTH / 2 + UNIT_LENGTH / 2 + line_offset, HEIGHT / 2.0f + i * d};

        DrawLineEx(unit_x_start, unit_x_end, unit_thickness, WHITE);
    }

    value = 0;
    value_y_offset = -14;

    // Positive imaginary axis
    for (int i = 1; i < MAX_UNITS + 1; i++)
    {
        if (i % 5 == 0)
        {
            line_offset = 5;
            value += 0.5;

            if (i % 2 == 0)
            {
                text = TextFormat("%di", (int)value);
                value_x_offset = -15;
            }
            else
            {
                text = TextFormat("%.1fi", value);
                value_x_offset = -23;
            }

            DrawText(text, unit_x_start.x + value_x_offset, unit_x_start.y + value_y_offset, font_size, WHITE);
        }
        else
        {
            line_offset = 0;
        }

        unit_x_start = (Vector2){WIDTH / 2 - UNIT_LENGTH / 2 - line_offset, HEIGHT / 2.0f - i * d};
        unit_x_end = (Vector2){WIDTH / 2 + UNIT_LENGTH / 2 + line_offset, HEIGHT / 2.0f - i * d};

        DrawLineEx(unit_x_start, unit_x_end, unit_thickness, WHITE);
    }
}

static bool point_near_center(float x, float y, float cx, float cy, float radius)
{
    return hypotf(x - cx, y - cy) <= radius;
}

void calculate_points(Point *points)
{
    for (int i = 0; i < point_count; i++)
    {
        points[i].pos_x = zeta(t).x * ZOOM + WIDTH / 2;
        points[i].pos_y = zeta(t).y * ZOOM + HEIGHT / 2;
        points[i].re = zeta(t).x;
        points[i].im = -zeta(t).y;
        points[i].t = t;

        t += STEP;
    }

    for (int i = 0; i < point_count; i++)
    {
        bool near_center = point_near_center(
            points[i].pos_x, points[i].pos_y,
            WIDTH / 2, HEIGHT / 2,
            ZERO_POINT_RAD);

        if (near_center)
        {
            crosses_center[i] = true;
            zero_count++;
        }
    }
}

void draw_points(Point *points)
{
    for (int i = 0; i < drawn_points - 1; i++)
    {
        float x1 = points[i].pos_x;
        float y1 = points[i].pos_y;
        float x2 = points[i + 1].pos_x;
        float y2 = points[i + 1].pos_y;

        Vector2 start = {x1, y1};
        Vector2 end = {x2, y2};
        float thickness = 3;

        DrawLineEx(start, end, thickness, BLUE);

        if (crosses_center[i] && !center_marker_drawn[i])
        {
            DrawCircle(WIDTH / 2, HEIGHT / 2, ZERO_POINT_RAD, RED);
            center_marker_drawn[i] = true;

            zero_t = points[i].t;
            zero_detected = true;
        }

        current_re = points[i].re;
        current_im = points[i].im;
    }
}

void draw_graph_info()
{
    int point_pos_x = 15;
    int point_pos_y = 5;

    int zero_pos_x = WIDTH - WIDTH / 4;
    int zero_pos_y = 5;

    int font_size = 20;

    const char *current_point_text;
    const char *latest_zero_text;

    if (current_im < 0)
    {
        current_point_text = TextFormat("%.2f - %.2fi", current_re, -current_im);
    }
    else
    {
        current_point_text = TextFormat("%.2f + %.2fi", current_re, current_im);
    }

    DrawText(current_point_text, point_pos_x, point_pos_y, font_size, WHITE);

    if (zero_detected)
    {
        latest_zero_text = TextFormat("zeta(0.5 ± %.2fi) = 0", zero_t);

        DrawText(latest_zero_text, zero_pos_x, zero_pos_y, font_size, WHITE);
    }
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Riemann Zeta Function");
    SetTargetFPS(15);

    Point points[MAX_POINT_COUNT];

    calculate_points(&points[0]);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_plane();
        draw_points(&points[0]);
        draw_graph_info();

        EndDrawing();

        // Gradually reveal more points
        if (drawn_points < point_count - 1)
        {
            drawn_points += POINTS_PER_FRAME;
        }
    }

    CloseWindow();

    return 0;
}