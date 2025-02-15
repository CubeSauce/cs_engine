# CS Engine
CS - CubeSauce
Amalgation of various project over the years
Current version supports DirectX and Windows only, Vulkan and OpenGL with Linux/MacOs coming soon
Build using CMake 
OpenVR needs to be in `tempBeta` branch

Proper game examples coming soon

## TODO:
### Engine
- [x] Basic resources
- [x] Event system (signal/brodcast-listener)
- [x] Name_Id (string - int32 - FName from unreal engine)
- [x] Basic containers (Dynamic_Array, Linked_List, Hash_Table) - needs improvement
- [x] CVar parsing
- [ ] CVar serializing
- [x] Basic math (vec2-4, mat4, quat) - needs improvement
### Rendering api - needs improvement
- [x] DirectX
- [ ] Vulkan
- [x] OpenGL
### Window framework
- [x] GLFW
- [ ] SDL
### OpenVR integration - needs improvement
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
