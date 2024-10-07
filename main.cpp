#include <raylib.h>
#include <assert.h>
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <vector>
#include <math.h>


constexpr size_t WIN_W = 1500;
constexpr size_t WIN_H = 900;

constexpr size_t MAP_W = 16;
constexpr size_t MAP_H = 16;

constexpr size_t RECT_W = WIN_W / (MAP_W*2);
constexpr size_t RECT_H = WIN_H / MAP_H;

float player_x = 3.5;
float player_y = 2.3;
float player_a = PI;

constexpr float FOV = PI/2.7;
constexpr float RAY_DISTANCE = 20.0f;

struct TextureData {
    const size_t text_num;
    const size_t img_size;
    // const size_t w;
    // const size_t h;
    Image image;
};

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t unpack_color(const uint32_t &color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    r = (color >> 0) & 225;
    b = (color >> 8) & 225;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

void draw_rectangle(Image* img, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    uint32_t* img_data = (uint32_t*) img->data;

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            size_t cx = x + i;
            size_t cy = y + j;

            if (cx < WIN_W && cy < WIN_H) {
                img_data[cx + cy * WIN_W] = color;
            }
        }
    }
}

void input() {
    if (IsKeyDown(KEY_Z)) {
        player_a += 0.05;
        if (player_a > 2 * PI) {
            player_a -= 2 * PI;
        }
    }

    if (IsKeyDown(KEY_W)) {
        player_y -= 0.05;
    }
    if (IsKeyDown(KEY_D)) {
        player_x += 0.05;
    }
    if (IsKeyDown(KEY_S)) {
        player_y += 0.05;
    }
    if (IsKeyDown(KEY_A)) {
        player_x -= 0.05;
    }
}

std::vector<uint32_t> texture_col(
    uint32_t* text_img_data,
    const size_t textsize,
    const size_t num_textures,
    const size_t textid,
    const size_t textcord,
    const size_t col_h
) {
    const size_t img_w = textsize * num_textures;
    const size_t img_h = textsize;
    std::vector<uint32_t> column(col_h);

    // ensure textcord and col_h are within valid bounds
    size_t pix_x = textid * textsize + textcord;
    if (pix_x >= img_w) {
        pix_x = img_w - 1; // clamp to valid range
        std::cout << "clamped";
    }

    for (size_t y = 0; y < col_h; y++) {
        size_t pix_y = (y * img_h) / col_h;  // corrected calculation for pix_y

        if (pix_y >= img_h) {
            pix_y = img_h - 1; // clamp to valid range
        }

        column[y] = text_img_data[pix_x + pix_y * img_w];
    }

    return column;
}

void calculate_rays(Image *img, const char* map, TextureData* walltext) {
    int num_rays = 512;
    float angle_step = FOV / (num_rays - 1);
    uint32_t* img_data = (uint32_t*) img->data;
    uint32_t* walltext_data = (uint32_t*) walltext->image.data;
    size_t text_width = walltext->image.width / walltext->text_num;
    // size_t single_text_size = (walltext->image.width * walltext->image.height) / walltext->text_num;

    for (size_t i = 0; i < num_rays; i++) {
        float ray_angle = player_a - (FOV / 2) + i * angle_step;

        for (float t = 0.0f; t < RAY_DISTANCE; t += 0.05f) {
            float ray_x = player_x + t * cos(ray_angle);
            float ray_y = player_y + t * sin(ray_angle);

            int map_x = static_cast<int>(ray_x);
            int map_y = static_cast<int>(ray_y);

            int pix_x = static_cast<int>(RECT_W * ray_x);
            int pix_y = static_cast<int>(RECT_H * ray_y);

            if (pix_x < 0 || pix_x >= WIN_W || pix_y < 0 || pix_y >= WIN_H) break;
            // if (map_x < 0 || map_x >= MAP_W || map_y < 0 || map_y >= MAP_H) break;

            if (map[map_x + map_y * MAP_W] != ' ') {
                size_t texid = map[map_x + map_y * MAP_W] - '0';
                assert(texid < walltext->text_num);

                size_t col_h = WIN_H / (t * cos(ray_angle - player_a));

                // Ensure col_h is within reasonable bounds
                if (col_h > WIN_H) {
                    col_h = WIN_H;
                }

                // Calculate texture coordinate
                float hitx = ray_x - floor(ray_x + 0.5f);
                float hity = ray_y - floor(ray_y + 0.5f);
                int x_textcord = std::abs(hitx) * text_width;

                if (std::abs(hity) > std::abs(hitx)) {
                    x_textcord = std::abs(hity) * text_width;
                }

                // Ensure texture coordinate is within bounds
                x_textcord = std::max(0, std::min(static_cast<int>(text_width) - 1, x_textcord));

                std::vector<uint32_t> col = texture_col(
                    walltext_data,
                    text_width,
                    walltext->text_num,
                    texid,
                    x_textcord,
                    col_h
                );

                pix_x = WIN_W / 2 + i;
                for (size_t j = 0; j < col_h; j++) {
                    pix_y = j + WIN_W / 2 - col_h / 2 - 150;
                    if (pix_y < 0 || pix_y >= (int)WIN_H) continue;
                    img_data[pix_x + pix_y * WIN_W] = col[j];
                }
                break;
            }

            img_data[pix_x + pix_y * WIN_W] = pack_color(15, 255, 0);
        }
    }
}

void update_image(
    const char *map,
    Image* img,
    Texture2D* img_text,
    std::vector<TextureData> *textures
    ) {
    // draw map objects
    for (size_t j = 0; j < MAP_H; j++) {
        for (size_t i = 0; i < MAP_W; i++) {
            if (map[i+j*MAP_W] == ' ') {
                draw_rectangle(img ,i * RECT_W, j * RECT_H, RECT_W, RECT_H, pack_color(255, 255, 255));
            } else {
                draw_rectangle(img, i * RECT_W, j * RECT_H, RECT_W, RECT_H, pack_color(100, 100, 0));
            }
        }
    }
    // player
    draw_rectangle(img, player_x * RECT_W, player_y * RECT_H, 5, 5, pack_color(255, 0, 255));
    // draw fov cone
    calculate_rays(img, map, &(*textures)[0]);
    // apply updates
    UpdateTexture(*img_text, img->data);
}

int main()
{
    InitWindow(WIN_W, WIN_H, "beta");
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   3   11100  0"\
                       "5   4   0      0"\
                       "5   4   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000";

    std::array<uint32_t, WIN_H * WIN_W> image_map_arr = [] {
        std::array<uint32_t, WIN_H * WIN_W> arr = {};

        for (size_t i = 0; i < WIN_W; i++) {
            for (size_t j = 0; j < WIN_H; j++) {
                arr[j * WIN_W + i] = pack_color(255, 255, 255);
            }
        }

        return arr;
    }();

    std::vector<uint32_t> image_map_vec(image_map_arr.begin(), image_map_arr.end());

    Image image_map = {0};
    image_map.width = WIN_W;
    image_map.height = WIN_H;
    image_map.mipmaps = 1;
    image_map.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image_map.data = image_map_vec.data();

    uint32_t* img_data = (uint32_t*) image_map.data;

    // load image buffer
    Texture2D img_texture = LoadTextureFromImage(image_map);
    draw_rectangle(&image_map, 50, 50, 50, 50, pack_color(255, 0, 255));
    UpdateTexture(img_texture, image_map.data);

    // load wall textures
    std::vector<TextureData> textures;
    Image walltext_img = LoadImageFromTexture(LoadTexture("walltext.png"));
    textures.push_back(
        TextureData{
            6,
            size_t(walltext_img.height * walltext_img.width),
            // size_t(walltext_img.width), size_t(walltext_img.height),
            walltext_img
        });
    // textures.push_back(LoadImageFromTexture(LoadTexture("monsters.png")));

    TextureData* walltext = &textures[0];
    uint32_t* walltext_data = (uint32_t*)walltext->image.data;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        input();

        // clear right side screen after the rays have been changed due to input
        for (int i = WIN_W / 2; i < WIN_W; i++) {
            for (int j = 0; j < WIN_H; j++) {
                img_data[i + j * WIN_W] = pack_color(255, 255, 255);
            }
        }

        update_image(map, &image_map, &img_texture, &textures);

        // size_t tex_idx = 4;
        // int texture_width = walltext_img.width / walltext->text_num;
        // int texture_height = walltext_img.height;
        // for (int i = 0; i < texture_width && i < WIN_W; i++) {
        //     for (int j = 0; j < texture_height && j < WIN_H; j++) {
        //         size_t idx = (tex_idx * texture_width) + i + j * walltext_img.width;
        //         img_data[i + j * WIN_W] = walltext_data[idx];
        //     }
        // }

        UpdateTexture(img_texture, image_map.data);

        DrawTexture(img_texture, 0, 0, WHITE);
        EndDrawing();
    }

    image_map_vec.clear();
    UnloadTexture(img_texture);
    UnloadImage(walltext_img);
}
