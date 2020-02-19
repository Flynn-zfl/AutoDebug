#include "i2c.h"

/* Microsoft compilers (Visual C++) */
typedef unsigned __int8   u08;
typedef unsigned __int16  u16;
typedef unsigned __int32  u32;
typedef unsigned __int64  u64;
typedef signed   __int8   s08;
typedef signed   __int16  s16;
typedef signed   __int32  s32;
typedef signed   __int64  s64;

typedef float   f32;
typedef double  f64;

typedef int Aardvark;
typedef u08 aa_u08;
typedef u16 aa_u16;
typedef u32 aa_u32;
typedef s08 aa_s08;
typedef s16 aa_s16;
typedef s32 aa_s32;

enum AardvarkI2cFlags {
    AA_I2C_NO_FLAGS          = 0x00,
    AA_I2C_10_BIT_ADDR       = 0x01,
    AA_I2C_COMBINED_FMT      = 0x02,
    AA_I2C_NO_STOP           = 0x04,
    AA_I2C_SIZED_READ        = 0x10,
    AA_I2C_SIZED_READ_EXTRA1 = 0x20
};


int aa_i2c_read (
    Aardvark         aardvark,
    u16              slave_addr,
    AardvarkI2cFlags flags,
    u16              num_bytes,
    u08 *            data_in
);

int aa_i2c_write (
    Aardvark         aardvark,
    u16              slave_addr,
    AardvarkI2cFlags flags,
    u16              num_bytes,
    const u08 *      data_out
)