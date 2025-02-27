// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/event.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/renderer/camera.hpp"

namespace VR_Eye
{
    enum Type : uint8
    {
        Left = 0,
        Right = 1,
        None = 2,
    };
};

#ifdef CS_WITH_VR_SUPPORT
#define OPENVR_BUILD_STATIC
#include "openvr.h"
#endif // CS_WITH_VR_SUPPORT

class VR_Camera : public Perspective_Camera
{
public:
    virtual ~VR_Camera() = default;

    void set_world(const mat4& in_world);
	void set_projection(const mat4& in_projection);
	void set_view(const mat4& in_view);
};

namespace VR_Device_Status
{
    enum Type : uint8
    {
        Activated,
        Deactivated,
        Updated
    };
};

struct VR_Device
{
private:
    int32 _device_index;
};

class VR_System : public Singleton<VR_System>
{
public:
    // Device Index, 
    Event<int32, VR_Device_Status::Type> on_vr_device_status_change;

public:
    void initialize();
    void shutdown();

    void poll_events();

    void update(float dt);
    void render();
    
#ifdef CS_WITH_VR_SUPPORT
    bool is_valid() { return _vr_system != nullptr; }
#else
    bool is_valid() { return false; }
#endif //CS_WITH_VR_SUPPORT 

    Shared_Ptr<Camera> get_camera(VR_Eye::Type eye) const;

    void set_custom_camera_pose(const mat4& pose);

    void get_viewport(uint32& width, uint32& height);

#ifdef CS_WITH_VR_SUPPORT
private:
    vr::IVRSystem* _vr_system { nullptr };
	vr::IVRRenderModels* _vr_render_models { nullptr };

private:
	bool _vr_show_tracked_device[ vr::k_unMaxTrackedDeviceCount ];

	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses; 

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class
	mat4 _pose_matrices[ vr::k_unMaxTrackedDeviceCount ];
#endif //CS_WITH_VR_SUPPORT

    Shared_Ptr<VR_Camera> _camera[3];

public:
    mat4 _head_view_matrix { mat4(1.0f) };
    mat4 _custom_pose { mat4(1.0f) };

	mat4 _get_eye_projection(VR_Eye::Type eye);
	mat4 _get_eye_pose(VR_Eye::Type eye);
	mat4 _get_current_view_projection(VR_Eye::Type eye);

    void _update_camera(float dt);    
};

#ifdef CS_WITH_VR_SUPPORT
mat4 vr_to_mat4(const vr::HmdMatrix34_t &mat);
mat4 vr_to_mat4(const vr::HmdMatrix44_t &mat);
#endif //CS_WITH_VR_SUPPORT

