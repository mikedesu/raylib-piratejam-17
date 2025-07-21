#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>

#define SFX_CONFIRM 0
#define SFX_HIT 1
#define SFX_GET_HIT 2
#define SFX_EQUIP 3
#define SFX_COIN 4

using std::map;
using std::string;
using std::unordered_map;
using std::vector;

typedef int entityid;
typedef int textureid;
typedef enum { C_NAME, C_TYPE, C_POSITION, C_HITBOX, C_VELOCITY, C_COLLIDES, C_DESTROY, C_SRC, C_COUNT } component;
typedef enum { ENTITY_NONE, ENTITY_HERO, ENTITY_SWORD, ENTITY_ORC, ENTITY_COIN, ENTITY_COUNT } entity_type;
typedef enum { SCENE_COMPANY, SCENE_TITLE, SCENE_GAMEPLAY, SCENE_GAMEOVER, SCENE_COUNT } game_scene;
typedef enum {
    TX_HERO,
    TX_SWORD,
    TX_ORC,
    TX_GRASS_00,
    TX_GRASS_01,
    TX_GRASS_02,
    TX_GRASS_03,
    TX_COIN,
    TX_COUNT
} tx_index;


bool create_player();
bool create_orc();
bool create_sword();
bool create_coin(entityid id);

const char* game_window_title = "evildojo666 presents: There can be...";
int window_w = 1920;
int window_h = 1080;
int target_w = 800;
int target_h = 480;
int window_size_min_w = 320;
int window_size_min_h = 240;
int target_fps = 60;
float default_zoom = 8;
game_scene current_scene = SCENE_COMPANY;
Color debug_txt_color = WHITE;
RenderTexture target_texture;
Rectangle target_src;
Rectangle target_dst;
Rectangle window_dst;
const Vector2 origin = {0, 0};
Camera2D cam2d;
int frame_count = 0;
Texture2D txinfo[32];
bool gameover = false;
int player_dir = 1;
Sound sfx[32];
vector<entityid> cleanup;
map<entityid, long> component_table;
unordered_map<entityid, string> names;
unordered_map<entityid, entity_type> types;
unordered_map<entityid, Vector2> positions;
unordered_map<entityid, Rectangle> hitboxes;
unordered_map<entityid, Vector2> velocities;
unordered_map<entityid, bool> collides;
unordered_map<entityid, bool> destroy;
unordered_map<entityid, Rectangle> sources;
entityid next_entityid = 0;
const entityid ENTITYID_INVALID = -1;
entityid hero_id = ENTITYID_INVALID;
entityid sword_id = ENTITYID_INVALID;
bool player_attacking = false;
int hero_collision_counter = 0;
int sword_collision_counter = 0;
int entities_destroyed = 0;
int coins_collected = 0;

void init_data() {
    if (!create_player() || !create_sword()) {
        fprintf(stderr, "Failed to create player or sword entity\n");
        exit(EXIT_FAILURE);
    }
}

void cleanup_data() {
    // clear all component tables
    component_table.clear();
    names.clear();
    types.clear();
    positions.clear();
    hitboxes.clear();
    velocities.clear();
    collides.clear();
    destroy.clear();
    sources.clear();
    // reset entity ids
    next_entityid = 0;
    hero_id = ENTITYID_INVALID;
    sword_id = ENTITYID_INVALID;
    player_attacking = false;
    // reset counters
    hero_collision_counter = 0;
    sword_collision_counter = 0;
    entities_destroyed = 0;
    coins_collected = 0;
}

bool entity_exists(entityid id) {
    return component_table.find(id) != component_table.end();
}

entityid add_entity() {
    entityid id = next_entityid;
    if (entity_exists(id)) return ENTITYID_INVALID;
    component_table[id] = 0;
    next_entityid++;
    return id;
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

bool set_src(entityid id, Rectangle src) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_SRC);
    sources[id] = src;
    return true;
}

Rectangle get_src(entityid id) {
    if (!has_comp(id, C_SRC)) return (Rectangle){-1, -1, -1, -1};
    auto it = sources.find(id);
    if (it != sources.end()) return it->second;
    return (Rectangle){-1, -1, -1, -1}; // Return invalid rectangle if not found
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

bool set_velocity(entityid id, Vector2 v) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_VELOCITY);
    velocities[id] = v;
    return true;
}

Vector2 get_velocity(entityid id) {
    if (!has_comp(id, C_VELOCITY)) return origin;
    auto it = velocities.find(id);
    if (it != velocities.end()) return it->second;
    return origin;
}

bool set_collides(entityid id, bool c) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_COLLIDES);
    collides[id] = c;
    return true;
}

bool get_collides(entityid id) {
    if (!has_comp(id, C_COLLIDES)) return false;
    auto it = collides.find(id);
    if (it != collides.end()) return it->second;
    return false;
}

bool set_destroy(entityid id, bool c) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_DESTROY);
    destroy[id] = c;
    return true;
}

bool get_destroy(entityid id) {
    if (!has_comp(id, C_DESTROY)) return false;
    auto it = destroy.find(id);
    if (it != destroy.end()) return it->second;
    return false;
}

bool create_player() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "hero");
    set_type(id, ENTITY_HERO);
    Rectangle src = {0, 0, 7, 7};
    set_src(id, src);
    float w = txinfo[0].width * 1.0f;
    float h = txinfo[0].height * 1.0f;
    float x = target_w / 16.0 + w;
    float y = target_h / 8.0 - h / 2;
    Vector2 v = {x, y};
    set_pos(id, v);
    Rectangle hitbox = {x + 2, y + 1, w - 4, h - 1};
    set_hitbox(id, hitbox);
    set_collides(id, true);
    set_destroy(id, false);
    hero_id = id;
    return true;
}

bool create_sword() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "sword");
    set_type(id, ENTITY_SWORD);
    Rectangle src = {0, 0, 8, 5};
    set_src(id, src);
    set_pos(id, (Vector2){-1, -1});
    set_hitbox(id, (Rectangle){-1, -1, -1, -1});
    set_collides(id, true);
    set_destroy(id, false);
    sword_id = id;
    return true;
}

bool create_orc() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "orc");
    set_type(id, ENTITY_ORC);
    Rectangle src = {0, 0, -7, 7};
    set_src(id, src);
    float w = txinfo[0].width * 1.0f;
    float h = txinfo[0].height * 1.0f;
    // Select a random x,yf appropriate to the scene
    Vector2 p = get_pos(hero_id);
    p.x = 200;
    p.y = 57;
    set_pos(id, p);
    Rectangle hitbox = {p.x, p.y, w, h};
    set_hitbox(id, hitbox);
    // Set a random velocity to the orc
    float base_speed = -0.25f;
    int random_max = 3;
    set_velocity(id, (Vector2){base_speed * GetRandomValue(1, random_max), 0});
    set_collides(id, true);
    set_destroy(id, false);
    return true;
}

bool create_coin(entityid id) {
    // spawm a coin at the location of id
    if (!entity_exists(id)) return false;
    entityid coin_id = add_entity();
    if (coin_id == ENTITYID_INVALID) return false;
    set_name(coin_id, "coin");
    set_type(coin_id, ENTITY_COIN);
    Vector2 pos = get_pos(id);
    set_pos(coin_id, pos);
    Rectangle src = {0, 0, 4, 6};
    set_src(coin_id, src);
    Rectangle hitbox = {pos.x, pos.y, 4, 6};
    set_hitbox(coin_id, hitbox);
    set_collides(coin_id, true);
    set_destroy(coin_id, false);
    set_velocity(coin_id, (Vector2){-0.1f, 0});
    set_pos(coin_id, pos);
    return true;
}

void handle_input_gameplay() {
    if (IsKeyDown(KEY_LEFT)) {
        update_x_pos(hero_id, -0.5);
        update_hitbox_x(hero_id, -0.5);
        player_dir = -1;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        update_x_pos(hero_id, 0.5);
        update_hitbox_x(hero_id, 0.5);
        player_dir = 1;
    }
    if (IsKeyPressed(KEY_A)) {
        player_attacking = true;
        PlaySound(sfx[SFX_EQUIP]);
    } else if (IsKeyUp(KEY_A)) {
        player_attacking = false;
    }
}

void handle_input_company() {
    if (IsKeyPressed(KEY_ENTER)) {
        current_scene = SCENE_TITLE;
        debug_txt_color = BLACK;
        PlaySound(sfx[SFX_CONFIRM]);
    }
}

void handle_input_title() {
    if (IsKeyPressed(KEY_ENTER)) {
        current_scene = SCENE_GAMEPLAY;
        debug_txt_color = WHITE;
        gameover = false;
        PlaySound(sfx[SFX_CONFIRM]);
        init_data(); // reset data for new game
    }
}

void handle_input_gameover() {
    if (IsKeyPressed(KEY_ENTER)) {
        current_scene = SCENE_TITLE;
        debug_txt_color = BLACK;
        PlaySound(sfx[SFX_CONFIRM]);
        cleanup_data(); // reset data for new game
    }
}

void handle_input() {
    if (current_scene == SCENE_COMPANY)
        handle_input_company();
    else if (current_scene == SCENE_TITLE)
        handle_input_title();
    else if (current_scene == SCENE_GAMEPLAY)
        handle_input_gameplay();
    else if (current_scene == SCENE_GAMEOVER)
        handle_input_gameover();
}

void draw_debug_panel() {
    int x = 10, y = 10, s = 20;
    Color c = debug_txt_color;
    Vector2 p = get_pos(hero_id);
    DrawText(TextFormat("Frame %d", frame_count), x, y, s, c);
    y += s;
    DrawText(TextFormat("FPS: %d", GetFPS()), x, y, s, c);
    y += s;
    DrawText(TextFormat("Cam.pos: %.1f,%.1f", cam2d.target.x, cam2d.target.y), x, y, s, c);
    y += s;
    DrawText(TextFormat("Cam.offset: %.1f,%.1f", cam2d.offset.x, cam2d.offset.y), x, y, s, c);
    y += s;
    DrawText(TextFormat("Zoom: %.1f", cam2d.zoom), x, y, s, c);
    y += s;
    DrawText(TextFormat("Hero.pos: %.1f,%.1f", p.x, p.y), x, y, s, c);
    y += s;
    DrawText(TextFormat("Hero Collisions: %d", hero_collision_counter), x, y, s, c);
    y += s;
    DrawText(TextFormat("Sword Collisions: %d", sword_collision_counter), x, y, s, c);
    y += s;
    DrawText(TextFormat("Entities created: %d", next_entityid), x, y, s, c);
    y += s;
    DrawText(TextFormat("Entities destroyed: %d", entities_destroyed), x, y, s, c);
    y += s;
    DrawText(TextFormat("Coins: %d", coins_collected), x, y, s, c);
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

void draw_gameover() {
    ClearBackground(BLACK);
    int s = 20;
    const char* text = "gameover";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 2 - s;
    Color c = {0xFF, 0, 0, 255};
    DrawText(text, x, y, s, c);
}

void draw_gameplay() {
    BeginMode2D(cam2d);
    ClearBackground(BLACK);
    Color c = BLUE;
    float x = target_w / 16.0f;
    float y = target_h / 16.0f;
    float w = target_w / 8.0f;
    float h = target_h / 8.0f;
    DrawRectangle(x, y, w, h, c);
    Rectangle src = {0, 0, 8, 8};
    y += 32;
    for (int j = 0; j < 4; j++) {
        Rectangle dst = {x, y, 8, 8};
        for (int i = 0; i < 13; i++) {
            DrawTexturePro(txinfo[TX_GRASS_00], src, dst, origin, 0.0f, WHITE);
            dst.x += 8;
        }
        y += 8;
    }
    for (auto it : component_table) {
        entityid id = it.first;
        if (!has_comp(id, C_POSITION)) continue;
        Vector2 pos = get_pos(id);
        if (pos.x < 0 || pos.y < 0) continue;
        entity_type type = get_type(id);
        if (type == ENTITY_HERO) {
            //Rectangle src = {0, 0, player_dir * 7.0f, 7};
            Rectangle src = get_src(id);
            Rectangle dst = {pos.x, pos.y, src.width, src.height};
            //Rectangle hb = get_hitbox(id);
            DrawTexturePro(txinfo[TX_HERO], src, dst, origin, 0.0f, WHITE);
            //DrawRectangleLinesEx(hb, 1.0f, (Color){0xFF, 0, 0, 127});
        } else if (type == ENTITY_SWORD) {
            Rectangle src = get_src(id);
            Rectangle dst = {pos.x, pos.y, src.width, src.height};
            //Rectangle hb = get_hitbox(id);
            DrawTexturePro(txinfo[TX_SWORD], src, dst, origin, 0.0f, WHITE);
            //DrawRectangleLinesEx(hb, 1.0f, (Color){0xFF, 0, 0, 127});
        } else if (type == ENTITY_ORC) {
            Rectangle src = get_src(id);
            Rectangle dst = {pos.x, pos.y, src.width, src.height};
            //Rectangle hb = get_hitbox(id);
            DrawTexturePro(txinfo[TX_ORC], src, dst, origin, 0.0f, WHITE);
            //DrawRectangleLinesEx(hb, 1.0f, (Color){0xFF, 0, 0, 127});
        } else if (type == ENTITY_COIN) {
            //Rectangle src = {0, 0, -4, 6};
            Rectangle src = get_src(id);
            Rectangle dst = {pos.x, pos.y, src.width, src.height};
            //Rectangle hb = get_hitbox(id);
            DrawTexturePro(txinfo[TX_COIN], src, dst, origin, 0.0f, WHITE);
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
    else if (current_scene == SCENE_GAMEOVER)
        draw_gameover();
    EndTextureMode();
    ClearBackground(BLACK);
    DrawTexturePro(target_texture.texture, target_src, window_dst, origin, 0.0f, WHITE);
    draw_debug_panel();
    EndDrawing();
    frame_count++;
}

void load_texture(int index, const char* path) {
    txinfo[index] = LoadTexture(path);
}

void load_textures() {
    load_texture(0, "img/human.png");
    load_texture(1, "img/sword.png");
    load_texture(2, "img/orc.png");
    load_texture(3, "img/tiles/grass-00.png");
    load_texture(4, "img/tiles/grass-01.png");
    load_texture(5, "img/tiles/grass-02.png");
    load_texture(6, "img/tiles/grass-03.png");
    load_texture(7, "img/coin.png");
}

void unload_textures() {
    UnloadTexture(txinfo[0]);
    UnloadTexture(txinfo[1]);
    UnloadTexture(txinfo[2]);
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

void update_state_player_attack() {
    if (player_attacking) {
        Vector2 pos = get_pos(hero_id);
        Rectangle hb = get_hitbox(hero_id);
        hb.x = hb.x + hb.width;
        hb.y = hb.y + hb.height / 2.0f - 2;
        Vector2 spos = {hb.x, hb.y};
        hb = {hb.x, hb.y + 1, 8, 3};
        set_hitbox(sword_id, hb);
        set_pos(sword_id, spos);
    } else {
        set_pos(sword_id, (Vector2){-1, -1});
        set_hitbox(sword_id, (Rectangle){-1, -1, -1, -1});
    }
}

void update_state_velocity() {
    // velocity-position update
    for (auto row : component_table) {
        if (has_comp(row.first, C_VELOCITY)) {
            Vector2 p = get_pos(row.first), v = get_velocity(row.first);
            Rectangle hb = get_hitbox(row.first);
            p.x += v.x, p.y += v.y;
            set_pos(row.first, p);
            hb.x += v.x, hb.y += v.y;
            set_hitbox(row.first, hb);
            // if the p.x is < 0, mark for destroy. also this might be gameover if an enemy hits the left
            if (p.x <= 42) {
                set_destroy(row.first, true);
                // check entity type
                entity_type t = get_type(row.first);
                if (t == ENTITY_ORC) {
                    gameover = true;
                    current_scene = SCENE_GAMEOVER;
                    return;
                }
            }
        }
    }
}

void update_state_hero_collision() {
    // collision update
    Rectangle hb = get_hitbox(hero_id);
    bool result = false;
    // check for collision with hero
    for (auto row : component_table) {
        entity_type t = get_type(row.first);
        if (t == ENTITY_ORC && CheckCollisionRecs(hb, get_hitbox(row.first))) {
            hero_collision_counter++;
            set_destroy(row.first, true);
            PlaySound(sfx[SFX_GET_HIT]);
        }
        if (t == ENTITY_COIN && CheckCollisionRecs(hb, get_hitbox(row.first))) {
            hero_collision_counter++;
            set_destroy(row.first, true);
            // play a sound effect
            PlaySound(sfx[SFX_COIN]);
            coins_collected++;
        }
    }
}

void update_state_sword_collision() {
    // check for collision with sword
    Rectangle shb = get_hitbox(sword_id);
    if (player_attacking) {
        for (auto row : component_table) {
            if (get_type(row.first) == ENTITY_ORC && CheckCollisionRecs(shb, get_hitbox(row.first))) {
                sword_collision_counter++;
                set_destroy(row.first, true);
                player_attacking = false;
                PlaySound(sfx[SFX_HIT]);
                create_coin(row.first); // create a coin at the orc's position
            }
        }
    }
}

void update_state_destroy() {
    cleanup.clear();
    for (auto row : component_table)
        if (get_destroy(row.first)) cleanup.push_back(row.first);
    for (entityid id : cleanup) {
        remove_entity(id);
    }
    entities_destroyed += cleanup.size();
}

void update_state() {
    if (current_scene != SCENE_GAMEPLAY) return;
    // every N frames, create_orc
    int spawn_freq = 60;
    if (frame_count % spawn_freq == 0) {
        create_orc();
    }
    update_state_player_attack();
    update_state_velocity();
    update_state_hero_collision();
    update_state_sword_collision();
    // perform entity cleanup based on the values in the 'destroy' table
    update_state_destroy();
}

void load_soundfile(int index, const char* path) {
    sfx[index] = LoadSound(path);
    if (sfx[index].stream.buffer == NULL) {
        fprintf(stderr, "Failed to load sound: %s\n", path);
        exit(EXIT_FAILURE);
    }
}

void init_sound() {
    InitAudioDevice();
    load_soundfile(SFX_CONFIRM, "sfx/020_Confirm_10.wav");
    load_soundfile(SFX_HIT, "sfx/03_Hit_Wet.wav");
    load_soundfile(SFX_GET_HIT, "sfx/62_Get_hit_01.wav");
    load_soundfile(SFX_EQUIP, "sfx/061_Equip_01.wav");
    load_soundfile(SFX_COIN, "sfx/Coins.wav");
}

void unload_soundfile(int index) {
    if (sfx[index].stream.buffer != NULL)
        UnloadSound(sfx[index]);
    else
        fprintf(stderr, "Sound %d was not loaded or already unloaded\n", index);
}

int main() {
    init_gfx();
    init_sound();
    while (!WindowShouldClose()) {
        handle_input();
        update_state();
        draw_frame();
    }
    unload_textures();
    UnloadRenderTexture(target_texture);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
