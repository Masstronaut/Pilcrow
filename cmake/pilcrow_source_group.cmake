function(pilcrow_source_group aRoot aTarget) 
  get_target_property(targetBinaryDir ${aTarget} BINARY_DIR)
  get_target_property(targetSourceDir ${aTarget} SOURCE_DIR)
  get_target_property(targetSources ${aTarget} SOURCES)

  # This will determine if the given files are in a folder or not and separate 
  # them on that alone. 
  foreach(aFile ${targetSources}) 
    file(TO_CMAKE_PATH ${aFile} resultFile) 
    get_filename_component(nameComponent ${resultFile} NAME) 
    get_filename_component(fullPath ${nameComponent} ABSOLUTE) 
    
    if(IS_ABSOLUTE ${aFile})
      # It's only safe to call RELATIVE_PATH if the path begins with our "root" dir.
      string(FIND "${aFile}" "${targetSourceDir}" checkIfRelativeToSourceDir)
      string(FIND "${aFile}" "${targetBinaryDir}" checkIfRelativeToBinaryDir)

      if ("${checkIfRelativeToSourceDir}" EQUAL 0)
        file(RELATIVE_PATH relativePath ${targetSourceDir} ${aFile})
      elseif ("${checkIfRelativeToBinaryDir}" EQUAL 0)
        file(RELATIVE_PATH relativePath ${targetBinaryDir} ${aFile})
        set(fullPath ${targetBinaryDir}/${nameComponent})
      endif()
    else()
      set(relativePath ${aFile})
    endif()

    if(EXISTS ${fullPath}) 
      set(notInAFolder ${notInAFolder} ${relativePath}) 
    else()
      set(inAFolder ${inAFolder} ${relativePath}) 
    endif() 
  endforeach() 

  # We use a no space prefix with files from folders, otherwise the filters  
  # don't get made. 
  source_group(TREE ${${aRoot}}  
               PREFIX "" 
               FILES ${inAFolder}) 

  # We use a one space prefix with files not in folders, otherwise the files 
  # are put into "Source Files" and "Header Files" filters. 
  source_group(TREE ${${aRoot}}  
               PREFIX " " 
               FILES ${notInAFolder}) 
endfunction() 


function (pilcrow_process_sources aTarget aList aResult)
  get_target_property(targetSourceDir ${aTarget} SOURCE_DIR)
  set(temporaryList "")
  foreach(item ${${aList}})
    if(IS_ABSOLUTE ${item})
      list(APPEND temporaryList ${item})
      #message(STATUS ${item})
    else()
      set(fullPath ${targetSourceDir}/${item})

      if (EXISTS ${fullPath})
        list(APPEND temporaryList ${fullPath})
        #message(STATUS ${fullPath})
      else()
        message(SEND_ERROR "Can't find absolute path to ${item}")
      endif()
    endif()
  endforeach()

  set(${aResult} ${temporaryList} PARENT_SCOPE)
endfunction()

function(pilcrow_populate_include_list aTarget aIncludeDirectoriesVariable aListOfParsedLibraries)
  if (${aTarget} IN_LIST ${aListOfParsedLibraries})
    return()
  endif()

  # Things depend on each other sometimes, so this solves that.
  list(APPEND tempList ${${aListOfParsedLibraries}})
  list(APPEND tempList ${aTarget})
  list(REMOVE_DUPLICATES tempList)
  set(${aListOfParsedLibraries} ${tempList} PARENT_SCOPE)

  # This is the start of the actual include directory when looking at BUILD_INTERFACE
  # generator expressions.    
  set(startOfIncludeDirectory 18)

  #Add on the original directories.
  list(APPEND initialIncludeDirectories ${${aIncludeDirectoriesVariable}})

  # Inteface libraries use different properties for their link libraries and 
  # include directories. So we'll look up if this target is one so we can
  # set the property type we're looking for.
  get_target_property(targetType ${aTarget} TYPE)

  if (NOT targetType STREQUAL "INTERFACE_LIBRARY")
    set(includeDirectoriesProperty INCLUDE_DIRECTORIES)
    set(linkLibrariesProperty LINK_LIBRARIES)
  else()
    set(includeDirectoriesProperty INTERFACE_INCLUDE_DIRECTORIES)
    set(linkLibrariesProperty INTERFACE_LINK_LIBRARIES)
  endif()

  # Now append on this targets include directories
  get_target_property(targetIncludeDirectories ${aTarget} ${includeDirectoriesProperty})

  if(NOT targetIncludeDirectories STREQUAL "targetIncludeDirectories-NOTFOUND")
    list(APPEND initialIncludeDirectories ${targetIncludeDirectories})
  endif()

  # Now we recurse to each target underneath this one.
  get_target_property(targetLinkLibraries ${aTarget} ${linkLibrariesProperty})

  if(NOT targetLinkLibraries STREQUAL "targetLinkLibraries-NOTFOUND")
    foreach(linkLibrary ${targetLinkLibraries})
      pilcrow_populate_include_list(${linkLibrary} initialIncludeDirectories tempList)
    endforeach()
  endif()

  # We need to convert BUILD_INTERFACE includes to normal directories, and remove
  # INSTALL_INTERFACE directories.
  foreach(includeDirectory ${initialIncludeDirectories})
    if(includeDirectory MATCHES "\\$<INSTALL_INTERFACE:.*>")
      continue()
    elseif(includeDirectory MATCHES "^\\$<BUILD_INTERFACE:.*>$")
      # Compute the length - 19, so we can get the length of _only_ the include directory
      # itself. This way we don't get any of the $<BUILD_INTERFACE:> part of the string.
      string(LENGTH ${includeDirectory} lengthOfString)
      math(EXPR finalStringLength "${lengthOfString} - 19")

      string(SUBSTRING ${includeDirectory} ${startOfIncludeDirectory} ${finalStringLength} newIncludeDirectory)
      list(APPEND finalIncludeDirectories ${newIncludeDirectory})
    else()
      list(APPEND finalIncludeDirectories ${includeDirectory})
    endif()
  endforeach()

  # Deduplicate the list
  list(REMOVE_DUPLICATES finalIncludeDirectories)
  
  # Return the list.
  set(${aIncludeDirectoriesVariable} ${finalIncludeDirectories} PARENT_SCOPE)
endfunction()

function(pilcrow_create_reflection_target aTarget) 
  get_target_property(targetBinaryDir ${aTarget} BINARY_DIR)
  get_target_property(targetSourceDir ${aTarget} SOURCE_DIR)
  get_target_property(targetSources ${aTarget} SOURCES)

  set(parsedLibraries "")
  pilcrow_populate_include_list(${aTarget} targetIncludeDirectories parsedLibraries)
  
  list(FILTER targetSources INCLUDE REGEX "^.*\.(c(pp)?)$")
  
  pilcrow_process_sources(${aTarget} targetSources processedTargetSources)

  list(JOIN processedTargetSources ";" targetSourcesList)
  list(JOIN targetIncludeDirectories ";" targetIncludeDirectoriesList)

  set(generatedReflectionSource ${targetBinaryDir}/GeneratedReflection/${aTarget}_ReflectionCode)

  add_custom_command(
    OUTPUT  
    ${generatedReflectionSource}.cpp
    ${generatedReflectionSource}.h
    # Runs ReflectionExporter
    COMMAND ${toolsDirectory}/ReflectionExporter/ReflectionExporter.exe
    # Includes
    --include ${targetIncludeDirectoriesList}
    # Sources
    --sources ${targetSourcesList}
    # Output file
    --outputFile "\"${generatedReflectionSource}\""
    # Target name
    --targetName "\"${aTarget}\""
    DEPENDS ${aTarget}
  )

  set(reflectionTarget ${aTarget}_SimpleReflection)
  add_library(${aTarget}_SimpleReflection STATIC)

  target_sources(${reflectionTarget}
  PRIVATE
    ${generatedReflectionSource}.cpp
    ${generatedReflectionSource}.h
  )

  target_include_directories(${reflectionTarget} PUBLIC ${targetBinaryDir})
  target_include_directories(${reflectionTarget} PUBLIC ${targetIncludeDirectories})
  target_link_libraries(${aTarget} PUBLIC ${reflectionTarget})
  target_link_libraries(${reflectionTarget} PUBLIC ${aTarget})

  # Let's set a folder to put reflected things into:
  get_target_property(ideFolder ${aTarget} FOLDER)
  set_target_properties(${reflectionTarget} PROPERTIES FOLDER ${ideFolder}/GeneratedReflection)
  
  # Maybe some other time when I can make source_group work nicely with binary dirs.
  #pilcrow_source_group(targetBinaryDir ${reflectionTarget})
endfunction() 