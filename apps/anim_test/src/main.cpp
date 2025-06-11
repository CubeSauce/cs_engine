
#include <string>

#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/engine.hpp"

class Anim_Test : public Game_Instance
{
	void init() override
	{

	}

	void pre_physics_update(float dt) override
	{

	}

	void post_physics_update(float dt) override
	{

	}

	void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) override
	{

	}

	void shutdown() override
	{

	}
};

int main(int argc, char** argv)
{
	Dynamic_Array<std::string> args;
	for (int32 a = 1; a < argc; ++a)
	{
		args.push_back(argv[a]);
	}

	Engine engine;
	engine.initialize(args);
	engine.game_instance = Shared_Ptr<Anim_Test>::create();
	engine.run();
	engine.shutdown();

	Profiler::get().write_to_chrometracing_json("profiling/anim_test.json");

	return 0;
}
