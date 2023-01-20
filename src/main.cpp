#include <iostream>
#include "dotembed.h"

int main()
{
    auto host = dot::HostFxr::Init();

    for (const auto& entry : std::filesystem::directory_iterator("."))
    {
        if(entry.path().extension() == ".json")
        {
            std::cout << "Find: " << entry.path() << std::endl;
            auto assembly = host->loadAssembly(entry.path());
            assembly->execute(L"Hello");
        }
    }

    return EXIT_SUCCESS;
}
