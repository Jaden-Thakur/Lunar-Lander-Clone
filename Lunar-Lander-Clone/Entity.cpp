/**
* Author: Jaden Thakur
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << std::endl;

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"


const float GRAVITY = 1.0f;
const float AIR_RESISTANCE = 0.5f;
const float max_accel = 1.0f;

Entity::Entity() {
    // PHYSICS (GRAVITY) 
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // TRANSFORMATIONS
    m_movement = glm::vec3(0.0f);
    m_speed = 0.0;
    m_angle = 0.0;
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
    delete[] m_animation;
};

void Entity::rotate(float angle) {
    //if ((m_angle + angle <= 90) || (m_angle + angle >= 270 && m_angle + angle <= 359)) {
        this->m_model_matrix = glm::rotate(this->m_model_matrix, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        m_angle += angle;
        LOG(angle);
    //} 
};

void Entity::scale() {
    // scales model to current height and width, yes you have to set the height and width first Jaden was lazy
    m_model_matrix = glm::scale(m_model_matrix, glm::vec3(m_width, m_height, 1.0));
}

void Entity::accelerate(float delta_time) {
    float amount_x = 0;
    float amount_y = 0;

    // choosing direction of acceleration and accelerating in that direction
    if (0 <= m_angle && m_angle <= 90) {
        amount_x = (m_angle / 90);
        amount_y = 1 - amount_x;
        m_acceleration.x -= amount_x * m_acceleration_rate;
        m_acceleration.y += amount_y * m_acceleration_rate;
    }
    else if (90 < m_angle && m_angle <= 180) {
        amount_y = ((m_angle - 90) / 90);
        amount_x = 1 - amount_y;
        m_acceleration.x -= amount_x * m_acceleration_rate;
        m_acceleration.y -= amount_y * m_acceleration_rate;
    }
    else if (180 < m_angle && m_angle <= 270) {
        amount_x = ((m_angle - 180) / 90);
        amount_y = 1 - amount_x;
        m_acceleration.x += amount_x * m_acceleration_rate;
        m_acceleration.y -= amount_y * m_acceleration_rate;
    }
    else if (270 < m_angle && m_angle <= 359) {
        amount_y = ((m_angle - 270) / 90);
        amount_x = 1 - amount_y;
        m_acceleration.x += amount_x * m_acceleration_rate;
        m_acceleration.y += amount_y * m_acceleration_rate;
    }

    if (m_velocity.x > max_accel) {
        m_velocity.x = max_accel;
    }

    if (m_velocity.y > max_accel) {
        m_velocity.y = max_accel;
    }

    
};

void Entity::activate() {
    this->m_is_active = true;
};
void Entity::deactivate() {
    this->m_is_active = false;
};


void Entity::update(float delta_time, Entity* player, Entity* collidable_entities1, Entity* collidable_entities2, int collidable_entity_count)
{
    if (!m_is_active) return;

    // collision checks
    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

  
    // replace negative angles with correct positive angle
    if (m_angle > 359) {
        m_angle = m_angle - 360.0f;
    }
    if (m_angle < 0) {
        m_angle += 360;
    }


    // accelerate only when player clicks space
    if (m_is_accelerating)
    {
        m_is_accelerating = false;
        accelerate(delta_time); 
    }
    else {
        m_acceleration.y = 0;
        m_acceleration.x = 0;
    }
    

    // gravity
    m_velocity += m_acceleration * delta_time;
    m_velocity.y -= GRAVITY * delta_time;

    // air resistance
    if (m_velocity.x > 0) {
        m_velocity.x -= AIR_RESISTANCE * delta_time;
    }
    else if (m_velocity.x < 0) {
        m_velocity.x += AIR_RESISTANCE * delta_time;
    }
    

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(collidable_entities1, collidable_entity_count);
    check_collision_y(collidable_entities2, collidable_entity_count);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities1, collidable_entity_count);
    check_collision_x(collidable_entities2, collidable_entity_count);

    if (m_collided_bottom) {
        m_acceleration.x = 0;
    }


    // ����� TRANSFORMATIONS ����� //
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(this->m_model_matrix, glm::radians(m_angle), glm::vec3(0.0f, 0.0f, 1.0f));
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->get_position().y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->get_height() / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;
                m_collided_top = true;
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_collided_bottom = true;
                
            }

            if (collidable_entity->get_entity_type() == LANDZONE) {
                m_landed = true;
            }
            else {
                m_crashed = true;
            }
            
        }

        
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            if (m_velocity.x > 0) {
                m_position.x -= x_overlap;
                m_velocity.x = 0;
                m_collided_right = true;
            }
            else if (m_velocity.x < 0) {
                m_position.x += x_overlap;
                m_velocity.x = 0;
                m_collided_left = true;
            }

            if (collidable_entity->get_entity_type() == LANDZONE) {
                m_landed = true;
            }
            else {
                m_crashed = true;
            }
        }
    }
}

void Entity::render(ShaderProgram* program)
{
    program->set_model_matrix(m_model_matrix);

    if (!m_is_active) { return; }

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    if (other == this) return false;
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}