//
// Created by ikigai on 17.10.18.
//

#include "Twofish.h"

static const uint32_t RO = 0x01010101;

#define    ROL32(a, b)    (((a) << (b)) | ((a) >> (32 - (b))))
#define ROR4(a, b)    ((((a) & 0xf) >> (b)) | (((a) & 0xf) << (4 - (b))))
#define ROR32(a, b)    (((a) >> (b)) | ((a) << (32 - (b))))

static const uint8_t MDS[] = {
        0x01, 0xEF, 0x5B, 0x5B,
        0x5B, 0xEF, 0xEF, 0x01,
        0xEF, 0x5B, 0x01, 0xEF,
        0xEF, 0x01, 0xEF, 0x5B
};

static const uint8_t RS[] = {
        0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E,
        0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5,
        0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19,
        0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03
};

static const uint8_t Q0_T0[] = {
        0x8, 0x1, 0x7, 0xD, 0x6, 0xf, 0x3, 0x2, 0x0, 0xb, 0x5, 0x9, 0xe, 0xc, 0xa, 0x4
};

static const uint8_t Q0_T1[] = {
        0xe, 0xc, 0xb, 0x8, 0x1, 0x2, 0x3, 0x5, 0xf, 0x4, 0xa, 0x6, 0x7, 0x0, 0x9, 0xd
};

static const uint8_t Q0_T2[] = {
        0xb, 0xa, 0x5, 0xe, 0x6, 0xd, 0x9, 0x0, 0xc, 0x8, 0xf, 0x3, 0x2, 0x4, 0x7, 0x1
};

static const uint8_t Q0_T3[] = {
        0xd, 0x7, 0xf, 0x4, 0x1, 0x2, 0x6, 0xe, 0x9, 0xb, 0x3, 0x0, 0x8, 0x5, 0xc, 0xa
};

static const uint8_t Q1_T0[] = {
        0x2, 0x8, 0xb, 0xd, 0xf, 0x7, 0x6, 0xe, 0x3, 0x1, 0x9, 0x4, 0x0, 0xa, 0xc, 0x5
};

static const uint8_t Q1_T1[] = {
        0x1, 0xe, 0x2, 0xb, 0x4, 0xc, 0x3, 0x7, 0x6, 0xd, 0xa, 0x5, 0xf, 0x9, 0x0, 0x8
};

static const uint8_t Q1_T2[] = {
        0x4, 0xc, 0x7, 0x5, 0x1, 0x6, 0x9, 0xa, 0x0, 0xe, 0xd, 0x8, 0x2, 0xb, 0x3, 0xf
};

static const uint8_t Q1_T3[] = {
        0xb, 0x9, 0x5, 0x1, 0xc, 0x3, 0xd, 0xe, 0x6, 0x4, 0x7, 0xf, 0x2, 0x0, 0x8, 0xa
};


static uint16_t gf28_mul(uint8_t a, uint8_t b) {
    uint8_t x;
    uint16_t res;

    res = 0;
    for (x = 0x80; x; x >>= 1) {
        if (a & x) {
            uint16_t tmp;

            tmp = static_cast<uint16_t>(0x0000 | b);
            if (x > 1) {
                int shift;

                shift = 1;
                while ((1 << shift) != x) {
                    shift++;
                }
                tmp <<= shift;
            }
            res ^= tmp;
        }
    }
    return (res);
}

static uint16_t gf28_mod(uint16_t a, uint16_t b) {
    uint16_t div;
    uint16_t x;

    if (b == 0) {
        return (0);
    }

    div = b;
    while (!(div & 0x8000)) {
        div <<= 1;
    }

    x = 0x8000;
    while (true) {
        if (a & x) {
            a ^= div;
        }
        if (div & 1) {
            break;
        }
        x >>= 1;
        div >>= 1;
    }
    return (a);
}

static uint8_t Q0(uint8_t x) {
    uint8_t a0, b0, a1, b1, a2, b2, a3, b3, a4, b4;

    a0 = x >> 4;
    b0 = static_cast<uint8_t>(x & 0xf);
    a1 = a0 ^ b0;
    b1 = static_cast<uint8_t>(a0 ^ ROR4(b0, 1) ^ (a0 << 3));
    a2 = Q0_T0[a1 & 0xf];
    b2 = Q0_T1[b1 & 0xf];
    a3 = a2 ^ b2;
    b3 = static_cast<uint8_t>(a2 ^ ROR4(b2, 1) ^ (a2 << 3));
    a4 = Q0_T2[a3 & 0xf];
    b4 = Q0_T3[b3 & 0xf];

    return static_cast<uint8_t>((b4 << 4) | (a4 & 0xf));
}

static uint8_t Q1(uint8_t x) {
    uint8_t a0, b0, a1, b1, a2, b2, a3, b3, a4, b4;

    a0 = x >> 4;
    b0 = static_cast<uint8_t>(x & 0xf);
    a1 = a0 ^ b0;
    b1 = static_cast<uint8_t>(a0 ^ ROR4(b0, 1) ^ (a0 << 3));
    a2 = Q1_T0[a1 & 0xf];
    b2 = Q1_T1[b1 & 0xf];
    a3 = a2 ^ b2;
    b3 = static_cast<uint8_t>(a2 ^ ROR4(b2, 1) ^ (a2 << 3));
    a4 = Q1_T2[a3 & 0xf];
    b4 = Q1_T3[b3 & 0xf];

    return static_cast<uint8_t>((b4 << 4) | (a4 & 0xf));
}

uint32_t H(uint32_t X, const uint32_t *L) {
    union {
        uint32_t dw;
        uint8_t b[4];
    } z{}, y{};

    y.b[0] = static_cast<uint8_t>(X % 256);
    y.b[1] = static_cast<uint8_t>((X >> 8) % 256);
    y.b[2] = static_cast<uint8_t>((X >> 16) % 256);
    y.b[3] = static_cast<uint8_t>((X >> 24) % 256);

    y.b[0] = Q1(y.b[0]);
    y.b[1] = Q0(y.b[1]);
    y.b[2] = Q0(y.b[2]);
    y.b[3] = Q1(y.b[3]);
    y.dw ^= L[3];

    y.b[0] = Q1(y.b[0]);
    y.b[1] = Q1(y.b[1]);
    y.b[2] = Q0(y.b[2]);
    y.b[3] = Q0(y.b[3]);
    y.dw ^= L[2];

    y.b[0] = Q0(y.b[0]);
    y.b[1] = Q1(y.b[1]);
    y.b[2] = Q0(y.b[2]);
    y.b[3] = Q1(y.b[3]);

    y.dw ^= L[1];

    y.b[0] = Q0(y.b[0]);
    y.b[1] = Q0(y.b[1]);
    y.b[2] = Q1(y.b[2]);
    y.b[3] = Q1(y.b[3]);
    y.dw ^= L[0];

    y.b[0] = Q1(y.b[0]);
    y.b[1] = Q0(y.b[1]);
    y.b[2] = Q1(y.b[2]);
    y.b[3] = Q0(y.b[3]);

    z.b[0] = static_cast<uint8_t>(gf28_mod(gf28_mul(y.b[0], MDS[0]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[1], MDS[1]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[2], MDS[2]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[3], MDS[3]), 0x169));
    z.b[1] = static_cast<uint8_t>(gf28_mod(gf28_mul(y.b[0], MDS[4]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[1], MDS[5]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[2], MDS[6]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[3], MDS[7]), 0x169));
    z.b[2] = static_cast<uint8_t>(gf28_mod(gf28_mul(y.b[0], MDS[8]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[1], MDS[9]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[2], MDS[10]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[3], MDS[11]), 0x169));
    z.b[3] = static_cast<uint8_t>(gf28_mod(gf28_mul(y.b[0], MDS[12]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[1], MDS[13]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[2], MDS[14]), 0x169) ^
                                  gf28_mod(gf28_mul(y.b[3], MDS[15]), 0x169));

    return (z.dw);
}

Twofish::Twofish() {
    this->__init = 0;
}

Twofish::~Twofish() {
    this->destroy();
}

int Twofish::init(const std::vector<uint32_t> &key) {
    uint32_t i, j, k;
    uint8_t *m, *S;

    //ключ длины N = 128, 192, 256 бит.
    if (key.size() > 8) {
        std::cerr << "Key size does not match." << std::endl;
        return (-1);
    }

    //группирование байт ключа для генерации вектора S
    m = (uint8_t *) key.data();

    //K = 4, 6, 8. Образуются два вектора длины K
    //М чет = (М0, М2, ..., М 2К - 2)
    //М нечет = (М1, М3, ..., М 2К - 1)
    //недостающие до ближайшего N биты ключа назначаются равными 0
    for (int i = 0, j = 0; i < 4, j < 8; i++, j += 2) {
        if (j > key.size()) {
            this->tf_ME[i] = 0x00000000;
            this->tf_MO[i] = 0x00000000;
            continue;
        }
        this->tf_ME[i] = key[j];
        this->tf_MO[i] = key[j + 1];
    }

    //Третий вектор длины К так же получается из исходного ключа.
    //Для этого байты ключа группируются по 8 (вектор m).
    //Затем эти вектора умножаются на матрицу 4*8 Рида-Соломона.

    for (i = 0; i < 4; i++) {
        S = (uint8_t *) &(this->tf_S[3 - i]);
        for (j = 0; j < 4; j++) {
            S[j] = static_cast<uint8_t>(gf28_mod(gf28_mul(RS[j << 3], m[i << 3]), 0x14d));
            for (k = 1; k < 8; k++) {
                S[j] ^= gf28_mod(gf28_mul(RS[(j << 3) + k], m[(i << 3) + k]), 0x14d);
            }
        }
    }

    //генерация 40 расширенных ключей
    for (i = 0; i < 20; i++) {
        uint32_t ai, bi;

        ai = H(RO * (i << 1), this->tf_ME);
        bi = ROL32(H(RO * ((i << 1) + 1), this->tf_MO), 8);

        this->tf_K[i << 1] = ai + bi;
        this->tf_K[(i << 1) + 1] = ROL32(ai + (bi << 1), 9);
    }

    this->__init = 1;

    return (0);
}

int Twofish::encrypt(const std::vector<uint32_t> &src, std::vector<uint32_t> &dst) {
    if (!this->__init) {
        std::cerr << "Keys and messages not generated." << std::endl;
        return (-1);
    }
    //байты 128-ого блока исходной информации
    uint32_t p[4];
    uint32_t round;
    //Результат первоначального сложения с блоками ключа
    p[0] = src[0] ^ this->tf_K[0];
    p[1] = src[1] ^ this->tf_K[1];
    p[2] = src[2] ^ this->tf_K[2];
    p[3] = src[3] ^ this->tf_K[3];


    for (round = 0; round < 16; round++) {
        uint32_t F0, F1, T0, T1;

        //Функция F


        T0 = H(p[0], this->tf_S);

        T1 = H(ROL32(p[1], 8), this->tf_S);

        //Блоки  трансформируются с помощью PHT
        F0 = T0 + T1 + this->tf_K[round * 2 + 8];
        F1 = T0 + (T1 << 1) + this->tf_K[round * 2 + 9];

        p[2] ^= F0;
        p[2] = ROR32(p[2], 1);
        p[3] = ROL32(p[3], 1) ^ F1;

        F0 = p[0];
        F1 = p[1];
        p[0] = p[2];
        p[1] = p[3];
        p[2] = F0;
        p[3] = F1;
    }

    //Заключительное преобразование
    dst.push_back(p[2] ^ this->tf_K[4]);
    dst.push_back(p[3] ^ this->tf_K[5]);
    dst.push_back(p[0] ^ this->tf_K[6]);
    dst.push_back(p[1] ^ this->tf_K[7]);

    return (0);
}

int Twofish::decrypt(const std::vector<uint32_t> &src, std::vector<uint32_t> &dst) {
    if (!this->__init) {
        std::cerr << "Keys and messages not generated" << std::endl;
        return (-1);
    }

    uint32_t p[4];
    uint32_t round;

    p[2] = src[0] ^ this->tf_K[4];
    p[3] = src[1] ^ this->tf_K[5];
    p[0] = src[2] ^ this->tf_K[6];
    p[1] = src[3] ^ this->tf_K[7];

    for (round = 15; round <= 15; round--) {
        uint32_t F0, F1, T0, T1;

        T0 = p[0];
        T1 = p[1];
        p[0] = p[2];
        p[1] = p[3];
        p[2] = T0;
        p[3] = T1;

        T0 = H(p[0], this->tf_S);
        T1 = H(ROL32(p[1], 8), this->tf_S);

        F0 = T0 + T1 + this->tf_K[round * 2 + 8];
        F1 = T0 + (T1 << 1) + this->tf_K[round * 2 + 9];

        p[2] = ROL32(p[2], 1) ^ F0;
        p[3] ^= F1;
        p[3] = ROR32(p[3], 1);
    }

    dst.push_back(p[0] ^ this->tf_K[0]);
    dst.push_back(p[1] ^ this->tf_K[1]);
    dst.push_back(p[2] ^ this->tf_K[2]);
    dst.push_back(p[3] ^ this->tf_K[3]);

    return (0);
}


void Twofish::destroy() {
    memset(this->tf_MO, 0, sizeof(this->tf_MO));
    memset(this->tf_ME, 0, sizeof(this->tf_ME));
    memset(this->tf_K, 0, sizeof(this->tf_K));
    memset(this->tf_S, 0, sizeof(this->tf_S));
    this->__init = 0;
}

std::string Twofish::genRandomKey() {
    std::string s;
    srand(static_cast<unsigned int>(time(nullptr)));
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = 0; i < 32; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return s;
}

