find_package(OpenCL REQUIRED)
if (NOT APPLE)
  find_package(GLEW REQUIRED)
endif ()
find_package(glfw3 REQUIRED)
find_package(DO_Sara COMPONENTS Core ImageIO REQUIRED)


add_subdirectory(Core)
add_subdirectory(GL)

if (SARA_BUILD_TESTS)
  add_subdirectory(test)
endif ()
add_subdirectory(examples)
