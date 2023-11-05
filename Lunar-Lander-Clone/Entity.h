enum EntityType { PLATFORM, PLAYER };

class Entity
{
private:
	bool m_is_active = true;

	int* m_boost_animation = NULL,
		* m_no_boost_animation = NULL;


	// PHYSICS (GRAVITY) 
	glm::vec3 m_position;
	glm::vec3 m_velocity;
	glm::vec3 m_acceleration;

	// TRANSFORMATIONS
	float m_speed;
	float m_angle;
	glm::vec3 m_movement;
	glm::mat4 m_model_matrix;



	float m_width = 1;
	float m_height = 1;

	EntityType m_entity_type;

public:

	// ANIMATION BOOST
	int** m_animation = new int* [2]
		{
			m_boost_animation,
			m_no_boost_animation
		};

	// PHYSICS FLYING
	bool m_is_accelerating = false;
	float m_acceleration_rate = 0;

	// PHYSICS COLLISIONS
	bool m_collided_top = false;
	bool m_collided_bottom = false;
	bool m_collided_left = false;
	bool m_collided_right = false;

	// TEXTURE
	GLuint m_texture_id;

	int* m_animation_indices = NULL;
	int m_animation_index = 0,
		m_animation_cols = 0,
		m_animation_rows = 0;

	// METHODS
	Entity();
	~Entity();

	void update(float delta_time, Entity* player, Entity* collidatble_entities, int collidable_entity_count);
	void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
	void render(ShaderProgram* program);

	bool const check_collision(Entity* collided_entity) const;
	void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
	void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

	void rotate(float angle);
	void accelerate(float delta_time);

	void activate();
	void deactivate();

	// GETTERS
	EntityType const get_entity_type()    const { return m_entity_type; };
	glm::vec3  const get_position()       const { return m_position; };
	glm::vec3  const get_movement()       const { return m_movement; };
	glm::vec3  const get_velocity()       const { return m_velocity; };
	glm::vec3  const get_acceleration()   const { return m_acceleration; };
	float      const get_acceeleration_rate()  const { return m_acceleration_rate; };
	float      const get_speed()          const { return m_speed; };
	float      const get_angle()          const { return m_angle; };
	int        const get_width()          const { return m_width; };
	int        const get_height()         const { return m_height; };

	// SETTERS
	void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
	void const set_position(glm::vec3 new_position) { m_position = new_position; };
	void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
	void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
	void const set_speed(float new_speed) { m_speed = new_speed; };
	void const set_acceleration_rate(float new_acceleration_rate) { m_acceleration_rate = new_acceleration_rate; };
	void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
	void const set_width(float new_width) { m_width = new_width; };
	void const set_height(float new_height) { m_height = new_height; };

};