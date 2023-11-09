/**
* Author: Jaden Thakur
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// #defines
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << std::endl;
#define STB_IMAGE_IMPLEMENTATION
#define FIXED_TIMESTEP 0.0166666f
#define LANDZONE_COUNT 5
#define PLATFORM_COUNT 222 - LANDZONE_COUNT


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

// includes
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>
#include "Entity.h"
#include "SDL_mixer.h"
#include "cmath"
#include <ctime>
#include <cstdlib>
#include <vector>
#include <time.h>



struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* landzones;
    Entity* ui;
};

// Globals 
const int WINDOW_WIDTH = 1000,
WINDOW_HEIGHT = 600;

const float BG_RED = 0.0f,
BG_BLUE = 0.1f,
BG_GREEN = 0.0f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/Ship_Sprite_Sheet_2.png";
const char PLATFORM_FILEPATH[] = "assets/platformPack_tile027.png"; 
const char LANDZONE_FILEPATH[] = "assets/platformPack_tile028.png";
const char WIN_SCREEN[] = "assets/win.png";
const char LOSE_SCREEN[] = "assets/game_over.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;


GameState g_state;

SDL_Window* g_display_window;
bool go = false;
bool win = false;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
int randint;

// Useful Functions
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Lunar Lander Clone",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Viewport Setup 
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-15.0f, 15.0f, -10.0f, 10.0f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);


    // Platforms and Landing Zones
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint landzone_texture_id = load_texture(LANDZONE_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];
    g_state.landzones = new Entity[LANDZONE_COUNT];

    
    std::vector<int> platform_heights = {11, 10, 9, 8, 7, 7, 8, 7, 7, 6, 6, 5, 5, 6, 6, 6, 7, 7, 8, 9, 10, 10, 9, 7, 5, 5, 6, 6, 7, 7, 5};
    int platform_count = 0;
    int landzone_counter = 0;

    std::vector<int> landzone_locations;
    srand(time(0));
    for (int i = 0; i < LANDZONE_COUNT; i++) {
        randint = std::rand() % 31;
        if (std::find(landzone_locations.begin(), landzone_locations.end(), randint) == landzone_locations.end()) {
            landzone_locations.emplace_back(randint);
        }
        else {
            i--;
        }
        
    }
    
    // Create platforms
    for (int j = 0; j < 31; j++) {
        for (int i = 0; i < platform_heights[j] ; i++)
            {
            bool found = std::find(landzone_locations.begin(), landzone_locations.end(), j) == landzone_locations.end();
                if (landzone_counter != LANDZONE_COUNT && (i == platform_heights[j]-1) && found) {
                    g_state.landzones[landzone_counter].m_texture_id = landzone_texture_id;
                    g_state.landzones[landzone_counter].set_position(glm::vec3(j - 15.0f, i - 10.0f, 0.0f));
                    g_state.landzones[landzone_counter].set_width(1.0f);
                    g_state.landzones[landzone_counter].scale();
                    g_state.landzones[landzone_counter].set_entity_type(LANDZONE);
                    g_state.landzones[landzone_counter].update(0.0f, g_state.player, NULL, NULL, 0);
                    landzone_counter++;
                }
                else {
                    g_state.platforms[platform_count].m_texture_id = platform_texture_id;
                    g_state.platforms[platform_count].set_entity_type(PLATFORM);
                    g_state.platforms[platform_count].set_position(glm::vec3(j - 15.0f, i - 10.0f, 0.0f));
                    g_state.platforms[platform_count].set_width(1.0f);
                    g_state.platforms[platform_count].scale();
                    g_state.platforms[platform_count].update(0.0f, g_state.player, NULL, NULL, 0);
                    platform_count++;
                }     
            }
    }
    

    // Player Stuff
    // Create Player
    g_state.player = new Entity();
    g_state.player->set_position(glm::vec3(-14.0f, 9.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_speed(1.0f);
    g_state.player->set_acceleration(glm::vec3(0.0f, -4.905f, 0.0f));
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Flying
    g_state.player->m_animation[0] = new int[1] {0};
    g_state.player->m_animation[1] = new int[1] {1};
    g_state.player->m_acceleration_rate = 0.75f;
    g_state.player->set_entity_type(PLAYER);
    g_state.player->m_animation_index = 0;
    g_state.player->m_animation_cols = 2;
    g_state.player->m_animation_rows = 1;
    g_state.player->m_animation_indices = g_state.player->m_animation[0];

    // UI
    g_state.ui = new Entity();
    g_state.ui->set_entity_type(UI);
    g_state.ui->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_state.ui->set_width(12.0f);
    g_state.ui->set_height(6.0f);
    g_state.ui->scale();
    g_state.ui->deactivate();

    // Needed stuff
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    
    // Stop player from moving without input
    g_state.player->set_movement(glm::vec3(0.0f));

    // One Click Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                // Quit
                g_game_is_running = false;
                break;
            case SDLK_SPACE:
                // Accelerate
                g_state.player->m_is_accelerating = true;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }

    
    // Holding Down Keys
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])
    {
        g_state.player->rotate(0.025);
    }
    if (key_state[SDL_SCANCODE_D])
    {
        g_state.player->rotate(-0.025);
    }
    if (key_state[SDL_SCANCODE_SPACE]) {
        g_state.player->m_animation_indices = g_state.player->m_animation[1];
        g_state.player->m_is_accelerating = true;
    }
    else if (!key_state[SDL_SCANCODE_SPACE]) {
        g_state.player->m_animation_indices = g_state.player->m_animation[0];
    }

    // normalize movement
    if (glm::length(g_state.player->get_movement()) > 1.0f)
    {
        g_state.player->set_movement(
            glm::normalize(
                g_state.player->get_movement()
            )
        );
    }
}

void update()
{
    // delta time
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.platforms, g_state.landzones, PLATFORM_COUNT + LANDZONE_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
     

    // Win and Lose States
    if (g_state.player->m_landed){
        if (g_state.player->m_collided_bottom) {
            go = true;
            win = true;
            g_state.player->m_landed = false;
        }
        else {
            g_state.player->m_landed = false;
            g_state.player->m_crashed = true;
        }
    }
    else if (g_state.player->m_crashed) {
        go = true;
        g_state.player->m_crashed = false;
    }


}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // deactivate all of the platforms and player on game over
    if (go) {
        g_state.player->deactivate();
        for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].deactivate();
        for (int i = 0; i < LANDZONE_COUNT; i++) g_state.landzones[i].deactivate();
    }

    // render everything except UI
    g_state.player->render(&g_program);

    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    for (int i = 0; i < LANDZONE_COUNT; i++) g_state.landzones[i].render(&g_program);
    
    // render UI elements based on win or lose
    if (win && go) {
        g_state.ui->activate();
        g_state.ui->m_texture_id = load_texture(WIN_SCREEN);
        g_state.ui->render(&g_program);
    }
    else if (go && !win) {
        g_state.ui->activate();
        g_state.ui->m_texture_id = load_texture(LOSE_SCREEN);
        g_state.ui->render(&g_program);
    }

    // Swap window
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();


    // delete everything 
    delete[] g_state.platforms;
    delete[] g_state.landzones;
    delete g_state.ui;
    delete g_state.player;
}

// GAME LOOP
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}