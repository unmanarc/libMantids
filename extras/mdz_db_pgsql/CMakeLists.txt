cmake_minimum_required(VERSION 3.10)

######################################
find_library (
    POSTGRESQL_LIB
    NAMES libpq pq # what to look for
)

if (NOT POSTGRESQL_LIB)
    message("-- WARNING: Did not found lib POSTGRESQL_LIB")
else()
    get_filename_component(LIB_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    get_filename_component(PARENT_DIRFULL ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    get_filename_component(PARENT_DIR ${PARENT_DIRFULL} NAME)

    find_path (
            PQ_INCLUDE_DIR NAMES libpq-fe.h PATHS /usr/include /usr/include/postgresql /usr/include/pgsql
        )

    if (PQ_INCLUDE_DIR)
        message("-- POSTGRESQL_LIB found at [${POSTGRESQL_LIB}] for ${LIB_NAME}")
        message("-- PQ_INCLUDE_DIR found at [${PQ_INCLUDE_DIR}]")

        project(${LIB_NAME})
        project(${LIB_NAME} VERSION ${SVERSION} DESCRIPTION "Mantids Libraries for PostgreSQL Database Connector")

        file(GLOB_RECURSE EDV_INCLUDE_FILES "./*.h*")
        file(GLOB_RECURSE EDV_SOURCE_FILES "./*.c*")

        add_library(${LIB_NAME} ${EDV_INCLUDE_FILES} ${EDV_SOURCE_FILES})
        

        

        set_target_properties(  ${LIB_NAME}
                                PROPERTIES VERSION ${PROJECT_VERSION}
                                SOVERSION 2
                                PUBLIC_HEADER "${EDV_INCLUDE_FILES}"
                              )

        configure_file( "genericlib.pc.in" "${LIB_NAME}.pc" @ONLY )

        target_include_directories(${LIB_NAME} PRIVATE .)

        install(
                TARGETS ${LIB_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
                ARCHIVE COMPONENT lib DESTINATION ${CMAKE_INSTALL_LIBDIR}
                LIBRARY COMPONENT lib DESTINATION ${CMAKE_INSTALL_LIBDIR}
                PUBLIC_HEADER COMPONENT dev DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}
                )

        install(
                FILES ${CMAKE_BINARY_DIR}/${PARENT_DIR}/${LIB_NAME}/${LIB_NAME}.pc
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
                )
        if (EXTRAPREFIX)
            target_include_directories(${LIB_NAME} PUBLIC ${EXTRAPREFIX}/include)
            target_link_libraries(${LIB_NAME} "-L${EXTRAPREFIX}/lib")
        endif()

        include_directories("${mdz_db_SOURCE_DIR}/../")
        target_link_libraries(${LIB_NAME} mdz_db)

        include_directories("${mdz_hlp_functions_SOURCE_DIR}/../")
        target_link_libraries(${LIB_NAME} mdz_hlp_functions)

        include_directories("${mdz_thr_mutex_SOURCE_DIR}/../")
        target_link_libraries(${LIB_NAME} mdz_thr_mutex)

        include_directories("${mdz_mem_vars_SOURCE_DIR}/../")
        target_link_libraries(${LIB_NAME} mdz_mem_vars)

        target_include_directories(${LIB_NAME} PRIVATE ${PQ_INCLUDE_DIR})
        target_link_libraries(${LIB_NAME} ${POSTGRESQL_LIB})
    else()
        message("-- WARNING: Did not found lib PQ_INCLUDE_DIR")
    endif()
endif()
