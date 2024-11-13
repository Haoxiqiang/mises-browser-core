use group::ff::PrimeField;
use pasta_curves::{arithmetic::CurveAffine, pallas};

/// The value commitment is a homomorphic Pedersen commitment used to check balance between inputs
/// and outputs. This is its value base, $\mathcal{V}^{\mathsf{Orchard}}$.
pub const GENERATOR: ([u8; 32], [u8; 32]) = (
    [
        103, 67, 249, 58, 110, 189, 167, 42, 140, 124, 90, 43, 127, 163, 4, 254, 50, 178, 155, 79,
        112, 106, 168, 247, 66, 15, 61, 142, 122, 89, 112, 47,
    ],
    [
        142, 242, 90, 175, 126, 196, 19, 164, 219, 227, 255, 167, 102, 167, 158, 29, 66, 108, 109,
        19, 99, 127, 145, 30, 175, 25, 25, 49, 105, 81, 14, 45,
    ],
);

/// Short signed z-values for GENERATOR.
/// These can be reproduced by [`halo2_gadgets::ecc::chip::constants::find_zs_and_us`].
pub const Z_SHORT: [u64; super::NUM_WINDOWS_SHORT] = [
    163547, 76040, 88852, 128479, 54088, 89871, 39598, 144309, 43471, 102492, 741, 55288, 33756,
    77312, 12095, 48253, 45718, 202901, 33132, 71081, 152108, 169712,
];

/// Short signed u-values for GENERATOR.
/// These can be reproduced by [`halo2_gadgets::ecc::chip::constants::find_zs_and_us`].
pub const U_SHORT: [[[u8; 32]; super::H]; super::NUM_WINDOWS_SHORT] = [
    [
        [
            16, 88, 158, 52, 92, 165, 59, 177, 224, 70, 108, 93, 144, 51, 8, 133, 114, 192, 151,
            40, 85, 33, 52, 118, 147, 163, 220, 193, 171, 43, 73, 27,
        ],
        [
            201, 84, 190, 165, 219, 228, 63, 155, 182, 74, 179, 74, 230, 96, 89, 168, 110, 162, 83,
            218, 185, 137, 194, 92, 244, 85, 1, 60, 59, 16, 119, 14,
        ],
        [
            113, 146, 25, 32, 206, 37, 173, 225, 58, 41, 184, 214, 177, 123, 211, 55, 42, 208, 214,
            45, 149, 99, 31, 171, 172, 226, 219, 171, 246, 85, 116, 59,
        ],
        [
            10, 79, 89, 241, 37, 161, 115, 213, 183, 43, 68, 233, 219, 249, 131, 137, 41, 119, 153,
            24, 92, 87, 138, 167, 93, 46, 32, 97, 102, 164, 164, 53,
        ],
        [
            50, 252, 2, 254, 234, 133, 190, 34, 244, 192, 31, 96, 156, 152, 150, 88, 154, 224, 223,
            144, 223, 113, 203, 226, 214, 44, 69, 198, 171, 60, 26, 45,
        ],
        [
            151, 65, 173, 0, 249, 13, 78, 201, 84, 58, 255, 120, 124, 169, 98, 242, 239, 132, 149,
            204, 146, 244, 55, 176, 56, 26, 19, 148, 125, 42, 26, 32,
        ],
        [
            6, 29, 196, 176, 135, 157, 248, 239, 196, 9, 100, 82, 220, 41, 203, 49, 35, 108, 76,
            59, 222, 47, 82, 39, 35, 23, 88, 136, 61, 90, 23, 57,
        ],
        [
            169, 203, 101, 116, 220, 68, 34, 248, 199, 107, 202, 59, 111, 171, 90, 121, 119, 80,
            146, 214, 191, 197, 220, 212, 125, 139, 12, 140, 198, 191, 110, 31,
        ],
    ],
    [
        [
            16, 171, 139, 236, 248, 28, 27, 145, 109, 161, 63, 121, 19, 190, 106, 193, 13, 122,
            108, 54, 95, 202, 195, 184, 118, 60, 72, 213, 154, 240, 52, 34,
        ],
        [
            131, 218, 16, 142, 97, 153, 190, 249, 118, 171, 187, 254, 21, 17, 202, 48, 107, 23,
            103, 72, 159, 238, 228, 38, 183, 166, 74, 136, 77, 51, 85, 18,
        ],
        [
            137, 78, 204, 243, 154, 87, 148, 200, 11, 98, 58, 33, 89, 170, 198, 87, 29, 187, 167,
            122, 157, 151, 5, 148, 254, 216, 113, 12, 158, 243, 102, 63,
        ],
        [
            16, 187, 216, 98, 244, 19, 70, 209, 217, 133, 240, 226, 111, 219, 247, 74, 4, 231, 183,
            216, 35, 174, 192, 96, 129, 159, 39, 96, 200, 174, 126, 1,
        ],
        [
            185, 2, 174, 129, 110, 163, 212, 152, 214, 139, 105, 150, 23, 209, 185, 245, 71, 8,
            171, 142, 43, 57, 167, 172, 233, 77, 194, 166, 118, 232, 127, 35,
        ],
        [
            57, 28, 236, 33, 234, 0, 214, 131, 161, 209, 42, 25, 194, 150, 82, 126, 216, 23, 81,
            85, 90, 47, 95, 46, 119, 108, 253, 12, 92, 29, 13, 25,
        ],
        [
            100, 162, 156, 175, 254, 203, 69, 171, 182, 190, 85, 161, 94, 117, 225, 0, 95, 121,
            190, 235, 79, 7, 147, 134, 48, 156, 113, 55, 114, 229, 174, 25,
        ],
        [
            243, 93, 223, 98, 132, 0, 171, 125, 135, 53, 111, 49, 49, 173, 69, 211, 62, 106, 168,
            221, 115, 208, 2, 20, 14, 150, 124, 130, 189, 48, 16, 15,
        ],
    ],
    [
        [
            43, 238, 4, 240, 186, 135, 47, 128, 185, 51, 70, 196, 3, 137, 22, 254, 29, 247, 161,
            233, 187, 255, 239, 183, 150, 179, 8, 64, 90, 33, 17, 54,
        ],
        [
            20, 226, 19, 60, 92, 195, 1, 255, 100, 188, 89, 54, 98, 225, 138, 133, 26, 140, 202,
            76, 107, 199, 46, 54, 167, 23, 119, 80, 19, 25, 34, 5,
        ],
        [
            145, 109, 10, 159, 200, 145, 32, 253, 32, 93, 187, 12, 66, 161, 44, 217, 16, 100, 225,
            37, 139, 125, 169, 119, 83, 34, 96, 130, 234, 255, 138, 38,
        ],
        [
            148, 127, 71, 227, 195, 244, 12, 49, 83, 78, 218, 9, 176, 158, 12, 5, 194, 54, 44, 157,
            252, 228, 175, 170, 54, 91, 49, 254, 83, 228, 180, 61,
        ],
        [
            36, 96, 238, 61, 227, 144, 153, 81, 121, 197, 74, 190, 35, 216, 255, 92, 70, 7, 168,
            219, 130, 255, 172, 71, 200, 31, 142, 232, 255, 117, 96, 15,
        ],
        [
            50, 152, 255, 238, 188, 127, 140, 240, 203, 33, 246, 193, 228, 179, 25, 1, 227, 194,
            79, 70, 41, 160, 83, 243, 148, 1, 95, 86, 63, 22, 55, 40,
        ],
        [
            99, 228, 147, 46, 232, 157, 225, 210, 45, 159, 169, 42, 184, 90, 3, 165, 62, 4, 93,
            181, 74, 187, 112, 156, 26, 103, 199, 84, 132, 148, 14, 24,
        ],
        [
            120, 163, 182, 125, 197, 141, 122, 21, 166, 103, 203, 57, 160, 228, 242, 192, 203, 40,
            233, 179, 8, 173, 199, 21, 213, 215, 222, 129, 23, 153, 25, 29,
        ],
    ],
    [
        [
            189, 6, 27, 46, 68, 6, 124, 69, 159, 212, 18, 104, 218, 16, 66, 183, 250, 92, 69, 5,
            218, 234, 31, 198, 123, 100, 216, 103, 106, 113, 192, 43,
        ],
        [
            226, 152, 83, 159, 143, 155, 164, 90, 171, 100, 22, 240, 187, 26, 55, 172, 170, 133,
            128, 192, 29, 33, 35, 8, 59, 142, 20, 204, 151, 172, 29, 28,
        ],
        [
            196, 57, 232, 112, 46, 200, 26, 93, 224, 191, 117, 154, 9, 47, 228, 145, 122, 137, 238,
            236, 70, 14, 112, 163, 96, 140, 239, 122, 75, 200, 206, 12,
        ],
        [
            91, 66, 65, 159, 86, 50, 113, 39, 30, 12, 213, 235, 15, 70, 163, 119, 112, 27, 22, 170,
            151, 20, 71, 172, 65, 251, 224, 76, 26, 189, 118, 42,
        ],
        [
            195, 200, 87, 207, 125, 122, 254, 50, 24, 40, 189, 169, 237, 228, 134, 66, 37, 220, 23,
            98, 202, 193, 152, 184, 171, 63, 105, 11, 70, 94, 154, 58,
        ],
        [
            4, 0, 82, 125, 41, 130, 11, 238, 112, 201, 95, 30, 144, 31, 31, 233, 86, 121, 145, 200,
            204, 89, 182, 51, 151, 169, 58, 206, 184, 219, 181, 46,
        ],
        [
            36, 240, 60, 205, 120, 239, 9, 98, 61, 228, 247, 9, 145, 71, 67, 10, 164, 160, 186,
            104, 25, 225, 210, 37, 31, 42, 58, 208, 44, 19, 30, 29,
        ],
        [
            163, 214, 140, 64, 48, 239, 54, 175, 108, 32, 187, 248, 142, 77, 244, 117, 10, 236, 39,
            80, 158, 44, 98, 44, 248, 24, 208, 60, 173, 247, 115, 28,
        ],
    ],
    [
        [
            230, 166, 7, 118, 178, 55, 26, 250, 91, 159, 230, 158, 34, 135, 91, 49, 166, 244, 124,
            175, 86, 228, 159, 119, 231, 15, 19, 199, 219, 85, 233, 19,
        ],
        [
            65, 240, 77, 9, 197, 255, 96, 35, 239, 161, 177, 115, 83, 114, 180, 179, 118, 158, 220,
            61, 177, 3, 175, 70, 132, 51, 87, 79, 36, 104, 129, 15,
        ],
        [
            245, 9, 141, 218, 228, 57, 204, 250, 83, 156, 160, 158, 146, 157, 206, 198, 16, 94, 80,
            59, 31, 242, 163, 30, 80, 99, 32, 142, 193, 101, 172, 22,
        ],
        [
            22, 68, 73, 27, 236, 149, 225, 78, 69, 83, 174, 202, 98, 113, 148, 167, 43, 132, 118,
            49, 153, 96, 186, 134, 20, 103, 123, 48, 75, 104, 207, 6,
        ],
        [
            186, 5, 66, 183, 166, 255, 222, 14, 34, 0, 105, 167, 36, 11, 210, 228, 91, 103, 65,
            248, 3, 48, 117, 240, 180, 214, 201, 222, 5, 202, 103, 42,
        ],
        [
            231, 31, 19, 122, 240, 72, 34, 93, 204, 125, 90, 215, 124, 174, 124, 28, 145, 103, 126,
            178, 1, 9, 152, 240, 251, 118, 14, 195, 197, 167, 136, 22,
        ],
        [
            121, 57, 39, 104, 26, 135, 98, 221, 85, 199, 94, 230, 223, 129, 28, 191, 185, 0, 46,
            200, 72, 116, 202, 255, 80, 180, 13, 98, 229, 217, 238, 17,
        ],
        [
            228, 158, 59, 241, 50, 224, 177, 78, 8, 121, 211, 157, 95, 196, 88, 59, 85, 141, 134,
            50, 123, 168, 175, 115, 240, 153, 223, 61, 71, 229, 77, 10,
        ],
    ],
    [
        [
            196, 151, 212, 168, 230, 19, 214, 179, 118, 17, 196, 50, 76, 231, 114, 80, 178, 137,
            232, 241, 60, 2, 0, 124, 25, 239, 98, 131, 220, 159, 221, 51,
        ],
        [
            207, 216, 50, 53, 106, 200, 85, 166, 137, 33, 29, 239, 97, 46, 104, 218, 177, 150, 178,
            64, 232, 238, 208, 98, 103, 58, 233, 97, 1, 243, 73, 5,
        ],
        [
            174, 215, 22, 124, 193, 136, 79, 91, 111, 146, 51, 128, 249, 129, 203, 168, 157, 164,
            252, 144, 206, 10, 63, 253, 250, 113, 251, 65, 218, 23, 12, 25,
        ],
        [
            186, 92, 109, 134, 176, 198, 214, 156, 148, 7, 106, 121, 226, 70, 250, 192, 180, 164,
            16, 29, 245, 196, 155, 183, 214, 26, 3, 26, 249, 134, 153, 38,
        ],
        [
            244, 56, 170, 227, 236, 145, 153, 150, 166, 77, 201, 48, 100, 130, 204, 194, 108, 207,
            3, 243, 227, 41, 45, 8, 136, 106, 58, 70, 126, 3, 132, 59,
        ],
        [
            215, 169, 24, 118, 5, 192, 171, 217, 47, 86, 212, 249, 228, 92, 248, 103, 242, 222, 17,
            206, 36, 157, 126, 224, 105, 113, 239, 102, 54, 25, 151, 14,
        ],
        [
            129, 213, 202, 163, 74, 201, 249, 43, 200, 62, 4, 232, 68, 211, 44, 153, 25, 71, 155,
            5, 93, 19, 117, 195, 201, 179, 158, 207, 252, 213, 166, 62,
        ],
        [
            86, 75, 236, 222, 217, 118, 49, 0, 141, 130, 201, 248, 34, 225, 69, 91, 186, 90, 246,
            36, 141, 152, 166, 216, 118, 7, 38, 133, 147, 190, 79, 3,
        ],
    ],
    [
        [
            226, 55, 121, 73, 104, 77, 129, 202, 168, 123, 192, 163, 28, 9, 195, 37, 116, 207, 125,
            198, 203, 244, 121, 236, 232, 91, 168, 14, 142, 241, 250, 60,
        ],
        [
            222, 86, 183, 37, 132, 107, 210, 125, 127, 46, 94, 81, 18, 91, 72, 160, 16, 193, 239,
            114, 238, 186, 186, 203, 96, 169, 87, 155, 108, 36, 97, 6,
        ],
        [
            110, 110, 99, 98, 167, 78, 75, 128, 33, 138, 18, 19, 194, 192, 219, 184, 74, 196, 82,
            115, 241, 102, 30, 197, 199, 194, 154, 120, 49, 10, 95, 37,
        ],
        [
            212, 206, 154, 98, 20, 33, 185, 182, 138, 207, 65, 197, 246, 19, 132, 52, 173, 186, 42,
            243, 88, 20, 51, 11, 206, 25, 216, 48, 162, 138, 124, 13,
        ],
        [
            32, 161, 64, 72, 1, 32, 243, 175, 251, 37, 86, 248, 136, 187, 181, 55, 39, 255, 98,
            228, 189, 235, 194, 2, 228, 39, 92, 104, 245, 17, 117, 7,
        ],
        [
            207, 205, 116, 251, 54, 21, 8, 82, 173, 45, 205, 38, 245, 155, 16, 56, 198, 232, 173,
            88, 97, 22, 234, 26, 139, 206, 108, 254, 123, 87, 181, 26,
        ],
        [
            38, 147, 223, 169, 68, 76, 49, 169, 137, 141, 72, 63, 166, 88, 34, 220, 163, 91, 167,
            251, 29, 160, 254, 199, 205, 74, 158, 105, 252, 182, 158, 21,
        ],
        [
            221, 44, 183, 72, 226, 191, 226, 165, 162, 153, 186, 190, 97, 53, 19, 115, 215, 71,
            155, 33, 79, 120, 197, 228, 216, 212, 249, 15, 179, 11, 216, 32,
        ],
    ],
    [
        [
            248, 136, 25, 30, 234, 18, 242, 209, 97, 211, 74, 228, 236, 199, 101, 200, 206, 52,
            146, 207, 72, 125, 28, 3, 60, 86, 34, 195, 250, 251, 204, 0,
        ],
        [
            204, 182, 197, 171, 247, 159, 161, 27, 18, 146, 249, 99, 198, 138, 25, 61, 119, 232,
            160, 152, 18, 149, 7, 67, 125, 231, 237, 3, 68, 190, 137, 0,
        ],
        [
            141, 245, 108, 181, 49, 171, 106, 247, 202, 169, 106, 39, 93, 40, 122, 2, 236, 255,
            198, 215, 122, 254, 242, 192, 49, 250, 243, 35, 7, 219, 21, 22,
        ],
        [
            239, 85, 174, 15, 207, 84, 128, 92, 87, 80, 129, 20, 21, 225, 233, 158, 193, 136, 141,
            114, 66, 146, 29, 193, 223, 250, 27, 56, 195, 15, 135, 17,
        ],
        [
            231, 242, 76, 43, 57, 10, 41, 166, 32, 254, 129, 47, 147, 118, 189, 200, 44, 102, 204,
            116, 96, 82, 186, 150, 106, 27, 30, 73, 237, 94, 36, 44,
        ],
        [
            240, 139, 69, 197, 199, 228, 206, 96, 255, 229, 189, 207, 65, 97, 93, 211, 161, 190,
            228, 249, 50, 82, 223, 251, 13, 173, 241, 221, 78, 243, 105, 19,
        ],
        [
            44, 224, 170, 161, 50, 93, 212, 80, 100, 243, 51, 74, 51, 165, 60, 208, 244, 18, 158,
            30, 158, 81, 111, 213, 136, 95, 125, 173, 143, 108, 106, 4,
        ],
        [
            134, 244, 131, 92, 152, 118, 30, 139, 153, 128, 62, 115, 88, 25, 58, 29, 205, 101, 47,
            208, 93, 89, 222, 17, 122, 112, 71, 56, 147, 68, 92, 22,
        ],
    ],
    [
        [
            59, 157, 112, 130, 217, 2, 102, 228, 79, 211, 152, 82, 183, 186, 47, 151, 125, 13, 97,
            121, 115, 253, 17, 121, 227, 250, 99, 14, 84, 249, 18, 30,
        ],
        [
            80, 180, 155, 59, 6, 182, 136, 39, 134, 168, 238, 138, 129, 174, 195, 206, 210, 167,
            214, 167, 35, 139, 130, 27, 21, 59, 7, 200, 165, 37, 91, 29,
        ],
        [
            220, 228, 189, 172, 68, 102, 135, 236, 7, 70, 152, 244, 120, 217, 67, 44, 43, 74, 155,
            179, 2, 148, 106, 238, 232, 186, 181, 130, 141, 114, 60, 1,
        ],
        [
            68, 132, 80, 55, 28, 52, 222, 165, 156, 6, 214, 236, 207, 37, 223, 118, 42, 55, 40,
            123, 208, 181, 240, 56, 14, 142, 58, 72, 193, 71, 120, 58,
        ],
        [
            93, 114, 68, 232, 179, 37, 202, 74, 41, 64, 245, 112, 233, 162, 231, 19, 223, 207, 232,
            213, 178, 60, 106, 26, 35, 191, 108, 19, 243, 220, 40, 41,
        ],
        [
            166, 223, 96, 196, 120, 210, 67, 47, 249, 123, 164, 213, 148, 138, 7, 155, 96, 222,
            176, 166, 88, 85, 95, 71, 221, 237, 138, 181, 198, 165, 163, 0,
        ],
        [
            241, 254, 24, 83, 47, 65, 146, 151, 5, 182, 233, 205, 182, 13, 75, 173, 10, 14, 48,
            223, 227, 201, 141, 212, 114, 205, 196, 92, 137, 253, 127, 60,
        ],
        [
            20, 41, 204, 77, 168, 230, 68, 202, 73, 251, 254, 88, 95, 80, 130, 216, 122, 75, 173,
            105, 236, 192, 177, 209, 26, 66, 205, 127, 154, 188, 245, 17,
        ],
    ],
    [
        [
            239, 194, 214, 218, 225, 244, 0, 110, 12, 75, 130, 236, 76, 102, 205, 64, 104, 144,
            198, 188, 183, 46, 119, 96, 230, 68, 210, 161, 253, 91, 8, 20,
        ],
        [
            84, 32, 226, 77, 213, 16, 207, 156, 234, 224, 147, 173, 186, 249, 186, 155, 90, 255,
            34, 55, 48, 108, 76, 214, 254, 66, 95, 200, 174, 191, 52, 43,
        ],
        [
            37, 103, 206, 174, 250, 172, 136, 87, 30, 68, 89, 230, 110, 190, 148, 71, 5, 249, 217,
            112, 54, 182, 127, 54, 173, 89, 6, 63, 230, 69, 32, 35,
        ],
        [
            160, 186, 242, 212, 179, 197, 16, 239, 56, 24, 91, 241, 68, 7, 138, 200, 93, 194, 45,
            155, 210, 60, 30, 4, 167, 246, 82, 244, 71, 217, 31, 20,
        ],
        [
            182, 132, 62, 134, 4, 186, 95, 160, 230, 255, 125, 156, 5, 134, 66, 99, 83, 182, 156,
            207, 98, 84, 197, 48, 160, 47, 126, 2, 253, 64, 69, 25,
        ],
        [
            135, 241, 60, 121, 32, 218, 195, 61, 68, 66, 190, 195, 208, 2, 201, 111, 158, 101, 108,
            228, 145, 141, 82, 80, 36, 16, 157, 212, 65, 213, 188, 61,
        ],
        [
            190, 186, 202, 30, 121, 177, 200, 82, 245, 162, 14, 253, 114, 50, 43, 134, 246, 12,
            100, 222, 149, 242, 117, 174, 136, 192, 117, 132, 228, 144, 238, 39,
        ],
        [
            160, 120, 19, 13, 34, 38, 71, 236, 116, 162, 150, 254, 247, 252, 222, 198, 196, 59, 98,
            165, 54, 33, 22, 120, 58, 73, 225, 42, 37, 211, 88, 21,
        ],
    ],
    [
        [
            252, 1, 229, 131, 50, 189, 111, 31, 191, 210, 177, 219, 234, 21, 100, 182, 115, 212,
            154, 111, 130, 59, 237, 32, 142, 202, 110, 96, 166, 120, 188, 1,
        ],
        [
            247, 244, 137, 120, 38, 62, 94, 38, 17, 38, 102, 240, 225, 129, 15, 214, 213, 142, 79,
            176, 156, 118, 85, 80, 167, 47, 122, 152, 206, 19, 67, 40,
        ],
        [
            27, 159, 102, 201, 17, 4, 75, 28, 159, 5, 194, 6, 63, 104, 157, 219, 53, 38, 84, 216,
            73, 181, 11, 118, 29, 177, 147, 135, 150, 5, 58, 10,
        ],
        [
            97, 168, 102, 245, 40, 187, 155, 99, 147, 65, 114, 119, 191, 225, 196, 34, 117, 134,
            116, 162, 73, 69, 158, 103, 144, 16, 22, 216, 146, 38, 10, 41,
        ],
        [
            149, 231, 10, 10, 17, 16, 88, 231, 24, 215, 115, 237, 123, 68, 9, 209, 24, 141, 150,
            207, 109, 56, 107, 192, 252, 112, 156, 0, 65, 234, 86, 10,
        ],
        [
            201, 24, 6, 113, 122, 123, 58, 3, 233, 141, 78, 228, 137, 112, 71, 121, 200, 171, 158,
            233, 87, 171, 121, 118, 205, 98, 38, 24, 176, 153, 170, 25,
        ],
        [
            6, 114, 137, 241, 204, 203, 173, 160, 14, 124, 220, 164, 166, 224, 0, 253, 255, 68, 40,
            182, 248, 135, 226, 25, 213, 247, 45, 116, 94, 147, 107, 3,
        ],
        [
            73, 103, 138, 222, 168, 203, 85, 216, 242, 63, 127, 158, 153, 60, 168, 180, 234, 71,
            27, 10, 38, 161, 207, 26, 81, 150, 195, 37, 91, 228, 57, 46,
        ],
    ],
    [
        [
            188, 220, 107, 162, 250, 116, 137, 134, 75, 73, 102, 28, 11, 158, 166, 162, 77, 99,
            159, 21, 166, 195, 208, 99, 28, 0, 51, 64, 126, 222, 203, 28,
        ],
        [
            115, 93, 10, 209, 3, 81, 82, 191, 158, 74, 26, 242, 145, 24, 85, 106, 28, 36, 54, 17,
            216, 109, 58, 102, 221, 11, 10, 157, 226, 90, 53, 3,
        ],
        [
            197, 172, 174, 245, 150, 142, 92, 221, 45, 118, 174, 8, 83, 195, 45, 83, 221, 212, 122,
            239, 218, 103, 89, 56, 184, 102, 73, 70, 1, 40, 246, 54,
        ],
        [
            131, 77, 239, 236, 59, 58, 35, 163, 25, 57, 251, 93, 224, 202, 225, 84, 189, 195, 1,
            234, 156, 138, 3, 2, 102, 170, 173, 235, 97, 41, 224, 0,
        ],
        [
            251, 165, 141, 221, 2, 154, 174, 224, 120, 187, 163, 188, 37, 146, 49, 193, 150, 241,
            183, 33, 12, 228, 96, 92, 105, 198, 238, 59, 247, 172, 247, 54,
        ],
        [
            31, 84, 10, 130, 68, 107, 203, 153, 201, 34, 69, 151, 1, 180, 37, 198, 113, 64, 82,
            116, 116, 142, 251, 62, 22, 122, 138, 130, 200, 159, 145, 2,
        ],
        [
            229, 126, 102, 192, 242, 5, 109, 247, 248, 70, 34, 78, 35, 23, 81, 67, 34, 226, 133,
            119, 200, 242, 142, 111, 223, 102, 159, 61, 162, 226, 222, 11,
        ],
        [
            171, 0, 253, 102, 188, 223, 208, 250, 186, 183, 127, 172, 10, 41, 201, 173, 242, 156,
            106, 219, 236, 139, 76, 115, 200, 123, 176, 228, 181, 248, 121, 38,
        ],
    ],
    [
        [
            187, 71, 125, 130, 250, 45, 125, 44, 56, 31, 103, 55, 71, 87, 166, 228, 184, 12, 252,
            79, 26, 221, 65, 188, 62, 254, 222, 87, 189, 71, 43, 0,
        ],
        [
            248, 127, 55, 175, 11, 237, 134, 201, 211, 212, 93, 115, 63, 118, 15, 121, 71, 55, 176,
            74, 3, 75, 20, 100, 177, 194, 39, 92, 67, 109, 243, 38,
        ],
        [
            147, 188, 248, 11, 127, 3, 176, 153, 109, 5, 65, 101, 2, 46, 70, 203, 246, 245, 254,
            67, 193, 214, 156, 21, 116, 165, 60, 79, 219, 45, 180, 47,
        ],
        [
            78, 126, 47, 15, 17, 83, 240, 144, 40, 174, 95, 250, 144, 43, 132, 67, 241, 189, 140,
            244, 41, 221, 164, 186, 104, 156, 223, 233, 160, 99, 190, 39,
        ],
        [
            29, 119, 16, 42, 190, 69, 200, 191, 3, 160, 164, 28, 189, 135, 85, 63, 59, 121, 213,
            143, 9, 96, 150, 14, 21, 93, 132, 57, 4, 165, 174, 12,
        ],
        [
            54, 200, 34, 46, 89, 210, 152, 121, 245, 147, 150, 48, 193, 246, 108, 154, 243, 12, 10,
            10, 97, 83, 225, 116, 187, 177, 176, 80, 248, 185, 5, 38,
        ],
        [
            245, 84, 103, 49, 77, 27, 84, 143, 30, 40, 54, 249, 178, 71, 191, 135, 199, 72, 204,
            162, 75, 110, 203, 246, 193, 61, 70, 158, 74, 154, 13, 45,
        ],
        [
            123, 98, 28, 217, 129, 160, 71, 205, 19, 41, 168, 124, 76, 145, 108, 71, 57, 60, 26,
            154, 163, 64, 250, 13, 52, 179, 197, 193, 54, 184, 29, 32,
        ],
    ],
    [
        [
            103, 140, 102, 88, 162, 193, 224, 59, 243, 31, 145, 100, 116, 71, 36, 129, 94, 248, 33,
            0, 102, 46, 146, 206, 22, 255, 216, 58, 61, 118, 226, 47,
        ],
        [
            21, 127, 228, 231, 155, 190, 28, 145, 48, 160, 35, 104, 47, 120, 243, 107, 145, 118,
            199, 126, 138, 164, 246, 143, 153, 59, 153, 209, 81, 118, 167, 9,
        ],
        [
            4, 84, 44, 30, 90, 253, 226, 166, 218, 12, 39, 214, 231, 241, 223, 87, 87, 82, 93, 220,
            65, 132, 166, 75, 221, 33, 236, 113, 198, 43, 210, 39,
        ],
        [
            243, 54, 41, 143, 244, 171, 75, 158, 218, 230, 55, 35, 236, 18, 40, 55, 157, 139, 180,
            29, 58, 159, 88, 208, 214, 87, 168, 227, 93, 211, 194, 17,
        ],
        [
            97, 131, 219, 190, 19, 178, 244, 173, 141, 143, 113, 3, 27, 63, 35, 185, 170, 43, 75,
            64, 75, 38, 5, 13, 123, 39, 147, 243, 141, 122, 217, 39,
        ],
        [
            3, 24, 126, 200, 122, 92, 125, 221, 95, 205, 139, 145, 231, 77, 223, 96, 84, 39, 33,
            66, 139, 41, 82, 182, 22, 102, 95, 173, 66, 125, 77, 21,
        ],
        [
            27, 50, 52, 183, 190, 198, 236, 248, 71, 251, 120, 132, 192, 227, 113, 36, 155, 81,
            225, 48, 72, 17, 246, 99, 208, 242, 236, 93, 2, 19, 53, 31,
        ],
        [
            99, 18, 31, 165, 229, 52, 216, 52, 162, 62, 66, 1, 190, 22, 69, 133, 11, 126, 106, 165,
            131, 180, 218, 253, 238, 124, 3, 16, 42, 196, 148, 57,
        ],
    ],
    [
        [
            0, 209, 105, 72, 69, 130, 81, 154, 136, 174, 169, 182, 42, 150, 112, 115, 234, 136, 47,
            170, 158, 213, 211, 65, 178, 62, 18, 172, 135, 59, 253, 19,
        ],
        [
            145, 192, 219, 168, 214, 190, 54, 248, 68, 248, 196, 148, 4, 254, 61, 193, 67, 218,
            131, 110, 235, 60, 159, 101, 200, 218, 208, 195, 30, 249, 163, 32,
        ],
        [
            21, 246, 3, 74, 137, 246, 202, 207, 71, 59, 198, 73, 117, 224, 124, 57, 2, 82, 110, 6,
            190, 80, 143, 143, 113, 62, 127, 122, 164, 202, 6, 54,
        ],
        [
            252, 245, 11, 63, 63, 70, 60, 82, 15, 154, 188, 35, 211, 222, 252, 180, 109, 109, 98,
            69, 197, 240, 137, 46, 189, 8, 167, 87, 15, 179, 18, 12,
        ],
        [
            125, 206, 204, 128, 43, 62, 39, 36, 246, 164, 44, 6, 250, 83, 14, 207, 53, 201, 166,
            231, 175, 110, 140, 200, 48, 239, 20, 171, 46, 80, 115, 54,
        ],
        [
            167, 7, 74, 225, 61, 229, 21, 154, 196, 11, 247, 27, 158, 112, 217, 238, 57, 53, 63,
            251, 162, 91, 168, 86, 37, 203, 207, 119, 68, 135, 205, 9,
        ],
        [
            84, 187, 71, 200, 46, 254, 136, 13, 25, 137, 121, 128, 232, 221, 40, 0, 175, 232, 153,
            227, 181, 162, 29, 67, 225, 234, 249, 102, 82, 171, 226, 1,
        ],
        [
            24, 185, 170, 6, 35, 57, 108, 85, 245, 134, 216, 239, 33, 12, 223, 38, 227, 73, 145,
            100, 25, 14, 244, 177, 84, 38, 101, 67, 21, 96, 249, 61,
        ],
    ],
    [
        [
            57, 9, 82, 174, 160, 195, 27, 106, 241, 225, 207, 16, 11, 131, 29, 63, 187, 187, 5, 76,
            34, 39, 136, 124, 56, 25, 58, 99, 70, 116, 170, 19,
        ],
        [
            143, 6, 32, 114, 74, 44, 29, 53, 226, 34, 62, 232, 111, 63, 201, 203, 46, 115, 209,
            118, 31, 27, 1, 120, 254, 70, 252, 80, 5, 111, 123, 55,
        ],
        [
            62, 18, 214, 41, 0, 12, 4, 12, 145, 201, 12, 6, 179, 4, 20, 84, 36, 155, 8, 99, 181,
            18, 150, 144, 203, 228, 172, 135, 166, 152, 214, 8,
        ],
        [
            49, 93, 249, 139, 121, 113, 205, 158, 145, 118, 40, 96, 206, 154, 71, 190, 146, 65,
            233, 104, 83, 91, 25, 118, 176, 14, 149, 115, 137, 27, 223, 41,
        ],
        [
            116, 160, 29, 244, 254, 193, 228, 122, 194, 168, 126, 1, 222, 247, 90, 191, 253, 101,
            123, 197, 178, 127, 30, 113, 38, 73, 48, 240, 82, 52, 161, 12,
        ],
        [
            156, 145, 203, 40, 113, 83, 199, 161, 230, 196, 203, 227, 217, 212, 254, 139, 37, 215,
            39, 230, 190, 141, 119, 120, 87, 23, 61, 21, 3, 209, 179, 47,
        ],
        [
            179, 114, 238, 159, 43, 22, 64, 61, 207, 56, 101, 90, 62, 245, 27, 21, 165, 0, 205, 34,
            104, 32, 170, 75, 215, 255, 83, 74, 123, 73, 159, 19,
        ],
        [
            14, 208, 162, 223, 209, 5, 175, 15, 1, 78, 222, 82, 21, 113, 25, 129, 103, 64, 139, 21,
            226, 245, 199, 114, 252, 69, 133, 254, 128, 63, 61, 13,
        ],
    ],
    [
        [
            255, 187, 20, 3, 51, 61, 230, 80, 83, 233, 71, 190, 94, 131, 225, 143, 139, 246, 196,
            161, 165, 85, 92, 167, 71, 198, 83, 10, 164, 120, 89, 26,
        ],
        [
            250, 108, 167, 151, 249, 92, 38, 36, 21, 96, 210, 31, 41, 91, 113, 183, 104, 192, 3,
            45, 165, 253, 37, 75, 239, 245, 28, 148, 5, 255, 134, 60,
        ],
        [
            59, 154, 220, 255, 37, 98, 169, 60, 50, 196, 202, 240, 225, 57, 165, 129, 255, 66, 169,
            162, 7, 30, 198, 27, 160, 208, 193, 106, 29, 119, 104, 48,
        ],
        [
            137, 180, 21, 151, 27, 173, 213, 11, 238, 163, 104, 192, 171, 59, 79, 249, 123, 55,
            183, 8, 94, 117, 32, 48, 41, 141, 231, 207, 61, 135, 104, 2,
        ],
        [
            242, 254, 15, 0, 58, 49, 204, 28, 27, 56, 2, 67, 248, 104, 160, 32, 214, 242, 10, 206,
            233, 61, 23, 103, 180, 53, 179, 198, 56, 254, 65, 6,
        ],
        [
            136, 214, 253, 248, 156, 140, 42, 172, 221, 187, 160, 233, 86, 213, 239, 5, 110, 252,
            70, 18, 193, 29, 156, 156, 136, 70, 167, 59, 98, 223, 7, 30,
        ],
        [
            84, 25, 227, 152, 61, 51, 53, 59, 135, 229, 159, 248, 6, 39, 151, 139, 121, 149, 226,
            142, 126, 136, 248, 196, 93, 176, 131, 254, 221, 204, 179, 36,
        ],
        [
            198, 74, 99, 58, 59, 34, 82, 94, 95, 64, 17, 241, 173, 114, 211, 57, 124, 181, 140,
            102, 105, 79, 13, 1, 60, 121, 143, 88, 192, 253, 159, 47,
        ],
    ],
    [
        [
            90, 115, 165, 218, 163, 197, 210, 143, 213, 125, 1, 77, 74, 165, 200, 244, 80, 39, 20,
            247, 86, 120, 109, 109, 93, 7, 209, 199, 109, 12, 144, 46,
        ],
        [
            231, 44, 48, 128, 109, 202, 114, 192, 218, 67, 233, 141, 64, 251, 104, 41, 58, 212, 60,
            65, 93, 58, 34, 149, 128, 90, 30, 197, 191, 244, 8, 37,
        ],
        [
            56, 47, 18, 80, 195, 143, 175, 35, 183, 225, 201, 236, 138, 29, 26, 229, 194, 202, 13,
            43, 71, 188, 3, 204, 12, 15, 218, 207, 15, 83, 219, 39,
        ],
        [
            50, 71, 182, 171, 33, 129, 211, 168, 40, 85, 193, 218, 165, 54, 220, 203, 164, 124, 8,
            37, 19, 210, 8, 253, 120, 158, 239, 239, 28, 195, 253, 37,
        ],
        [
            245, 191, 155, 103, 118, 221, 209, 204, 89, 48, 249, 160, 180, 1, 114, 3, 254, 220, 94,
            244, 221, 122, 224, 55, 184, 106, 99, 11, 236, 89, 211, 38,
        ],
        [
            182, 208, 168, 152, 15, 192, 45, 31, 93, 181, 13, 203, 128, 82, 126, 145, 129, 220, 19,
            252, 188, 247, 49, 216, 218, 198, 178, 70, 180, 209, 175, 22,
        ],
        [
            72, 71, 200, 22, 21, 120, 50, 111, 112, 195, 141, 79, 49, 52, 98, 8, 37, 130, 142, 13,
            78, 197, 15, 92, 203, 50, 108, 82, 109, 254, 158, 12,
        ],
        [
            71, 44, 114, 76, 152, 26, 79, 25, 44, 244, 191, 178, 150, 102, 34, 230, 54, 251, 209,
            155, 90, 28, 81, 49, 127, 246, 116, 238, 106, 105, 196, 29,
        ],
    ],
    [
        [
            208, 87, 78, 186, 184, 128, 38, 190, 131, 156, 221, 119, 87, 12, 144, 4, 240, 77, 118,
            209, 74, 131, 37, 155, 247, 155, 206, 167, 80, 71, 127, 18,
        ],
        [
            190, 54, 229, 228, 15, 167, 185, 240, 161, 238, 216, 88, 210, 31, 242, 20, 81, 147, 48,
            54, 38, 226, 251, 64, 69, 196, 67, 166, 242, 34, 118, 39,
        ],
        [
            58, 171, 187, 174, 13, 247, 253, 15, 102, 171, 48, 63, 136, 157, 55, 28, 117, 130, 104,
            23, 145, 203, 155, 105, 121, 249, 115, 106, 88, 114, 86, 11,
        ],
        [
            105, 30, 100, 75, 20, 206, 29, 147, 150, 37, 48, 216, 33, 147, 61, 193, 82, 230, 205,
            122, 142, 65, 148, 102, 47, 185, 182, 147, 185, 31, 29, 54,
        ],
        [
            158, 245, 169, 236, 26, 185, 17, 174, 156, 69, 81, 196, 60, 109, 99, 91, 19, 208, 93,
            58, 9, 109, 228, 186, 109, 127, 171, 156, 229, 215, 195, 59,
        ],
        [
            23, 42, 4, 183, 91, 177, 2, 172, 168, 182, 158, 185, 157, 118, 199, 184, 237, 203, 60,
            170, 35, 121, 162, 7, 130, 171, 121, 207, 32, 2, 227, 62,
        ],
        [
            0, 139, 174, 217, 13, 116, 28, 230, 238, 117, 190, 91, 86, 105, 38, 231, 147, 100, 233,
            187, 70, 128, 111, 82, 184, 113, 154, 136, 59, 27, 21, 10,
        ],
        [
            4, 208, 53, 136, 59, 196, 102, 52, 69, 1, 231, 8, 254, 19, 67, 134, 251, 73, 157, 156,
            30, 94, 170, 147, 185, 72, 11, 143, 226, 255, 0, 60,
        ],
    ],
    [
        [
            214, 131, 68, 196, 131, 169, 22, 250, 29, 101, 142, 26, 106, 96, 18, 190, 18, 15, 19,
            59, 203, 203, 119, 251, 61, 221, 198, 116, 24, 178, 61, 42,
        ],
        [
            101, 161, 133, 103, 0, 112, 204, 255, 98, 240, 20, 161, 242, 253, 216, 204, 83, 96, 93,
            228, 77, 76, 63, 70, 116, 156, 69, 253, 121, 189, 2, 36,
        ],
        [
            156, 83, 226, 206, 4, 35, 12, 137, 209, 181, 109, 81, 194, 119, 188, 216, 30, 233, 135,
            220, 213, 40, 74, 152, 49, 14, 0, 3, 223, 41, 238, 54,
        ],
        [
            47, 25, 110, 4, 111, 57, 200, 91, 168, 73, 47, 175, 189, 60, 49, 243, 128, 11, 63, 17,
            151, 123, 80, 140, 139, 202, 93, 104, 190, 32, 67, 54,
        ],
        [
            90, 100, 132, 107, 167, 162, 164, 62, 239, 68, 20, 223, 157, 1, 90, 95, 248, 82, 65,
            61, 241, 63, 238, 10, 2, 160, 230, 104, 101, 197, 60, 52,
        ],
        [
            41, 144, 80, 156, 134, 224, 6, 48, 188, 57, 30, 205, 84, 135, 190, 75, 213, 94, 16, 72,
            11, 96, 41, 117, 75, 60, 62, 133, 29, 133, 105, 15,
        ],
        [
            219, 66, 247, 117, 3, 137, 38, 43, 131, 177, 137, 150, 9, 65, 160, 206, 235, 121, 121,
            245, 205, 233, 229, 78, 72, 200, 171, 149, 240, 64, 184, 5,
        ],
        [
            22, 179, 118, 116, 100, 222, 159, 96, 236, 247, 38, 23, 224, 103, 6, 5, 42, 95, 161, 4,
            128, 2, 240, 122, 117, 247, 127, 207, 76, 205, 137, 31,
        ],
    ],
    [
        [
            255, 68, 73, 184, 204, 219, 231, 9, 237, 101, 142, 55, 146, 252, 138, 14, 186, 62, 32,
            108, 79, 130, 251, 188, 101, 134, 179, 162, 172, 160, 149, 22,
        ],
        [
            93, 226, 69, 177, 229, 17, 78, 185, 6, 206, 195, 246, 145, 189, 141, 7, 197, 148, 166,
            43, 203, 235, 170, 119, 102, 76, 108, 98, 216, 237, 121, 34,
        ],
        [
            211, 167, 46, 90, 228, 111, 217, 129, 255, 3, 113, 207, 200, 221, 28, 48, 33, 62, 31,
            245, 116, 175, 130, 128, 180, 252, 132, 178, 56, 58, 16, 2,
        ],
        [
            159, 176, 149, 39, 220, 58, 146, 80, 175, 91, 125, 15, 166, 114, 133, 117, 52, 243,
            219, 221, 223, 114, 140, 236, 106, 39, 65, 168, 43, 244, 140, 57,
        ],
        [
            144, 68, 49, 189, 208, 94, 145, 108, 143, 62, 16, 188, 15, 110, 23, 239, 71, 48, 32,
            238, 96, 19, 43, 91, 231, 90, 77, 162, 159, 162, 71, 15,
        ],
        [
            103, 8, 114, 153, 156, 97, 188, 167, 128, 217, 58, 42, 208, 82, 234, 142, 53, 71, 10,
            38, 177, 2, 13, 35, 8, 49, 196, 134, 215, 255, 42, 54,
        ],
        [
            229, 29, 149, 199, 252, 232, 6, 148, 31, 243, 79, 192, 221, 191, 136, 186, 249, 198,
            35, 155, 198, 198, 19, 183, 159, 123, 65, 127, 169, 3, 156, 59,
        ],
        [
            2, 244, 213, 144, 80, 83, 125, 211, 252, 98, 209, 105, 104, 213, 143, 183, 164, 199,
            103, 53, 110, 48, 230, 35, 34, 129, 221, 255, 225, 224, 42, 42,
        ],
    ],
    [
        [
            145, 41, 77, 21, 230, 237, 146, 98, 160, 218, 242, 227, 198, 83, 11, 39, 148, 69, 31,
            185, 143, 52, 71, 75, 157, 26, 157, 188, 179, 27, 114, 24,
        ],
        [
            160, 247, 33, 120, 242, 78, 125, 237, 149, 68, 194, 190, 248, 145, 93, 23, 171, 167,
            181, 242, 226, 41, 104, 67, 0, 116, 81, 246, 87, 82, 103, 51,
        ],
        [
            84, 35, 131, 165, 134, 206, 147, 191, 7, 3, 253, 142, 49, 128, 111, 47, 53, 169, 88,
            17, 31, 193, 20, 98, 19, 173, 111, 175, 134, 186, 166, 27,
        ],
        [
            49, 150, 139, 110, 180, 138, 202, 107, 41, 238, 123, 185, 17, 161, 67, 30, 2, 2, 39,
            91, 7, 35, 69, 121, 34, 12, 247, 78, 138, 39, 59, 8,
        ],
        [
            64, 14, 249, 58, 50, 65, 122, 135, 174, 11, 102, 220, 221, 64, 29, 66, 24, 169, 57,
            114, 140, 176, 7, 149, 78, 15, 211, 255, 101, 244, 151, 46,
        ],
        [
            127, 185, 215, 42, 158, 164, 234, 37, 140, 239, 228, 75, 189, 8, 197, 4, 206, 24, 136,
            191, 73, 206, 141, 195, 85, 123, 141, 189, 82, 250, 65, 21,
        ],
        [
            49, 239, 163, 97, 219, 143, 242, 84, 53, 166, 149, 155, 243, 11, 207, 69, 250, 25, 159,
            142, 240, 8, 72, 229, 91, 179, 218, 39, 128, 133, 201, 6,
        ],
        [
            96, 144, 236, 91, 71, 246, 217, 36, 27, 102, 209, 14, 75, 249, 185, 211, 2, 97, 216,
            204, 141, 6, 234, 251, 183, 215, 152, 151, 125, 210, 121, 14,
        ],
    ],
];

pub fn generator() -> pallas::Affine {
    pallas::Affine::from_xy(
        pallas::Base::from_repr(GENERATOR.0).unwrap(),
        pallas::Base::from_repr(GENERATOR.1).unwrap(),
    )
    .unwrap()
}

#[cfg(test)]
mod tests {
    use super::super::{NUM_WINDOWS_SHORT, VALUE_COMMITMENT_PERSONALIZATION};
    use super::*;
    use group::Curve;
    use halo2_gadgets::ecc::chip::constants::{test_lagrange_coeffs, test_zs_and_us};
    use pasta_curves::{
        arithmetic::{CurveAffine, CurveExt},
        pallas,
    };

    #[test]
    fn generator() {
        let hasher = pallas::Point::hash_to_curve(VALUE_COMMITMENT_PERSONALIZATION);
        let point = hasher(b"v");
        let coords = point.to_affine().coordinates().unwrap();

        assert_eq!(*coords.x(), pallas::Base::from_repr(GENERATOR.0).unwrap());
        assert_eq!(*coords.y(), pallas::Base::from_repr(GENERATOR.1).unwrap());
    }

    #[test]
    fn lagrange_coeffs() {
        let base = super::generator();
        test_lagrange_coeffs(base, NUM_WINDOWS_SHORT);
    }

    #[test]
    fn z() {
        let base = super::generator();
        test_zs_and_us(base, &Z_SHORT, &U_SHORT, NUM_WINDOWS_SHORT);
    }
}
