#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <string>
#include <raylib.h>


using std::string;
using std::unordered_map;


typedef int entityid;
typedef int textureid;
typedef enum { C_NAME, C_TYPE, C_POSITION, C_HITBOX, C_COUNT } component;
typedef enum { ENTITY_NONE, ENTITY_HERO, ENTITY_SWORD, ENTITY_ORC, ENTITY_COUNT } entity_type;
typedef enum { SCENE_COMPANY, SCENE_TITLE, SCENE_GAMEPLAY, SCENE_GAMEOVER, SCENE_COUNT } game_scene;


bool create_player();
bool set_pos(entityid id, Vector2 pos);
Vector2 get_pos(entityid id);
string comp2str(component c);
string entity_type2str(entity_type t);
string comp2str(component c);
void init_data();
void handle_input_company();
void handle_input_title();
void handle_input();
void handle_input_gameplay();
void draw_gameplay();


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
RenderTexture target_texture;
Rectangle target_src;
Rectangle target_dst;
Rectangle window_dst;
Vector2 origin;
Camera2D cam2d;
int frame_count = 0;
texture_info txinfo[32];
unordered_map<entityid, long> component_table;
unordered_map<entityid, string> names;
unordered_map<entityid, entity_type> types;
unordered_map<entityid, Vector2> positions;
unordered_map<entityid, Rectangle> hitboxes;
entityid next_entityid = 0;
const entityid ENTITYID_INVALID = -1;
entityid hero_id = ENTITYID_INVALID;
entityid sword_id = ENTITYID_INVALID;


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
        return "NONE";
    case ENTITY_HERO:
        return "HERO";
    default:
        return "UNKNOWN_TYPE";
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


bool update_y_pos(entityid id, float incr) {
    Vector2 pos = get_pos(id);
    pos.y += incr;
    set_pos(id, pos);
    return true;
}


bool update_x_pos(entityid id, float incr) {
    Vector2 pos = get_pos(id);
    pos.x += incr;
    set_pos(id, pos);
    return true;
}


bool update_xy_pos(entityid id, float incr_x, float incr_y) {
    Vector2 pos = get_pos(id);
    pos.x += incr_x;
    pos.y += incr_y;
    set_pos(id, pos);
    return true;
}


bool set_hitbox(entityid id, Rectangle rect) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_HITBOX);
    hitboxes[id] = rect;
    return true;
}


Rectangle get_hitbox(entityid id) {
    if (!has_comp(id, C_HITBOX)) return (Rectangle){-1, -1, -1, -1};
    auto it = hitboxes.find(id);
    if (it != hitboxes.end()) return it->second;
    return (Rectangle){-1, -1, -1, -1};
}


bool update_hitbox_x(entityid id, float incr) {
    Rectangle hitbox = get_hitbox(id);
    if (hitbox.x < 0 || hitbox.y < 0) return false; // Invalid hitbox
    hitbox.x += incr;
    set_hitbox(id, hitbox);
    return true;
}


bool update_hitbox_y(entityid id, float incr) {
    Rectangle hitbox = get_hitbox(id);
    if (hitbox.x < 0 || hitbox.y < 0) return false; // Invalid hitbox
    hitbox.y += incr;
    set_hitbox(id, hitbox);
    return true;
}


bool create_player() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "hero");
    set_type(id, ENTITY_HERO);
    float x = target_w / 16.0, y = target_h / 16.0;
    Vector2 v = {x, y};
    set_pos(id, v);
    Rectangle hitbox = {x, y, 7, 7};
    set_hitbox(id, hitbox);
    hero_id = id;
    return true;
}


bool create_sword() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "sword");
    set_type(id, ENTITY_SWORD);
    set_pos(id, (Vector2){-1, -1});
    set_hitbox(id, (Rectangle){-1, -1, -1, -1});
    sword_id = id;
    return true;
}


void handle_input_gameplay() {
    if (IsKeyDown(KEY_DOWN)) {
        update_y_pos(hero_id, 0.5);
        update_hitbox_y(hero_id, 0.5);
    }
    if (IsKeyDown(KEY_UP)) {
        update_y_pos(hero_id, -0.5);
        update_hitbox_y(hero_id, -0.5);
    }
    if (IsKeyDown(KEY_LEFT)) {
        update_x_pos(hero_id, -0.5);
        update_hitbox_x(hero_id, -0.5);
    }
    if (IsKeyDown(KEY_RIGHT)) {
        update_x_pos(hero_id, 0.5);
        update_hitbox_x(hero_id, 0.5);
    }
    if (IsKeyDown(KEY_SPACE)) {
        Vector2 pos = get_pos(hero_id);
        set_pos(sword_id, pos);
        Rectangle hb = get_hitbox(hero_id);
        hb.x = hb.x + hb.width;
        hb.y = hb.y + hb.height / 2.0f - 2;
        hb = {hb.x, hb.y, 8, 5};
        set_hitbox(sword_id, hb);
    } else if (IsKeyUp(KEY_SPACE)) {
        set_pos(sword_id, (Vector2){-1, -1});
        set_hitbox(sword_id, (Rectangle){-1, -1, -1, -1});
    }
}


void handle_input_company() {
    if (IsKeyPressed(KEY_ENTER)) {
        current_scene = SCENE_TITLE;
        debug_txt_color = BLACK;
    }
}


void handle_input_title() {
    if (IsKeyPressed(KEY_ENTER)) {
        current_scene = SCENE_GAMEPLAY;
        debug_txt_color = WHITE;
    }
}


void handle_input() {
    if (current_scene == SCENE_COMPANY)
        handle_input_company();
    else if (current_scene == SCENE_TITLE)
        handle_input_title();
    else if (current_scene == SCENE_GAMEPLAY)
        handle_input_gameplay();
}


void draw_debug_panel() {
    int x = 10, y = 10, s = 20;
    DrawText(TextFormat("Frame %d", frame_count), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("FPS: %d", GetFPS()), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("Cam.pos: %.1f,%.1f", cam2d.target.x, cam2d.target.y), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("Cam.offset: %.1f,%.1f", cam2d.offset.x, cam2d.offset.y), x, y, s, debug_txt_color);
    y += s;
    DrawText(TextFormat("Zoom: %.1f", cam2d.zoom), x, y, s, debug_txt_color);
    y += s;
    Vector2 p = get_pos(hero_id);
    DrawText(TextFormat("Hero.pos: %.1f,%.1f", p.x, p.y), x, y, s, debug_txt_color);
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
    BeginMode2D(cam2d);
    ClearBackground(BLACK);
    Color c = {0x33, 0x33, 0x33, 255};
    DrawRectangle(0, 0, target_w / 8, target_h / 8, c);
    for (auto it : component_table) {
        entityid id = it.first;
        if (!has_comp(id, C_POSITION)) continue;
        Vector2 pos = get_pos(id);
        if (pos.x < 0 || pos.y < 0) continue;
        entity_type type = get_type(id);
        //string name = get_name(id);
        if (type == ENTITY_HERO) {
            Rectangle src = {0, 0, 7, 7};
            Rectangle dst = {pos.x, pos.y, 7, 7};
            Vector2 origin = {0, 0};
            Rectangle hit_box = get_hitbox(id);
            DrawTexturePro(txinfo[0].texture, src, dst, origin, 0.0f, WHITE);
            //DrawRectangleLinesEx(dst, 1.0f, RED);
            //DrawRectangleLinesEx(hit_box, 1.0f, BLUE);
        } else if (type == ENTITY_SWORD) {
            Rectangle src = {0, 0, 8, 5};
            Vector2 origin = {0, 0};
            Rectangle hit_box = get_hitbox(id);
            Rectangle dst = {hit_box.x, hit_box.y, 8, 5};
            float rotation = 0.0f;
            DrawTexturePro(txinfo[1].texture, src, dst, origin, rotation, WHITE);
            //DrawRectangleLinesEx(hit_box, 1.0f, BLUE);
        }
    }
    EndMode2D();
}


void draw_frame() {
    if (IsWindowResized()) {
        window_dst.width = GetScreenWidth();
        window_dst.height = GetScreenHeight();
    }
    BeginDrawing();
    BeginTextureMode(target_texture);
    if (current_scene == SCENE_COMPANY)
        draw_company();
    else if (current_scene == SCENE_TITLE)
        draw_title();
    else if (current_scene == SCENE_GAMEPLAY)
        draw_gameplay();
    EndTextureMode();
    ClearBackground(BLACK);
    DrawTexturePro(target_texture.texture, target_src, window_dst, origin, 0.0f, WHITE);
    draw_debug_panel();
    EndDrawing();
    frame_count++;
}


void load_texture(int index, int rows, int cols, const char* path) {
    if (index < 0 || rows < 0 || cols < 0) return;
    txinfo[index].cols = cols;
    txinfo[index].rows = rows;
    txinfo[index].texture = LoadTexture(path);
}


void load_textures() {
    load_texture(0, 1, 1, "img/human.png");
    load_texture(1, 1, 1, "img/sword.png");
}


void unload_textures() {
    UnloadTexture(txinfo[0].texture);
    UnloadTexture(txinfo[1].texture);
    UnloadTexture(txinfo[2].texture);
}


void init_gfx() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(window_w, window_h, game_window_title);
    SetWindowMinSize(window_size_min_w, window_size_min_h);
    SetTargetFPS(target_fps);
    target_texture = LoadRenderTexture(target_w, target_h);
    target_src = {0, 0, 1.0f * target_w, -1.0f * target_h};
    target_dst = {0, 0, 1.0f * target_w, 1.0f * target_h};
    window_dst.x = window_dst.y = 0;
    window_dst.width = GetScreenWidth();
    window_dst.height = GetScreenHeight();
    cam2d.target = (Vector2){0, 0};
    cam2d.offset = (Vector2){-target_w / 2.0f, -target_h / 2.0f};
    cam2d.rotation = 0.0f;
    cam2d.zoom = default_zoom;
    load_textures();
}


void init_data() {
    if (!create_player() || !create_sword()) {
        fprintf(stderr, "Failed to create player or sword entity\n");
        exit(EXIT_FAILURE);
    }
}


int main() {
    init_gfx();
    init_data();
    while (!WindowShouldClose()) {
        handle_input();
        draw_frame();
    }
    unload_textures();
    UnloadRenderTexture(target_texture);
    CloseWindow();
    return 0;
}
