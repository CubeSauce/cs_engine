#include "cs/engine/engine.hpp"
#include "cs/engine/input.hpp"

struct Game
{
	bool initialized{false};

	struct State
	{
		vec3 raw_input{vec3::zero_vector};
		vec3 movement_vec{vec3::zero_vector};
		vec3 position{vec3::zero_vector};
	} state;

	// @TODO: Expose to a config file - rebinding?
	Input_Event_Setup movement_forward {
		"movement_forward", {
				{"KEY_W", 1.0f},
				{"KEY_S", -1.0f},
				{"KEY_UP", 1.0f},
				{"KEY_DOWN", -1.0f}
		}};
	Input_Event_Setup movement_right {
		"movement_right", {
				{"KEY_A", -1.0f},
				{"KEY_D", 1.0f},
				{"KEY_LEFT", -1.0f},
				{"KEY_RIGHT", 1.0f}
		}};

	void initialize()
	{
		printf("Initializing game ... ");
		// resetting here so I have some values to debug at the end;
		state = State{};

		Input_System::get().register_event(movement_forward).bind(this, &Game::on_movement_forward);
		Input_System::get().register_event(movement_right).bind(this, &Game::on_movement_right);

		Input_System::get().register_event({"test", {
			{"KEY_F5", 1.0f}
		}}).bind([this](float value) { if (value > 0.5f) return;
			printf("F5 %f %f\n", state.position.x, state.position.y);
			flushall();
		});
		printf("DONE!\n");
		flushall();

		initialized = true;
	}

	void update(float dt)
	{
		state.position += state.movement_vec * (dt * 10.0f);
	}

	void shutdown()
	{
		printf("Shutting game down ... ");

		initialized = false;

		Input_System::get().deregister_event("movement_forward");
		Input_System::get().deregister_event("movement_right");
		Input_System::get().deregister_event("test");

		printf("DONE!\n");
		flushall();
	}

	void on_movement_forward(float value)
	{
		state.raw_input.y = value;
		float len = state.raw_input.length();
		len = len > 1e-4f ? len : 1.0f;
		state.movement_vec = state.raw_input / len;
	}

	void on_movement_right(float value)
	{
		state.raw_input.x = value;
		float len = state.raw_input.length();
		len = len > 1e-4f ? len : 1.0f;
		state.movement_vec = state.raw_input / len;
	}
};


struct Editor_Entry_Point : Entry_Point
{
	bool wants_to_exit{false};
	Game game;

public:
	void initialize() override
	{
		printf("Editor initializing... \n");

		register_inputs();

		flushall();
	}

	void update(float dt) override
	{
		if (game.initialized)
		{
			game.update(dt);
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

private:
	void register_inputs()
	{
		Input_System::get().register_event({"game_start",
			{{"KEY_P", 1.0f}, {"KEY_F5", 1.0f}}}).bind(
			[this](float value) {
				if (value > 0.5f) return;

				if (!game.initialized)
					game.initialize();
			});

		Input_System::get().register_event({"game_stop", {{"KEY_ESCAPE", 1.0f}}}).bind(
			[this](float value) {
			if (value > 0.5f) return;

			if (game.initialized)
				game.shutdown();
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
