

set BINARIES_DIR="build"


pushd ..\..\bootstrapping\src\vulkan_headers\

echo Building Vulkan Headers...
mkdir %BINARIES_DIR%
cd %BINARIES_DIR%
set DIR="%CD%\..\..\..\..\vulkan_headers"
cmake -D CMAKE_INSTALL_PREFIX=%DIR% ..
cmake --build . --target install

echo Vulkan Headers Done.
popd