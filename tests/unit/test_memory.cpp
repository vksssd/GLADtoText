#include <iostream>
#include "../../model/memory.h"

int main() {
    int dim = 4;
    ContextMemory mem(dim);

    Vector h(dim);
    h.v = {1, 1, 1, 1};

    mem.update(h);

    Vector h2(dim);
    h2.v = {2, 2, 2, 2};

    Vector out = mem.apply(h2);

    std::cout << "Memory applied output:\n";
    for (float x : out.v)
        std::cout << x << " ";
    std::cout << "\n";

    if (out.v[0] != h2.v[0])
        std::cout << "PASS\n";
    else
        std::cout << "FAIL\n";
}
