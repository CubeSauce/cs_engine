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
};

struct Test_Entity
{
	Component_Handle<Transform_Component> h_transform;
	Component_Handle<Render_Component> h_render;
};

struct My_Game
{
	bool initialized {false};

	Shared_Ptr<Perspective_Camera> scene_camera;

	Dynamic_Component_Storage<
		Transform_Component,
		Render_Component
	> components;

	Player_Entity player;
	Dynamic_Array<Test_Entity> tests;

	Shared_Ptr<Mesh_Resource> unit_sphere;
	Shared_Ptr<Mesh_Resource> unit_box;

	float t = 0.0f;
	void initialize()
	{
		components.initialize();

		Renderer& renderer = Renderer::get();
		renderer.window->on_window_resize.bind([this](uint32 width, uint32 height) {
			initialize_camera(width, height);
		});

		uint32 width, height;
		renderer.window->get_window_size(width, height);
		initialize_camera(width, height);

		renderer.set_active_camera(scene_camera);

		player = make_player();

		if (!unit_sphere.is_valid())
		{
			unit_sphere = Shared_Ptr<Mesh_Resource>::create();
			unit_sphere->initialize_from_file("assets/meshes/test/unit_sphere.obj");
		}

		if (!unit_box.is_valid())
		{
			unit_box = Shared_Ptr<Mesh_Resource>::create();
			unit_box->initialize_from_file("assets/meshes/test/unit_box.obj");
		}

		tests.clear();
		tests.push_back(make_test(unit_sphere, vec3(-3.0f, 3.0f, -1.0f)));
		tests.push_back(make_test(unit_box, vec3(3.0f, 3.0f, 1.0f)));

		t = 0.0f;

		initialized = true;
	}

	void update(float dt)
	{
		t += dt;

		if (scene_camera.is_valid())
		{
			scene_camera->position = components.get(player.h_transform).local_position;
		}

		for (Test_Entity& test : tests)
		{
			Transform_Component& transform_component = components.get<Transform_Component>(test.h_transform);
			transform_component.local_orientation = quat::from_euler_angles(vec3(0.0f ,0.0f, t));
			transform_component.calculate_local_matrix();

			Render_Component& render_component = components.get<Render_Component>(test.h_render);
			render_component.model_matrix = transform_component.local_matrix;
		}
	}

	void render()
	{
		Renderer& renderer = Renderer::get();

		for (Render_Component& component : components.get_array<Render_Component>())
		{
			renderer.backend->draw_mesh(component.mesh, component.model_matrix);
		}
	}

	void shutdown()
	{
		initialized = false;
	}

	void initialize_camera(uint32 width, uint32 height)
	{
		if (!scene_camera.is_valid())
		{
			scene_camera = Shared_Ptr<Perspective_Camera>::create();
		}

		scene_camera->aspect_ratio = width / static_cast<float>(height);
		scene_camera->position = vec3::zero_vector;
		scene_camera->orientation = quat::from_direction(vec3::forward_vector);
	}

	Player_Entity make_player()
	{
		Player_Entity player;
		Transform_Component& transform_component = components.add<Transform_Component>(player.h_transform);
		transform_component.local_position = vec3(0.0f, -15.0f, 0.0f);
		return player;
	}

	Test_Entity make_test(const Shared_Ptr<Mesh_Resource> &test_mesh, const vec3& position)
	{
		Test_Entity test;

		Transform_Component& transform_component = components.add<Transform_Component>(test.h_transform);
		transform_component.local_position = position;

		Render_Component& render_component = components.add<Render_Component>(test.h_render);

		Renderer& renderer = Renderer::get();

		// TODO: this probably should happen in-engine when it's requested to be used (will also handle if we change
		// the renderer on-fly) as well as reusing the same mesh for multiple instances (this will make copies, ok for now)
		render_component.mesh = renderer.backend->create_mesh(test_mesh);
		render_component.mesh->upload_data();

		return test;
	}
};

struct Editor_Entry_Point : Entry_Point
{
	bool wants_to_exit{false};
	Shared_Ptr<Camera> editor_camera;

	My_Game game;

public:
	void initialize() override
	{
		printf("Editor initializing... ");

		register_inputs();
		initialize_editor_camera();

		printf("DONE\n");
		flushall();
	}

	void update(float dt) override
	{
		if (game.initialized)
		{
			game.update(dt);
		}
	}

	void render(VR_Eye::Type eye) override
	{
		Renderer& renderer = Renderer::get();
		Shared_Ptr<Renderer_Backend> renderer_backend = Renderer::get().backend;

		game.render();
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

	void initialize_editor_camera()
	{
		Renderer& renderer = Renderer::get();
		renderer.window->on_window_resize.bind([this](uint32 width, uint32 height) {
			initialize_camera(width, height);
		});

		uint32 width, height;
		renderer.window->get_window_size(width, height);
		initialize_camera(width, height);
	}

	void initialize_camera(uint32 width, uint32 height)
	{
		if (!editor_camera.is_valid())
		{
			editor_camera = Shared_Ptr<Perspective_Camera>::create();
		}

		editor_camera->aspect_ratio = width / static_cast<float>(height);
		editor_camera->position = vec3(-10.0f, -10.0, 10.0f);
		editor_camera->orientation = quat::from_direction(-editor_camera->position);
	}

private:
	void register_inputs()
	{
		Input_System::get().register_event({"game_start",
			{{"KEY_P", 1.0f}, {"KEY_F5", 1.0f}}}).bind(
			[this](float value) {
				if (value > 0.5f) return;

				if (!game.initialized)
				{
					game.initialize();
				}
			});

		Input_System::get().register_event({"game_stop", {{"KEY_ESCAPE", 1.0f}}}).bind(
			[this](float value) {
			if (value > 0.5f) return;

			if (game.initialized)
			{
				game.shutdown();

				Renderer& renderer = Renderer::get();
				renderer.set_active_camera(editor_camera);
			}
			else
				wants_to_exit = true;
		});
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
