# dotembed

C++ [Native Host](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting) embedding C# cross-platform (works with GNU/Linux).

## Build with [VCPKG](https://vcpkg.io/en/getting-started.html) (Win64)
```shell
vcpkg install nethost --triplet x64-windows
vcpkg integrate install

cmake .. -G "Visual Studio 17" -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

you need to build C# test libraries with dotnet, example:
```shell
cd managed/ManagedLibA
dotnet build
```
In the bin folder, copy/paste the files in the dotest directory:
* ManagedLibA.runtimeconfig  
* ManagedLibA.dll