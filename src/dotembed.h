//
// Created by loulfy on 20/01/2023.
//

#ifndef DOT_EMBED_H
#define DOT_EMBED_H

#include <memory>
#include <string>
#include <filesystem>

namespace dot
{
    #ifndef WIN32
    using char_t = char;
    #else
    using char_t = wchar_t;
    #endif
    using string_t = std::basic_string<char_t>;
    class Assembly
    {
    public:

        Assembly() = default;
        virtual ~Assembly() = default;

        // Non-copyable and non-movable
        Assembly(const Assembly&) = delete;
        Assembly(const Assembly&&) = delete;
        Assembly& operator=(const Assembly&) = delete;
        Assembly& operator=(const Assembly&&) = delete;

        virtual void execute(const string_t& method) = 0;
    };

    using AssemblyPtr = std::unique_ptr<Assembly>;

    class HostFxr
    {
    public:

        virtual ~HostFxr() = default;
        virtual AssemblyPtr loadAssembly(const std::filesystem::path& config_path) = 0;
        static std::unique_ptr<HostFxr> Init();
    };

    using HostFxrManager = std::unique_ptr<HostFxr>;
}

#endif //DOT_EMBED_H
