#pragma once

#include "cs/cs.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/vr/vr_system.hpp"

class Renderer;
class Game_Instance
{
public:
    virtual ~Game_Instance() {};

    virtual void init() = 0;
    virtual void pre_physics_update(float dt) = 0;
    virtual void post_physics_update(float dt) = 0;
    virtual void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) = 0;
    virtual void shutdown() = 0;
};
