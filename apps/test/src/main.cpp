#include "cs/engine/engine.hpp"
#include "cs/engine/input.hpp"
#include "cs/game/game.hpp"

struct Transform_Component : Component
{
	static constexpr Name_Id id = Name_Id("Transform_Component");
	virtual constexpr Name_Id get_id() const { return id; }

	vec3 local_position { vec3::zero_vector };
	quat local_orientation { quat::zero_quat };
	mat4 local_matrix { mat4(1.0f) };

	void calculate_local_matrix()
	{
		local_matrix = translate(mat4(1.0f), local_position);
		local_matrix = local_matrix * local_orientation.to_mat4();
	}
};

struct Render_Component : Component
{
	static constexpr Name_Id id = Name_Id("Render_Component");
	virtual constexpr Name_Id get_id() const { return id; }

	Shared_Ptr<Mesh> mesh;
	mat4 model_matrix { mat4(1.0f) };
};

struct Player_Entity
{
	Component_Handle<Transform_Component> h_transform;
	Shared_Ptr<Perspective_Camera> fpv_camera;

	float movement_speed{10.0};
	vec3 movement_input{0.0f};
	float look_speed{25_deg};
	vec3 input_mouse{vec3::zero_vector}, input_mouse_previous{vec3::zero_vector}, look_angles{vec3::zero_vector};
};

struct Test_Entity
{
	Component_Handle<Transform_Component> h_transform;
	Component_Handle<Render_Component> h_render;

	vec3 rotate_axis{vec3::forward_vector};
	float rotate_speed{0.0f};
	float rotate_angle{0.0f};
};

struct My_Game
{
	bool initialized {false};
	bool started { false };

	Dynamic_Component_Storage<
		Transform_Component,
		Render_Component
	> components;

	Player_Entity player;
	Dynamic_Array<Test_Entity> tests;

	// TODO: Resource manager of sorts
	Shared_Ptr<Mesh_Resource> unit_capsule;
	Shared_Ptr<Mesh_Resource> unit_sphere;
	Shared_Ptr<Mesh_Resource> unit_box;

	void initialize()
	{
		components.initialize();

		Renderer& renderer = Renderer::get();
		renderer.window->on_window_resize.bind([this](uint32 width, uint32 height) {
			_initialize_camera(width, height);
		});

		uint32 width, height;
		renderer.window->get_window_size(width, height);
		_initialize_camera(width, height);

		if (!unit_sphere.is_valid())
		{
			unit_sphere = Shared_Ptr<Mesh_Resource>::create();
			unit_sphere->name = Name_Id("unit_sphere");
			unit_sphere->initialize_from_file("assets/meshes/test/unit_sphere.obj");
		}

		if (!unit_box.is_valid())
		{
			unit_box = Shared_Ptr<Mesh_Resource>::create();
			unit_box->name = Name_Id("unit_box");
			unit_box->initialize_from_file("assets/meshes/test/unit_box.obj");
		}

		if (!unit_capsule.is_valid())
		{
			unit_capsule = Shared_Ptr<Mesh_Resource>::create();
			unit_capsule->name = Name_Id("unit_capsule");
			unit_capsule->initialize_from_file("assets/meshes/test/unit_capsule.obj");
		}

		Shared_Ptr<Mesh_Resource> rand_m[] = { unit_sphere, unit_box, unit_capsule };

		tests.clear();
		for (int32 i = 0; i < 10000; ++i)
		{
			float rx = static_cast<float>(rand() % 10000) / 100.0f;
			float ry = static_cast<float>(rand() % 10000) / 100.0f;
			float rz = static_cast<float>(rand() % 10000) / 100.0f;
			float ra = 360_deg * static_cast<float>(rand() % 1000) / 1000.0f;
			tests.push_back(_make_test(rand_m[rand() % 3], vec3(rx, ry, rz),
				vec3(rx, 0, rz).normalized(), ra));
		}

		initialized = true;
	}

	void start()
	{
		_initialize_player();
		started = true;
	}

	void update(float dt)
	{
		_pre_update_player(dt);

		for (Test_Entity& test : tests)
		{
			test.rotate_angle += dt * test.rotate_speed;

			Transform_Component& transform_component = components.get<Transform_Component>(test.h_transform);
			transform_component.local_orientation =
				quat::from_euler_angles(test.rotate_axis * test.rotate_angle);

			transform_component.calculate_local_matrix();

			Render_Component& render_component = components.get<Render_Component>(test.h_render);
			render_component.model_matrix = transform_component.local_matrix;
		}

		_post_update_player(dt);
	}

	Dynamic_Array<Name_Id> renderer_meshes_used;
	Hash_Map<Name_Id, Shared_Ptr<Mesh>> renderer_mesh_map;
	Hash_Map<Name_Id, Dynamic_Array<Instance_Data>> renderer_instance_map;

	void render()
	{
		for (const Name_Id&  mesh_id : renderer_meshes_used)
		{
			Dynamic_Array<Instance_Data>* p_instances = renderer_instance_map.find(mesh_id);
			if (p_instances)
			{
				p_instances->clear();
			}
		}
		renderer_meshes_used.clear();

		for (Test_Entity& test : tests)
		{
			Render_Component& render_component = components.get<Render_Component>(test.h_render);
			const Name_Id& mesh_id = render_component.mesh->mesh_resource->name;
			if (renderer_meshes_used.find_first(mesh_id) == -1)
			{
				renderer_meshes_used.push_back(mesh_id);
				renderer_mesh_map.insert(mesh_id, render_component.mesh);
			}

			Dynamic_Array<Instance_Data>& instances = renderer_instance_map.find_or_add(mesh_id);
			if (instances.size() == 0)
			{
				instances.reserve(tests.size());
			}
			instances.push_back({render_component.model_matrix.transposed()});
		}

		Renderer& renderer = Renderer::get();
		for (const Name_Id&  mesh_id : renderer_meshes_used)
		{
			Shared_Ptr<Mesh>& mesh = renderer_mesh_map.find_or_add(mesh_id);
			Dynamic_Array<Instance_Data>& instance_datas =  renderer_instance_map.find_or_add(mesh_id);
			renderer.backend->draw_mesh_instanced(mesh, instance_datas);
		}
	}

	void shutdown()
	{
		_shutdown_player();
		initialized = false;
		started = false;

		renderer_meshes_used.clear();
		renderer_mesh_map.clear();
		renderer_instance_map.clear();
	}

	void _initialize_camera(uint32 width, uint32 height)
	{
		if (!player.fpv_camera.is_valid())
		{
			player.fpv_camera = Shared_Ptr<Perspective_Camera>::create();
		}

		player.fpv_camera->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		player.fpv_camera->position = vec3::zero_vector;
		player.fpv_camera->orientation = quat::from_direction(vec3::forward_vector);
	}

	void _initialize_player()
	{
		Transform_Component& transform_component = components.add<Transform_Component>(player.h_transform);
		transform_component.local_position = vec3(0.0f, -15.0f, 0.0f);

		_initialize_player_input();

		Renderer& renderer = Renderer::get();
		uint32 width, height;
		renderer.window->get_window_size(width, height);
		_initialize_camera(width, height);
		renderer.set_active_camera(player.fpv_camera);
	}

	Test_Entity _make_test(const Shared_Ptr<Mesh_Resource> &test_mesh, const vec3& position, const vec3& rotate_axis, float rotate_speed)
	{
		Test_Entity test;

		Transform_Component& transform_component = components.add<Transform_Component>(test.h_transform);
		transform_component.local_position = position;
		transform_component.calculate_local_matrix();

		Render_Component& render_component = components.add<Render_Component>(test.h_render);
		render_component.model_matrix = transform_component.local_matrix;

		Renderer& renderer = Renderer::get();

		// TODO: this probably should happen in-engine when it's requested to be used (will also handle if we change
		// the renderer on-fly) as well as reusing the same mesh for multiple instances (this will make copies, ok for now)
		render_component.mesh = renderer.backend->get_mesh(test_mesh);

		test.rotate_axis = rotate_axis;
		test.rotate_speed = rotate_speed;

		return test;
	}

	void _initialize_player_input()
	{
		Input_System::get().register_event({"player_forward",
			{
				{"KEY_W", 1.0f},
				{"KEY_S", -1.0f},
				{"KEY_UP", 1.0f},
				{"KEY_DOWN", -1.0f}
			}}).bind(this, &My_Game::_player_input_forward);

		Input_System::get().register_event({"player_right",
			{
				{"KEY_A", -1.0f},
				{"KEY_D", 1.0f},
				{"KEY_LEFT", -1.0f},
				{"KEY_RIGHT", 1.0f}
			}}).bind(this, &My_Game::_player_input_right);

		Input_System::get().register_event({"player_look_up",
			{{"MOUSE_POS_Y", -1.0f}
			}}).bind(this, &My_Game::_player_input_mouse_up);

		Input_System::get().register_event({"player_look_right",
			{{"MOUSE_POS_X", -1.0f}
			}}).bind(this, &My_Game::_player_input_mouse_right);
	}

	void _shutdown_player()
	{
		Input_System::get().deregister_event("player_forward");
		Input_System::get().deregister_event("player_right");
		Input_System::get().deregister_event("player_look_up");
		Input_System::get().deregister_event("player_look_right");
	}

	void _pre_update_player(float dt)
	{
		Transform_Component& transform_component = components.get<Transform_Component>(player.h_transform);

		const vec3 mouse_delta = player.input_mouse - player.input_mouse_previous;
		player.input_mouse_previous = player.input_mouse;
		player.look_angles.x += mouse_delta.y * player.look_speed;
		player.look_angles.x = clamp(player.look_angles.x, -85_deg, 85_deg);
		player.look_angles.z += mouse_delta.x * player.look_speed;

		vec3 editor_xy_movement_input = player.movement_input;
		editor_xy_movement_input.z = 0.0f;

		const quat yaw_rotation = quat::from_rotation_axis(vec3::up_vector, player.look_angles.z);
		const vec3 player_forward_move = yaw_rotation.mul(editor_xy_movement_input.normalized());
		transform_component.local_position += player_forward_move.normalized() * (player.movement_speed * dt);

		player.fpv_camera->orientation = quat::from_euler_angles(player.look_angles);
		player.fpv_camera->position = transform_component.local_position;
	}

	void _post_update_player(float dt)
	{

	}

	void _player_input_forward(float value)
	{
		player.movement_input.y = value;
	}

	void _player_input_right(float value)
	{
		player.movement_input.x = value;
	}

	void _player_input_mouse_up(float value)
	{
		player.input_mouse.y = value;
	}

	void _player_input_mouse_right(float value)
	{
		player.input_mouse.x = value;
	}
};

struct Editor_Entry_Point : Entry_Point
{
	bool wants_to_exit{false};
	float editor_movement_speed{10.0f};
	vec3 editor_movement_input{vec3::zero_vector};
	float editor_rotation_speed{25_deg};
	vec3 editor_rotation_angles{vec3::zero_vector};
	vec3 editor_rotation_input{vec3::zero_vector}, editor_rotation_input_previous{vec3::zero_vector};
	Shared_Ptr<Camera> editor_camera;

	My_Game game, active_game;

public:
	void initialize() override
	{
		printf("Editor initializing... ");

		_initialize_editor_camera();
		_register_inputs();

		game.initialize();

		printf("DONE\n");
		flushall();
	}

	void update(float dt) override
	{
		if (active_game.started)
		{
			active_game.update(dt);
		}
		else
		{
			const vec3 mouse_delta = editor_rotation_input - editor_rotation_input_previous;
			editor_rotation_input_previous = editor_rotation_input;
			editor_rotation_angles.x += mouse_delta.y * editor_rotation_speed;
			editor_rotation_angles.x = clamp(editor_rotation_angles.x, -85_deg, 85_deg);
			editor_rotation_angles.z += mouse_delta.x * editor_rotation_speed;
			editor_camera->orientation = quat::from_euler_angles(editor_rotation_angles);

			vec3 editor_xy_movement_input = editor_movement_input;
			editor_xy_movement_input.z = 0.0f;
			const vec3 player_forward_move = editor_camera->orientation.mul(editor_xy_movement_input.normalized());
			editor_camera->position += player_forward_move.normalized() * (editor_movement_speed * dt);
			// We want Q/E input to be vertical regardless of camera orientation
			editor_camera->position.z += editor_movement_input.z * editor_movement_speed * dt;
		}
	}

	void render(VR_Eye::Type eye) override
	{
		Renderer& renderer = Renderer::get();
		Shared_Ptr<Renderer_Backend> renderer_backend = Renderer::get().backend;

		if (active_game.started)
		{
			active_game.render();
		}
		else
		{
			game.render();
		}
	}

	void shutdown() override
	{
		printf("Editor shutdown... \n");
		flushall();
	}

	[[nodiscard]] bool should_shutdown() const override
	{
		return wants_to_exit;
	}

	void _initialize_editor_camera()
	{
		Renderer& renderer = Renderer::get();
		renderer.window->on_window_resize.bind([this](uint32 width, uint32 height) {
			_initialize_camera(width, height);
		});

		uint32 width, height;
		renderer.window->get_window_size(width, height);
		_initialize_camera(width, height);

		renderer.set_active_camera(editor_camera);
	}

	void _initialize_camera(uint32 width, uint32 height)
	{
		if (!editor_camera.is_valid())
		{
			editor_camera = Shared_Ptr<Perspective_Camera>::create();
		}

		editor_camera->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		editor_camera->position = vec3(0.0f, -10.0, 0.0f);
		editor_camera->orientation = quat::from_direction(vec3::forward_vector);
	}

private:
	void _register_inputs()
	{
		Input_System::get().register_event({"game_start",{
			{"KEY_P", 1.0f}, {"KEY_F5", 1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_game_start);

		Input_System::get().register_event({"game_stop", {
			{"KEY_ESCAPE", 1.0f}
		}}).bind(this, &Editor_Entry_Point::_editor_input_game_stop);

		_register_camera_inputs();
	}

	void _register_camera_inputs()
	{
		Input_System::get().register_event({"editor_camera_speed",
			{
				{"MOUSE_SCROLL_Y", 1.0f},
			}}).bind(this, &Editor_Entry_Point::_editor_input_camera_speed);

		Input_System::get().register_event({"editor_forward",
			{
				{"KEY_W", 1.0f},
				{"KEY_S", -1.0f},
				{"KEY_UP", 1.0f},
				{"KEY_DOWN", -1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_forward);

		Input_System::get().register_event({"editor_right",
			{
				{"KEY_A", -1.0f},
				{"KEY_D", 1.0f},
				{"KEY_LEFT", -1.0f},
				{"KEY_RIGHT", 1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_right);

		Input_System::get().register_event({"editor_up",
			{
				{"KEY_Q", -1.0f},
				{"KEY_E", 1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_up);

		Input_System::get().register_event({"editor_look_up",
			{{"MOUSE_POS_Y", -1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_mouse_up);

		Input_System::get().register_event({"editor_look_right",
			{{"MOUSE_POS_X", -1.0f}
			}}).bind(this, &Editor_Entry_Point::_editor_input_mouse_right);
	}

	void _deregister_camera_inputs()
	{
		Input_System::get().deregister_event("editor_camera_speed");
		Input_System::get().deregister_event("editor_forward");
		Input_System::get().deregister_event("editor_right");
		Input_System::get().deregister_event("editor_look_up");
		Input_System::get().deregister_event("editor_look_right");
	}

	void _editor_input_game_start(float value)
	{
		if (value > 0.5f)
		{
			return;
		}

		if (!active_game.started)
		{
			_deregister_camera_inputs();

			active_game = game;
			active_game.initialize();
			active_game.start();
		}
	}

	void _editor_input_game_stop(float value)
	{
		if (value > 0.5f)
		{
			return;
		}

		if (active_game.started)
		{
			active_game.shutdown();

			Renderer& renderer = Renderer::get();
			renderer.set_active_camera(editor_camera);

			_register_camera_inputs();
		}
		else
		{
			wants_to_exit = true;
		}
	}

	void _editor_input_camera_speed(float value)
	{
		if (value > 0.0f) value = 1.25f;
		else value = 0.75f;
		editor_movement_speed *= value;
	}

	void _editor_input_forward(float value)
	{
		editor_movement_input.y = value;
	}

	void _editor_input_right(float value)
	{
		editor_movement_input.x = value;
	}

	void _editor_input_up(float value)
	{
		editor_movement_input.z = value;
	}

	void _editor_input_mouse_up(float value)
	{
		editor_rotation_input.y = value;
	}

	void _editor_input_mouse_right(float value)
	{
		editor_rotation_input.x = value;
	}
};

int main()
{
	Engine engine;
	engine.initialize({});

	Editor_Entry_Point editor;
	engine.run(editor);

	return 0;
}
