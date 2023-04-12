#include <iostream>
#include "DummyClass.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::cout << "DummyClass is invoked : " << DummyClass::canBeInvoked() << std::endl;

    return 0;
}
