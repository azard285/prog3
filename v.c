#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#define MAX 1000000
/*size_t - 8 байт
 * Диапазон             Вероятность
 * -------------------- -----------
 * [0; 128)             90%
 * [128; 16384)         5%
 * [16384; 2097152)     4%
 * [2097152; 268435455) 1%
 */
uint32_t generate_number()
{
    const int r = rand();
    const int p = r % 100;
    if (p < 90)
    {
        return r % 128;
    }
    if (p < 95)
    {
        return r % 16384;
    }
    if (p < 99)
    {
        return r % 2097152;
    }
    return r % 268435455;
}
size_t encode_varint(uint32_t value, uint8_t *buf)
{
    assert(buf != NULL);
    uint8_t *cur = buf;
    while (value >= 0x80) // 0x80 - 1000 0000, елси чило меньше, например 0111 1111, то оно уже считается закодированныи т.к. первый бит - ноль
    {
        const uint8_t byte = (value & 0x7f) | 0x80; // Первое: вырубает первый байт, второе: если первый байт 0,  то он становится 1;
        // если первый байт 1,  то он становится 1
        *cur = byte;
        value >>= 7;
        ++cur;
    }
    *cur = value;
    ++cur;
    return cur - buf;
}
uint32_t decode_varint(const uint8_t **bufp)
{
    const uint8_t *cur = *bufp;
    uint8_t byte = *cur++;
    uint32_t value = byte & 0x7f; // 0111 1111
    size_t shift = 7;
    while (byte >= 0x80)
    {
        byte = *cur++;
        value += (byte & 0x7f) << shift;
        shift += 7;
    }
    *bufp = cur;
    return value;
}
void printArr(uint32_t *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (i % 10 == 0)
        {
            printf("\n");
        }
        printf(" %d ", arr[i]);
    }
    printf("\n-------------------------------------");
}

int main()
{
    
    // Кодирование
    srand(time(NULL));
    uint32_t *values = malloc(sizeof(uint32_t) * MAX);
    size_t size = 0;
    uint8_t buf[4] = {};
    uint8_t *compressed = malloc(sizeof(uint8_t) * MAX * 2);
    uint8_t *cur = compressed;

    for (int i = 0; i < MAX; i++) // заполняет массив
    {
        values[i] = generate_number();
    }
    int max = 0;
    for (int i = 0; i < MAX; i++) // Кодирует числа в массив commpressed
    {
        size = encode_varint(values[i], buf);
        for (int j = 0; j < size; j++)
        {
            *cur = buf[j];
            cur++;
            max++;
        }
    }
    FILE *f;
    f = fopen("uncompressed.dat", "wb"); // записывает из массив в uncompressed файл
    fwrite(values, sizeof(values[0]), MAX, f);
    fclose(f);

    f = fopen("compressed.dat", "wb"); // записывает из массив в compressed файл
    fwrite(compressed, sizeof(compressed[0]), max, f);
    fclose(f);

    free(values);
    free(compressed);

    // Чтение
    uint32_t *ValuesFromFile = malloc(sizeof(uint32_t) * MAX);
    uint8_t *CompressedValuesFromFile = malloc(sizeof(uint8_t) * max);
    uint8_t *curp = CompressedValuesFromFile;
    const uint8_t **bufp = &curp;

    f = fopen("uncompressed.dat", "rb"); // читает из uncompressed файла в массив CompressedValuesFromFile
    if (f != NULL)
    {
        for (int i = 0; !feof(f); i++)
        {
            fread(&ValuesFromFile[i], sizeof(uint32_t), 1, f);
        }
    }
    fclose(f);
    f = fopen("compressed.dat", "rb"); // читает из compressed файла в массив CompressedValuesFromFile
    if (f != NULL)
    {
        for (int i = 0; !feof(f); i++)
        {
            fread(&CompressedValuesFromFile[i], sizeof(uint8_t), 1, f);
        }
    }
    fclose(f);

    uint32_t UnCompressedValuesFromFile[MAX] = {};
    for (int i = 0; i < MAX; i++) // Декодирует
    {
        UnCompressedValuesFromFile[i] = decode_varint(bufp);
    }

    for (int i = 0; i < MAX; i++)
    {
        assert((ValuesFromFile[i] == UnCompressedValuesFromFile[i]) && "Значения из сжатого и несжатого файла не совпадают"); // Выполняется, когда возвращает 0
    }
    printArr(ValuesFromFile, MAX);
    printArr(UnCompressedValuesFromFile, MAX);

    free(ValuesFromFile);
    free(CompressedValuesFromFile);
    /*
    srand(time(NULL));
    uint32_t *values = malloc(sizeof(uint32_t) * MAX);
    size_t size = 0;
    uint8_t buf[4] = {};
    uint8_t *compressed = malloc(sizeof(uint8_t) * MAX * 2);
    uint8_t *cur = compressed;

    int count = 1;
    values[0] = 0x77bb;

    int max = 0;
    for (int i = 0; i < count; i++) // Кодирует числа в массив commpressed
    {
        size = encode_varint(values[i], buf);
        for (int j = 0; j < size; j++)
        {
            *cur = buf[j];
            cur++;
            max++;
        }
    }
    FILE *f;
    f = fopen("uncompressed.dat", "wb"); // записывает из массив в uncompressed файл
    fwrite(values, sizeof(values[0]), count, f);
    fclose(f);

    f = fopen("compressed.dat", "wb"); // записывает из массив в compressed файл
    fwrite(compressed, sizeof(compressed[0]), max, f);
    fclose(f);

    free(values);
    free(compressed);
    */
    return 0;
}