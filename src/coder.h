#include <stdint.h>
#include <stdio.h>
#pragma once
 
enum
{
  MaxCodeLength = 4
};
typedef struct // Пусть 	110xxxxx 10xxxxxx, тогда length = 2, а code[0] = 110xxxxx, code[1] = 10xxxxxx
{
  uint8_t code[MaxCodeLength];
  size_t length;
} CodeUnit;
int encode(uint32_t code_point, CodeUnit *code_units);
uint32_t decode(const CodeUnit *code_unit);
int read_next_code_unit(FILE *in, CodeUnit *code_units);
int write_code_unit(FILE *out, const CodeUnit *code_unit);