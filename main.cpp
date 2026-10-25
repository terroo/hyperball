#include "hyperball.hpp"
#include <memory>

auto main() -> int {
  auto h = std::unique_ptr<Hyperball>();
  h->run();
  return 0;
}
// DEBUG:
// g++ -std=c++23 -O2 -fno-omit-frame-pointer -Wfloat-conversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wcast-align -Wconversion -g -Wall -Werror -Wextra -Wshadow -Wpedantic -fsanitize=address -fsanitize=undefined -fsanitize=leak main.cpp hyperball.cpp -lsfml-graphics -lsfml-window -lsfml-system
//
// COMPILE e RODE:
// cmake . -B build
// cmake --build build
// ./build/hyperball
