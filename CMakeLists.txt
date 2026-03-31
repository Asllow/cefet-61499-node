# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cefet-61499-node)

# Ordena a criacao da particao SPIFFS usando o conteudo da pasta "data"
spiffs_create_partition_image(spiffs data FLASH_IN_PROJECT)