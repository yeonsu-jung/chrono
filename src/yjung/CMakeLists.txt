#--------------------------------------------------------------
# Add executables
MESSAGE(STATUS "Yeonsu Jung's sketchy code...")

include_directories( ${CH_IRRLICHTINC} )
MESSAGE(STATUS ${CH_IRRLICHTINC} )

ADD_EXECUTABLE(yjung yjung.cpp)
TARGET_LINK_LIBRARIES(yjung ChronoEngine)
ADD_DEPENDENCIES(yjung ChronoEngine)
INSTALL(TARGETS yjung DESTINATION ${CH_INSTALL_DEMO})

# FOREACH(PROGRAM ${DEMOS})
#     MESSAGE(STATUS "...add ${PROGRAM}")

#     ADD_EXECUTABLE(${PROGRAM}  "${PROGRAM}.cpp")
#     SOURCE_GROUP(""  FILES "${PROGRAM}.cpp")

#     SET_TARGET_PROPERTIES(${PROGRAM} PROPERTIES
#         FOLDER demos
#         COMPILE_FLAGS "${CH_CXX_FLAGS}"
#         LINK_FLAGS "${CH_LINKERFLAG_EXE}")
#     SET_PROPERTY(TARGET ${PROGRAM} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROGRAM}>")
#     TARGET_LINK_LIBRARIES(${PROGRAM} ChronoEngine)
#     ADD_DEPENDENCIES(${PROGRAM} ChronoEngine)

#     INSTALL(TARGETS ${PROGRAM} DESTINATION ${CH_INSTALL_DEMO})
# ENDFOREACH(PROGRAM)
