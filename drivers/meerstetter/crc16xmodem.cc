#include <stdint.h>
#include "crc16xmodem.h"

// This code assumes that unsigned is 4 bytes.

#ifdef XMODEM_BIT_DEFS
unsigned crc16xmodem_bit(unsigned crc, void const *mem, size_t len) {
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    while (len--) {
        crc ^= (unsigned)(*data++) << 8;
        for (unsigned k = 0; k < 8; k++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    crc &= 0xffff;
    return crc;
}

unsigned crc16xmodem_rem(unsigned crc, unsigned val, unsigned bits) {
    val &= 0x100 - (0x100 >> bits) ;
    crc ^= (unsigned)val << 8;
    while (bits--)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    crc &= 0xffff;
    return crc;
}
#endif

static unsigned short const table_byte[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129,
    0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252,
    0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
    0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672,
    0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
    0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861,
    0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5,
    0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b,
    0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9,
    0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3,
    0x5004, 0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
    0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3,
    0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676,
    0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
    0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16,
    0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b,
    0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36,
    0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

#ifdef XMODEM_WORD_DEFS
static unsigned short const table_word[][256] = {
   {0x0000, 0x2110, 0x4220, 0x6330, 0x8440, 0xa550, 0xc660, 0xe770, 0x0881, 0x2991,
    0x4aa1, 0x6bb1, 0x8cc1, 0xadd1, 0xcee1, 0xeff1, 0x3112, 0x1002, 0x7332, 0x5222,
    0xb552, 0x9442, 0xf772, 0xd662, 0x3993, 0x1883, 0x7bb3, 0x5aa3, 0xbdd3, 0x9cc3,
    0xfff3, 0xdee3, 0x6224, 0x4334, 0x2004, 0x0114, 0xe664, 0xc774, 0xa444, 0x8554,
    0x6aa5, 0x4bb5, 0x2885, 0x0995, 0xeee5, 0xcff5, 0xacc5, 0x8dd5, 0x5336, 0x7226,
    0x1116, 0x3006, 0xd776, 0xf666, 0x9556, 0xb446, 0x5bb7, 0x7aa7, 0x1997, 0x3887,
    0xdff7, 0xfee7, 0x9dd7, 0xbcc7, 0xc448, 0xe558, 0x8668, 0xa778, 0x4008, 0x6118,
    0x0228, 0x2338, 0xccc9, 0xedd9, 0x8ee9, 0xaff9, 0x4889, 0x6999, 0x0aa9, 0x2bb9,
    0xf55a, 0xd44a, 0xb77a, 0x966a, 0x711a, 0x500a, 0x333a, 0x122a, 0xfddb, 0xdccb,
    0xbffb, 0x9eeb, 0x799b, 0x588b, 0x3bbb, 0x1aab, 0xa66c, 0x877c, 0xe44c, 0xc55c,
    0x222c, 0x033c, 0x600c, 0x411c, 0xaeed, 0x8ffd, 0xeccd, 0xcddd, 0x2aad, 0x0bbd,
    0x688d, 0x499d, 0x977e, 0xb66e, 0xd55e, 0xf44e, 0x133e, 0x322e, 0x511e, 0x700e,
    0x9fff, 0xbeef, 0xdddf, 0xfccf, 0x1bbf, 0x3aaf, 0x599f, 0x788f, 0x8891, 0xa981,
    0xcab1, 0xeba1, 0x0cd1, 0x2dc1, 0x4ef1, 0x6fe1, 0x8010, 0xa100, 0xc230, 0xe320,
    0x0450, 0x2540, 0x4670, 0x6760, 0xb983, 0x9893, 0xfba3, 0xdab3, 0x3dc3, 0x1cd3,
    0x7fe3, 0x5ef3, 0xb102, 0x9012, 0xf322, 0xd232, 0x3542, 0x1452, 0x7762, 0x5672,
    0xeab5, 0xcba5, 0xa895, 0x8985, 0x6ef5, 0x4fe5, 0x2cd5, 0x0dc5, 0xe234, 0xc324,
    0xa014, 0x8104, 0x6674, 0x4764, 0x2454, 0x0544, 0xdba7, 0xfab7, 0x9987, 0xb897,
    0x5fe7, 0x7ef7, 0x1dc7, 0x3cd7, 0xd326, 0xf236, 0x9106, 0xb016, 0x5766, 0x7676,
    0x1546, 0x3456, 0x4cd9, 0x6dc9, 0x0ef9, 0x2fe9, 0xc899, 0xe989, 0x8ab9, 0xaba9,
    0x4458, 0x6548, 0x0678, 0x2768, 0xc018, 0xe108, 0x8238, 0xa328, 0x7dcb, 0x5cdb,
    0x3feb, 0x1efb, 0xf98b, 0xd89b, 0xbbab, 0x9abb, 0x754a, 0x545a, 0x376a, 0x167a,
    0xf10a, 0xd01a, 0xb32a, 0x923a, 0x2efd, 0x0fed, 0x6cdd, 0x4dcd, 0xaabd, 0x8bad,
    0xe89d, 0xc98d, 0x267c, 0x076c, 0x645c, 0x454c, 0xa23c, 0x832c, 0xe01c, 0xc10c,
    0x1fef, 0x3eff, 0x5dcf, 0x7cdf, 0x9baf, 0xbabf, 0xd98f, 0xf89f, 0x176e, 0x367e,
    0x554e, 0x745e, 0x932e, 0xb23e, 0xd10e, 0xf01e},
   {0x0000, 0x3133, 0x6266, 0x5355, 0xc4cc, 0xf5ff, 0xa6aa, 0x9799, 0xa989, 0x98ba,
    0xcbef, 0xfadc, 0x6d45, 0x5c76, 0x0f23, 0x3e10, 0x7303, 0x4230, 0x1165, 0x2056,
    0xb7cf, 0x86fc, 0xd5a9, 0xe49a, 0xda8a, 0xebb9, 0xb8ec, 0x89df, 0x1e46, 0x2f75,
    0x7c20, 0x4d13, 0xe606, 0xd735, 0x8460, 0xb553, 0x22ca, 0x13f9, 0x40ac, 0x719f,
    0x4f8f, 0x7ebc, 0x2de9, 0x1cda, 0x8b43, 0xba70, 0xe925, 0xd816, 0x9505, 0xa436,
    0xf763, 0xc650, 0x51c9, 0x60fa, 0x33af, 0x029c, 0x3c8c, 0x0dbf, 0x5eea, 0x6fd9,
    0xf840, 0xc973, 0x9a26, 0xab15, 0xcc0d, 0xfd3e, 0xae6b, 0x9f58, 0x08c1, 0x39f2,
    0x6aa7, 0x5b94, 0x6584, 0x54b7, 0x07e2, 0x36d1, 0xa148, 0x907b, 0xc32e, 0xf21d,
    0xbf0e, 0x8e3d, 0xdd68, 0xec5b, 0x7bc2, 0x4af1, 0x19a4, 0x2897, 0x1687, 0x27b4,
    0x74e1, 0x45d2, 0xd24b, 0xe378, 0xb02d, 0x811e, 0x2a0b, 0x1b38, 0x486d, 0x795e,
    0xeec7, 0xdff4, 0x8ca1, 0xbd92, 0x8382, 0xb2b1, 0xe1e4, 0xd0d7, 0x474e, 0x767d,
    0x2528, 0x141b, 0x5908, 0x683b, 0x3b6e, 0x0a5d, 0x9dc4, 0xacf7, 0xffa2, 0xce91,
    0xf081, 0xc1b2, 0x92e7, 0xa3d4, 0x344d, 0x057e, 0x562b, 0x6718, 0x981b, 0xa928,
    0xfa7d, 0xcb4e, 0x5cd7, 0x6de4, 0x3eb1, 0x0f82, 0x3192, 0x00a1, 0x53f4, 0x62c7,
    0xf55e, 0xc46d, 0x9738, 0xa60b, 0xeb18, 0xda2b, 0x897e, 0xb84d, 0x2fd4, 0x1ee7,
    0x4db2, 0x7c81, 0x4291, 0x73a2, 0x20f7, 0x11c4, 0x865d, 0xb76e, 0xe43b, 0xd508,
    0x7e1d, 0x4f2e, 0x1c7b, 0x2d48, 0xbad1, 0x8be2, 0xd8b7, 0xe984, 0xd794, 0xe6a7,
    0xb5f2, 0x84c1, 0x1358, 0x226b, 0x713e, 0x400d, 0x0d1e, 0x3c2d, 0x6f78, 0x5e4b,
    0xc9d2, 0xf8e1, 0xabb4, 0x9a87, 0xa497, 0x95a4, 0xc6f1, 0xf7c2, 0x605b, 0x5168,
    0x023d, 0x330e, 0x5416, 0x6525, 0x3670, 0x0743, 0x90da, 0xa1e9, 0xf2bc, 0xc38f,
    0xfd9f, 0xccac, 0x9ff9, 0xaeca, 0x3953, 0x0860, 0x5b35, 0x6a06, 0x2715, 0x1626,
    0x4573, 0x7440, 0xe3d9, 0xd2ea, 0x81bf, 0xb08c, 0x8e9c, 0xbfaf, 0xecfa, 0xddc9,
    0x4a50, 0x7b63, 0x2836, 0x1905, 0xb210, 0x8323, 0xd076, 0xe145, 0x76dc, 0x47ef,
    0x14ba, 0x2589, 0x1b99, 0x2aaa, 0x79ff, 0x48cc, 0xdf55, 0xee66, 0xbd33, 0x8c00,
    0xc113, 0xf020, 0xa375, 0x9246, 0x05df, 0x34ec, 0x67b9, 0x568a, 0x689a, 0x59a9,
    0x0afc, 0x3bcf, 0xac56, 0x9d65, 0xce30, 0xff03},
   {0x0000, 0x3037, 0x606e, 0x5059, 0xc0dc, 0xf0eb, 0xa0b2, 0x9085, 0xa1a9, 0x919e,
    0xc1c7, 0xf1f0, 0x6175, 0x5142, 0x011b, 0x312c, 0x6343, 0x5374, 0x032d, 0x331a,
    0xa39f, 0x93a8, 0xc3f1, 0xf3c6, 0xc2ea, 0xf2dd, 0xa284, 0x92b3, 0x0236, 0x3201,
    0x6258, 0x526f, 0xc686, 0xf6b1, 0xa6e8, 0x96df, 0x065a, 0x366d, 0x6634, 0x5603,
    0x672f, 0x5718, 0x0741, 0x3776, 0xa7f3, 0x97c4, 0xc79d, 0xf7aa, 0xa5c5, 0x95f2,
    0xc5ab, 0xf59c, 0x6519, 0x552e, 0x0577, 0x3540, 0x046c, 0x345b, 0x6402, 0x5435,
    0xc4b0, 0xf487, 0xa4de, 0x94e9, 0xad1d, 0x9d2a, 0xcd73, 0xfd44, 0x6dc1, 0x5df6,
    0x0daf, 0x3d98, 0x0cb4, 0x3c83, 0x6cda, 0x5ced, 0xcc68, 0xfc5f, 0xac06, 0x9c31,
    0xce5e, 0xfe69, 0xae30, 0x9e07, 0x0e82, 0x3eb5, 0x6eec, 0x5edb, 0x6ff7, 0x5fc0,
    0x0f99, 0x3fae, 0xaf2b, 0x9f1c, 0xcf45, 0xff72, 0x6b9b, 0x5bac, 0x0bf5, 0x3bc2,
    0xab47, 0x9b70, 0xcb29, 0xfb1e, 0xca32, 0xfa05, 0xaa5c, 0x9a6b, 0x0aee, 0x3ad9,
    0x6a80, 0x5ab7, 0x08d8, 0x38ef, 0x68b6, 0x5881, 0xc804, 0xf833, 0xa86a, 0x985d,
    0xa971, 0x9946, 0xc91f, 0xf928, 0x69ad, 0x599a, 0x09c3, 0x39f4, 0x5a3b, 0x6a0c,
    0x3a55, 0x0a62, 0x9ae7, 0xaad0, 0xfa89, 0xcabe, 0xfb92, 0xcba5, 0x9bfc, 0xabcb,
    0x3b4e, 0x0b79, 0x5b20, 0x6b17, 0x3978, 0x094f, 0x5916, 0x6921, 0xf9a4, 0xc993,
    0x99ca, 0xa9fd, 0x98d1, 0xa8e6, 0xf8bf, 0xc888, 0x580d, 0x683a, 0x3863, 0x0854,
    0x9cbd, 0xac8a, 0xfcd3, 0xcce4, 0x5c61, 0x6c56, 0x3c0f, 0x0c38, 0x3d14, 0x0d23,
    0x5d7a, 0x6d4d, 0xfdc8, 0xcdff, 0x9da6, 0xad91, 0xfffe, 0xcfc9, 0x9f90, 0xafa7,
    0x3f22, 0x0f15, 0x5f4c, 0x6f7b, 0x5e57, 0x6e60, 0x3e39, 0x0e0e, 0x9e8b, 0xaebc,
    0xfee5, 0xced2, 0xf726, 0xc711, 0x9748, 0xa77f, 0x37fa, 0x07cd, 0x5794, 0x67a3,
    0x568f, 0x66b8, 0x36e1, 0x06d6, 0x9653, 0xa664, 0xf63d, 0xc60a, 0x9465, 0xa452,
    0xf40b, 0xc43c, 0x54b9, 0x648e, 0x34d7, 0x04e0, 0x35cc, 0x05fb, 0x55a2, 0x6595,
    0xf510, 0xc527, 0x957e, 0xa549, 0x31a0, 0x0197, 0x51ce, 0x61f9, 0xf17c, 0xc14b,
    0x9112, 0xa125, 0x9009, 0xa03e, 0xf067, 0xc050, 0x50d5, 0x60e2, 0x30bb, 0x008c,
    0x52e3, 0x62d4, 0x328d, 0x02ba, 0x923f, 0xa208, 0xf251, 0xc266, 0xf34a, 0xc37d,
    0x9324, 0xa313, 0x3396, 0x03a1, 0x53f8, 0x63cf},
   {0x0000, 0xb476, 0x68ed, 0xdc9b, 0xf1ca, 0x45bc, 0x9927, 0x2d51, 0xc385, 0x77f3,
    0xab68, 0x1f1e, 0x324f, 0x8639, 0x5aa2, 0xeed4, 0xa71b, 0x136d, 0xcff6, 0x7b80,
    0x56d1, 0xe2a7, 0x3e3c, 0x8a4a, 0x649e, 0xd0e8, 0x0c73, 0xb805, 0x9554, 0x2122,
    0xfdb9, 0x49cf, 0x4e37, 0xfa41, 0x26da, 0x92ac, 0xbffd, 0x0b8b, 0xd710, 0x6366,
    0x8db2, 0x39c4, 0xe55f, 0x5129, 0x7c78, 0xc80e, 0x1495, 0xa0e3, 0xe92c, 0x5d5a,
    0x81c1, 0x35b7, 0x18e6, 0xac90, 0x700b, 0xc47d, 0x2aa9, 0x9edf, 0x4244, 0xf632,
    0xdb63, 0x6f15, 0xb38e, 0x07f8, 0x9c6e, 0x2818, 0xf483, 0x40f5, 0x6da4, 0xd9d2,
    0x0549, 0xb13f, 0x5feb, 0xeb9d, 0x3706, 0x8370, 0xae21, 0x1a57, 0xc6cc, 0x72ba,
    0x3b75, 0x8f03, 0x5398, 0xe7ee, 0xcabf, 0x7ec9, 0xa252, 0x1624, 0xf8f0, 0x4c86,
    0x901d, 0x246b, 0x093a, 0xbd4c, 0x61d7, 0xd5a1, 0xd259, 0x662f, 0xbab4, 0x0ec2,
    0x2393, 0x97e5, 0x4b7e, 0xff08, 0x11dc, 0xa5aa, 0x7931, 0xcd47, 0xe016, 0x5460,
    0x88fb, 0x3c8d, 0x7542, 0xc134, 0x1daf, 0xa9d9, 0x8488, 0x30fe, 0xec65, 0x5813,
    0xb6c7, 0x02b1, 0xde2a, 0x6a5c, 0x470d, 0xf37b, 0x2fe0, 0x9b96, 0x38dd, 0x8cab,
    0x5030, 0xe446, 0xc917, 0x7d61, 0xa1fa, 0x158c, 0xfb58, 0x4f2e, 0x93b5, 0x27c3,
    0x0a92, 0xbee4, 0x627f, 0xd609, 0x9fc6, 0x2bb0, 0xf72b, 0x435d, 0x6e0c, 0xda7a,
    0x06e1, 0xb297, 0x5c43, 0xe835, 0x34ae, 0x80d8, 0xad89, 0x19ff, 0xc564, 0x7112,
    0x76ea, 0xc29c, 0x1e07, 0xaa71, 0x8720, 0x3356, 0xefcd, 0x5bbb, 0xb56f, 0x0119,
    0xdd82, 0x69f4, 0x44a5, 0xf0d3, 0x2c48, 0x983e, 0xd1f1, 0x6587, 0xb91c, 0x0d6a,
    0x203b, 0x944d, 0x48d6, 0xfca0, 0x1274, 0xa602, 0x7a99, 0xceef, 0xe3be, 0x57c8,
    0x8b53, 0x3f25, 0xa4b3, 0x10c5, 0xcc5e, 0x7828, 0x5579, 0xe10f, 0x3d94, 0x89e2,
    0x6736, 0xd340, 0x0fdb, 0xbbad, 0x96fc, 0x228a, 0xfe11, 0x4a67, 0x03a8, 0xb7de,
    0x6b45, 0xdf33, 0xf262, 0x4614, 0x9a8f, 0x2ef9, 0xc02d, 0x745b, 0xa8c0, 0x1cb6,
    0x31e7, 0x8591, 0x590a, 0xed7c, 0xea84, 0x5ef2, 0x8269, 0x361f, 0x1b4e, 0xaf38,
    0x73a3, 0xc7d5, 0x2901, 0x9d77, 0x41ec, 0xf59a, 0xd8cb, 0x6cbd, 0xb026, 0x0450,
    0x4d9f, 0xf9e9, 0x2572, 0x9104, 0xbc55, 0x0823, 0xd4b8, 0x60ce, 0x8e1a, 0x3a6c,
    0xe6f7, 0x5281, 0x7fd0, 0xcba6, 0x173d, 0xa34b},
   {0x0000, 0x51aa, 0x8344, 0xd2ee, 0x0689, 0x5723, 0x85cd, 0xd467, 0x2d02, 0x7ca8,
    0xae46, 0xffec, 0x2b8b, 0x7a21, 0xa8cf, 0xf965, 0x5a04, 0x0bae, 0xd940, 0x88ea,
    0x5c8d, 0x0d27, 0xdfc9, 0x8e63, 0x7706, 0x26ac, 0xf442, 0xa5e8, 0x718f, 0x2025,
    0xf2cb, 0xa361, 0xb408, 0xe5a2, 0x374c, 0x66e6, 0xb281, 0xe32b, 0x31c5, 0x606f,
    0x990a, 0xc8a0, 0x1a4e, 0x4be4, 0x9f83, 0xce29, 0x1cc7, 0x4d6d, 0xee0c, 0xbfa6,
    0x6d48, 0x3ce2, 0xe885, 0xb92f, 0x6bc1, 0x3a6b, 0xc30e, 0x92a4, 0x404a, 0x11e0,
    0xc587, 0x942d, 0x46c3, 0x1769, 0x6811, 0x39bb, 0xeb55, 0xbaff, 0x6e98, 0x3f32,
    0xeddc, 0xbc76, 0x4513, 0x14b9, 0xc657, 0x97fd, 0x439a, 0x1230, 0xc0de, 0x9174,
    0x3215, 0x63bf, 0xb151, 0xe0fb, 0x349c, 0x6536, 0xb7d8, 0xe672, 0x1f17, 0x4ebd,
    0x9c53, 0xcdf9, 0x199e, 0x4834, 0x9ada, 0xcb70, 0xdc19, 0x8db3, 0x5f5d, 0x0ef7,
    0xda90, 0x8b3a, 0x59d4, 0x087e, 0xf11b, 0xa0b1, 0x725f, 0x23f5, 0xf792, 0xa638,
    0x74d6, 0x257c, 0x861d, 0xd7b7, 0x0559, 0x54f3, 0x8094, 0xd13e, 0x03d0, 0x527a,
    0xab1f, 0xfab5, 0x285b, 0x79f1, 0xad96, 0xfc3c, 0x2ed2, 0x7f78, 0xd022, 0x8188,
    0x5366, 0x02cc, 0xd6ab, 0x8701, 0x55ef, 0x0445, 0xfd20, 0xac8a, 0x7e64, 0x2fce,
    0xfba9, 0xaa03, 0x78ed, 0x2947, 0x8a26, 0xdb8c, 0x0962, 0x58c8, 0x8caf, 0xdd05,
    0x0feb, 0x5e41, 0xa724, 0xf68e, 0x2460, 0x75ca, 0xa1ad, 0xf007, 0x22e9, 0x7343,
    0x642a, 0x3580, 0xe76e, 0xb6c4, 0x62a3, 0x3309, 0xe1e7, 0xb04d, 0x4928, 0x1882,
    0xca6c, 0x9bc6, 0x4fa1, 0x1e0b, 0xcce5, 0x9d4f, 0x3e2e, 0x6f84, 0xbd6a, 0xecc0,
    0x38a7, 0x690d, 0xbbe3, 0xea49, 0x132c, 0x4286, 0x9068, 0xc1c2, 0x15a5, 0x440f,
    0x96e1, 0xc74b, 0xb833, 0xe999, 0x3b77, 0x6add, 0xbeba, 0xef10, 0x3dfe, 0x6c54,
    0x9531, 0xc49b, 0x1675, 0x47df, 0x93b8, 0xc212, 0x10fc, 0x4156, 0xe237, 0xb39d,
    0x6173, 0x30d9, 0xe4be, 0xb514, 0x67fa, 0x3650, 0xcf35, 0x9e9f, 0x4c71, 0x1ddb,
    0xc9bc, 0x9816, 0x4af8, 0x1b52, 0x0c3b, 0x5d91, 0x8f7f, 0xded5, 0x0ab2, 0x5b18,
    0x89f6, 0xd85c, 0x2139, 0x7093, 0xa27d, 0xf3d7, 0x27b0, 0x761a, 0xa4f4, 0xf55e,
    0x563f, 0x0795, 0xd57b, 0x84d1, 0x50b6, 0x011c, 0xd3f2, 0x8258, 0x7b3d, 0x2a97,
    0xf879, 0xa9d3, 0x7db4, 0x2c1e, 0xfef0, 0xaf5a},
   {0x0000, 0xa045, 0x408b, 0xe0ce, 0xa106, 0x0143, 0xe18d, 0x41c8, 0x420d, 0xe248,
    0x0286, 0xa2c3, 0xe30b, 0x434e, 0xa380, 0x03c5, 0x841a, 0x245f, 0xc491, 0x64d4,
    0x251c, 0x8559, 0x6597, 0xc5d2, 0xc617, 0x6652, 0x869c, 0x26d9, 0x6711, 0xc754,
    0x279a, 0x87df, 0x0835, 0xa870, 0x48be, 0xe8fb, 0xa933, 0x0976, 0xe9b8, 0x49fd,
    0x4a38, 0xea7d, 0x0ab3, 0xaaf6, 0xeb3e, 0x4b7b, 0xabb5, 0x0bf0, 0x8c2f, 0x2c6a,
    0xcca4, 0x6ce1, 0x2d29, 0x8d6c, 0x6da2, 0xcde7, 0xce22, 0x6e67, 0x8ea9, 0x2eec,
    0x6f24, 0xcf61, 0x2faf, 0x8fea, 0x106a, 0xb02f, 0x50e1, 0xf0a4, 0xb16c, 0x1129,
    0xf1e7, 0x51a2, 0x5267, 0xf222, 0x12ec, 0xb2a9, 0xf361, 0x5324, 0xb3ea, 0x13af,
    0x9470, 0x3435, 0xd4fb, 0x74be, 0x3576, 0x9533, 0x75fd, 0xd5b8, 0xd67d, 0x7638,
    0x96f6, 0x36b3, 0x777b, 0xd73e, 0x37f0, 0x97b5, 0x185f, 0xb81a, 0x58d4, 0xf891,
    0xb959, 0x191c, 0xf9d2, 0x5997, 0x5a52, 0xfa17, 0x1ad9, 0xba9c, 0xfb54, 0x5b11,
    0xbbdf, 0x1b9a, 0x9c45, 0x3c00, 0xdcce, 0x7c8b, 0x3d43, 0x9d06, 0x7dc8, 0xdd8d,
    0xde48, 0x7e0d, 0x9ec3, 0x3e86, 0x7f4e, 0xdf0b, 0x3fc5, 0x9f80, 0x20d4, 0x8091,
    0x605f, 0xc01a, 0x81d2, 0x2197, 0xc159, 0x611c, 0x62d9, 0xc29c, 0x2252, 0x8217,
    0xc3df, 0x639a, 0x8354, 0x2311, 0xa4ce, 0x048b, 0xe445, 0x4400, 0x05c8, 0xa58d,
    0x4543, 0xe506, 0xe6c3, 0x4686, 0xa648, 0x060d, 0x47c5, 0xe780, 0x074e, 0xa70b,
    0x28e1, 0x88a4, 0x686a, 0xc82f, 0x89e7, 0x29a2, 0xc96c, 0x6929, 0x6aec, 0xcaa9,
    0x2a67, 0x8a22, 0xcbea, 0x6baf, 0x8b61, 0x2b24, 0xacfb, 0x0cbe, 0xec70, 0x4c35,
    0x0dfd, 0xadb8, 0x4d76, 0xed33, 0xeef6, 0x4eb3, 0xae7d, 0x0e38, 0x4ff0, 0xefb5,
    0x0f7b, 0xaf3e, 0x30be, 0x90fb, 0x7035, 0xd070, 0x91b8, 0x31fd, 0xd133, 0x7176,
    0x72b3, 0xd2f6, 0x3238, 0x927d, 0xd3b5, 0x73f0, 0x933e, 0x337b, 0xb4a4, 0x14e1,
    0xf42f, 0x546a, 0x15a2, 0xb5e7, 0x5529, 0xf56c, 0xf6a9, 0x56ec, 0xb622, 0x1667,
    0x57af, 0xf7ea, 0x1724, 0xb761, 0x388b, 0x98ce, 0x7800, 0xd845, 0x998d, 0x39c8,
    0xd906, 0x7943, 0x7a86, 0xdac3, 0x3a0d, 0x9a48, 0xdb80, 0x7bc5, 0x9b0b, 0x3b4e,
    0xbc91, 0x1cd4, 0xfc1a, 0x5c5f, 0x1d97, 0xbdd2, 0x5d1c, 0xfd59, 0xfe9c, 0x5ed9,
    0xbe17, 0x1e52, 0x5f9a, 0xffdf, 0x1f11, 0xbf54},
   {0x0000, 0x61b8, 0xe360, 0x82d8, 0xc6c1, 0xa779, 0x25a1, 0x4419, 0xad93, 0xcc2b,
    0x4ef3, 0x2f4b, 0x6b52, 0x0aea, 0x8832, 0xe98a, 0x7b37, 0x1a8f, 0x9857, 0xf9ef,
    0xbdf6, 0xdc4e, 0x5e96, 0x3f2e, 0xd6a4, 0xb71c, 0x35c4, 0x547c, 0x1065, 0x71dd,
    0xf305, 0x92bd, 0xf66e, 0x97d6, 0x150e, 0x74b6, 0x30af, 0x5117, 0xd3cf, 0xb277,
    0x5bfd, 0x3a45, 0xb89d, 0xd925, 0x9d3c, 0xfc84, 0x7e5c, 0x1fe4, 0x8d59, 0xece1,
    0x6e39, 0x0f81, 0x4b98, 0x2a20, 0xa8f8, 0xc940, 0x20ca, 0x4172, 0xc3aa, 0xa212,
    0xe60b, 0x87b3, 0x056b, 0x64d3, 0xecdd, 0x8d65, 0x0fbd, 0x6e05, 0x2a1c, 0x4ba4,
    0xc97c, 0xa8c4, 0x414e, 0x20f6, 0xa22e, 0xc396, 0x878f, 0xe637, 0x64ef, 0x0557,
    0x97ea, 0xf652, 0x748a, 0x1532, 0x512b, 0x3093, 0xb24b, 0xd3f3, 0x3a79, 0x5bc1,
    0xd919, 0xb8a1, 0xfcb8, 0x9d00, 0x1fd8, 0x7e60, 0x1ab3, 0x7b0b, 0xf9d3, 0x986b,
    0xdc72, 0xbdca, 0x3f12, 0x5eaa, 0xb720, 0xd698, 0x5440, 0x35f8, 0x71e1, 0x1059,
    0x9281, 0xf339, 0x6184, 0x003c, 0x82e4, 0xe35c, 0xa745, 0xc6fd, 0x4425, 0x259d,
    0xcc17, 0xadaf, 0x2f77, 0x4ecf, 0x0ad6, 0x6b6e, 0xe9b6, 0x880e, 0xf9ab, 0x9813,
    0x1acb, 0x7b73, 0x3f6a, 0x5ed2, 0xdc0a, 0xbdb2, 0x5438, 0x3580, 0xb758, 0xd6e0,
    0x92f9, 0xf341, 0x7199, 0x1021, 0x829c, 0xe324, 0x61fc, 0x0044, 0x445d, 0x25e5,
    0xa73d, 0xc685, 0x2f0f, 0x4eb7, 0xcc6f, 0xadd7, 0xe9ce, 0x8876, 0x0aae, 0x6b16,
    0x0fc5, 0x6e7d, 0xeca5, 0x8d1d, 0xc904, 0xa8bc, 0x2a64, 0x4bdc, 0xa256, 0xc3ee,
    0x4136, 0x208e, 0x6497, 0x052f, 0x87f7, 0xe64f, 0x74f2, 0x154a, 0x9792, 0xf62a,
    0xb233, 0xd38b, 0x5153, 0x30eb, 0xd961, 0xb8d9, 0x3a01, 0x5bb9, 0x1fa0, 0x7e18,
    0xfcc0, 0x9d78, 0x1576, 0x74ce, 0xf616, 0x97ae, 0xd3b7, 0xb20f, 0x30d7, 0x516f,
    0xb8e5, 0xd95d, 0x5b85, 0x3a3d, 0x7e24, 0x1f9c, 0x9d44, 0xfcfc, 0x6e41, 0x0ff9,
    0x8d21, 0xec99, 0xa880, 0xc938, 0x4be0, 0x2a58, 0xc3d2, 0xa26a, 0x20b2, 0x410a,
    0x0513, 0x64ab, 0xe673, 0x87cb, 0xe318, 0x82a0, 0x0078, 0x61c0, 0x25d9, 0x4461,
    0xc6b9, 0xa701, 0x4e8b, 0x2f33, 0xadeb, 0xcc53, 0x884a, 0xe9f2, 0x6b2a, 0x0a92,
    0x982f, 0xf997, 0x7b4f, 0x1af7, 0x5eee, 0x3f56, 0xbd8e, 0xdc36, 0x35bc, 0x5404,
    0xd6dc, 0xb764, 0xf37d, 0x92c5, 0x101d, 0x71a5},
   {0x0000, 0xd347, 0xa68f, 0x75c8, 0x6d0f, 0xbe48, 0xcb80, 0x18c7, 0xda1e, 0x0959,
    0x7c91, 0xafd6, 0xb711, 0x6456, 0x119e, 0xc2d9, 0xb43d, 0x677a, 0x12b2, 0xc1f5,
    0xd932, 0x0a75, 0x7fbd, 0xacfa, 0x6e23, 0xbd64, 0xc8ac, 0x1beb, 0x032c, 0xd06b,
    0xa5a3, 0x76e4, 0x687b, 0xbb3c, 0xcef4, 0x1db3, 0x0574, 0xd633, 0xa3fb, 0x70bc,
    0xb265, 0x6122, 0x14ea, 0xc7ad, 0xdf6a, 0x0c2d, 0x79e5, 0xaaa2, 0xdc46, 0x0f01,
    0x7ac9, 0xa98e, 0xb149, 0x620e, 0x17c6, 0xc481, 0x0658, 0xd51f, 0xa0d7, 0x7390,
    0x6b57, 0xb810, 0xcdd8, 0x1e9f, 0xd0f6, 0x03b1, 0x7679, 0xa53e, 0xbdf9, 0x6ebe,
    0x1b76, 0xc831, 0x0ae8, 0xd9af, 0xac67, 0x7f20, 0x67e7, 0xb4a0, 0xc168, 0x122f,
    0x64cb, 0xb78c, 0xc244, 0x1103, 0x09c4, 0xda83, 0xaf4b, 0x7c0c, 0xbed5, 0x6d92,
    0x185a, 0xcb1d, 0xd3da, 0x009d, 0x7555, 0xa612, 0xb88d, 0x6bca, 0x1e02, 0xcd45,
    0xd582, 0x06c5, 0x730d, 0xa04a, 0x6293, 0xb1d4, 0xc41c, 0x175b, 0x0f9c, 0xdcdb,
    0xa913, 0x7a54, 0x0cb0, 0xdff7, 0xaa3f, 0x7978, 0x61bf, 0xb2f8, 0xc730, 0x1477,
    0xd6ae, 0x05e9, 0x7021, 0xa366, 0xbba1, 0x68e6, 0x1d2e, 0xce69, 0x81fd, 0x52ba,
    0x2772, 0xf435, 0xecf2, 0x3fb5, 0x4a7d, 0x993a, 0x5be3, 0x88a4, 0xfd6c, 0x2e2b,
    0x36ec, 0xe5ab, 0x9063, 0x4324, 0x35c0, 0xe687, 0x934f, 0x4008, 0x58cf, 0x8b88,
    0xfe40, 0x2d07, 0xefde, 0x3c99, 0x4951, 0x9a16, 0x82d1, 0x5196, 0x245e, 0xf719,
    0xe986, 0x3ac1, 0x4f09, 0x9c4e, 0x8489, 0x57ce, 0x2206, 0xf141, 0x3398, 0xe0df,
    0x9517, 0x4650, 0x5e97, 0x8dd0, 0xf818, 0x2b5f, 0x5dbb, 0x8efc, 0xfb34, 0x2873,
    0x30b4, 0xe3f3, 0x963b, 0x457c, 0x87a5, 0x54e2, 0x212a, 0xf26d, 0xeaaa, 0x39ed,
    0x4c25, 0x9f62, 0x510b, 0x824c, 0xf784, 0x24c3, 0x3c04, 0xef43, 0x9a8b, 0x49cc,
    0x8b15, 0x5852, 0x2d9a, 0xfedd, 0xe61a, 0x355d, 0x4095, 0x93d2, 0xe536, 0x3671,
    0x43b9, 0x90fe, 0x8839, 0x5b7e, 0x2eb6, 0xfdf1, 0x3f28, 0xec6f, 0x99a7, 0x4ae0,
    0x5227, 0x8160, 0xf4a8, 0x27ef, 0x3970, 0xea37, 0x9fff, 0x4cb8, 0x547f, 0x8738,
    0xf2f0, 0x21b7, 0xe36e, 0x3029, 0x45e1, 0x96a6, 0x8e61, 0x5d26, 0x28ee, 0xfba9,
    0x8d4d, 0x5e0a, 0x2bc2, 0xf885, 0xe042, 0x3305, 0x46cd, 0x958a, 0x5753, 0x8414,
    0xf1dc, 0x229b, 0x3a5c, 0xe91b, 0x9cd3, 0x4f94}
};
#endif

unsigned crc16xmodem_byte(unsigned crc, void const *mem, size_t len) {
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    while (len--)
        crc = (crc << 8) ^
              table_byte[((crc >> 8) ^ *data++) & 0xff];
    crc &= 0xffff;
    return crc;
}

#ifdef XMODEM_WORD_DEFS
static inline unsigned swaplow(unsigned crc) {
    return
        ((crc & 0xff) << 8) +
        ((crc & 0xff00) >> 8);
}

// This code assumes that integers are stored little-endian.

unsigned crc16xmodem_word(unsigned crc, void const *mem, size_t len) {
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    while (len && ((ptrdiff_t)data & 0x7)) {
        crc = (crc << 8) ^
              table_byte[((crc >> 8) ^ *data++) & 0xff];
        len--;
    }
    if (len >= 8) {
        crc = swaplow(crc);
        do {
            uintmax_t word = crc ^ *(uintmax_t const *)data;
            crc = table_word[7][word & 0xff] ^
                  table_word[6][(word >> 8) & 0xff] ^
                  table_word[5][(word >> 16) & 0xff] ^
                  table_word[4][(word >> 24) & 0xff] ^
                  table_word[3][(word >> 32) & 0xff] ^
                  table_word[2][(word >> 40) & 0xff] ^
                  table_word[1][(word >> 48) & 0xff] ^
                  table_word[0][word >> 56];
            data += 8;
            len -= 8;
        } while (len >= 8);
        crc = swaplow(crc);
    }
    while (len--)
        crc = (crc << 8) ^
              table_byte[((crc >> 8) ^ *data++) & 0xff];
    crc &= 0xffff;
    return crc;
}
#endif
