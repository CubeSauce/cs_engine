#include "component.hpp"

DEFINE_PRINT_TYPE(Component);
DEFINE_PRINT_TYPE(Transform_Component);
DEFINE_PRINT_TYPE(Physics_Body_Component);
DEFINE_PRINT_TYPE(Render_Component);

mat4 Transform_Component::get_world_matrix() const
{
    mat4 ret = translate(mat4(1.0f), local_position);
    ret  = ret * local_orientation.to_mat4();
    return local_to_world * ret;
}

vec3 Transform_Component::get_world_position() const
{
    return local_to_world * local_position;
}

quat Transform_Component::get_world_orientation() const
{
    return {local_to_world * local_orientation.v, local_orientation.w};
}
