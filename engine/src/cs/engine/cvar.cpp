#include "cs/engine/cvar.hpp"

template<> 
CVar_Registry* Singleton<CVar_Registry>::_singleton { nullptr };
