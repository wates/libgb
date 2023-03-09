#ifndef BINCC_H_INCLUDED
#define BINCC_H_INCLUDED
#include <stdint.h>
namespace app_bincc_data{
extern const uint8_t cpu_instrs_gb[];
extern const uint8_t dmg_acid2_gb[];
static const int cpu_instrs_gb_size=65536;
static const int dmg_acid2_gb_size=32768;
static const uint8_t *data[]={
  cpu_instrs_gb,
  dmg_acid2_gb,
};
static const int data_size[]={
  65536,
  32768,
};
static const char* data_names[]={
  "cpu_instrs.gb",
  "dmg-acid2.gb",
};
}
#endif