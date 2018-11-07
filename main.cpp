//
// Created by ikigai on 16.10.18.
//

#include <iostream>
#include <vector>
#include <fstream>
#include "Twofish.h"
#include "workWithMessages.h"


int main() {
    std::vector<uint32_t> arr = readFile("/home/ikigai/CLionProjects/Twofish/Files/black.bmp");
    Twofish twofish;

    std::string key = twofish.genRandomKey(); //генерация ключа
    std::cout << "Key:" << key << std::endl << std::endl;
    std::vector<uint32_t> keyDWORD = stringToInt(key);
    twofish.init(keyDWORD); //генерация ключей алгоритма

    std::vector<uint32_t> encrypt;
    std::cout << "Encrypt.." << std::endl;

    for (int i = 0; i < arr.size(); i += 4)
        twofish.encrypt({arr[i], arr[i + 1], arr[i + 2], arr[i + 3]}, encrypt);
    std::cout << "Encrypt done." << std::endl << std::endl;

    writeFile("/home/ikigai/CLionProjects/Twofish/Files/black_enc.bmp", encrypt, true);
    arr = readFile("/home/ikigai/CLionProjects/Twofish/Files/black_enc.bmp");

    std::vector<uint32_t> decrypt;
    std::cout << "Dencrypt.." << std::endl;
    for (int i = 0; i < encrypt.size(); i += 4)
        twofish.decrypt({encrypt[i], encrypt[i + 1], encrypt[i + 2], encrypt[i + 3]}, decrypt);
    std::cout << "Dencrypt done." << std::endl << std::endl;

    distribution(encrypt);

    writeFile("/home/ikigai/CLionProjects/Twofish/Files/black_dec.bmp", decrypt);

    corrCoef(encrypt, decrypt);

    return 0;
}