// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/thread_pool.hpp"

template<> 
Thread_Pool* Singleton<Thread_Pool>::_singleton { nullptr };
