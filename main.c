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

typedef struct
{
    float pos_x;
    float pos_y;
    float re;
    float im;
    float t;
} Point;

typedef struct
{
    Point points[MAX_POINT_COUNT];
    float current_t;
    float current_re;
    float current_im;
    float zero_t;
    int zero_count;
    int point_count;
    int drawn_points;
    bool zero_detected;
    bool crosses_center[MAX_POINT_COUNT];
    bool center_marker_drawn[MAX_POINT_COUNT];
} Graph;

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

void calculate_points(Graph *graph)
{
    for (int i = 0; i < graph->point_count; i++)
    {
        // Precalculate screen and complex plane position for each point
        graph->points[i].pos_x = zeta(graph->current_t).x * ZOOM + WIDTH / 2;
        graph->points[i].pos_y = zeta(graph->current_t).y * ZOOM + HEIGHT / 2;
        graph->points[i].re = zeta(graph->current_t).x;
        graph->points[i].im = -zeta(graph->current_t).y;
        graph->points[i].t = graph->current_t;

        bool near_center = point_near_center(
            graph->points[i].pos_x, graph->points[i].pos_y,
            WIDTH / 2, HEIGHT / 2,
            ZERO_POINT_RAD);

        if (near_center)
        {
            graph->crosses_center[i] = true;
            graph->zero_count++;
        }

        graph->current_t += STEP;
    }
}

void draw_points(Graph *graph)
{
    for (int i = 0; i < graph->drawn_points - 1; i++)
    {
        float x1 = graph->points[i].pos_x;
        float y1 = graph->points[i].pos_y;
        float x2 = graph->points[i + 1].pos_x;
        float y2 = graph->points[i + 1].pos_y;

        Vector2 start = {x1, y1};
        Vector2 end = {x2, y2};
        float thickness = 3;

        DrawLineEx(start, end, thickness, BLUE);

        // Handle non trivial zero detection
        if (graph->crosses_center[i] && !graph->center_marker_drawn[i])
        {
            DrawCircle(WIDTH / 2, HEIGHT / 2, ZERO_POINT_RAD, RED);
            graph->center_marker_drawn[i] = true;

            graph->zero_t = graph->points[i].t;
            graph->zero_detected = true;
        }

        graph->current_re = graph->points[i].re;
        graph->current_im = graph->points[i].im;
    }
}

void draw_graph_info(Graph *graph)
{
    int point_pos_x = 15;
    int point_pos_y = 5;

    int zero_pos_x = WIDTH - WIDTH / 4;
    int zero_pos_y = 5;

    int font_size = 20;

    const char *current_point_text;
    const char *latest_zero_text;

    // Adjust complex number sign if the graph is below real axis
    if (graph->current_im < 0)
    {
        current_point_text = TextFormat("%.2f - %.2fi", graph->current_re, -graph->current_im);
    }
    else
    {
        current_point_text = TextFormat("%.2f + %.2fi", graph->current_re, graph->current_im);
    }

    DrawText(current_point_text, point_pos_x, point_pos_y, font_size, WHITE);

    if (graph->zero_detected)
    {
        latest_zero_text = TextFormat("zeta(0.5 ± %.2fi) = 0", graph->zero_t);

        DrawText(latest_zero_text, zero_pos_x, zero_pos_y, font_size, WHITE);
    }
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Riemann Zeta Function");
    SetTargetFPS(15);

    Graph graph;

    graph.current_t = 0;
    graph.current_re = 0;
    graph.current_im = 0;
    graph.zero_t = 0;

    graph.zero_count = 0;
    graph.point_count = (MAX_DISTANCE / STEP) + 1;
    graph.drawn_points = 0;
    graph.zero_detected = false;

    // Initialize non trivial zero detection
    for (int i = 0; i < MAX_POINT_COUNT; i++)
    {
        graph.crosses_center[i] = false;
        graph.center_marker_drawn[i] = false;
    }

    calculate_points(&graph);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_plane();
        draw_points(&graph);
        draw_graph_info(&graph);

        EndDrawing();

        // Gradually reveal more points
        if (graph.drawn_points < graph.point_count - 1)
        {
            graph.drawn_points += POINTS_PER_FRAME;
        }
    }

    CloseWindow();

    return 0;
}