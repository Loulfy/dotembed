//
// Created by loulfy on 20/01/2023.
//

#include "dotembed.h"
#include <cassert>
#include <stdexcept>
#include <unordered_map>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

namespace dot
{
    struct HostFxrImpl : public HostFxr
    {
        HostFxrImpl();
        AssemblyPtr loadAssembly(const std::filesystem::path& config_path) override;

        // Globals to hold HostFxr exports
        hostfxr_initialize_for_runtime_config_fn init_fptr;
        hostfxr_get_runtime_delegate_fn get_delegate_fptr;
        hostfxr_close_fn close_fptr;

        static void* load_library(const wchar_t* path);
        static void* get_export(void* h, const char* name);
    };

    struct AssemblyImpl : public Assembly
    {
        void loadFunction(const std::wstring& method);
        void execute(const std::wstring& method) override;

        std::wstring name;
        load_assembly_and_get_function_pointer_fn handle = nullptr;
        std::unordered_map<std::wstring, component_entry_point_fn> funcs;
    };

    #ifdef WIN32
    #include <Windows.h>

    void* HostFxrImpl::load_library(const wchar_t* path)
    {
        HMODULE h = LoadLibraryW(path);
        assert(h != nullptr);
        return static_cast<void*>(h);
    }

    void* HostFxrImpl::get_export(void* h, const char* name)
    {
        auto mod = static_cast<HMODULE>(h);
        auto f = GetProcAddress(mod, name);
        assert(f != nullptr);
        return reinterpret_cast<void*>(f);
    }

    #else
    #include <dlfcn.h>
    #include <limits.h>

    #define MAX_PATH PATH_MAX

    void* HostFxrImpl::load_library(const wchar_t* path)
    {
        void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }

    void* HostFxrImpl::get_export(void* h, const char* name)
    {
        void* f = dlsym(h, name);
        assert(f != nullptr);
        return f;
    }

    #endif

    HostFxrImpl::HostFxrImpl()
    {
        // Pre-allocate a large buffer for the path to hostfxr
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
        if (rc != 0)
            throw std::runtime_error("Can not get HostFxr path");

        // Load HostFxr and get desired exports
        auto* lib = load_library(buffer);
        init_fptr = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(get_export(lib, "hostfxr_initialize_for_runtime_config"));
        get_delegate_fptr = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(get_export(lib, "hostfxr_get_runtime_delegate"));
        close_fptr = reinterpret_cast<hostfxr_close_fn>(get_export(lib, "hostfxr_close"));
    }

    HostFxrManager HostFxr::Init()
    {
        return std::make_unique<HostFxrImpl>();
    }

    AssemblyPtr HostFxrImpl::loadAssembly(const std::filesystem::path& config_path)
    {
        // Load .NET Core
        hostfxr_handle cxt = nullptr;
        void* load_assembly_and_get_function_pointer = nullptr;
        int rc = init_fptr(config_path.c_str(), nullptr, &cxt);
        if (rc < 0 || cxt == nullptr)
        {
            close_fptr(cxt);
            throw std::runtime_error("Load Assembly Failed");
        }

        auto assembly = std::make_unique<AssemblyImpl>();
        auto name = config_path.stem().stem().string();
        assembly->name = std::wstring(name.begin(), name.end());

        // Get the load assembly function pointer
        rc = get_delegate_fptr(cxt, hdt_load_assembly_and_get_function_pointer,
                &load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
            throw std::runtime_error("Get Delegate Failed");

        close_fptr(cxt);

        assembly->handle = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(load_assembly_and_get_function_pointer);
        return assembly;
    }

    void AssemblyImpl::loadFunction(const std::wstring& method)
    {
        const std::wstring path = name + L".dll";
        const std::wstring type = name + L".Lib, " + name;

        component_entry_point_fn entry = nullptr;
        int rc = handle(
            path.c_str(),
            type.c_str(),
            method.c_str(),
            nullptr /*delegate_type_name*/,
            nullptr,
            (void**)&entry);

        assert(rc == 0 && entry != nullptr);
        funcs[method] = entry;
    }

    void AssemblyImpl::execute(const std::wstring& method)
    {
        struct lib_args
        {
            const char_t *message;
            int number;
        };

        lib_args args{L"from host!", 42};

        if(funcs.contains(method))
            funcs[method](&args, sizeof(args));
        else
        {
            loadFunction(method);
            funcs[method](&args, sizeof(args));
        }
    }
}
