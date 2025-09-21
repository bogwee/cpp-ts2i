cmake_minimum_required(VERSION 3.10)
project(cpp-ts2i)

set(CMAKE_CXX_STANDARD 17)

# Trouver les packages externes
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# Ajouter les fichiers sources
set(SOURCES
    main.cpp
    DCT.cpp   # votre fichier DCT
)

# Créer l'exécutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Lier les bibliothèques externes
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS} Eigen3::Eigen)

# Inclure les répertoires de headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}  # optionnel avec Eigen3::Eigen
    .   # pour inclure DCT.h si dans le même dossier
)
