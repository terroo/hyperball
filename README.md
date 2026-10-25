# hyperball
Simples jogo de futebol feito com C++ e SFML, baseado no Hyperball online

---

## Dependências:
+ Compilador C++: GCC ou Clang
+ SFML 2.6
+ CMake

---

## Clone, compile e rode:
```bash
git clone https://github.com/terroo/hyperball
cd hyperball
cmake . -B build
cmake --build build
./build/hyperball
```

---

> SE quiser debugar:
```bash
g++ -std=c++23 -O2 -fno-omit-frame-pointer -Wfloat-conversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wcast-align -Wconversion -g -Wall -Werror -Wextra -Wshadow -Wpedantic -fsanitize=address -fsanitize=undefined -fsanitize=leak main.cpp hyperball.cpp -lsfml-graphics -lsfml-window -lsfml-system
```
