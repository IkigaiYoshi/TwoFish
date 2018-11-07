//
// Created by ikigai on 17.10.18.
//

#ifndef TWOFISH_TWOFISH_H
#define TWOFISH_TWOFISH_H

#include <vector>
#include <cstdint>
#include <string.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>


class Twofish {
private:
    int __init;
    //S-box (базис для алгоритма генерации ключей
    //с использованием функции h) для использования в функции g
    uint32_t tf_ME[4]; //четные вектор M
    uint32_t tf_MO[4]; //нечетный вектор M
    uint32_t tf_S[4];  //вектор S
    uint32_t tf_K[40]; //расширенный ключ

public:
    Twofish();

    ~Twofish();

    int init(const std::vector<uint32_t> &key); //генерация S-box

    int encrypt(const std::vector<uint32_t> &src, std::vector<uint32_t> &dst);

    int decrypt(const std::vector<uint32_t> &src, std::vector<uint32_t> &dst);

    void destroy();

    std::string genRandomKey(); //генерация случайного ключа
};


#endif //TWOFISH_TWOFISH_H
