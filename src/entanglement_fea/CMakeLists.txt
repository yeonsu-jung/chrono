#--------------------------------------------------------------
# List all FEA demos that do have no other dependencies
set(FEA_DEMOS
    entanglement_fea222
)

# TO DO: ASSERT Irrlicht (ans see if there are any other dependencies)

#--------------------------------------------------------------

# Set include paths, compiler & linker flags, and libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CH_INCLUDES})
set(COMPILER_FLAGS "${CH_CXX_FLAGS}")
set(LINKER_FLAGS "${CH_LINKERFLAG_EXE}")
list(APPEND LIBS "ChronoEngine")

if(ENABLE_MODULE_IRRLICHT)
  include_directories(${CH_IRRLICHTINC})
  set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CH_IRRLICHT_CXX_FLAGS}")
  list(APPEND LIBS "ChronoEngine_irrlicht")
endif()

# if(ENABLE_MODULE_PARDISO_MKL)
#   include_directories(${CH_MKL_INCLUDES})
#   set(COMPILER_FLAGS "${COMPILER_FLAGS} ${CH_MKL_CXX_FLAGS}")
#   set(LINKER_FLAGS "${LINKER_FLAGS} ${CH_MKL_LINK_FLAGS}")
#   list(APPEND LIBS "ChronoEngine_pardisomkl")
# endif()

# if(ENABLE_MODULE_MUMPS)
#   include_directories(${CH_MUMPS_INCLUDES})
#   list(APPEND LIBS "ChronoEngine_mumps")
# endif()

# if(ENABLE_MODULE_MATLAB)
#   include_directories(${CH_MATLABINC})
#   list(APPEND LIBS "ChronoEngine_matlab")
# endif()

# if(ENABLE_MODULE_PARDISO_PROJECT)
#   include_directories(${CH_PARDISOPROJECT_INCLUDES})
#   list(APPEND LIBS "ChronoEngine_pardisoproject")
#   list(APPEND LIBS ${PARDISOPROJECT_LIBRARIES})
# endif()

# if(ENABLE_MODULE_POSTPROCESS)
#   list(APPEND LIBS "ChronoEngine_postprocess")
# endif()

#--------------------------------------------------------------

message(STATUS "Entanglement simulation with FEA...")

foreach(PROGRAM ${FEA_DEMOS})
    message(STATUS "...add ${PROGRAM}")

    add_executable(${PROGRAM}  "${PROGRAM}.cpp")
    source_group(""  FILES "${PROGRAM}.cpp")

    set_target_properties(${PROGRAM} PROPERTIES
        FOLDER demos
        COMPILE_FLAGS "${COMPILER_FLAGS}"
        LINK_FLAGS "${LINKER_FLAGS}")
    if(APPLE)
      set_target_properties(${PROGRAM} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${MACOSX_BUNDLE_INFO_PLIST}")
    endif()
    set_property(TARGET ${PROGRAM} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROGRAM}>")
    target_link_libraries(${PROGRAM} ${LIBS})
    
    install(TARGETS ${PROGRAM} DESTINATION ${CH_INSTALL_DEMO})
endforeach(PROGRAM)

