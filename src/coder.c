#include "coder.h"
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

int encode(uint32_t code_point,
           CodeUnit *code_unit) // code_point - то, что нужно закодировать,
                                // code_unit - туда будет помещен результат
{
    if (code_point < 0x80) // 1000 0000 - 7 бит(количество значащих бит кодируемого числа)
    {
        code_unit->length = 1;
        code_unit->code[0] = code_point;
    }
    else if (code_point < 0x800) // 1000 0000 0000 - 11 бит(количество значащих бит кодируемого числа)
    {
        code_unit->length = 2;
        code_unit->code[0] = 0xc0 | (code_point >> 6);   // 0xc0 - 1100 0000
        code_unit->code[1] = 0x80 | (code_point & 0x3f); // 0x3f -0000! 0011 1111 - Зануляет лишние биты; 0x80 | - добавляет служебные биты
    }
    else if (code_point < 0x10000) // 0001 0000 0000 0000 0000 - 16 бит
    {
        code_unit->length = 3;
        code_unit->code[0] = 0xe0 | (code_point >> 12);
        code_unit->code[1] = 0x80 | ((code_point >> 6) & 0x3f);
        code_unit->code[2] = 0x80 | (code_point & 0x3f);
    }
    else if (code_point < 0x200000) // 0010 0000 0000 0000 0000 0000 - 21 бит
    {
        code_unit->length = 4;
        code_unit->code[0] = 0xf0 | (code_point >> 18);
        code_unit->code[1] = 0x80 | ((code_point >> 12) & 0x3f);
        code_unit->code[2] = 0x80 | ((code_point >> 6) & 0x3f);
        code_unit->code[3] = 0x80 | (code_point & 0x3f);
    }
    else
    {
        return -1;
    }
    return 0;
}

uint32_t decode(const CodeUnit *code_unit)
{
    uint32_t output;
    if (code_unit->length == 1)
    {
        output = code_unit->code[0];
        return output;
    }
    else if (code_unit->length == 2)
    {
        // Пусть число 110(1 0101) 10(10 1010) = 0x56 =1386 - закодированное число
        output = code_unit->code[0] & 0x1f;    // 0001 1111 - Зануляем служебные биты. output =0000 0000 0001 0101
        output <<= 6;                          // 0000 0101 0100 0000
        output |= (code_unit->code[1] & 0x3f); // 0011 1111 - Зануляем служебные биты и ищем пересечение с output. output =0000 0101 0110 1010
        return output;
    }
    else if (code_unit->length == 3)
    {
        output = code_unit->code[0] & 0xf; // 0000 1111
        output <<= 6;
        output |= (code_unit->code[1] & 0x3f);
        output <<= 6;
        output |= (code_unit->code[2] & 0x3f);
        return output;
    }
    else if (code_unit->length == 4)
    {
        output = code_unit->code[0] & 0x7; // 0000 0111
        output <<= 6;
        output |= (code_unit->code[1] & 0x3f);
        output <<= 6;
        output |= (code_unit->code[2] & 0x3f);
        output <<= 6;
        output |= (code_unit->code[3] & 0x3f);
        return output;
    }
    else
    {
        return -1;
    }
}
int read_next_code_unit(FILE *in, CodeUnit *code_unit)
{
    uint8_t first = 0;
    uint8_t *ptr_code_unit = code_unit->code;
    fread(&first, sizeof(uint8_t), 1, in);
    if (first <= 0x7f) // 0111 1111 = 127
    {
        code_unit->length = 1;
        code_unit->code[0] = first;
    }
    else if ((first <= 0xdf) && (first >= 0xc0)) // 1101 1111 = 223-max 1100 0000 = 192 -min
    {
        code_unit->length = 2;
        code_unit->code[0] = first;
        for (size_t i = 1; i < code_unit->length; i++)
        {
            fread(ptr_code_unit + i, sizeof(uint8_t), 1, in);
            if (code_unit->code[i] > 0xbf) // 0xbf - 1011 1111
            {
                return -1;
            }
        }
    }
    else if ((first <= 0xef) && (first >= 0xe0)) // 1110 1111 = 239-max 1110 0000 = 224 -min
    {
        code_unit->length = 3;
        code_unit->code[0] = first;
        for (size_t i = 1; i < code_unit->length; i++)
        {
            fread(ptr_code_unit + i, sizeof(uint8_t), 1, in);
            if (code_unit->code[i] > 0xbf) // 0xbf - 1011 1111
            {
                return -1;
            }
        }
    }
    else if ((first <= 0xf7) && (first >= 0xf0)) // 1111 0111 = 247-max 1111 0000 = 240 -min
    {
        code_unit->length = 4;
        code_unit->code[0] = first;
        for (size_t i = 1; i < code_unit->length; i++)
        {
            fread(ptr_code_unit + i, sizeof(uint8_t), 1, in);
            if (code_unit->code[i] > 0xbf) // 0xbf - 1011 1111
            {
                return -1;
            }
        }
    }
    else
    {
        return -1; // Ошибка в случае первого битового файла
    }

    return 0;
}
int write_code_unit(FILE *out, const CodeUnit *code_unit)
{
    int result = fwrite(code_unit->code, 1, code_unit->length, out);
    return result;
}