#include <iostream>
#include "include/primer/p0_starter.h"

int main() {
    bustub::RowMatrix<int> mat1(2, 2);
    bustub::RowMatrix<int> mat2(2, 2);

    mat1.SetElem(0, 0, 1);
    mat1.SetElem(0, 1, 2);
    mat1.SetElem(1, 0, 3);
    mat1.SetElem(1, 1, 4);

    mat2.SetElem(0, 0, 5);
    mat2.SetElem(0, 1, 6);
    mat2.SetElem(1, 0, 7);
    mat2.SetElem(1, 1, 8);

    auto result = bustub::RowMatrixOperations<int>::AddMatrices(
        std::make_unique<bustub::RowMatrix<int>>(mat1),
        std::make_unique<bustub::RowMatrix<int>>(mat2)
    );

    if (result) {
        std::cout << "Matrix Addition Result:" << std::endl;
        for (int i = 0; i < result->GetRows(); i++) {
            for (int j = 0; j < result->GetColumns(); j++) {
                std::cout << result->GetElem(i, j) << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "Matrix addition failed due to dimension mismatch." << std::endl;
    }

    return 0;
}

