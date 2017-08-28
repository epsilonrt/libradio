cmake_minimum_required(VERSION 3.5)

project (radio_demo)

find_package(radio REQUIRED)
link_directories(${RADIO_LIBRARY_DIRS})

include (PiBoardInfo)
if (NOT PIBOARD_ID)
  message (STATUS "Check the target platform, you can use PIBOARD_ID to force the target...")
  GetPiBoardInfo()
endif (NOT PIBOARD_ID)

file(GLOB_RECURSE demo_list *.c)
file(GLOB_RECURSE demo_cpp_list *.cpp)
#message("demos=${demo_list};${demo_cpp_list}")

foreach(f ${demo_list} ${demo_cpp_list})
  get_filename_component(demo_name ${f} NAME_WE)
  get_filename_component(demo_dir ${f} DIRECTORY)
  get_filename_component(demo_ext ${f} EXT)

if ("${demo_name}" MATCHES "${PROJECT_NAME}*")
  
    message (STATUS "Add ${demo_name} executable...")
    add_executable (${demo_name} ${f}) 
    
    if (demo_ext STREQUAL ".c")
      set_property(TARGET ${demo_name} PROPERTY C_STANDARD 99)
    endif()
    target_link_libraries(${demo_name} ${RADIO_LIBRARIES})
    target_include_directories(${demo_name} PUBLIC ${demo_dir} ${RADIO_INCLUDE_DIRS})
    target_compile_options(${demo_name} PUBLIC ${RADIO_CFLAGS})
    
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${demo_name}
      DESTINATION bin)
  endif()
endforeach(f)
