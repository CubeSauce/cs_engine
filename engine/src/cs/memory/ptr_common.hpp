// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

struct Ptr_Control_Block
{
    int32 strong_count = 1;
    int32 weak_count = 0;
};
