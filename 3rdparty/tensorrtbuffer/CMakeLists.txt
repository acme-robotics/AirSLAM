cmake_minimum_required(VERSION 3.6)
project(tensorrtbuffer)

set(CMAKE_CXX_STANDARD 17)

if(POLICY CMP0146)
    cmake_policy(SET CMP0146 OLD)
endif()

find_package(CUDA REQUIRED)
include_directories("${CUDA_INCLUDE_DIRS}")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# n6-orin-patches: sample_options.cpp / sample_utils.cpp are trtexec CLI infra that
# AirSLAM never uses, and they reference TensorRT-8-only APIs removed in TRT 10
# (IFullyConnectedLayer, PreviewFeature::kFASTER_DYNAMIC_SHAPES_0805, ...). AirSLAM
# only needs the header-only BufferManager/common helpers + the logger, so we build
# just logger.cpp here.
add_library(tensorrtbuffer SHARED src/logger.cpp)



