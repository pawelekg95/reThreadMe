function(doxygen_add_target)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY            ${CMAKE_BINARY_DIR}/docs)
        set(DOXYGEN_EXTRACT_PRIVATE             YES)
        set(DOXYGEN_EXTRACT_PACKAGE             YES)
        set(DOXYGEN_EXTRACT_STATIC              YES)
        set(DOXYGEN_WARN_NO_PARAMDOC            YES)
        set(DOXYGEN_HTML_OUTPUT                 .)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE      README.md)
        set(DOXYGEN_FILE_PATTERNS               *.c *.cc *.cxx *.cpp *.c++ *.ii *.ixx *.ipp *.i++ *.inl *.h *.hh *.hxx *.hpp *.h++ *.inc README.md)
        set(DOXYGEN_WARN_LOGFILE                ${DOXYGEN_OUTPUT_DIRECTORY}/doxygen.warn)
        set(DOXYGEN_CLASS_DIAGRAMS              YES)
        set(DOXYGEN_EXTRACT_ALL                 YES)
        set(DOXYGEN_CLASS_DIAGRAMS              YES)
        set(DOXYGEN_HIDE_UNDOC_RELATIONS        NO)
        set(DOXYGEN_HAVE_DOT                    YES)
        set(DOXYGEN_CLASS_GRAPH                 YES)
        set(DOXYGEN_COLLABORATION_GRAPH         YES)
        set(DOXYGEN_UML_LOOK                    YES)
        set(DOXYGEN_UML_LIMIT_NUM_FIELDS        50)
        set(DOXYGEN_TEMPLATE_RELATIONS          YES)
        set(DOXYGEN_DOT_GRAPH_MAX_NODES         100)
        set(DOXYGEN_MAX_DOT_GRAPH_DEPTH         0)
        set(DOXYGEN_DOT_TRANSPARENT             YES)
        set(DOXYGEN_DOT_IMAGE_FORMAT            svg)

        set(DOXYGEN_GENERATE_HTML               YES)
        set(DOXYGEN_GENERATE_TREEVIEW           YES)
        set(DOXYGEN_DISABLE_INDEX               NO)
        set(DOXYGEN_FULL_SIDEBAR                NO)
        # set(DOXYGEN_HTML_EXTRA_STYLESHEET       doxygen-awesome-css/doxygen-awesome.css doxygen-awesome-css/doxygen-awesome-sidebar-only.css)
        set(DOXYGEN_HTML_COLORSTYLE             LIGHT)

        # configure_file(${PROJECT_SOURCE_DIR}/tools/doxygen-awesome-css/doxygen-awesome-sidebar-only.css ${DOXYGEN_OUTPUT_DIRECTORY}/doxygen-awesome-css/doxygen-awesome-sidebar-only.css COPYONLY)
        # configure_file(${PROJECT_SOURCE_DIR}/tools/doxygen-awesome-css/doxygen-awesome.css ${DOXYGEN_OUTPUT_DIRECTORY}/doxygen-awesome-css/doxygen-awesome.css COPYONLY)

        # Add target to generate doxygen docs in the build directory.
        doxygen_add_docs(doxygen ${ARGN} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    endif ()
endfunction()
