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
#define PLATFORM_COUNT 20
#define LANDZONE_COUNT 1

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

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* landzones;
    Entity* ui;
};

// ––––– CONSTANTS ––––– //
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

const int CD_QUAL_FREQ = 44100,
AUDIO_CHAN_AMT = 2,     // stereo
AUDIO_BUFF_SIZE = 4096;

const char BGM_FILEPATH[] = "assets/crypto.mp3", // change
SFX_FILEPATH[] = "assets/bounce.wav"; // change

const int PLAY_ONCE = 0,    // play once, loop never
NEXT_CHNL = -1,   // next available channel
ALL_SFX_CHNL = -1;


Mix_Music* g_music;
Mix_Chunk* g_jump_sfx;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool go = false;
bool win = false;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// ––––– GENERAL FUNCTIONS ––––– //
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

    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-15.0f, 15.0f, -10.0f, 10.0f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– BGM ––––– //
    Mix_OpenAudio(CD_QUAL_FREQ, MIX_DEFAULT_FORMAT, AUDIO_CHAN_AMT, AUDIO_BUFF_SIZE);

    // STEP 1: Have openGL generate a pointer to your music file
    g_music = Mix_LoadMUS(BGM_FILEPATH); // works only with mp3 files

    // STEP 2: Play music
    Mix_PlayMusic(
        g_music,  // music file
        -1        // -1 means loop forever; 0 means play once, look never
    );

    // STEP 3: Set initial volume
    Mix_VolumeMusic(MIX_MAX_VOLUME / 5.0);

    // ––––– SFX ––––– //
    g_jump_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– PLATFORMS ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint landzone_texture_id = load_texture(LANDZONE_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];
    g_state.landzones = new Entity[LANDZONE_COUNT];


    // Set the type of every platform entity to PLATFORM
    for (int i = 0; i < 5; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 2.0f, -3.0f, 0.0f));
        g_state.platforms[i].set_width(1.0f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, NULL, 0);
    }

    for (int i = 5; i < 10; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 7.0f, 3.0f, 0.0f));
        g_state.platforms[i].set_width(1.0f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, NULL, 0);
    }

    for (int i = 10; i < 15; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(-2.0f, i - 12.0f, 0.0f));
        g_state.platforms[i].set_width(1.0f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, NULL, 0);
        g_state.platforms[i].deactivate();
    }

    for (int i = 15; i < 20; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(2.0f, i - 17.0f, 0.0f));
        g_state.platforms[i].set_width(1.0f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, g_state.player, NULL, NULL, 0);
    }

    g_state.landzones[0].m_texture_id = landzone_texture_id;
    g_state.landzones[0].set_position(glm::vec3(-3.0f, 3.0f, 0.0f));
    g_state.landzones[0].set_width(1.0f);
    g_state.landzones[0].set_entity_type(LANDZONE);
    g_state.landzones[0].update(0.0f, g_state.player, NULL, NULL, 0);

    // ––––– PLAYER ––––– //
    // Existing
    g_state.player = new Entity();
    //g_state.player->set_position(glm::vec3(0.0f));
    g_state.player->set_position(glm::vec3(-4.0f, 10.0f, 0.0f));
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

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

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

    

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    //if (key_state[SDL_SCANCODE_LEFT])
    if (key_state[SDL_SCANCODE_A])
    {
        g_state.player->rotate(0.025);
    }
    //else if (key_state[SDL_SCANCODE_RIGHT])
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
        //g_state.player->update(FIXED_TIMESTEP, g_state.player, g_state.landzones, LANDZONE_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
     
    if (g_state.player->m_landed){
        go = true;
        win = true;
    }
    else if (g_state.player->m_crashed) {
        go = true;
    }


}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (go) {
        g_state.player->deactivate();
        for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].deactivate();
        for (int i = 0; i < LANDZONE_COUNT; i++) g_state.landzones[i].deactivate();
    }

    g_state.player->render(&g_program);

    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    for (int i = 0; i < LANDZONE_COUNT; i++) g_state.landzones[i].render(&g_program);


    
    
    
    if (win && go) {
        g_state.ui->activate();
        g_state.ui->m_texture_id = load_texture(WIN_SCREEN);
        g_state.ui->render(&g_program);
    }
    else if (go && !win) {
        g_state.ui->m_texture_id = load_texture(LOSE_SCREEN);
        g_state.ui->activate();
        g_state.ui->render(&g_program);
    }


    

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
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