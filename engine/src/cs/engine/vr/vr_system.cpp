#include "cs/engine/vr/vr_system.hpp"
#include "cs/engine/profiling/profiler.hpp"

void VR_Camera::set_world(const mat4& in_world)
{
    _world = in_world;
}

void VR_Camera::set_projection(const mat4& in_projection)
{
    _projection = in_projection;
}

void VR_Camera::set_view(const mat4& in_view)
{
    _view = in_view;
}

template<> 
VR_System* Singleton<VR_System>::_singleton { nullptr };

void VR_System::initialize()
{
#ifdef CS_WITH_VR_SUPPORT
    PROFILE_FUNCTION()

    if (!vr::VR_IsRuntimeInstalled())
    {
        return;
    }

    vr::EVRInitError vr_error = vr::VRInitError_None;
    _vr_system = vr::VR_Init( &vr_error, vr::VRApplication_Scene );	
    if (vr_error != vr::VRInitError_None)
    {
        shutdown();
        return;
    }
    
    _vr_render_models = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(
        vr::IVRRenderModels_Version, &vr_error);
    assert(vr_error == vr::VRInitError_None);

    assert(vr::VRCompositor());
#endif //CS_WITH_VR_SUPPORT

    for (int i = 0; i < 3; ++i)
    {
        _camera[i] = Shared_Ptr<VR_Camera>::create();
    }

    _camera[2]->FOV = 45_deg;
    _camera[2]->near_d = 0.0f;
    _camera[2]->far_d = 1000.0f;
    _camera[2]->aspect_ratio = 1.0f;
    _camera[2]->position = {0.0f, 0.0f, -3.0f};
}

void VR_System::shutdown()
{
#ifdef CS_WITH_VR_SUPPORT
    PROFILE_FUNCTION()
    vr::VR_Shutdown();
    _vr_system = nullptr;
#endif //CS_WITH_VR_SUPPORT
}

void VR_System::poll_events()
{
#ifdef CS_WITH_VR_SUPPORT
    PROFILE_FUNCTION()

    if (!_vr_system)
    {
        return;
    }

    vr::VREvent_t event;
    while(_vr_system->PollNextEvent(&event, sizeof( event )))
    {
        switch( event.eventType )
        {
        case vr::VREvent_TrackedDeviceActivated:
        {
            on_vr_device_status_change.broadcast(event.trackedDeviceIndex, VR_Device_Status::Activated);
            break;
        }
        case vr::VREvent_TrackedDeviceDeactivated:
        {
            on_vr_device_status_change.broadcast(event.trackedDeviceIndex, VR_Device_Status::Deactivated);
            break;
        }
        case vr::VREvent_TrackedDeviceUpdated:
        {
            on_vr_device_status_change.broadcast(event.trackedDeviceIndex, VR_Device_Status::Updated);
            break;
        }
        }
    }

    for( vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++ )
    {
        vr::VRControllerState_t state;
        if( _vr_system->GetControllerState( unDevice, &state, sizeof(state) ) )
        {
            _vr_show_tracked_device[unDevice] = state.ulButtonPressed == 0;
        }
    }
#endif //CS_WITH_VR_SUPPORT
}

void VR_System::update(float dt)
{
#ifdef CS_WITH_VR_SUPPORT
    PROFILE_FUNCTION()

    if (!is_valid())
    {
        return;
    }

    vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;
			_pose_matrices[nDevice] = vr_to_mat4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if (m_rDevClassChar[nDevice]==0)
			{
				switch (_vr_system->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		_head_view_matrix = _pose_matrices[vr::k_unTrackedDeviceIndex_Hmd].inverse();
	}

    _update_camera(dt);
#endif //CS_WITH_VR_SUPPORT
}

void VR_System::render()
{
}

Shared_Ptr<Camera> VR_System::get_camera(VR_Eye::Type eye) const 
{
    return _camera[eye]; 
}

void VR_System::set_custom_camera_pose(const mat4& pose)
{
    _custom_pose = pose;
}

void VR_System::get_viewport(uint32& width, uint32& height)
{
#ifdef CS_WITH_VR_SUPPORT
    _vr_system->GetRecommendedRenderTargetSize(&width, &height);
#endif //CS_WITH_VR_SUPPORT
}

mat4 VR_System::_get_eye_projection(VR_Eye::Type eye)
{
#ifdef CS_WITH_VR_SUPPORT
    if (!is_valid())
    {
        return mat4(1.0f);
    }

    if (eye == VR_Eye::None)
    {
        return _camera[eye]->get_projection();
    }

	vr::HmdMatrix44_t mat = _vr_system->GetProjectionMatrix((vr::Hmd_Eye) eye, 0.1f, 10000.0f);
    return vr_to_mat4(mat);
#else
        return mat4(1.0f);
#endif //CS_WITH_VR_SUPPORT
}

mat4 VR_System::_get_eye_pose(VR_Eye::Type eye)
{
#ifdef CS_WITH_VR_SUPPORT
	if (!is_valid() || eye == VR_Eye::None)
    {
		return mat4(1.0f);
    }

	vr::HmdMatrix34_t pose = _vr_system->GetEyeToHeadTransform((vr::Hmd_Eye) eye);
	return vr_to_mat4(pose).inverse();
#else
    return mat4(1.0f);
#endif //CS_WITH_VR_SUPPORT
}

mat4 VR_System::_get_current_view_projection(VR_Eye::Type eye)
{
	return _head_view_matrix * _get_eye_pose(eye) * _get_eye_projection(eye);
}

#ifdef CS_WITH_VR_SUPPORT
mat4 vr_to_mat4(const vr::HmdMatrix34_t &mat)
{
    return mat4(
		{ mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f },
		{ mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f }, 
		{ mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f }, 
		{ mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f }
    );
}

mat4 vr_to_mat4(const vr::HmdMatrix44_t &mat)
{
    return mat4(
		{ mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0] },
		{ mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1] }, 
		{ mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2] }, 
		{ mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3] }
    );
}
#endif //CS_WITH_VR_SUPPORT

static mat4 toZup = quat::from_euler_angles(vec3(90_deg, 0.0f, 0.0f)).to_mat4();
void VR_System::_update_camera(float dt)
{
    PROFILE_FUNCTION()
    
    mat4 pose = _custom_pose;
    pose = pose.inverse();

#ifdef CS_WITH_VR_SUPPORT
    for (uint8 eye = VR_Eye::Left; eye <= VR_Eye::None; eye++)
    {
        _camera[eye]->set_projection(_get_eye_projection((VR_Eye::Type)eye));
        _camera[eye]->set_view(_get_eye_pose((VR_Eye::Type)eye) * _head_view_matrix * toZup * pose);
    }
#endif //CS_WITH_VR_SUPPORT
}