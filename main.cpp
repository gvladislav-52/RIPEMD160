#include <iostream>
#include <vector>
#include <cstdint>
using namespace std;

//Функции
uint32_t function(int j, uint32_t x, uint32_t y, uint32_t z)
{
    if (j >= 0 && j <= 15)
        return x ^ y ^ z;
    else if (j >= 16 && j <= 31)
        return (x & y) | (~x & z);
    else if (j >= 32 && j <= 47)
        return (x | ~y) ^ z;
    else if (j >= 48 && j <= 63)
        return (x & z) | (y & ~z);
    else
        return x ^ (y | ~z);
}

// Добавляемые шестнадцатеричные константы
uint32_t function_K(int j) {
    if (j >= 0 && j <= 15)
        return 0x00000000;
    else if (j >= 16 && j <= 31)
        return 0x5A827999;
    else if (j >= 32 && j <= 47)
        return 0x6ED9EBA1;
    else if (j >= 48 && j <= 63)
        return 0x8F1BBCDC;
    else
        return 0xA953FD4E;
}

// Добавляемые шестнадцатеричные константы
uint32_t function_K_dot(int j) {
    if (j >= 0 && j <= 15)
        return 0x50A28BE6;
    else if (j >= 16 && j <= 31)
        return 0x5C4DD124;
    else if (j >= 32 && j <= 47)
        return 0x6D703EF3;
    else if (j >= 48 && j <= 63)
        return 0x7A6D76E9;
    else
        return 0x00000000;
}

// Добавление недостающих битов
void padMessage(std::vector<uint8_t>& message) {
    // Добавление бита "1"
    message.push_back(0x80);

    // Добавление нулевых битов до длины сообщения (в битах) равной 448 модуль 512
    while ((message.size() * 8) % 512 != 448) {
        message.push_back(0x00);
    }
}

// Добавление длины сообщения (в битах)
void addMessageLength(std::vector<uint8_t>& message, uint64_t messageLength) {
    // Добавление длины сообщения в виде 8-байтного little-endian числа
    for (int i = 0; i < 8; i++) {
        message.push_back((messageLength >> (i * 8)) & 0xFF);
    }
}

// Выбор 32-битных слов из сообщения
std::vector<uint32_t> extractWords(const std::vector<uint8_t>& message) {
    std::vector<uint32_t> words;

    for (std::size_t i = 0; i < message.size(); i += 4) {
        uint32_t word = (message[i] << 24) | (message[i + 1] << 16) | (message[i + 2] << 8) | message[i + 3];
        words.push_back(word);
    }

    return words;
}

int rols[80] =
        {
                0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8,
                3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12,
                1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2,
                4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13
        };

int rols_dot[80] =
        {
                5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12,
                6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2,
                15,5,1,3,7,14,6,9,11,8,12,2,10,0,4,13,
                8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14,
                12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11
        };

int function_R[80] =
        {
                11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8,
                7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12,
                11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5,
                11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12,
                9,15,5,11,5,8,13,12,5,12,13,14,11,8,5,6
        };

int function_R_dot[80] =
        {
                8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6,
                9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11,
                9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5,
                15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8,
                8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11
        };

std::vector<uint32_t> ripemd160(const std::vector<uint8_t>& message)
{

    std::vector<uint8_t> paddedMessage = message;
    padMessage(paddedMessage);
    addMessageLength(paddedMessage, message.size() * 8);

    // Выбор 32-битных слов из сообщения
    std::vector<uint32_t> words = extractWords(paddedMessage);
    uint32_t h[5] =
            {
                    0x67452301,
                    0xEFCDAB89,
                    0x98BADCFE,
                    0x10325476,
                    0xC3D2E1F0
            };

    for (std::size_t i = 0; i < words.size(); i += 16) {
        uint32_t A = h[0], B = h[1], C = h[2], D = h[3], E = h[4];
        uint32_t A_dot = h[0], B_dot = h[1], C_dot = h[2], D_dot = h[3], E_dot = h[4];

        for (int j = 0; j < 80; j++) {
            uint32_t T = rols[j]* (A + function(j, B, C, D) + words[function_R[j]] + function_K(j)) + E;
            A = E;
            E = D;
            D = rols[10]*C;
            C = B;
            B = T;

            T = rols_dot[j]* (A_dot + function((79 - j), B_dot, C_dot, D_dot) + words[function_R_dot[j]] + function_K_dot(j)) + E_dot;
            A_dot = E_dot;
            E_dot = D_dot;
            D_dot = rols_dot[10] *C_dot;
            C_dot= B_dot;
            B_dot = T;
        }

        uint32_t T = h[1] + C + D_dot;
        h[1] = h[2] + D + E_dot;
        h[2] = h[3] + E + A_dot;
        h[3] = h[4] + A + B_dot;
        h[4] = h[0] + B + C_dot;
        h[0] = T;
    }

    return {h[0],h[1],h[2],h[3],h[4]};

}

int main() {
    std::string message;
    std::cin >> message;
    // Преобразуем сообщение в вектор байтов
    std::vector<uint8_t> byteMessage(message.begin(), message.end());

    // Вычисляем хеш-сумму
    std::vector<uint32_t> hash = ripemd160(byteMessage);

    // Выводим хеш-сумму
    for (const auto& word : hash) {
        std::cout << std::hex << word;
    }
    std::cout << std::endl;

    return 0;
}