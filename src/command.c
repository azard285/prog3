#include "command.h"
#include "coder.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
int encode_file(const char *in_file_name, const char *out_file_name)
{
  FILE *in;
  FILE *out;
  in = fopen(in_file_name, "r");
  if (!in)
  {
    printf("\nОшибка. Не удалось открыть первый файл. ");
    return -1;
  }
  out = fopen(out_file_name, "wb");
  if (!out)
  {
    printf("\nОшибка. Не удалось открыть второй файл. ");
    return -1;
  }
  uint32_t code_point;
  CodeUnit code_unit;
  while (fscanf(in, "%" SCNx32, &code_point) == 1)
  {
    printf("%" PRIx32, code_point);

    if (encode(code_point, &code_unit) < 0)
    {
      printf("\nОшибка. Число слишком большое.");
      return -1;
    }
    if (write_code_unit(out, &code_unit) < code_unit.length) // Если количество записанных объектов меньше, чем это указано при вызове, то произошла ошибка
    {
      printf("\nОшибка. Не удалось запсать числа в файл");
      return -1;
    }
  }

  fclose(in);
  fclose(out);

  return 0;
}

int decode_file(const char *in_file_name, const char *out_file_name)
{
  FILE *in;
  FILE *out;
  in = fopen(in_file_name, "rb");
  if (!in)
  {
    printf("\nОшибка. Не удалось открыть первый файл. ");
    return -1;
  }

  out = fopen(out_file_name, "w");

  if (!out)
  {
    printf("\nОшибка. Не удалось открыть второй файл. ");
    return -1;
  }

  CodeUnit code_unit;

  while (!feof(in))
  {
    if (read_next_code_unit(in, &code_unit) != 0)
    {
      printf("Ошибка. Битый байт(\n");
      continue;
    }
    if (!feof(in))
    {
      fprintf(out, "%" PRIx32, decode(&code_unit));
    }
  }
  fclose(in);
  fclose(out);

  return 0;
}