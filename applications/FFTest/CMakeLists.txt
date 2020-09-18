# create the target
add_pdb_integration_test(FFTest)

target_link_libraries(FFTest ${MKL_LIBRARIES})

# add a build dependency to build-tests target
add_dependencies(build-integration-tests FFTest)

# compile all the objects
file(GLOB files "${PROJECT_SOURCE_DIR}/applications/FFTest/sharedLibraries/source/*.cc")
foreach(file ${files})
    get_filename_component(fileName "${file}" NAME_WE)
    add_library(${fileName} MODULE ${file})
    if(fileName MATCHES FFAggBlockToRowStrip)
    	target_compile_definitions(FFAggBlockToRowStrip PRIVATE -DBLOCK_TO_ROW_STRIP)
    elseif(fileName MATCHES FFAggBlockToColStrip)
    	target_compile_definitions(FFAggBlockToColStrip PRIVATE -DBLOCK_TO_COL_STRIP)
    endif ()
    #target_compile_definitions(FFAggBlockToColStrip PRIVATE -DBLOCK_TO_COL_STRIP)
    target_link_libraries(${fileName} pdb-shared-common)
    target_link_libraries(${fileName} ${MKL_LIBRARIES})
    add_dependencies(shared-libraries ${fileName})
    #target_compile_definitions(FFAggBlockToRowStrip PRIVATE -DBLOCK_TO_ROW_STRIP)
endforeach()

# add build dependencies to shared libraries it uses

add_dependencies(FFTest FFAggMatrix)
add_dependencies(FFTest FFGradientJoin)
add_dependencies(FFTest FFHiddenLayerJoin)
add_dependencies(FFTest FFInputLayerJoin)
add_dependencies(FFTest FFJoinBackTransposeMult)
add_dependencies(FFTest FFJoinBackMultTranspose)
add_dependencies(FFTest FFMatrixBlock)
add_dependencies(FFTest FFMatrixData)
add_dependencies(FFTest FFMatrixMeta)
add_dependencies(FFTest FFMatrixScanner)
add_dependencies(FFTest FFMatrixWriter)
add_dependencies(FFTest FFSelectionGradient2)
add_dependencies(FFTest FFUpdateJoin)
add_dependencies(FFTest FFAggBlockToRowStrip)
add_dependencies(FFTest FFAggBlockToColStrip)
add_dependencies(FFTest FFSingleToBlock)
add_dependencies(FFTest FFSingleToRow)
add_dependencies(FFTest FFSingleToCol)
