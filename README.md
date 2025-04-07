# CS Engine
CS - CubeSauce
Every iteration of every engine/game I tried to do from scratch is a part of this project. "Learn from failing" is basically my motto, a lot of stuff is wrong, maybe even all of it, but at least I learn why. Having fun is more important then being right.

Current version supports DirectX and Windows only, Vulkan and OpenGL with Win/Linux/MacOs coming soon

Build using CMake, requires manual GLEW snapshot (with generated sources). Requires glslangValidator & Spirv-Cross installed for shader compilation.

## TODO:
### Engine
- [x] Basic resources
- [x] Event system (signal/brodcast-listener)
- [x] Name_Id (string - int32 - FName from unreal engine)
- [x] Basic containers (Dynamic_Array, Linked_List, Hash_Table)
- [x] CVar parsing
- [ ] CVar serializing
- [x] Basic math (vec2-4, mat4, quat)
- [x] Input system
- [x] Multithreading (Task/Job system)
- [x] Profiling ([chrometracing](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool/))
### Physics
- [x] Broadphase Sweep and Prune
- [x] GJK and EPA for Convex to Convex collisions
- [ ] Example integration with game code
### Rendering
- [x] DirectX
- [ ] Vulkan
- [x] OpenGL
- [ ] Batch rendering
- [ ] Hot-Reloading shaders
- [ ] HLSL into SPIR-V/GLSL (kinda done, but there are errors in the shaders)
### Window framework
- [x] GLFW
- [ ] SDL
### OpenVR integration
- [x] Get a headset to connect (PSVR)
- [x] Make it work in my coordinate space (right handed +Z as up)
- [x] Orientation tracking
- [ ] Movement tracking
- [ ] Hand tracking
### Networking - needs improvement
- [x] Basic sockets
- [ ] Data serialization
- [x] Net roles (Server/Client/Offline) - although not happy with it
### Audio
- [ ] Plugins (FMOD/WWise)
