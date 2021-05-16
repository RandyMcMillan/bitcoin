/**********************************************************************
 * Copyright (c) 2018 Pieter Wuille, Greg Maxwell, Gleb Naumenko      *
 * Distributed under the MIT software license, see the accompanying   *
 * file LICENSE or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

/* This file was substantially auto-generated by doc/gen_params.sage. */

#if !defined(DISABLE_FIELD_17) || !defined(DISABLE_FIELD_18) || !defined(DISABLE_FIELD_19) || !defined(DISABLE_FIELD_20) || !defined(DISABLE_FIELD_21) || !defined(DISABLE_FIELD_22) || !defined(DISABLE_FIELD_23) || !defined(DISABLE_FIELD_24)

#include "generic_common_impl.h"

#include "../lintrans.h"
#include "../sketch_impl.h"

#endif

#include "../sketch.h"

namespace {
#ifndef DISABLE_FIELD_17
// 17 bit field
typedef RecLinTrans<uint32_t, 6, 6, 5> StatTable17;
typedef RecLinTrans<uint32_t, 4, 4, 3, 3, 3> DynTable17;
constexpr StatTable17 SQR_TABLE_17({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x12, 0x48, 0x120, 0x480, 0x1200, 0x4800, 0x12000, 0x8012});
constexpr StatTable17 QRT_TABLE_17({0, 0x4c3e, 0x4c3c, 0x1a248, 0x4c38, 0x428, 0x1a240, 0x1b608, 0x4c28, 0x206, 0x408, 0x4000, 0x1a200, 0x18006, 0x1b688, 0x14d2e, 0x4d28});
typedef Field<uint32_t, 17, 9, StatTable17, DynTable17, &SQR_TABLE_17, &QRT_TABLE_17> Field17;
#endif

#ifndef DISABLE_FIELD_18
// 18 bit field
typedef RecLinTrans<uint32_t, 6, 6, 6> StatTable18;
typedef RecLinTrans<uint32_t, 4, 4, 4, 3, 3> DynTable18;
constexpr StatTable18 SQR_TABLE_18({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x9, 0x24, 0x90, 0x240, 0x900, 0x2400, 0x9000, 0x24000, 0x10012});
constexpr StatTable18 QRT_TABLE_18({0x9208, 0x422, 0x420, 0x8048, 0x424, 0x68b0, 0x8040, 0x30086, 0x434, 0x1040, 0x6890, 0x30ca2, 0x8000, 0x32896, 0x30006, 0, 0x534, 0x20532});
typedef Field<uint32_t, 18, 9, StatTable18, DynTable18, &SQR_TABLE_18, &QRT_TABLE_18> Field18;
#endif

#ifndef DISABLE_FIELD_19
// 19 bit field
typedef RecLinTrans<uint32_t, 5, 5, 5, 4> StatTable19;
typedef RecLinTrans<uint32_t, 4, 4, 4, 4, 3> DynTable19;
constexpr StatTable19 SQR_TABLE_19({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x4e, 0x138, 0x4e0, 0x1380, 0x4e00, 0x13800, 0x4e000, 0x3804e, 0x6011f});
constexpr StatTable19 QRT_TABLE_19({0x5d6b0, 0x2f476, 0x2f474, 0x1d6a2, 0x2f470, 0x42a, 0x1d6aa, 0x1060, 0x2f460, 0x19e92, 0x40a, 0x1da98, 0x1d6ea, 0x28c78, 0x10e0, 0xf56a, 0x2f560, 0, 0x19c92});
typedef Field<uint32_t, 19, 39, StatTable19, DynTable19, &SQR_TABLE_19, &QRT_TABLE_19> Field19;
#endif

#ifndef DISABLE_FIELD_20
// 20 bit field
typedef RecLinTrans<uint32_t, 5, 5, 5, 5> StatTable20;
typedef RecLinTrans<uint32_t, 4, 4, 4, 4, 4> DynTable20;
constexpr StatTable20 SQR_TABLE_20({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x9, 0x24, 0x90, 0x240, 0x900, 0x2400, 0x9000, 0x24000, 0x90000, 0x40012});
constexpr StatTable20 QRT_TABLE_20({0xc5dea, 0xc0110, 0xc0112, 0xe11de, 0xc0116, 0x24814, 0xe11d6, 0x20080, 0xc0106, 0xfe872, 0x24834, 0xe4106, 0xe1196, 0x1d9a4, 0x20000, 0x31190, 0xc0006, 0, 0xfea72, 0x7ea74});
typedef Field<uint32_t, 20, 9, StatTable20, DynTable20, &SQR_TABLE_20, &QRT_TABLE_20> Field20;
#endif

#ifndef DISABLE_FIELD_21
// 21 bit field
typedef RecLinTrans<uint32_t, 6, 5, 5, 5> StatTable21;
typedef RecLinTrans<uint32_t, 4, 4, 4, 3, 3, 3> DynTable21;
constexpr StatTable21 SQR_TABLE_21({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x100000, 0xa, 0x28, 0xa0, 0x280, 0xa00, 0x2800, 0xa000, 0x28000, 0xa0000, 0x80005});
constexpr StatTable21 QRT_TABLE_21({0x1bd5fc, 0xbc196, 0xbc194, 0x74b96, 0xbc190, 0x1048, 0x74b9e, 0x672c8, 0xbc180, 0x4080, 0x1068, 0xc8200, 0x74bde, 0x64280, 0x67248, 0xc4280, 0xbc080, 0x80000, 0x4280, 0, 0x1468});
typedef Field<uint32_t, 21, 5, StatTable21, DynTable21, &SQR_TABLE_21, &QRT_TABLE_21> Field21;
#endif

#ifndef DISABLE_FIELD_22
// 22 bit field
typedef RecLinTrans<uint32_t, 6, 6, 5, 5> StatTable22;
typedef RecLinTrans<uint32_t, 4, 4, 4, 4, 3, 3> DynTable22;
constexpr StatTable22 SQR_TABLE_22({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x100000, 0x3, 0xc, 0x30, 0xc0, 0x300, 0xc00, 0x3000, 0xc000, 0x30000, 0xc0000, 0x300000});
constexpr StatTable22 QRT_TABLE_22({0x210d16, 0x104a, 0x1048, 0x4088, 0x104c, 0x200420, 0x4080, 0x492dc, 0x105c, 0x1a67f0, 0x200400, 0x21155c, 0x40c0, 0x20346c, 0x4925c, 0x1af7ac, 0x115c, 0x2274ac, 0x1a65f0, 0x2a65f0, 0x200000, 0});
typedef Field<uint32_t, 22, 3, StatTable22, DynTable22, &SQR_TABLE_22, &QRT_TABLE_22> Field22;
#endif

#ifndef DISABLE_FIELD_23
// 23 bit field
typedef RecLinTrans<uint32_t, 6, 6, 6, 5> StatTable23;
typedef RecLinTrans<uint32_t, 4, 4, 4, 4, 4, 3> DynTable23;
constexpr StatTable23 SQR_TABLE_23({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x100000, 0x400000, 0x42, 0x108, 0x420, 0x1080, 0x4200, 0x10800, 0x42000, 0x108000, 0x420000, 0x80042, 0x200108});
constexpr StatTable23 QRT_TABLE_23({0, 0x1040, 0x1042, 0x43056, 0x1046, 0x121d76, 0x4305e, 0x40a0, 0x1056, 0x15176, 0x121d56, 0x7ee1f6, 0x4301e, 0x40000, 0x4020, 0x4f0be, 0x1156, 0x7cf0a0, 0x15376, 0x1ee9e8, 0x121956, 0x3ac9f6, 0x7ee9f6});
typedef Field<uint32_t, 23, 33, StatTable23, DynTable23, &SQR_TABLE_23, &QRT_TABLE_23> Field23;
#endif

#ifndef DISABLE_FIELD_24
// 24 bit field
typedef RecLinTrans<uint32_t, 6, 6, 6, 6> StatTable24;
typedef RecLinTrans<uint32_t, 4, 4, 4, 4, 4, 4> DynTable24;
constexpr StatTable24 SQR_TABLE_24({0x1, 0x4, 0x10, 0x40, 0x100, 0x400, 0x1000, 0x4000, 0x10000, 0x40000, 0x100000, 0x400000, 0x1b, 0x6c, 0x1b0, 0x6c0, 0x1b00, 0x6c00, 0x1b000, 0x6c000, 0x1b0000, 0x6c0000, 0xb0001b, 0xc0005a});
constexpr StatTable24 QRT_TABLE_24({0x104e, 0xaf42a8, 0xaf42aa, 0xb78186, 0xaf42ae, 0x4090, 0xb7818e, 0x4a37c, 0xaf42be, 0x3688c0, 0x40b0, 0x80080e, 0xb781ce, 0xaf2232, 0x4a3fc, 0x856a82, 0xaf43be, 0x29c970, 0x368ac0, 0x968ace, 0x44b0, 0x77d570, 0x80000e, 0});
typedef Field<uint32_t, 24, 27, StatTable24, DynTable24, &SQR_TABLE_24, &QRT_TABLE_24> Field24;
#endif
}

Sketch* ConstructGeneric3Bytes(int bits, int implementation)
{
    switch (bits) {
#ifndef DISABLE_FIELD_17
    case 17: return new SketchImpl<Field17>(implementation, 17);
#endif
#ifndef DISABLE_FIELD_18
    case 18: return new SketchImpl<Field18>(implementation, 18);
#endif
#ifndef DISABLE_FIELD_19
    case 19: return new SketchImpl<Field19>(implementation, 19);
#endif
#ifndef DISABLE_FIELD_20
    case 20: return new SketchImpl<Field20>(implementation, 20);
#endif
#ifndef DISABLE_FIELD_21
    case 21: return new SketchImpl<Field21>(implementation, 21);
#endif
#ifndef DISABLE_FIELD_22
    case 22: return new SketchImpl<Field22>(implementation, 22);
#endif
#ifndef DISABLE_FIELD_23
    case 23: return new SketchImpl<Field23>(implementation, 23);
#endif
#ifndef DISABLE_FIELD_24
    case 24: return new SketchImpl<Field24>(implementation, 24);
#endif
    default: return nullptr;
    }
}
