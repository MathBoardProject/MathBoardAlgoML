# ===================================================================================
#  The Tesseract CMake configuration file
#
#             ** File generated automatically, do not modify **
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    This file will define the following variables:
#      - Tesseract_LIBRARIES             : The list of all imported targets.
#      - Tesseract_INCLUDE_DIRS          : The Tesseract include directories.
#
# ===================================================================================

include(CMakeFindDependencyMacro)
find_dependency(Leptonica)

set(Tesseract_INCLUDE_DIRS "/usr/include/tesseract")
set(Tesseract_LIBRARIES "/usr/lib/libtesseract.so")
