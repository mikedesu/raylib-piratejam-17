#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <raylib.h>


using std::string;
using std::unordered_map;


typedef int entityid;
typedef int textureid;


typedef enum { C_NAME, C_TYPE, C_POSITION, C_COUNT } component;


typedef enum { ENTITY_NONE, ENTITY_HERO, ENTITY_COUNT } entity_type;


typedef enum { SCENE_COMPANY, SCENE_TITLE, SCENE_GAMEPLAY, SCENE_GAMEOVER, SCENE_COUNT } game_scene;


const char* game_window_title = "evildojo666 presents: gamejam 2025";
int window_w = 1280;
int window_h = 720;
int target_w = 800;
int target_h = 480;
int window_size_min_w = 320;
int window_size_min_h = 240;
int target_fps = 60;
float default_zoom = 8;
game_scene current_scene = SCENE_COMPANY;
Color debug_txt_color = WHITE;


typedef struct {
    int rows;
    int cols;
    Texture2D texture;
} texture_info;


typedef struct {
    int row;
    int rows;
    int col;
    int cols;
    Texture2D* texture;
} sprite;


RenderTexture target_texture;
Rectangle target_src;
Rectangle target_dst;
Rectangle window_dst;
Vector2 origin;
Camera2D cam2d;
int frame_count = 0;

texture_info txinfo[32];
unordered_map<entityid, sprite> sprites;


unordered_map<entityid, long> component_table;
unordered_map<entityid, string> names;
unordered_map<entityid, entity_type> types;
unordered_map<entityid, Vector2> positions;

entityid next_entityid = 0;
const entityid ENTITYID_INVALID = -1;


string comp2str(component c) {
    switch (c) {
    case C_NAME:
        return "C_NAME";
    case C_TYPE:
        return "C_TYPE";
    case C_POSITION:
        return "C_POSITION";
    case C_COUNT:
        return "C_COUNT";
    default:
        return "UNKNOWN_COMPONENT";
    }
}


string entity_type2str(entity_type t) {
    switch (t) {
    case ENTITY_NONE:
        return "ENTITY_NONE";
    default:
        return "UNKNOWN_ENTITY_TYPE";
    }
}


string game_scene2str(game_scene s) {
    switch (s) {
    case SCENE_COMPANY:
        return "COMPANY";
    case SCENE_TITLE:
        return "TITLE";
    case SCENE_GAMEPLAY:
        return "GAMEPLAY";
    case SCENE_GAMEOVER:
        return "GAMEOVER";
    default:
        return "UNKNOWN_SCENE";
    }
}


entityid add_entity() {
    entityid id = next_entityid;
    if (component_table.find(id) != component_table.end()) return ENTITYID_INVALID;
    component_table[id] = 0; // Initialize with no components
    next_entityid++;
    return id;
}


bool entity_exists(entityid id) {
    return component_table.find(id) != component_table.end();
}


bool remove_entity(entityid id) {
    auto it = component_table.find(id);
    if (it != component_table.end()) {
        component_table.erase(it);
        return true;
    }
    return false;
}


bool set_comp(entityid id, component c) {
    if (!entity_exists(id)) return false;
    if (c < 0 || c >= C_COUNT) return false;
    component_table[id] |= (1L << c); // Set the component bit
    return true;
}


bool has_comp(entityid id, component c) {
    if (!entity_exists(id)) return false;
    if (c < 0 || c >= C_COUNT) return false;
    return (component_table[id] & (1L << c)) != 0; // Check if the component bit is set
}


bool set_name(entityid id, string name) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_NAME);
    names[id] = name;
    return true;
}


string get_name(entityid id) {
    if (!has_comp(id, C_NAME)) return "no-name";
    auto it = names.find(id);
    if (it != names.end()) return it->second;
    return "no-name"; // Return empty string if name not found
}


bool set_type(entityid id, entity_type t) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_TYPE);
    types[id] = t;
    return true;
}


entity_type get_type(entityid id) {
    if (!has_comp(id, C_TYPE)) return ENTITY_NONE;
    auto it = types.find(id);
    if (it != types.end()) return it->second;
    return ENTITY_NONE; // Return default type if not found
}


bool set_pos(entityid id, Vector2 pos) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_POSITION);
    positions[id] = pos;
    return true;
}


Vector2 get_pos(entityid id) {
    if (!has_comp(id, C_POSITION)) return (Vector2){-1, -1};
    auto it = positions.find(id);
    if (it != positions.end()) return it->second;
    return (Vector2){-1, -1};
}


void handle_input_gameplay() {
    if (IsKeyDown(KEY_Z)) cam2d.zoom += 0.1f;
    if (IsKeyDown(KEY_X)) cam2d.zoom -= 0.1f;

    //if (IsKeyPressed(KEY_C)) {
    // create a new entity
    //    entityid id = add_entity();
    //    set_name(id, "darkmage");
    //    set_type(id, ENTITY_NONE);
    //
    //        float x = GetRandomValue(10, target_w / 8);
    //        float y = GetRandomValue(10, target_h / 8);
    //        Vector2 v = {x, y};
    //        set_pos(id, v);
    //    }
}


void handle_input() {
    if (current_scene == SCENE_COMPANY) {
        if (IsKeyPressed(KEY_ENTER)) {
            current_scene = SCENE_TITLE;
            debug_txt_color = BLACK;
        }
    } else if (current_scene == SCENE_TITLE) {
        if (IsKeyPressed(KEY_ENTER)) {
            current_scene = SCENE_GAMEPLAY;
            debug_txt_color = BLACK;
        }
    } else if (current_scene == SCENE_GAMEPLAY) {
        handle_input_gameplay();
    }
}


void draw_debug_panel() {
    int x = 10;
    int y = 10;
    int s = 20;
    DrawText(TextFormat("Frame %d", frame_count), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("FPS: %d", GetFPS()), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("Zoom: %.1f", cam2d.zoom), x, y, s, debug_txt_color);
}


void draw_company() {
    ClearBackground(BLACK);
    int s = 20;
    const char* text = "evildojo666";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 2 - s;
    Color c = {0x66, 0x66, 0x66, 255};
    DrawText(text, x, y, s, c);
}


void draw_title() {
    ClearBackground(WHITE);
    int s = 30;
    const char* text = "There can be...";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 2 - s;
    DrawText(text, x, y, s, BLACK);
}


void draw_gameplay() {
    ClearBackground(WHITE);
}


void draw_frame() {
    if (IsWindowResized()) {
        window_dst.width = GetScreenWidth();
        window_dst.height = GetScreenHeight();
    }

    BeginDrawing();
    BeginTextureMode(target_texture);

    //BeginMode2D(cam2d);
    //ClearBackground(BLACK);
    //DrawRectangle(0, 0, target_w / 8, target_h / 8, RED);
    //DrawText(TextFormat("entity count: %ld\n", component_table.size()), 0, 10 + 10, 10, (Color){0xFF, 0xFF, 0xFF, 255});
    //for (auto id : component_table) {
    //if (has_comp(id.first, C_POSITION)) {
    //    Vector2 pos = get_pos(id.first);
    //    string name = get_name(id.first);
    //    //DrawRectangle(pos.x, pos.y, 2, 2, WHITE);
    //    DrawText(name.c_str(), pos.x, pos.y + 8, 10, WHITE);
    //
    //            DrawTexturePro(txinfo[0].texture,
    //                           (Rectangle){0, 0, 32, 32},
    //                           (Rectangle){pos.x, pos.y, 32, 32},
    //                           (Vector2){0, 0},
    //                           0,
    //                           WHITE);
    //        }
    //}
    //EndMode2D();

    if (current_scene == SCENE_COMPANY) {
        draw_company();
    } else if (current_scene == SCENE_TITLE) {
        draw_title();
    } else if (current_scene == SCENE_GAMEPLAY) {
        draw_gameplay();
    }

    EndTextureMode();

    ClearBackground(BLACK);
    DrawTexturePro(target_texture.texture, target_src, window_dst, origin, 0.0f, WHITE);


    draw_debug_panel();
    EndDrawing();
    frame_count++;
}


void load_texture(int index, int rows, int cols, const char* path) {
    txinfo[index].cols = cols;
    txinfo[index].rows = rows;
    txinfo[index].texture = LoadTexture(path);
}


void load_textures() {
    //load_texture(0, 4, 16, "img/human_idle.png");
}


void unload_textures() {
    //UnloadTexture(txinfo[0].texture);
}


void init_gfx() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(window_w, window_h, game_window_title);
    SetWindowMinSize(window_size_min_w, window_size_min_h);
    SetTargetFPS(target_fps);
    target_texture = LoadRenderTexture(target_w, target_h);
    target_src = {0, 0, 1.0f * target_w, -1.0f * target_h};
    target_dst = {0, 0, 1.0f * target_w, 1.0f * target_h};
    window_dst.x = 0;
    window_dst.y = 0;
    window_dst.width = GetScreenWidth();
    window_dst.height = GetScreenHeight();
    cam2d.target = (Vector2){0, 0};
    cam2d.offset = (Vector2){0, 0};
    cam2d.rotation = 0.0f;
    cam2d.zoom = default_zoom;

    load_textures();
}


int main() {
    init_gfx();
    while (!WindowShouldClose()) {
        handle_input();
        draw_frame();
    }
    unload_textures();
    UnloadRenderTexture(target_texture);
    CloseWindow();
    return 0;
}
