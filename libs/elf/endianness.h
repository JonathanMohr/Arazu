#ifndef ARAZU_LELF_ENDIANNESS_H
#define ARAZU_LELF_ENDIANNESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/types.h>

Arazu_u8  Arazu_Endianness_Convert_LittleEndian_u8(const Arazu_u8 value);
Arazu_u16 Arazu_Endianness_Convert_LittleEndian_u16(const Arazu_u16 value);
Arazu_u32 Arazu_Endianness_Convert_LittleEndian_u32(const Arazu_u32 value);
Arazu_u64 Arazu_Endianness_Convert_LittleEndian_u64(const Arazu_u64 value);

Arazu_i8  Arazu_Endianness_Convert_LittleEndian_i8(const Arazu_i8 value);
Arazu_i16 Arazu_Endianness_Convert_LittleEndian_i16(const Arazu_i16 value);
Arazu_i32 Arazu_Endianness_Convert_LittleEndian_i32(const Arazu_i32 value);
Arazu_i64 Arazu_Endianness_Convert_LittleEndian_i64(const Arazu_i64 value);


Arazu_u8  Arazu_Endianness_Convert_BigEndian_u8(const Arazu_u8 value);
Arazu_u16 Arazu_Endianness_Convert_BigEndian_u16(const Arazu_u16 value);
Arazu_u32 Arazu_Endianness_Convert_BigEndian_u32(const Arazu_u32 value);
Arazu_u64 Arazu_Endianness_Convert_BigEndian_u64(const Arazu_u64 value);

Arazu_i8  Arazu_Endianness_Convert_BigEndian_i8(const Arazu_i8 value);
Arazu_i16 Arazu_Endianness_Convert_BigEndian_i16(const Arazu_i16 value);
Arazu_i32 Arazu_Endianness_Convert_BigEndian_i32(const Arazu_i32 value);
Arazu_i64 Arazu_Endianness_Convert_BigEndian_i64(const Arazu_i64 value);

#ifdef __cplusplus
}
#endif

#endif
