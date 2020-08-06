include(FetchContent REQUIRED)

function(fetch_dep REPO_NAME GIT_REPO GIT_TAG ADD_SUBDIR)
    FetchContent_Declare(
        ${REPO_NAME}
        GIT_REPOSITORY ${GIT_REPO}
        #GIT_TAG f6b406427400ed7ddb56cfc2577b6af571827c8c
        GIT_TAG ${GIT_TAG}
        )
    if(ADD_SUBDIR)
        if(${CMAKE_VERSION} VERSION_LESS 3.14)
            FetchContent_Populate(${REPO_NAME})
            add_subdirectory(${${REPO_NAME}_SOURCE_DIR} ${${REPO_NAME}_BINARY_DIR})
        else()
            FetchContent_MakeAvailable(${REPO_NAME})
        endif()
    else()
        FetchContent_Populate(${REPO_NAME})
    endif()
    set(${REPO_NAME}_SOURCE_DIR ${${REPO_NAME}_SOURCE_DIR} PARENT_SCOPE)
    set(${REPO_NAME}_BINARY_DIR ${${REPO_NAME}_BINARY_DIR} PARENT_SCOPE)
endfunction()

if(NOT Eigen3_FOUND)
    set(BUILD_TESTING OFF)
    fetch_dep(eigen https://gitlab.com/libeigen/eigen.git bcbaad6d874d451817457ae0603f953cda3c0c06 OFF)
    set(EIGEN3_INCLUDE_DIR ${eigen_SOURCE_DIR})
    find_package(Eigen3 REQUIRED)
endif()

if(ART_USE_OPENGL)
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)

    find_package(glfw 3.3.2 QUIET)
    if(NOT glfw_FOUND)
        fetch_dep(glfw https://github.com/glfw/glfw 3.3.2 ON)
    endif()

    if(NOT DEFINED IMGUI_DIR)
        fetch_dep(imgui https://github.com/ocornut/imgui.git v1.74 OFF)
        set(IMGUI_DIR ${imgui_SOURCE_DIR})
    endif()
    find_package(ImGui COMPONENTS Sources REQUIRED)
    set(ImGui_INCLUDE_DIR ${ImGui_INCLUDE_DIR} CACHE STRING "Location of the imgui library headers")


endif()


find_package(fmt QUIET)
if(NOT fmt_FOUND)
fetch_dep(fmt https://github.com/fmtlib/fmt.git 6.2.1 ON)
endif()

find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
fetch_dep(spdlog https://github.com/gabime/spdlog.git v1.5.0 ON)
endif()

if(BUILD_TESTING)
    if(NOT Catch2_FOUND)
        fetch_dep(
            catch2
            https://github.com/catchorg/Catch2.git
            v2.9.1
            ON
            )

    endif()
endif()


option(CXXOPTS_BUILD_EXAMPLES "Set to ON to build cxxopts examples" OFF)
option(CXXOPTS_BUILD_TESTS "Set to ON to build cxxopts tests" OFF)
find_package(cxxopts 2.2.0 QUIET)
if(NOT cxxopts_FOUND)
    fetch_dep(cxxopts https://github.com/jarro2783/cxxopts v2.2.0 ON)
endif()


find_package(FMT REQUIRED)
