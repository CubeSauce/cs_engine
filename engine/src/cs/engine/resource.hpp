// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/engine/name_id.hpp"

class Resource
{
public:
    Name_Id name;
    
public:
    virtual ~Resource() = default;
};
