#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>

#define WIDTH 720
#define HEIGHT 405
#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 32

typedef struct
{
    unsigned char b, g, r, a;
}pixel;

typedef struct
{
    pixel top, left, right;
    unsigned char noise_strength, height;
}block_variables;

typedef struct
{
    unsigned char type, height;
    unsigned int timer;
}block_status;

typedef struct
{
    unsigned int x, y, z;
}vector3;

enum
{
    EMPTY, GRASS, STONE, SAND, BEDROCK, WOOD, BIRCH, WATER, CURSOR
};

block_variables block_types[] = {
    {.top = { 0, 0, 0}, .left = { 0, 0, 0 }, .right = { 0, 0, 0 }, .noise_strength = 0, .height = 0 },
    {.top = { 30, 130, 30 }, .left = { 12 , 52 , 110 }, .right = { 10 , 40 , 80 }, .noise_strength = 5, .height = 8 },
    {.top = { 88, 88, 88 }, .left = { 100 , 100 , 100 }, .right = { 76 , 76 , 76 }, .noise_strength = 6, .height = 8 },
    {.top = { 140, 180, 190 }, .left = { 160, 200, 210 }, .right = { 120, 160, 170 }, .noise_strength = 10, .height = 8 },
    {.top = { 45, 45, 45 }, .left = { 52, 52, 52 }, .right = { 36, 36, 36 }, .noise_strength = 32, .height = 8 },
    {.top = { 50, 90, 110 }, .left = { 60, 105, 125 }, .right = { 40, 75, 95 }, .noise_strength = 5, .height = 8 },
    {.top = { 70, 110, 125 }, .left = { 78, 120, 137 }, .right = { 62, 100, 112 }, .noise_strength = 5, .height = 8 },
    {.top = { 200, 20, 20 }, .left = { 220, 22, 22 }, .right = { 180, 18, 18 }, .noise_strength = 5, .height = 7 },
    {.top = { 0, 255, 0 }, .left = { 0, 255, 0 }, .right = { 0, 255, 0 }, .noise_strength = 0, .height = 8 }
};

pixel vram[HEIGHT * WIDTH];

BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER),WIDTH,HEIGHT,1,32,BI_RGB };
HWND window;
HDC WindowDC;
    
int windowMessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSA window_class = { .lpfnWndProc = windowMessageHandler,.lpszClassName = "class",.lpszMenuName = "class" };

int global_seed;
int screen_width;
int screen_heigth;
unsigned int blink_timer;
unsigned int selected_block = 1;
vector3 cursor;
block_status grid[CHUNK_HEIGHT][CHUNK_SIZE][CHUNK_SIZE];

int windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        if (grid[cursor.z][cursor.y][cursor.x].type != BEDROCK)
        {
            grid[cursor.z][cursor.y][cursor.x].type = selected_block;
            grid[cursor.z][cursor.y][cursor.x].height = block_types[selected_block].height;
        }
        break;
    case WM_RBUTTONDOWN:
        if (grid[cursor.z][cursor.y][cursor.x].type != BEDROCK)
        {
            grid[cursor.z][cursor.y][cursor.x].type = EMPTY;
            grid[cursor.z][cursor.y][cursor.x].height = 0;
        }
        break;
    case WM_MOUSEWHEEL:;
        short scroll = (short)(wParam >> 16) / 120;
        selected_block += scroll;
        if (selected_block < 1)
        {
            selected_block = 1;
        }
        else if (selected_block > sizeof(block_types) / sizeof(block_variables) - 2)
        {
            selected_block = sizeof(block_types) / sizeof(block_variables) - 2;
        }
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'A':
            if (cursor.y < CHUNK_SIZE - 1)
            {
                cursor.y++;
                blink_timer = 0;
            }
            break;
        case 'W':
            if (cursor.x > 0)
            {
                cursor.x--;
                blink_timer = 0;
            }
            break;
        case 'S':
            if (cursor.x < CHUNK_SIZE - 1)
            {
                cursor.x++;
                blink_timer = 0;
            }
            break;
        case 'D':
            if (cursor.y > 0)
            {
                cursor.y--;
                blink_timer = 0;
            }
            break;
        case VK_CONTROL:
            if (cursor.z > 0)
            {
                cursor.z--;
                blink_timer = 0;
            }
            break;
        case VK_SHIFT:
            if (cursor.z < CHUNK_HEIGHT - 1)
            {
                cursor.z++;
                blink_timer = 0;
            }
            break;
        }
        break;
    case WM_QUIT:
    case WM_CLOSE:
        ExitProcess(0);
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

int tRand(int range)
{
    unsigned int x = __rdtsc();
    x += (x << 10);
    x ^= (x >> 6);
    x += (x << 3);
    x ^= (x >> 11);
    x += (x << 15);
    return (x % range);
}

int noise(int range, int seed)
{
    unsigned int x = seed + global_seed;
    x += (x << 10);
    x ^= (x >> 6);
    x += (x << 3);
    x ^= (x >> 11);
    x += (x << 15);
    return (x % range);
}

int get_minimum(int i, int j)
{
    return(i < j ? i : j);
}

int get_maximum(int i, int j)
{
    return(i > j ? i : j);
}

void draw_pixel(int x, int y, pixel colour)
{
    if (vram[y * WIDTH + x].a != 0 && colour.a > 0)
    {
        return;
    }
    colour.r = (colour.r * (255 - colour.a) + vram[y * WIDTH + x].r * colour.a) >> 8;
    colour.g = (colour.g * (255 - colour.a) + vram[y * WIDTH + x].g * colour.a) >> 8;
    colour.b = (colour.b * (255 - colour.a) + vram[y * WIDTH + x].b * colour.a) >> 8;
    vram[y * WIDTH + x] = colour;
}

void drawrow(int x, int y, int length, pixel colour, unsigned char noise_strength)
{
    int add = length >= 0 ? 1 : -1;
    length = length < 0 ? -length : length;
    for (int i = 0; i < length && -i < length; i += add)
    {
        if (noise_strength == 0)
        {
            draw_pixel(x + i, y, colour);
            continue;
        }
        int offset = noise(noise_strength * 2, x + i + y * 1200) - noise_strength;
        vector3 colours = { colour.b + offset, colour.g + offset, colour.r + offset };
        colours.x = get_minimum(get_maximum(colours.x, 0), 255);
        colours.y = get_minimum(get_maximum(colours.y, 0), 255);
        colours.z = get_minimum(get_maximum(colours.z, 0), 255);
        pixel current_pixel = { colours.x, colours.y, colours.z, colour.a };
        draw_pixel(x + i, y, current_pixel);
    }
}

void drawtriangle(int x, int y, int width, bool pointing_right, pixel colour, unsigned char noise_strength)
{
    int add = pointing_right ? 2 : -2;
    int i = add;
    for (; i < width && -i < width; i += add, y++)
    {
        drawrow(x, y, i, colour, noise_strength);
    }
    for (; i != 0; i -= add, y++)
    {
        drawrow(x, y, i, colour, noise_strength);
    }
}

void draw_uneven_hexagon(int x, int y, int diameter, int block_ID, unsigned char height)
{
    int i = height == 0 ? 0 : 1;

    for (; i < height; i++)
    {
        drawrow(x, y + i + 3, diameter / 2, block_types[block_ID].left, block_types[block_ID].noise_strength);
        drawrow(x + diameter / 2, y + i + 3, diameter / 2, block_types[block_ID].right, block_types[block_ID].noise_strength);
    }
    drawtriangle(x + diameter / 2 - 1, y, diameter / 2, false, block_types[block_ID].left, block_types[block_ID].noise_strength);

    drawtriangle(x + diameter / 2, y, diameter / 2, true, block_types[block_ID].right, block_types[block_ID].noise_strength);

    drawtriangle(x + diameter / 2 - 1, y + i, diameter / 2, false, block_types[block_ID].top, block_types[block_ID].noise_strength);
    drawtriangle(x + diameter / 2, y + i, diameter / 2, true, block_types[block_ID].top, block_types[block_ID].noise_strength);
}

void drawhexagon(int x, int y, int diameter, int block_ID, vector3 block_pos)
{
    if (block_pos.z >= CHUNK_HEIGHT - 1 || grid[block_pos.z + 1][block_pos.y][block_pos.x].type == EMPTY)
    {
        drawtriangle(x + diameter / 2 - 1, y + diameter / 2, diameter / 2, false, block_types[block_ID].top, block_types[block_ID].noise_strength);
        drawtriangle(x + diameter / 2, y + diameter / 2, diameter / 2, true, block_types[block_ID].top, block_types[block_ID].noise_strength);
    }

    if (block_pos.x >= CHUNK_SIZE - 1 || grid[block_pos.z][block_pos.y][block_pos.x + 1].height != 8)
    {
        drawtriangle(x + diameter / 2, y, diameter / 2, true, block_types[block_ID].right, block_types[block_ID].noise_strength);
        drawtriangle(x + diameter - 1, y + diameter / 4, diameter / 2, false, block_types[block_ID].right, block_types[block_ID].noise_strength);
    }

    if (block_pos.y >= CHUNK_SIZE - 1 || grid[block_pos.z][block_pos.y + 1][block_pos.x].height != 8)
    {
        drawtriangle(x + diameter / 2 - 1, y, diameter / 2, false, block_types[block_ID].left, block_types[block_ID].noise_strength);
        drawtriangle(x, y + diameter / 4, diameter / 2, true, block_types[block_ID].left, block_types[block_ID].noise_strength);
    }
}

void drawrectangle(int x, int y, int width, int height, pixel colour)
{
    int tempwidth = width;

    width += x;
    height += y;

    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }

    if (width > WIDTH)
    {
        width = WIDTH - 1;
    }
    if (height > HEIGHT)
    {
        height = HEIGHT - 1;
    }
    for (; y < height; y++)
    {
        for (; x < width; x++)
        {
            vram[x + y * WIDTH] = colour;
        }
        x -= tempwidth;
    }
}

unsigned char check_nearby_blocks(vector3 pos, unsigned int block_ID)
{
    int max_height = 0;

    if (pos.x < CHUNK_SIZE - 1 && grid[pos.z][pos.y][pos.x + 1].type == block_ID)
    {
        max_height = grid[pos.z][pos.y][pos.x + 1].height;
    }
    if (pos.y < CHUNK_SIZE - 1 && grid[pos.z][pos.y + 1][pos.x].type == block_ID)
    {
        max_height = get_maximum(grid[pos.z][pos.y + 1][pos.x].height, max_height);
    }
    if (pos.x != 0 && grid[pos.z][pos.y][pos.x - 1].type == block_ID)
    {
        max_height = get_maximum(grid[pos.z][pos.y][pos.x - 1].height, max_height);
    }
    if (pos.y != 0 && grid[pos.z][pos.y - 1][pos.x].type == block_ID)
    {
        max_height = get_maximum(grid[pos.z][pos.y - 1][pos.x].height, max_height);
    }
    return (max_height);
}

void grow_tree(vector3 pos)
{
    int length = 4 + tRand(2);
    for (int i = 0; i < length; i++)
    {
        grid[pos.z + i][pos.y][pos.x].type = WOOD;
    }
}

void init()
{
    cursor.x = CHUNK_SIZE - 1;
    cursor.y = CHUNK_SIZE - 1;
    cursor.z = CHUNK_HEIGHT / 3;
    for (int z = 0; z < CHUNK_HEIGHT; z++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int x = 0; x < CHUNK_SIZE; x++)
            {
                if (z == 0 || (z == 1 && tRand(5) <= 1))
                {
                    grid[z][y][x].type = BEDROCK;
                }
                else if (z < 8 || (z == 8 && noise(5, 5 * x >> y) <= 1))
                {
                    grid[z][y][x].type = STONE;
                }
                else if (grid[z - 3][y][x].type == STONE || grid[z - 3][y][x].type == BEDROCK)
                {
                    if (noise(10, x / 2 + y / 2) == 0)
                    {
                        grid[z][y][x].type = SAND;
                    }
                    else
                    {
                        grid[z][y][x].type = GRASS;
                    }
                }
                else if (grid[z - 1][y][x].type == GRASS && tRand(100) == 0)
                {
                    grow_tree((vector3) { x, y, z });
                }
                grid[z][y][x].height = block_types[grid[z][y][x].type].height;
            }
        }
    }
}

void block_handler(int z, int y, int x)
{
    block_status* current_block = &grid[z][y][x];

    if (current_block->type == WATER)
    {
        if (current_block->timer < 15)
        {
            current_block->timer++;
            return;
        }
        if (grid[z - 1][y][x].type == EMPTY || grid[z - 1][y][x].type == WATER)
        {
            grid[z - 1][y][x].type = WATER;
            grid[z - 1][y][x].height = 8;
            return;
        }
        if (current_block->height == 0)
        {
            return;
        }
        for (int i = 0; i < 4; i++)
        {
            int x_offset = 0, y_offset = 0;
            if (i & 2) {
                y_offset = i & 1 ? 1 : -1;
            }
            else {
                x_offset = i & 1 ? 1 : -1;
            }
            if (x + x_offset < 0 || x + x_offset > CHUNK_SIZE - 1 || y + y_offset < 0 || y + y_offset > CHUNK_SIZE - 1)
            {
                continue;
            }
            block_status* neighbour_block = &grid[z][y + y_offset][x + x_offset];
            if (neighbour_block->type == EMPTY || (neighbour_block->type == WATER && neighbour_block->height < current_block->height - 1))
            {
                neighbour_block->type = WATER;
                neighbour_block->height = current_block->height - 1;
            }
            current_block->timer = 0;
        }
        return;
    }
    if (z != 0 && current_block->type == SAND && (grid[z - 1][y][x].type == EMPTY || grid[z - 1][y][x].type == WATER))
    {
        if (current_block->timer < 30)
        {
            current_block->timer++;
            return;
        }
        grid[z - 1][y][x].type = SAND;
        current_block->type = EMPTY;
        current_block->timer = 0;
        grid[z - 1][y][x].height = 8;
    }
}

void physics()
{
    for (;;)
    {
        for (int z = 0; z < CHUNK_HEIGHT; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    block_handler(z, y, x);
                }
            }
        }
        Sleep(16);
    }
}

void draw()
{
    init();
    for (;;)
    {
        for (int z = 0; z < CHUNK_HEIGHT; z++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    if (grid[z][y][x].type == EMPTY)
                    {
                        continue;
                    }
                    if (grid[z][y][x].height == 8)
                    {
                        drawhexagon(WIDTH / 2 - (y - x) * 8, HEIGHT - 8 * CHUNK_HEIGHT - (x + y) * 4 + z * 8, 16, grid[z][y][x].type, (vector3) { x, y, z });
                        continue;
                    }
                    draw_uneven_hexagon(WIDTH / 2 - (y - x) * 8, HEIGHT - 8 * CHUNK_HEIGHT - (x + y) * 4 + z * 8, 16, grid[z][y][x].type, grid[z][y][x].height);
                }
            }
        }
        drawhexagon(WIDTH - 48, 48, 32, selected_block, (vector3) { CHUNK_SIZE, CHUNK_SIZE, CHUNK_HEIGHT });
        if (blink_timer % 80 < 40)
        {
            drawhexagon(WIDTH / 2 - (cursor.y - cursor.x) * 8, HEIGHT - 8 * CHUNK_HEIGHT - (cursor.x + cursor.y) * 4 + cursor.z * 8, 16, CURSOR, (vector3) { cursor.x, cursor.y, cursor.z });
        }
        StretchDIBits(WindowDC, 0, 0, screen_width, screen_heigth, 0, 0, WIDTH, HEIGHT, vram, &bmi, 0, SRCCOPY);
        memset(vram, 0, WIDTH * HEIGHT * sizeof(pixel));
        blink_timer++;
        Sleep(16);
    }
}

void main()
{
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_heigth = GetSystemMetrics(SM_CYSCREEN);

    ShowCursor(false);
    RegisterClassA(&window_class);
    window = CreateWindowExA(0, "class", "hello", WS_VISIBLE | WS_POPUP, 0, 0, screen_width, screen_heigth, 0, 0, window_class.hInstance, 0);
    MSG message;
    WindowDC = GetDC(window);
    global_seed = tRand(1000);
#pragma comment(lib,"winmm")
    timeBeginPeriod(1);
    CreateThread(0, 0, physics, 0, 0, 0);
    CreateThread(0, 0, draw, 0, 0, 0);

    while (GetMessageA(&message, window, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}