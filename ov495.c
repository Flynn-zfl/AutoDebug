#include "ov495.h"

static Aardvark m_hAardvark = NULL;

int aa_i2c_read (
    Aardvark         aardvark,
    u16              slave_addr,
    AardvarkI2cFlags flags,
    u16              num_bytes,
    u08 *            data_in
)
{
    return c_aa_i2c_read(aardvark, slave_addr, flags, num_bytes, data_in);
}



int aa_i2c_write (
    Aardvark         aardvark,
    u16              slave_addr,
    AardvarkI2cFlags flags,
    u16              num_bytes,
    const u08 *      data_out
)
{
    if (c_aa_i2c_write == 0) {
        int res = 0;
        if (!(c_aa_i2c_write = _loadFunction("c_aa_i2c_write", &res)))
            return res;
    }
    return c_aa_i2c_write(aardvark, slave_addr, flags, num_bytes, data_out);
}
