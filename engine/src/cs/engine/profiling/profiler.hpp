// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/containers/hash_table.hpp"

//TODO: make own stack
#include <stack>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>

#define CS_WITH_PROFILING
//#undef CS_WITH_PROFILING

class Profiler : public Singleton<Profiler>
{
    struct Profiling_Entry
    {
        Name_Id name;
        char phase; // B or E
        long long timestamp;
        std::thread::id tid;
    };

public:
    void start(const Name_Id& name);
    void stop();

    void clear();
    void write_to_chrometracing_json(const std::string& filename);

private:
    static std::stack<Name_Id>& _get_tls_stack();
    std::mutex _mutex;    // Maybe make the whole class TLS_Singleton to avoid locking?
    Dynamic_Array<Profiling_Entry> _entries;
};

class Scoped_Profiler
{
public:
    Scoped_Profiler(const Name_Id& name);
    ~Scoped_Profiler();
};

#ifdef CS_WITH_PROFILING
    #define PROFILE_FUNCTION() Scoped_Profiler profiler(__func__);
#else
    #define PROFILE_FUNCTION()
#endif //CS_WITH_PROFILING
