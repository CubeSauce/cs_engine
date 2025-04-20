#include "cs/engine/profiling/profiler.hpp"

#include <fstream>

template<> 
Profiler* Singleton<Profiler>::_singleton { nullptr };

void Profiler::start(const Name_Id& name)
{
    auto now = std::chrono::high_resolution_clock::now();
    long long timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                              now.time_since_epoch())
                              .count();

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _entries.push_back({name, 'B', timestamp, std::this_thread::get_id()});
    }

    _get_tls_stack().push(name);
}

void Profiler::stop()
{
    std::stack<Name_Id>& stack = _get_tls_stack();

    if (stack.empty()) return;

    Name_Id name = stack.top();
    stack.pop();

    auto now = std::chrono::high_resolution_clock::now();
    long long timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                              now.time_since_epoch())
                              .count();

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _entries.push_back({name, 'E', timestamp, std::this_thread::get_id()});
    }
}

void Profiler::clear()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _entries.clear();
}

void Profiler::write_to_chrometracing_json(const std::string& filename)
{
#ifdef CS_WITH_PROFILING
    PROFILE_FUNCTION()

    //TODO: shared_lock as we're only reading
    std::lock_guard<std::mutex> lock(_mutex);

    std::ofstream file(filename);
    file << "{ \"traceEvents\": [\n";
    for (int32 i = 0; i < _entries.size(); ++i) {
        file << "  { \"name\": \"" << _entries[i].name.str << "\", "
            << "\"ph\": \"" << _entries[i].phase << "\", "
            << "\"ts\": " << _entries[i].timestamp << ", "
            << "\"pid\": 0, "
            << "\"tid\": " << _entries[i].tid << " }";
        if (i != _entries.size() - 1) file << ",";
        file << "\n";
    }
    file << "]}\n";
    file.close();
#else
#endif //CS_WITH_PROFILING
}

std::stack<Name_Id>& Profiler::_get_tls_stack()
{
    thread_local std::stack<Name_Id> _stack;
    return _stack;
}

Scoped_Profiler::Scoped_Profiler(const Name_Id& name)
{
    Profiler::get().start(name);
}

Scoped_Profiler::~Scoped_Profiler()
{
    Profiler::get().stop();
}
