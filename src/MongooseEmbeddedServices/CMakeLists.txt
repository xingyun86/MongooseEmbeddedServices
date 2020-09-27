# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("MongooseEmbeddedServices")

# Include sub-projects.
add_subdirectory ("vendor/mongoose")
add_subdirectory ("MongooseEmbeddedServices")
add_subdirectory ("ResourceServices")
