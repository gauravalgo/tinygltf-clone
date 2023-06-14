all:
clang++ -Wall -Werror -g -O0 -o loader_test test.cc && ./loader_test face3d.gltf

beautify:
clang-format -i tiny_gltf_loader.h