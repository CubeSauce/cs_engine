# CS Engine
CS - CubeSauce
Amalgation of various project over the years - the point is not the best possible engine, it's just to learn what to do and what not to do and how to do it
Current version supports DirectX and Windows only, Vulkan and OpenGL with Linux/MacOs coming soon
Build using CMake 
OpenVR needs to be in `tempBeta` branch

Proper game examples coming soon

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
### Rendering
- [x] DirectX
- [ ] Vulkan
- [x] OpenGL
- [ ] Batch rendering
- [ ] Hot-Reloading shaders
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
