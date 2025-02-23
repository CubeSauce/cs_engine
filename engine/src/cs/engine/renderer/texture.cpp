// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/renderer/texture.hpp"

#include "stb/stb_image.h"

Texture_Resource::Texture_Resource(const std::string& filepath)
{
    assert(initialize_from_file(filepath));
}

Texture_Resource::~Texture_Resource()
{
    if (data)
    {
        stbi_image_free(data);
        data = nullptr;
    }
}

bool Texture_Resource::initialize_from_file(const std::string& filepath)
{
    int width_i, height_i, num_channels_i;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(filepath.c_str(), &width_i, &height_i, &num_channels_i, 4);

    if (data == nullptr)
    {
        return false;
    }

    width = (uint32)width_i;
    height = (uint32)height_i;
    num_channels = 4; // Always set because of stbi_load param
    
    return true;
}
