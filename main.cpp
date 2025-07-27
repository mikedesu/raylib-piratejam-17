#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>

#define ORC_SPAWN_X_RIGHT 150
#define ORC_SPAWN_X_LEFT 42
#define ORC_SPAWN_Y 57

#define DEFAULT_ORCS_TO_CREATE 1

#define HERO_VELO_X_DEFAULT 0.25f
#define HERO_VELO_Y_DEFAULT 0.25f
#define BASE_ORC_SPEED -0.20f
#define RANDOM_ORC_SPEED_MOD_MAX 2

#define SH_RED_GLOW 0
#define SH_INVERT 1
#define SH_INTENSE_RED_GLOW 2
#define SH_BLACK_GLOW 3
#define SH_HP_RED_GLOW 4
#define SH_BLUE_GLOW 5
#define SH_DURABILITY_BLACK 6
#define NUM_SHADERS 7


#define NUM_TEXTURES 32
#define NUM_SFX 32

#define TX_HERO 0
#define TX_SWORD 1
#define TX_ORC 2
#define TX_GRASS_00 3
#define TX_GRASS_01 4
#define TX_GRASS_02 5
#define TX_GRASS_03 6
#define TX_COIN 7
#define TX_SWORD_UP 8
#define TX_WILD_ORC 9
#define TX_DWARF_MERCHANT 10
#define TX_BOOTS 11
#define TX_HEALTH_REPLENISH 12
#define TX_HEALTH_EXPANSION 13
#define TX_COUNT 14

#define SFX_CONFIRM 0
#define SFX_HIT 1
#define SFX_GET_HIT 2
#define SFX_EQUIP 3
#define SFX_COIN 4

#define DEFAULT_SPAWN_FREQ 300

#define MERCHANT_ITEM_SELECTION_MAX 3

#define DEFAULT_SPAWN_FREQ_INCR 30.0f

using std::map;
using std::string;
using std::unordered_map;
using std::vector;

typedef int entityid;
typedef int textureid;

typedef enum
{
    C_NAME,
    C_TYPE,
    C_POSITION,
    C_HITBOX,
    C_VELOCITY,
    C_COLLIDES,
    C_DESTROY,
    C_SRC,
    C_HP,
    C_DURABILITY,
    C_DIRECTION,
    C_SIZE,
    C_COUNT
} component;

typedef enum
{
    ENTITY_NONE,
    ENTITY_HERO,
    ENTITY_SWORD,
    ENTITY_ORC,
    ENTITY_COIN,
    ENTITY_DWARF_MERCHANT,
    ENTITY_HEALTH_REPLENISH,
    ENTITY_COUNT
} entity_type;

typedef enum
{
    ITEM_SWORD,
    ITEM_SWORD_SIZE,
    ITEM_BOOTS,
    ITEM_HEALTH_EXPANSION,
    ITEM_COUNT
} item_type;

typedef enum
{
    SCENE_COMPANY,
    SCENE_TITLE,
    SCENE_GAMEPLAY,
    SCENE_GAMEOVER,
    SCENE_MERCHANT,
    SCENE_COUNT
} game_scene;

bool create_player();
bool create_orc();
bool create_sword();
bool create_coin(entityid id);
bool create_health_replenish(entityid id);
bool create_dwarf_merchant();
void randomize_merchant_items();

const char* game_window_title = "evildojo666 presents: There can be...";
int window_w = 1920;
int window_h = 1080;
int target_w = 800;
int target_h = 480;
int window_size_min_w = 320;
int window_size_min_h = 240;
int target_fps = 60;

float default_zoom = 8;
//float default_zoom = 4;

game_scene current_scene = SCENE_COMPANY;
Color debug_txt_color = WHITE;
RenderTexture target_texture;
RenderTexture company_texture;
RenderTexture title_texture;
RenderTexture gameover_texture;
Rectangle target_src;
Rectangle target_dst;
Rectangle window_dst;
const Vector2 origin = {0, 0};
Camera2D cam2d;
int frame_count = 0;
int frame_updates = 0;
bool do_frame_update = false;


int spawn_freq = DEFAULT_SPAWN_FREQ;
int spawn_freq_incr = DEFAULT_SPAWN_FREQ_INCR;

float base_orc_speed = BASE_ORC_SPEED;
float current_orc_speed = BASE_ORC_SPEED;
int random_orc_speed_mod_max = RANDOM_ORC_SPEED_MOD_MAX;
int num_orcs_to_create = DEFAULT_ORCS_TO_CREATE;

int player_level = 1;
bool levelup_flag = false;
int base_coin_level_up_amount = 5;
int merchant_item_selection = 0;

bool do_spawn_merchant = false;
bool merchant_spawned = false;

Texture2D txinfo[NUM_TEXTURES];
bool gameover = false;

//int player_dir = 1;

Sound sfx[NUM_SFX];
Music music = {0};
vector<entityid> cleanup;
map<entityid, long> component_table;
unordered_map<entityid, string> names;
unordered_map<entityid, entity_type> types;
unordered_map<entityid, Vector2> positions;
unordered_map<entityid, Rectangle> hitboxes;
unordered_map<entityid, Vector2> velocities;
unordered_map<entityid, Vector2> directions;
unordered_map<entityid, bool> collides;
unordered_map<entityid, bool> destroy;
unordered_map<entityid, Rectangle> sources;
unordered_map<entityid, Vector2> hp;
unordered_map<entityid, Vector2> durability;
unordered_map<entityid, float> sizes;
entityid next_entityid = 0;
const entityid ENTITYID_INVALID = -1;
entityid hero_id = ENTITYID_INVALID;
entityid sword_id = ENTITYID_INVALID;
bool player_attacking = false;
int hero_collision_counter = 0;
int sword_collision_counter = 0;
int entities_destroyed = 0;

int enemies_killed = 0;
int total_enemies_killed = 0;
int enemies_spawned = 0;
int enemies_missed = 0;

int current_coins = 0;
int coins_spent = 0;
int coins_collected = 0;
int coins_spawned = 0;
int coins_lost = 0;
int hero_total_damage_received = 0;
int continues = 0;
float starting_sword_durability = 1.0f;
float current_sword_durability = 1.0f;

item_type merchant_items[MERCHANT_ITEM_SELECTION_MAX] = {
    ITEM_SWORD, ITEM_HEALTH_EXPANSION, ITEM_BOOTS};


Shader shaders[NUM_SHADERS];


void init_data() {
    if (!create_player() || !create_sword()) {
        fprintf(stderr, "Failed to create player or sword entity\n");
        exit(EXIT_FAILURE);
    }
}


void load_shader(int index, const char* path) {
    //shaders[index] = LoadShader(0, path);
    shaders[index] = LoadShader(0, TextFormat("shaders/%s.frag", path));
    if (shaders[index].id == 0) {
        fprintf(stderr, "Failed to load shader: %s\n", path);
        exit(EXIT_FAILURE);
    }
}

void load_shaders() {
    load_shader(SH_RED_GLOW, "red-glow");
    load_shader(SH_INTENSE_RED_GLOW, "intense-red-glow");
    load_shader(SH_INVERT, "invert");
    load_shader(SH_BLACK_GLOW, "black-glow");
    load_shader(SH_HP_RED_GLOW, "hp-red-glow");
    load_shader(SH_BLUE_GLOW, "blue-glow");
    load_shader(SH_DURABILITY_BLACK, "durability-black");
}

void unload_shaders() {
    for (int i = 0; i < NUM_SHADERS; i++) {
        if (shaders[i].id != 0) {
            UnloadShader(shaders[i]);
        }
    }
}


void randomize_merchant_items() {
    vector<item_type> items;
    items.push_back(ITEM_BOOTS);
    items.push_back(ITEM_HEALTH_EXPANSION);
    items.push_back(ITEM_SWORD);
    items.push_back(ITEM_SWORD_SIZE);

    int i = 0;

    while (i < 3) {
        // select a random index from items
        int r_index = GetRandomValue(0, items.size() - 1);
        merchant_items[i] = items[r_index];
        // remove the item at r_index
        items.erase(items.begin() + r_index);
        i++;
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
    hp.clear();
    durability.clear();
    directions.clear();
    sizes.clear();
    // reset entity ids
    hero_id = ENTITYID_INVALID;
    sword_id = ENTITYID_INVALID;
    player_attacking = false;
    // reset counters
    next_entityid = 0;
    hero_collision_counter = 0;
    sword_collision_counter = 0;
    entities_destroyed = 0;
    coins_collected = 0;
    coins_spent = 0;
    current_coins = 0;
    coins_spawned = 0;
    coins_lost = 0;
    hero_total_damage_received = 0;
    player_level = 1;
    do_spawn_merchant = false;
    merchant_spawned = false;
    current_orc_speed = base_orc_speed;
    // reset game state
    gameover = false;
    spawn_freq = DEFAULT_SPAWN_FREQ;
    spawn_freq_incr = DEFAULT_SPAWN_FREQ_INCR;
    current_sword_durability = starting_sword_durability;
    enemies_killed = 0;
    base_orc_speed = BASE_ORC_SPEED;
    num_orcs_to_create = DEFAULT_ORCS_TO_CREATE;
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
    return (component_table[id] & (1L << c)) !=
           0; // Check if the component bit is set
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

bool set_hp(entityid id, Vector2 myhp) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_HP);
    hp[id] = myhp;
    return true;
}

Vector2 get_hp(entityid id) {
    if (!has_comp(id, C_HP)) return (Vector2){-1, -1};
    auto it = hp.find(id);
    if (it != hp.end()) return it->second;
    return (Vector2){-1, -1}; // Return invalid hp if not found
}

bool set_durability(entityid id, Vector2 dura) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_DURABILITY);
    durability[id] = dura;
    return true;
}

Vector2 get_durability(entityid id) {
    if (!has_comp(id, C_DURABILITY)) return (Vector2){-1, -1};
    auto it = durability.find(id);
    if (it != durability.end()) return it->second;
    return (Vector2){-1, -1}; // Return invalid durability if not found
}

bool set_dir(entityid id, Vector2 dir) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_DIRECTION);
    directions[id] = dir;
    return true;
}

Vector2 get_dir(entityid id) {
    if (!has_comp(id, C_DIRECTION)) return (Vector2){-1, -1};
    auto it = directions.find(id);
    if (it != directions.end()) return it->second;
    return (Vector2){-1, -1}; // Return invalid direction if not found
}

bool set_size(entityid id, float sz) {
    if (!entity_exists(id)) return false;
    set_comp(id, C_SIZE);
    sizes[id] = sz;
    return true;
}

float get_size(entityid id) {
    if (!has_comp(id, C_SIZE)) return -1.0f; // Invalid size
    auto it = sizes.find(id);
    if (it != sizes.end()) return it->second;
    return -1.0f; // Return invalid size if not found
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
    set_velocity(id, (Vector2){HERO_VELO_X_DEFAULT, HERO_VELO_Y_DEFAULT});
    set_hp(id, (Vector2){3.0f, 3.0f});
    set_dir(id, (Vector2){1.0f, 0.0f}); // facing right by default
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
    set_durability(
        id, (Vector2){starting_sword_durability, starting_sword_durability});
    set_dir(id, (Vector2){1.0f, 0.0f});
    set_size(id, 1.0f); // default size
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
    // Select a random x,y appropriate to the scene based on randomly-chosen direction to start
    int side = GetRandomValue(0, 1);
    if (side == 0) {
        side = -1;
    }

    float xpos = ORC_SPAWN_X_RIGHT;
    float ypos = ORC_SPAWN_Y;
    if (side == -1) {
        xpos = ORC_SPAWN_X_LEFT;
    }

    Vector2 p = {xpos, ypos};

    int random_y = 0;

    //random_y = GetRandomValue(-1, 1);
    random_y = GetRandomValue(-2, 2);
    p.y += random_y * h;

    set_pos(id, p);
    Rectangle hitbox = {p.x, p.y, w, h};
    set_hitbox(id, hitbox);
    // Set a random velocity to the orc
    set_velocity(id,
                 (Vector2){side * current_orc_speed *
                               GetRandomValue(1, random_orc_speed_mod_max),
                           0});
    set_collides(id, true);
    set_destroy(id, false);
    set_hp(id, (Vector2){1.0f, 1.0f});
    enemies_spawned++;
    return true;
}


bool create_dwarf_merchant() {
    entityid id = add_entity();
    if (id == ENTITYID_INVALID) return false;
    set_name(id, "dwarf merchant");
    set_type(id, ENTITY_DWARF_MERCHANT);
    Rectangle src = {0, 0, -9, 7};
    set_src(id, src);
    float w = txinfo[0].width * 1.0f;
    float h = txinfo[0].height * 1.0f;
    // Select a random x,yf appropriate to the scene
    Vector2 p = {ORC_SPAWN_X_RIGHT, ORC_SPAWN_Y};
    int random_y = 0;
    random_y = GetRandomValue(-1, 1);
    p.y += random_y * h;
    set_pos(id, p);
    Rectangle hitbox = {p.x, p.y, w, h};
    set_hitbox(id, hitbox);
    set_velocity(id, (Vector2){-0.25f, 0});
    set_collides(id, true);
    set_destroy(id, false);
    set_hp(id, (Vector2){1.0f, 1.0f});
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
    Vector2 velo = get_velocity(id);
    if (velo.x < 0) {
        set_velocity(coin_id, (Vector2){-0.1f, 0});
    } else {
        set_velocity(coin_id, (Vector2){0.1f, 0});
    }
    set_pos(coin_id, pos);
    coins_spawned++;
    return true;
}

bool create_health_replenish(entityid id) {
    // spawm a health replenish at the location of id
    if (!entity_exists(id)) return false;
    entityid heart_id = add_entity();
    if (heart_id == ENTITYID_INVALID) return false;
    set_name(heart_id, "heart");
    set_type(heart_id, ENTITY_HEALTH_REPLENISH);
    Vector2 pos = get_pos(id);
    set_pos(heart_id, pos);
    Rectangle src = {0, 0, 7, 7};
    set_src(heart_id, src);
    Rectangle hitbox = {pos.x, pos.y, src.width, src.height};
    set_hitbox(heart_id, hitbox);
    set_collides(heart_id, true);
    set_destroy(heart_id, false);
    set_velocity(heart_id, (Vector2){-0.1f, 0});
    set_pos(heart_id, pos);
    return true;
}

void handle_input_gameplay() {
    Vector2 velocity = get_velocity(hero_id);
    float vx = velocity.x; // use the x component for movement speed
    float vy = velocity.y; // use the y component for movement speed

    if (IsKeyDown(KEY_LEFT)) {
        update_x_pos(hero_id, -vx);
        update_hitbox_x(hero_id, -vx);

        Vector2 d = get_dir(hero_id);
        d.x = -1;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    }

    if (IsKeyDown(KEY_RIGHT)) {
        update_x_pos(hero_id, vx);
        update_hitbox_x(hero_id, vx);
        Vector2 d = get_dir(hero_id);
        d.x = 1;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    }

    if (IsKeyDown(KEY_UP)) {
        update_y_pos(hero_id, -vy);
        update_hitbox_y(hero_id, -vy);
        Vector2 d = get_dir(hero_id);
        d.y = -1;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    } else if (IsKeyUp(KEY_UP)) {
        Vector2 d = get_dir(hero_id);
        d.y = 0;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    }

    if (IsKeyDown(KEY_DOWN)) {
        update_y_pos(hero_id, vy);
        update_hitbox_y(hero_id, vy);
        Vector2 d = get_dir(hero_id);
        d.y = 1;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    } else if (IsKeyUp(KEY_UP)) {
        Vector2 d = get_dir(hero_id);
        d.y = 0;
        set_dir(hero_id, d);
        set_dir(sword_id, d);
    }
    //else if (IsKeyUp(KEY_DOWN)) {
    //    Vector2 dir = get_dir(hero_id);
    //    dir.y = 0;
    //    set_dir(hero_id, dir);
    //    set_dir(sword_id, dir);
    //}


    if (IsKeyPressed(KEY_A)) {
        player_attacking = true;
        set_durability(
            sword_id,
            (Vector2){current_sword_durability, current_sword_durability});
        PlaySound(sfx[SFX_EQUIP]);
    } else if (IsKeyUp(KEY_A)) {
        player_attacking = false;
    }
}

void handle_input_company() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_A)) {
        current_scene = SCENE_TITLE;
        debug_txt_color = BLACK;
        PlaySound(sfx[SFX_CONFIRM]);
    }
}

void handle_input_title() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_A)) {
        current_scene = SCENE_GAMEPLAY;
        debug_txt_color = WHITE;
        gameover = false;
        PlaySound(sfx[SFX_CONFIRM]);
        init_data(); // reset data for new game
    }
}

void handle_input_gameover() {
    //if (IsKeyPressed(KEY_ENTER)) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_A)) {
        current_scene = SCENE_TITLE;
        debug_txt_color = BLACK;
        PlaySound(sfx[SFX_CONFIRM]);
        cleanup_data(); // reset data for new game
        continues++;
    }
}

void handle_input_merchant() {
    //if (IsKeyPressed(KEY_ENTER)) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_A)) {

        // we have to handle what happens on purchasing an item

        if (merchant_items[merchant_item_selection] == ITEM_SWORD) {
            current_sword_durability++;
        } else if (merchant_items[merchant_item_selection] == ITEM_BOOTS) {
            Vector2 velo = get_velocity(hero_id);
            velo.x += 0.25f;
            velo.y += 0.25f;
            set_velocity(hero_id, velo);
        } else if (merchant_items[merchant_item_selection] ==
                   ITEM_HEALTH_EXPANSION) {
            Vector2 myhp = get_hp(hero_id);
            myhp.y++;
            myhp.x = myhp.y;
            set_hp(hero_id, myhp);
        } else if (merchant_items[merchant_item_selection] == ITEM_SWORD_SIZE) {
            float sz = get_size(sword_id);
            sz += 0.1f;
            set_size(sword_id, sz);
        }
        current_coins -= base_coin_level_up_amount * player_level;
        coins_spent += base_coin_level_up_amount * player_level;
        levelup_flag = true;
        merchant_spawned = false;
        current_scene = SCENE_GAMEPLAY;
        PlaySound(sfx[SFX_CONFIRM]);

    } else if (IsKeyPressed(KEY_LEFT)) {
        merchant_item_selection--;
        if (merchant_item_selection < 0) {
            merchant_item_selection = MERCHANT_ITEM_SELECTION_MAX;
        }

    } else if (IsKeyPressed(KEY_RIGHT)) {
        merchant_item_selection++;
        if (merchant_item_selection >= MERCHANT_ITEM_SELECTION_MAX) {
            merchant_item_selection = 0;
        }
    }
}

void handle_input() {
    // take screenshot
    if (IsKeyPressed(KEY_F12)) {
        TakeScreenshot(TextFormat("screenshot_%d.png", frame_count));
        PlaySound(sfx[SFX_CONFIRM]);
    }

    if (current_scene == SCENE_COMPANY)
        handle_input_company();
    else if (current_scene == SCENE_TITLE)
        handle_input_title();
    else if (current_scene == SCENE_GAMEPLAY)
        handle_input_gameplay();
    else if (current_scene == SCENE_GAMEOVER)
        handle_input_gameover();
    else if (current_scene == SCENE_MERCHANT)
        handle_input_merchant();
}

void draw_hud() {
    int x = 10, y = 10, s = 30;
    Color c = WHITE;
    DrawText(TextFormat("Level: %d Coins: %d", player_level, current_coins),
             x,
             y,
             s,
             c);
}

void draw_debug_panel() {
    int x = 10, y = 10, s = 10;
    Color c = debug_txt_color;
    Vector2 p = get_pos(hero_id);
    DrawText(TextFormat("Frame %d", frame_count), x, y, s, c);
    y += s;
    DrawText(TextFormat("FPS: %d", GetFPS()), x, y, s, c);
    y += s;
    DrawText(TextFormat("Cam.pos: %.1f,%.1f", cam2d.target.x, cam2d.target.y),
             x,
             y,
             s,
             c);
    y += s;
    DrawText(
        TextFormat("Cam.offset: %.1f,%.1f", cam2d.offset.x, cam2d.offset.y),
        x,
        y,
        s,
        c);
    y += s;
    DrawText(TextFormat("Zoom: %.1f", cam2d.zoom), x, y, s, c);
    y += s;
    DrawText(TextFormat("Hero.pos: %.1f,%.1f", p.x, p.y), x, y, s, c);
    y += s;
    DrawText(
        TextFormat("Hero Collisions: %d", hero_collision_counter), x, y, s, c);
    y += s;
    DrawText(TextFormat("Sword Collisions: %d", sword_collision_counter),
             x,
             y,
             s,
             c);
    y += s;
    DrawText(TextFormat("Entities created: %d", next_entityid), x, y, s, c);
    y += s;
    DrawText(
        TextFormat("Entities destroyed: %d", entities_destroyed), x, y, s, c);
    y += s;
    DrawText(TextFormat("Coins collected: %d", coins_collected), x, y, s, c);
    y += s;
    DrawText(TextFormat("Coins Lost: %d", coins_lost), x, y, s, c);
    y += s;

    Vector2 myhp = get_hp(hero_id);
    DrawText(TextFormat("HP: %0.1f/%0.1f", myhp.x, myhp.y, coins_collected),
             x,
             y,
             s,
             c);
    y += s;
    DrawText(TextFormat("Level: %d", player_level), x, y, s, c);
    y += s;
    DrawText(TextFormat("Continues: %d", continues), x, y, s, c);
    y += s;
    DrawText(TextFormat("spawn_freq: %d", spawn_freq), x, y, s, c);
    y += s;
    DrawText(TextFormat("Current coins: %d", current_coins), x, y, s, c);
    y += s;
    DrawText(TextFormat("Coins spent: %d", coins_spent), x, y, s, c);
    y += s;

    Vector2 dura = get_durability(sword_id);
    DrawText(TextFormat("Durability: %0.1f/%0.1f", dura.x, dura.y), x, y, s, c);
    y += s;

    DrawText(
        TextFormat("Enemies killed this game: %d", enemies_killed), x, y, s, c);
    y += s;
    DrawText(TextFormat("Total enemies killed: %d", total_enemies_killed),
             x,
             y,
             s,
             c);
    y += s;
}


void draw_company_to_texture() {
    BeginDrawing();
    BeginTextureMode(company_texture);
    ClearBackground(BLACK);
    int s = 40;
    const char* text = "@evildojo666";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 2 - s;
    Color c = {0x66, 0x66, 0x66, 255};
    DrawText(text, x, y, s, c);
    // below the text, draw another text in smaller font
    s = 30;
    text = "presents...";
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    y += s + 10; // move down by s + 10 pixels
    //c = {0x99, 0x99, 0x99, 255};
    DrawText(text, x, y, s, c);
    // below the text, draw another text in smaller font
    s = 10;
    text = "Press A or ENTER to continue";
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    y += 40; // move down by s + 10 pixels

    //c = {0x99, 0x99, 0x99, 255};
    DrawText(text, x, y, s, c);
    EndTextureMode();
    EndDrawing();

    frame_updates++;
}


void draw_company() {
    DrawTexturePro(
        company_texture.texture, target_src, target_dst, origin, 0.0f, WHITE);
}

void draw_title_to_texture() {
    BeginDrawing();
    BeginTextureMode(title_texture);
    Color bg = WHITE;
    ClearBackground(bg);
    int s = 50;
    const char* text = "There can be...";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 2 - s;
    // lets try drawing TX_SWORD_UP directly beneath...
    Rectangle src = {0, 0, 5, 8};
    float scale = 16.0f;
    float w = src.width * scale;
    float h = src.height * scale;
    float x0 = target_w / 2.0f - w / 2;
    //float y0 = target_h / 2.0f + s + 10;
    // lets make y0 such that the sword is dead-center on top of the text
    float y0 = target_h / 2.0f + s / 2.0f - h / 2 + 2 * scale;
    Rectangle dst = {x0, y0, w, h};
    DrawTexturePro(txinfo[TX_SWORD_UP], src, dst, origin, 0.0f, WHITE);
    Color fg = BLACK;
    DrawText(text, x, y, s, fg);
    EndTextureMode();
    EndDrawing();

    frame_updates++;
}

void draw_title() {
    DrawTexturePro(
        title_texture.texture, target_src, target_dst, origin, 0.0f, WHITE);
}

void draw_gameover() {
    BeginDrawing();
    ClearBackground(BLACK);
    int s = 30;
    const char* text = "gameover";
    int m = MeasureText(text, s);
    int x = target_w / 2 - m / 2;
    int y = target_h / 8 - s;
    Color c = {0xFF, 0, 0, 255};
    DrawText(text, x, y, s, c);

    // we want to draw some text representing stats from the
    // most recent playthrough
    // lets start with displaying how many enemies you killed

    c = WHITE;
    y += s + 10;
    s = 20;

    text = TextFormat("Enemies killed/spawned: %d/%d (%0.2f%)",
                      enemies_killed,
                      enemies_spawned,
                      (float)enemies_killed / (float)enemies_spawned * 100.0f);
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    DrawText(text, x, y, s, c);

    //y += s + 10;
    //text = TextFormat("Enemies spawned: %d", enemies_spawned);
    //m = MeasureText(text, s);
    //x = target_w / 2 - m / 2;
    //DrawText(text, x, y, s, c);

    y += s + 10;
    text = TextFormat("Enemies missed/spawned: %d/%d (%0.2f%)",
                      enemies_missed,
                      enemies_spawned,
                      (float)enemies_missed / (float)enemies_spawned * 100.0f);
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    DrawText(text, x, y, s, c);

    //y += s + 10;
    //text = TextFormat("Enemies killed-spawned ratio: %0.2f",
    //                  (float)enemies_killed / (float)enemies_spawned);
    //m = MeasureText(text, s);
    //x = target_w / 2 - m / 2;
    //DrawText(text, x, y, s, c);

    //y += s + 10;
    //text = TextFormat("Enemies missed-spawned ratio: %0.2f",
    //                  (float)enemies_missed / (float)enemies_spawned);
    //m = MeasureText(text, s);
    //x = target_w / 2 - m / 2;
    //DrawText(text, x, y, s, c);

    y += s + 10;
    text = TextFormat("Coins collected/spawned: %d/%d (%0.2f%)",
                      coins_collected,
                      coins_spawned,
                      (float)coins_collected / (float)coins_spawned * 100.0f);
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    DrawText(text, x, y, s, c);

    y += s + 10;
    text = TextFormat("Coins missed: %d", coins_lost);
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    DrawText(text, x, y, s, c);

    y += s + 10;
    text = TextFormat("Coins spent: %d", coins_spent);
    m = MeasureText(text, s);
    x = target_w / 2 - m / 2;
    DrawText(text, x, y, s, c);


    //EndTextureMode();
    EndDrawing();
    frame_updates++;
}

//void draw_gameover() {
//    DrawTexturePro(
//        gameover_texture.texture, target_src, target_dst, origin, 0.0f, WHITE);
//}

void draw_merchant() {
    ClearBackground(BLACK);

    // draw some text above the dwarf in big font
    int s = 30;
    const char* text = "y0, watchu need?";
    int m = MeasureText(text, s);
    Rectangle src = {0, 0, 9, 7};
    float scale = 16.0f;
    float w = src.width * scale;
    float h = src.height * scale;
    float x0 = target_w / 2.0f - w / 2;
    float y0 = target_h / 4.0f - h / 2;
    Rectangle dst = {x0, y0, w, h};

    int x = target_w / 2 - m / 2;
    // position it above the dwarf
    int y = y0 - s - 10; // move up by s + 10 pixels
    Color c = WHITE;
    DrawText(text, x, y, s, c);

    // draw the dwarf merchant texture in the center of the screen
    DrawTexturePro(txinfo[TX_DWARF_MERCHANT], src, dst, origin, 0.0f, WHITE);

    float base_box_size = 8;
    float w1 = base_box_size * scale;
    float h1 = base_box_size * scale;
    float x1 = target_w / 6.0f - w1 / 2;
    float x2 = 3 * target_w / 6.0f - w1 / 2;
    float x3 = 5 * target_w / 6.0f - w1 / 2;
    float xs[3] = {x1, x2, x3};
    float y1 = 5 * target_h / 8.0f - h1 / 2;
    Rectangle dst1 = {xs[0], y1, w1, h1};
    Rectangle dst2 = {xs[1], y1, w1, h1};
    Rectangle dst3 = {xs[2], y1, w1, h1};
    float y2 = y1 + h1 + 10.0f;
    //DrawRectangleLinesEx(dst1, 1.0f, RED);
    //DrawRectangleLinesEx(dst2, 1.0f, RED);
    //DrawRectangleLinesEx(dst3, 1.0f, RED);

    int txkey = TX_SWORD_UP;
    Rectangle src1 = {
        0, 0, (float)txinfo[txkey].width, (float)txinfo[txkey].height};
    //dst1.width = src1.width * scale;
    //dst2.width = src1.width * scale;
    //dst3.width = src1.width * scale;

    int index = SH_BLUE_GLOW;
    float time = (float)GetTime();

    Rectangle dsts[MERCHANT_ITEM_SELECTION_MAX] = {dst1, dst2, dst3};
    dsts[0].width = src1.width * scale;
    dsts[1].width = src1.width * scale;
    dsts[2].width = src1.width * scale;

    string item_text = "";

    for (int i = 0; i < MERCHANT_ITEM_SELECTION_MAX; i++) {
        if (merchant_items[i] == ITEM_SWORD) {
            txkey = TX_SWORD_UP;
            item_text = "Durability";
        } else if (merchant_items[i] == ITEM_BOOTS) {
            txkey = TX_BOOTS;
            item_text = "Speed";
        } else if (merchant_items[i] == ITEM_HEALTH_EXPANSION) {
            txkey = TX_HEALTH_EXPANSION;
            item_text = "HP Up";
        } else if (merchant_items[i] == ITEM_SWORD_SIZE) {
            txkey = TX_SWORD_UP;
            item_text = "Size";
        }

        src1 = {0, 0, (float)txinfo[txkey].width, (float)txinfo[txkey].height};
        dsts[i].width = src1.width * scale;

        if (merchant_item_selection == i) {
            DrawRectangleLinesEx(dsts[i], 2.0f, BLUE);
            SetShaderValue(shaders[index],
                           GetShaderLocation(shaders[index], "time"),
                           &time,
                           SHADER_UNIFORM_FLOAT);
            BeginShaderMode(shaders[index]);
        }
        DrawTexturePro(txinfo[txkey], src1, dsts[i], origin, 0.0f, WHITE);
        if (merchant_item_selection == i) {
            EndShaderMode();
        }
        dsts[i] = {xs[i], y2, w1, h1};
        DrawText(item_text.c_str(), dsts[i].x, dsts[i].y, 20, WHITE);
    }
}

void draw_gameplay() {
    BeginMode2D(cam2d);
    Vector2 pos = get_pos(hero_id);
    Rectangle src = get_src(hero_id);
    float w = target_w / 8.0f;
    float x = target_w / 16.0f + w / 2;
    float x1 = target_w / 16.0f + 3 * w / 4;
    ClearBackground(BLUE);
    x = target_w / 16.0f;
    float y = target_h / 16.0f;
    w = target_w / 8.0f;
    float h = target_h / 8.0f;

    float src_w = txinfo[TX_GRASS_00].width;
    float src_h = txinfo[TX_GRASS_00].height;
    src = {0, 0, src_w, src_h};
    y += 32;
    int tiles_high = 4;
    int tiles_wide = 13;
    for (int j = 0; j < tiles_high; j++) {
        Rectangle dst = {x, y, src_w, src_h};
        for (int i = 0; i < tiles_wide; i++) {
            DrawTexturePro(txinfo[TX_GRASS_00], src, dst, origin, 0.0f, WHITE);
            dst.x += src_w;
        }
        y += src_h;
    }

    for (auto it : component_table) {
        entityid id = it.first;
        if (!has_comp(id, C_POSITION)) continue;
        Vector2 pos = get_pos(id);
        if (pos.x < 0 || pos.y < 0) continue;
        entity_type type = get_type(id);
        Rectangle src = get_src(id);
        Rectangle dst = {pos.x, pos.y, src.width, src.height};
        Color c = WHITE;
        if (type == ENTITY_HERO) {
            // modify the src.width by multiplying by hero's direction.x

            Vector2 mydir = get_dir(hero_id);
            if (mydir.x != 0) {
                src.width *= mydir.x;
            }

            int index = SH_HP_RED_GLOW;
            Vector2 myhp = get_hp(id);
            float hp_frac = myhp.x / myhp.y;
            SetShaderValue(shaders[index],
                           GetShaderLocation(shaders[index], "hp_frac"),
                           &hp_frac,
                           SHADER_UNIFORM_FLOAT);
            BeginShaderMode(shaders[index]);
            DrawTexturePro(txinfo[TX_HERO], src, dst, origin, 0.0f, c);
            EndShaderMode();
        } else if (type == ENTITY_SWORD) {
            //src.width = txinfo[TX_SWORD_UP].width;
            //src.height = txinfo[TX_SWORD_UP].height;
            //dst = {pos.x, pos.y, src.width, src.height};

            Vector2 mydir = get_dir(sword_id);
            int txindex = TX_SWORD;
            //if (mydir.y == -1) {
            //    txindex = TX_SWORD_UP;
            //    src.width = txinfo[txindex].width;
            //    src.height = txinfo[txindex].height;
            //    Vector2 spos = get_pos(sword_id);
            //    dst = {spos.x, spos.y, src.width, src.height};
            //}
            //else
            if (mydir.x != 0) {
                src.width *= mydir.x;
            }
            dst.width *= get_size(sword_id);
            dst.height *= get_size(sword_id);

            int index = SH_DURABILITY_BLACK;
            Vector2 dura = get_durability(id);
            float d_frac = dura.x / dura.y;
            SetShaderValue(
                shaders[index],
                GetShaderLocation(shaders[index], "durability_fraction"),
                &d_frac,
                SHADER_UNIFORM_FLOAT);
            BeginShaderMode(shaders[index]);
            DrawTexturePro(txinfo[txindex], src, dst, origin, 0.0f, c);
            EndShaderMode();
        } else if (type == ENTITY_ORC) {
            DrawTexturePro(txinfo[TX_ORC], src, dst, origin, 0.0f, c);
        } else if (type == ENTITY_COIN) {
            DrawTexturePro(txinfo[TX_COIN], src, dst, origin, 0.0f, c);
        } else if (type == ENTITY_DWARF_MERCHANT) {
            DrawTexturePro(
                txinfo[TX_DWARF_MERCHANT], src, dst, origin, 0.0f, WHITE);
        } else if (type == ENTITY_HEALTH_REPLENISH) {
            DrawTexturePro(
                txinfo[TX_HEALTH_REPLENISH], src, dst, origin, 0.0f, WHITE);
        }
    }
    //x = target_w / 16.0f + w / 2;
    //y = target_h / 16.0f;
    //w = target_w / 8.0f;
    //h = target_h / 8.0f;
    //DrawLineEx((Vector2){x, y}, (Vector2){x, y + h}, 1.0f, (Color){0xff, 0xff, 0xff, 96});
    //x = target_w / 16.0f + 3 * w / 4;
    //DrawLineEx((Vector2){x, y}, (Vector2){x, y + h}, 1.0f, (Color){0xff, 0xff, 0xff, 96});
    EndMode2D();
}

void draw_frame() {
    UpdateMusicStream(music);
    //if (IsWindowResized()) {
    //    window_dst.width = GetScreenWidth();
    //    window_dst.height = GetScreenHeight();
    //}
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
    else if (current_scene == SCENE_MERCHANT)
        draw_merchant();
    EndTextureMode();
    ClearBackground(BLACK);
    DrawTexturePro(
        target_texture.texture, target_src, window_dst, origin, 0.0f, WHITE);
    //draw_debug_panel();
    draw_hud();
    EndDrawing();
    frame_count++;
}

void load_texture(int index, const char* path) {
    // create a buffer, and TextFormat it so "img/" is prepended
    if (index < 0 || index >= NUM_TEXTURES) {
        fprintf(stderr, "Invalid texture index: %d\n", index);
        exit(EXIT_FAILURE);
    }
    if (txinfo[index].id != 0) {
        fprintf(stderr, "Texture %d already loaded\n", index);
        return; // Texture already loaded
    }
    const char* full_path = TextFormat("img/%s.png", path);
    // txinfo[index] = LoadTexture(path);
    txinfo[index] = LoadTexture(full_path);
    if (txinfo[index].id == 0) {
        fprintf(stderr, "Failed to load texture: %s\n", full_path);
        exit(EXIT_FAILURE);
    }
    // Set texture filter to point for pixel art
    SetTextureFilter(txinfo[index], TEXTURE_FILTER_POINT);
}

void load_textures() {
    load_texture(TX_HERO, "human");
    load_texture(TX_SWORD, "sword");
    load_texture(TX_ORC, "orc");
    load_texture(TX_GRASS_00, "tiles/grass-00");
    load_texture(TX_GRASS_01, "tiles/grass-01");
    load_texture(TX_GRASS_02, "tiles/grass-02");
    load_texture(TX_GRASS_03, "tiles/grass-03");
    load_texture(TX_COIN, "coin");
    load_texture(TX_SWORD_UP, "sword-up");
    load_texture(TX_WILD_ORC, "wild-orc");
    load_texture(TX_DWARF_MERCHANT, "dwarf-merchant");
    load_texture(TX_BOOTS, "boots");
    load_texture(TX_HEALTH_REPLENISH, "heart-replenish");
    load_texture(TX_HEALTH_EXPANSION, "heart-expansion");
    draw_company_to_texture();
    draw_title_to_texture();
}

void unload_textures() {
    unload_shaders();
    for (int i = TX_HERO; i < TX_COUNT; i++)
        UnloadTexture(txinfo[i]);
    UnloadRenderTexture(target_texture);
    UnloadRenderTexture(company_texture);
    UnloadRenderTexture(title_texture);
    UnloadRenderTexture(gameover_texture);
}

void init_gfx() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(window_w, window_h, game_window_title);
    SetWindowMinSize(window_size_min_w, window_size_min_h);
    SetTargetFPS(target_fps);
    target_texture = LoadRenderTexture(target_w, target_h);
    company_texture = LoadRenderTexture(target_w, target_h);
    title_texture = LoadRenderTexture(target_w, target_h);
    gameover_texture = LoadRenderTexture(target_w, target_h);
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
    load_shaders();
}

void update_state_player_attack() {
    if (player_attacking) {
        Vector2 pos = get_pos(hero_id);
        Rectangle hb = get_hitbox(hero_id);
        Vector2 dir = get_dir(hero_id);
        float sz = get_size(sword_id);
        float w = txinfo[TX_SWORD].width * sz;
        float h = txinfo[TX_SWORD].height * sz;

        hb.y = hb.y + hb.height / 2.0f - 2;

        if (dir.x == 1) {
            Vector2 spos = {-1, hb.y};
            hb.x = hb.x + hb.width;
            spos.x = hb.x;
            hb = {hb.x, hb.y + 1, w, h};
            set_hitbox(sword_id, hb);
            set_pos(sword_id, spos);
        } else if (dir.x == -1) {
            Vector2 spos = {-1, hb.y};
            hb.x = hb.x - w;
            spos.x = hb.x;
            hb = {hb.x, hb.y + 1, w, h};
            set_hitbox(sword_id, hb);
            set_pos(sword_id, spos);
        }
        //else if (dir.x == 0 && dir.y == -1) {
        //} else if (dir.x == 0 && dir.y == 1) {
        //}
    } else {
        set_pos(sword_id, (Vector2){-1, -1});
        set_hitbox(sword_id, (Rectangle){-1, -1, -1, -1});
    }
}

void update_state_velocity() {
    // velocity-position update
    for (auto row : component_table) {
        // skip the hero
        if (row.first == hero_id) continue;
        if (has_comp(row.first, C_VELOCITY)) {
            Vector2 p = get_pos(row.first), v = get_velocity(row.first);
            Rectangle hb = get_hitbox(row.first);
            p.x += v.x;
            p.y += v.y;
            set_pos(row.first, p);
            hb.x += v.x;
            hb.y += v.y;
            set_hitbox(row.first, hb);
            // if the p.x is < 0, mark for destroy. also this might be gameover if an enemy hits the left
            if (p.x <= ORC_SPAWN_X_LEFT || p.x > ORC_SPAWN_X_RIGHT) {
                set_destroy(row.first, true);
                entity_type t = get_type(row.first);
                if (t == ENTITY_ORC) {
                    enemies_missed++;
                } else if (t == ENTITY_COIN) {
                    coins_lost++;
                } else if (t == ENTITY_DWARF_MERCHANT) {
                    // we want to spawn again
                    do_spawn_merchant = true;
                }
            }
        }
    }
}

void damage_hero(int damage) {
    Vector2 myhp = get_hp(hero_id);
    myhp.x -= damage;
    set_hp(hero_id, myhp);
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
            hero_total_damage_received++;
            set_destroy(row.first, true);
            damage_hero(1);
            enemies_killed++;
            total_enemies_killed++;
            PlaySound(sfx[SFX_GET_HIT]);
        } else if (t == ENTITY_COIN &&
                   CheckCollisionRecs(hb, get_hitbox(row.first))) {
            hero_collision_counter++;
            set_destroy(row.first, true);
            PlaySound(sfx[SFX_COIN]);
            coins_collected++;
            current_coins++;
        } else if (t == ENTITY_DWARF_MERCHANT &&
                   CheckCollisionRecs(hb, get_hitbox(row.first))) {
            hero_collision_counter++;
            set_destroy(row.first, true);
            PlaySound(sfx[SFX_EQUIP]);
            randomize_merchant_items();
            current_scene = SCENE_MERCHANT;
        }
        //else if (t == ENTITY_HEALTH_REPLENISH &&
        //           CheckCollisionRecs(hb, get_hitbox(row.first))) {
        //    hero_collision_counter++;
        //    set_destroy(row.first, true);
        //    PlaySound(sfx[SFX_CONFIRM]);
        //    Vector2 myhp = get_hp(hero_id);
        //    myhp.x++;
        //    if (myhp.x > myhp.y) {
        //        myhp.x = myhp.y;
        //    }
        //    set_hp(hero_id, myhp);
        //}
    }
}

bool hero_health_maxed() {
    Vector2 myhp = get_hp(hero_id);
    return myhp.x == myhp.y;
}

void spawn_drop(entityid id) {
    //if (hero_health_maxed()) {
    create_coin(id); // create a coin at the orc's position
    //}
    //else {
    //    // 50-50 chance of heart or coin
    //    int r = GetRandomValue(0, 1);
    //    if (r) {
    //        create_coin(id);
    //    } else {
    //        create_health_replenish(id);
    //    }
    //}
}

void update_state_sword_collision() {
    // check for collision with sword
    Rectangle shb = get_hitbox(sword_id);
    Vector2 dura = get_durability(sword_id);
    if (player_attacking) {
        for (auto row : component_table) {
            if (get_type(row.first) == ENTITY_ORC &&
                CheckCollisionRecs(shb, get_hitbox(row.first))) {
                sword_collision_counter++;
                set_destroy(row.first, true);
                PlaySound(sfx[SFX_HIT]);
                spawn_drop(row.first); // create a coin at the orc's position

                enemies_killed++;
                total_enemies_killed++;
                dura.x--;
                if (dura.x <= 0) {
                    player_attacking = false;
                }
                set_durability(sword_id, dura);
            }
        }
    }
}

void update_state_destroy() {
    cleanup.clear();
    for (auto row : component_table)
        if (get_destroy(row.first)) cleanup.push_back(row.first);
    for (entityid id : cleanup) {
        // we should first remove the entity from any maps
        // that it might have a component for
        names.erase(id);
        types.erase(id);
        positions.erase(id);
        hitboxes.erase(id);
        velocities.erase(id);
        collides.erase(id);
        destroy.erase(id);
        sources.erase(id);
        hp.erase(id);

        // finally, remove the entity
        remove_entity(id);
    }
    entities_destroyed += cleanup.size();
}

void update_state_hero_hp() {
    Vector2 myhp = get_hp(hero_id);
    if (myhp.x <= 0) {
        gameover = true;
        current_scene = SCENE_GAMEOVER;
    }
}

void handle_level_up() {
    if (levelup_flag) {
        player_level++;
        levelup_flag = false;
        spawn_freq -= spawn_freq_incr; // increase spawn frequency
        if (spawn_freq < 30) {
            spawn_freq = 30.0f;
        } else if (spawn_freq < 60) {
            spawn_freq_incr = 5.0f;
        }

        if (player_level >= 2) {
            num_orcs_to_create = 2;
        }

        if (player_level >= 5) {
            num_orcs_to_create = 3;
        }
        if (player_level >= 10) {
            num_orcs_to_create = 4;
        }

        //current_orc_speed = base_orc_speed - player_level * 0.05f; // increase orc speed
    }
}

void update_level_up() {
    //if (coins_collected >= base_coin_level_up_amount * player_level && !levelup_flag && !do_spawn_merchant) {
    if (current_coins >= base_coin_level_up_amount * player_level &&
        !do_spawn_merchant && !merchant_spawned) {
        do_spawn_merchant = true;
    }

    handle_level_up();

    if (do_spawn_merchant) {
        create_dwarf_merchant();
        do_spawn_merchant = false;
        merchant_spawned = true;
    }
}

void update_state() {

    if (current_scene != SCENE_GAMEPLAY) return;
    // every N frames, create_orc
    if (frame_count % spawn_freq == 0) {
        for (int i = 0; i < num_orcs_to_create; i++) {
            create_orc();
        }
    }

    update_state_hero_hp();
    update_state_player_attack();
    update_state_velocity();
    update_state_hero_collision();
    update_state_sword_collision();
    // perform entity cleanup based on the values in the 'destroy' table
    update_state_destroy();
    update_level_up();
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
    SetAudioStreamBufferSizeDefault(4096);

    load_soundfile(SFX_CONFIRM, "sfx/020_Confirm_10.wav");
    load_soundfile(SFX_HIT, "sfx/03_Hit_Wet.wav");
    load_soundfile(SFX_GET_HIT, "sfx/62_Get_hit_01.wav");
    load_soundfile(SFX_EQUIP, "sfx/061_Equip_01.wav");
    load_soundfile(SFX_COIN, "sfx/Coins.wav");

    music = LoadMusicStream("music/music.mp3");
    if (music.stream.buffer == NULL) {
        fprintf(stderr, "Failed to load music: music/music.wav\n");
        exit(EXIT_FAILURE);
    }
    // Set music volume
    PlayMusicStream(music);
}

void unload_soundfile(int index) {
    if (sfx[index].stream.buffer != NULL)
        UnloadSound(sfx[index]);
    else
        fprintf(stderr, "Sound %d was not loaded or already unloaded\n", index);
}

void unload_soundfiles() {
    // stop playing sfx if any was still playing
    StopMusicStream(music);
    unload_soundfile(SFX_CONFIRM);
    unload_soundfile(SFX_HIT);
    unload_soundfile(SFX_GET_HIT);
    unload_soundfile(SFX_EQUIP);
    unload_soundfile(SFX_COIN);
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
    unload_soundfiles();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
