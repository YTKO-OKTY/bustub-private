#include <iostream>
#include "include/primer/p0_starter.h"

int main() {
    Matrix<int> mat(3, 3);

    std::cout << "초기값 확인:" << std::endl;
    for (int i = 0; i < mat.GetRows(); i++) {
        for (int j = 0; j < mat.GetColumns(); j++) {
            std::cout << mat.GetElem(i, j) << " ";
        }
        std::cout << std::endl;
    }

    mat.SetElem(1, 1, 42);
    mat.SetElem(2, 2, 99);

    std::cout << "\n값 변경 후:" << std::endl;
    for (int i = 0; i < mat.GetRows(); i++) {
        for (int j = 0; j < mat.GetColumns(); j++) {
            std::cout << mat.GetElem(i, j) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

