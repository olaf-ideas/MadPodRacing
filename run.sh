g++ draw.cpp -o draw -std=c++17 -g -fsanitize=undefined,address -lsfml-graphics -lsfml-window -lsfml-system && ./draw < arena_input