# Install script for directory: /home/ubuntu/svn_repo/apriltags

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/home/ubuntu/svn_repo/apriltags/build")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/ubuntu/svn_repo/apriltags/pod-build/lib/libapriltags.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/AprilTags" TYPE FILE FILES
    "/home/ubuntu/svn_repo/apriltags/AprilTags/MathUtil.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag16h5_other.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag25h7.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Edge.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Homography33.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/TagDetection.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/TagDetector.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/FloatImage.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Segment.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag36h11.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/GLineSegment2D.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag36h9.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/TagFamily.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag16h5.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/djicam.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/GLine2D.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag36h11_other.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/XYWeight.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/pch.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Gaussian.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Gridder.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Quad.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/UnionFindSimple.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/Tag25h9.h"
    "/home/ubuntu/svn_repo/apriltags/AprilTags/GrayModel.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ubuntu/svn_repo/apriltags/pod-build/lib/pkgconfig/apriltags.pc")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/ubuntu/svn_repo/apriltags/pod-build/example/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/ubuntu/svn_repo/apriltags/pod-build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/ubuntu/svn_repo/apriltags/pod-build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
