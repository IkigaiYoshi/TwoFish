//
// Created by ikigai on 16.10.18.
//

#include <iterator>
#include <bitset>
#include <vector>
#include <math.h>
#include <iostream>
#include <cstring>

std::string getBits(const std::vector<uint32_t> &arr) {
    std::string arrBits;
    for (const auto uint : arr) {
        std::bitset<32> bits(uint);
        arrBits += bits.to_string();
    }

    return arrBits;
}

std::vector<uint32_t> stringToInt(const std::string &str) {
    std::vector<uint32_t> arr;
    uint32_t num = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (i != 0 and i % 4 == 0) {
            arr.push_back(num);
            num = 0;
        }
        num = num | (uint8_t(str[i]) << ((i % 4) * 8));
    }
    arr.push_back(num);

    for (int i = 0; i < (arr.size() % 4); ++i)
        arr.push_back(0);

    return arr;
}

std::vector<uint32_t> readFile(const std::string &filePath) {
    std::ifstream f(filePath, std::ios_base::in | std::ios_base::binary);
    if (!f.good())
        exit(1);

    std::vector<uint32_t> arr;
    f.seekg(0, std::ios::end);
    int length = static_cast<int>(f.tellg());
    f.seekg(0, std::ios::beg);

    auto sizeOfBuffer = length / sizeof(unsigned);

    arr.resize(static_cast<unsigned long>(sizeOfBuffer + 1));
    f.read(reinterpret_cast<char *>(arr.data()), length);

    if (arr.size() % 4 != 0)
        arr.resize(arr.size() + arr.size() % 4);
    return arr;
}


void writeFile(const std::string &filePath, std::vector<uint32_t> arr, bool encrypted = false) {
    std::ofstream fs(filePath, std::ios_base::out | std::ios_base::binary);
    if (fs.fail()) {
        exit(1);
    }

    std::vector<unsigned> arrCopy(arr.size());
    std::copy(arr.begin(), arr.end(), arrCopy.begin());
    while (arrCopy.back() == 0)
        arrCopy.pop_back();

    if (encrypted) {
        fs.write(reinterpret_cast<const char *>(arrCopy.data()), arrCopy.size() * 4);
        return;
    }

    unsigned int back = arrCopy[arrCopy.size() - 1];
    arrCopy.pop_back();
    fs.write(reinterpret_cast<const char *>(arrCopy.data()), arrCopy.size() * 4);

    for (int i = 0; i < 4; i++) {
        int shift = 8 * i;
        auto r = back >> shift;
        r &= 255;
        if (r != 0) {
            char backChar = static_cast<unsigned char>(r);
            fs.write(&backChar, 1);
        } else {
            break;
        }
    }

}

void distribution(const std::vector<uint32_t> &encrypt) {
    std::string encBits = getBits(encrypt);

    uint32_t col = 0;
    for (auto bit : encBits) {
        if (bit == '0')
            col++;
    }
    std::cout << "Number of zeros: " << col << " " << col * 1.0 / encBits.length() * 100 << "%\n";
    std::cout << "Number of units: " << encBits.length() - col << " ";
    std::cout << (encBits.length() - col) * 1.0 / encBits.length() * 100 << "%\n";
}

void corrCoef(const std::vector<uint32_t> &encrypt, const std::vector<uint32_t> &decrypt) {
    std::string encBits = getBits(encrypt);
    std::string decBits = getBits(decrypt);

    double _x = 0;
    for (const auto bit : encBits)
        _x += static_cast<uint32_t> (bit);
    _x /= encBits.length();

    double _y = 0;
    for (const auto bit : decBits)
        _y += static_cast<uint32_t> (bit);
    _y /= decBits.length();

    double numerator = 0;
    for (int i = 0; i < encBits.length(); ++i)
        numerator += (static_cast<uint32_t> (encBits[i]) - _x) * (static_cast<uint32_t> (decBits[i]) - _y);

    double denominator = 0;
    double denominatorLeft = 0;
    for (char encBit : encBits)
        denominatorLeft += (static_cast<double> (encBit) - _x) * (static_cast<double> (encBit) - _x);
    double denominatorRight = 0;
    for (char decBit : decBits)
        denominatorRight += (static_cast<double> (decBit) - _y) * (static_cast<double> (decBit) - _y);
    denominator = sqrt(denominatorLeft * denominatorRight);

    double r = numerator / denominator;
    std::cout << "\nCorrcoef: " << r << std::endl;
}

