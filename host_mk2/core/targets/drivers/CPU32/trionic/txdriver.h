const uint8_t txDriver[] = {
	0x2E, 0x7C, 0x00, 0x10, 0x07, 0xFC, 0x0C, 0x00, 0x00, 0x01, 0x67, 0x16,
	0x0C, 0x00, 0x00, 0x02, 0x67, 0x3C, 0x0C, 0x00, 0x00, 0x03, 0x67, 0x5E,
	0x0C, 0x00, 0x00, 0x04, 0x67, 0x00, 0x02, 0x92, 0x60, 0x4E, 0x22, 0x7C,
	0x00, 0x10, 0x00, 0x00, 0x32, 0x3C, 0x02, 0x00, 0x0C, 0x46, 0x00, 0x01,
	0x67, 0x00, 0x01, 0xE6, 0x0C, 0x46, 0x00, 0x02, 0x67, 0x00, 0x01, 0xC0,
	0x0C, 0x46, 0x00, 0x03, 0x67, 0x00, 0x01, 0x56, 0x0C, 0x46, 0x00, 0x04,
	0x67, 0x00, 0x02, 0x98, 0x60, 0x22, 0x91, 0xC8, 0x0C, 0x46, 0x00, 0x01,
	0x67, 0x00, 0x02, 0x00, 0x0C, 0x46, 0x00, 0x02, 0x67, 0x00, 0x01, 0x76,
	0x0C, 0x46, 0x00, 0x03, 0x67, 0x00, 0x01, 0x2A, 0x0C, 0x46, 0x00, 0x04,
	0x67, 0x00, 0x03, 0x2C, 0x61, 0x4E, 0x42, 0x80, 0x4A, 0xFA, 0x70, 0x01,
	0x72, 0x40, 0x20, 0x7C, 0x00, 0xFF, 0xFC, 0x14, 0x38, 0x3C, 0x55, 0x55,
	0x24, 0x7C, 0x00, 0x00, 0xAA, 0xAA, 0x2C, 0x7C, 0x00, 0x00, 0x55, 0x54,
	0x9B, 0xCD, 0x36, 0x15, 0x34, 0x8A, 0x3C, 0x84, 0x34, 0xBC, 0x90, 0x90,
	0x61, 0x22, 0x3E, 0x1D, 0xBE, 0x43, 0x67, 0x26, 0x1E, 0x15, 0x36, 0x15,
	0x34, 0x8A, 0x3C, 0x84, 0x34, 0xBC, 0xF0, 0xF0, 0x61, 0x0E, 0x61, 0x0C,
	0x61, 0x0A, 0x61, 0x08, 0x61, 0x06, 0x7C, 0x02, 0x60, 0x00, 0x00, 0x46,
	0x34, 0x3C, 0x18, 0x00, 0x51, 0xCA, 0xFF, 0xFE, 0x4E, 0x75, 0x7C, 0x01,
	0x30, 0xC1, 0x83, 0x50, 0x61, 0xEE, 0x3B, 0x02, 0x3A, 0x82, 0x61, 0xE8,
	0x3A, 0xBC, 0x90, 0x90, 0x3E, 0x1D, 0xBE, 0x43, 0x67, 0x00, 0x00, 0x9C,
	0x1E, 0x15, 0x42, 0x65, 0x41, 0xFA, 0x02, 0xE2, 0x76, 0x03, 0x7A, 0x02,
	0x74, 0x01, 0xBE, 0x18, 0x67, 0x00, 0x00, 0x90, 0x51, 0xCA, 0xFF, 0xF8,
	0xE3, 0x4D, 0x53, 0x03, 0x66, 0xEE, 0x60, 0x7A, 0x7A, 0x08, 0x32, 0x07,
	0xEA, 0x69, 0x0C, 0x01, 0x00, 0x01, 0x67, 0x62, 0x0C, 0x01, 0x00, 0x20,
	0x67, 0x5C, 0x0C, 0x01, 0x00, 0x1C, 0x67, 0x56, 0x0C, 0x47, 0x37, 0xA4,
	0x67, 0x62, 0x0C, 0x47, 0xDA, 0xA1, 0x67, 0x5C, 0x0C, 0x47, 0x9D, 0x1C,
	0x67, 0x56, 0x0C, 0x47, 0x9D, 0x4D, 0x67, 0x52, 0x0C, 0x47, 0xBF, 0xB4,
	0x67, 0x48, 0x0C, 0x47, 0xBF, 0xB5, 0x67, 0x44, 0x0C, 0x47, 0xBF, 0xB6,
	0x67, 0x40, 0x0C, 0x47, 0x00, 0x22, 0x67, 0x16, 0x7C, 0x03, 0x0C, 0x47,
	0x1F, 0x5D, 0x67, 0x2E, 0x0C, 0x47, 0x1F, 0xD5, 0x67, 0x2A, 0x0C, 0x47,
	0x1F, 0xDA, 0x67, 0x26, 0x60, 0x1C, 0x7A, 0x10, 0x0C, 0x43, 0x22, 0x23,
	0x67, 0x1A, 0x0C, 0x43, 0x22, 0x81, 0x67, 0x16, 0x60, 0x0C, 0x0C, 0x07,
	0x00, 0x21, 0x67, 0x0A, 0x0C, 0x07, 0x00, 0x20, 0x67, 0x06, 0x42, 0x86,
	0x42, 0x80, 0xE2, 0x4D, 0xE2, 0x4D, 0x48, 0x45, 0x22, 0x45, 0x42, 0x85,
	0x53, 0x85, 0x4A, 0xFA, 0x61, 0x00, 0xFF, 0x2E, 0x70, 0x01, 0x4A, 0xFA,
	0x76, 0x7F, 0x26, 0x48, 0x28, 0x49, 0x30, 0x03, 0xB7, 0x4C, 0x66, 0x10,
	0x51, 0xC8, 0xFF, 0xFA, 0x20, 0x4B, 0x22, 0x4C, 0x92, 0x43, 0x53, 0x41,
	0x67, 0x1E, 0x60, 0xE6, 0x26, 0x48, 0x28, 0x49, 0x30, 0x03, 0x34, 0x8A,
	0x3C, 0x84, 0x34, 0xBC, 0xA0, 0xA0, 0x36, 0xDC, 0x51, 0xC8, 0xFF, 0xFC,
	0x30, 0x10, 0xB0, 0x50, 0x66, 0xFA, 0x60, 0xCA, 0x70, 0x01, 0x4A, 0xFA,
	0xBA, 0x50, 0x67, 0x18, 0x34, 0x8A, 0x3C, 0x84, 0x34, 0xBC, 0x80, 0x80,
	0x34, 0x8A, 0x3C, 0x84, 0x34, 0xBC, 0x10, 0x10, 0x34, 0x10, 0xB4, 0x50,
	0x66, 0xFA, 0x60, 0xE4, 0x54, 0x88, 0xB1, 0xC9, 0x65, 0xDE, 0x70, 0x01,
	0x4A, 0xFA, 0xB3, 0x48, 0x67, 0x12, 0x34, 0x8A, 0x3C, 0x84, 0x34, 0xBC,
	0xA0, 0xA0, 0x31, 0x21, 0x30, 0x10, 0xB0, 0x50, 0x66, 0xFA, 0x60, 0xEA,
	0x53, 0x41, 0x66, 0xE6, 0x70, 0x01, 0x4A, 0xFA, 0x70, 0x19, 0xB3, 0x48,
	0x67, 0x1E, 0x31, 0x3C, 0x40, 0x40, 0x30, 0xA1, 0x61, 0x20, 0x30, 0xBC,
	0xC0, 0xC0, 0x61, 0x22, 0xB3, 0x48, 0x66, 0x06, 0x42, 0x60, 0x4A, 0x61,
	0x60, 0xE4, 0x53, 0x00, 0x66, 0xE4, 0x4A, 0xFA, 0x53, 0x41, 0x66, 0xD8,
	0x70, 0x01, 0x42, 0x50, 0x4A, 0xFA, 0x76, 0x0F, 0x53, 0x83, 0x66, 0xFC,
	0x4E, 0x75, 0x76, 0x08, 0x53, 0x83, 0x66, 0xFC, 0x4E, 0x75, 0x24, 0x48,
	0x70, 0x19, 0x4A, 0x5A, 0x67, 0x1C, 0x35, 0x3C, 0x40, 0x40, 0x42, 0x52,
	0x61, 0xE0, 0x34, 0xBC, 0xC0, 0xC0, 0x61, 0xE2, 0x4A, 0x5A, 0x66, 0x04,
	0x42, 0x62, 0x60, 0xE6, 0x53, 0x00, 0x67, 0x36, 0x60, 0xE4, 0xB5, 0xC9,
	0x65, 0xDA, 0x30, 0x3C, 0x03, 0xE8, 0x32, 0x3C, 0x20, 0x20, 0xBA, 0x58,
	0x67, 0x1E, 0x31, 0x01, 0x30, 0x81, 0x36, 0x3C, 0x42, 0x40, 0x51, 0xCB,
	0xFF, 0xFE, 0x30, 0xBC, 0xA0, 0xA0, 0x61, 0xB2, 0xBA, 0x50, 0x66, 0x04,
	0x42, 0x50, 0x60, 0xE2, 0x53, 0x40, 0x67, 0x06, 0xB1, 0xC9, 0x65, 0xDA,
	0x70, 0x01, 0x4A, 0xFA, 0x33, 0xFC, 0xD0, 0x84, 0x00, 0xFF, 0xFA, 0x04,
	0x28, 0x7C, 0x00, 0xFF, 0xF8, 0x00, 0x38, 0xBC, 0x98, 0x00, 0x2A, 0x7C,
	0x00, 0xFF, 0xF8, 0x08, 0x42, 0x9D, 0x38, 0xBC, 0x18, 0x00, 0x2C, 0x7C,
	0x00, 0xFF, 0xF8, 0x0E, 0x42, 0x85, 0x53, 0x85, 0x7C, 0x04, 0x70, 0x01,
	0x61, 0x00, 0x00, 0x84, 0x4A, 0xFA, 0x61, 0x7E, 0xB1, 0xFC, 0x00, 0x04,
	0x00, 0x00, 0x6D, 0x08, 0x61, 0x7C, 0x32, 0x3C, 0x00, 0x80, 0x91, 0xC8,
	0x26, 0x08, 0x34, 0x3C, 0x01, 0x00, 0xE0, 0x8B, 0xEE, 0x8B, 0xE7, 0x6A,
	0x14, 0x3C, 0x00, 0x32, 0x24, 0x48, 0x26, 0x49, 0x76, 0x40, 0x61, 0x44,
	0xB7, 0x4A, 0x66, 0x12, 0x55, 0x83, 0x66, 0xF8, 0x20, 0x4A, 0x22, 0x4B,
	0x04, 0x81, 0x00, 0x00, 0x00, 0x20, 0x66, 0xE8, 0x4A, 0xFA, 0x3C, 0x82,
	0x24, 0x48, 0x26, 0x49, 0x76, 0x40, 0x34, 0xDB, 0x55, 0x43, 0x66, 0xFA,
	0x00, 0x56, 0x00, 0x01, 0x61, 0x1A, 0x4A, 0x55, 0x6B, 0xFA, 0x02, 0x56,
	0xFF, 0xFE, 0x24, 0x48, 0x76, 0x40, 0x4A, 0x1A, 0x66, 0xEA, 0x53, 0x03,
	0x66, 0xF8, 0x02, 0x56, 0xFF, 0xFD, 0x60, 0xB4, 0x13, 0xFC, 0x00, 0x55,
	0x00, 0xFF, 0xFA, 0x27, 0x13, 0xFC, 0x00, 0xAA, 0x00, 0xFF, 0xFA, 0x27,
	0x4E, 0x75, 0x61, 0xEC, 0x02, 0x54, 0xDF, 0xFF, 0x4E, 0x75, 0x61, 0xE4,
	0x00, 0x54, 0x20, 0x00, 0x4E, 0x75, 0x20, 0x7C, 0x00, 0x04, 0x00, 0x00,
	0x50, 0xC1, 0x93, 0xC9, 0x61, 0xD2, 0xBA, 0x59, 0x66, 0x14, 0xB3, 0xC8,
	0x6D, 0xF6, 0x61, 0xE2, 0x20, 0x7C, 0x00, 0x00, 0x01, 0x00, 0xB2, 0xC8,
	0x67, 0x02, 0x60, 0xE6, 0x42, 0x81, 0x61, 0xCA, 0x4E, 0x75, 0x3A, 0xBC,
	0x22, 0x3C, 0x61, 0xC2, 0x3C, 0xBC, 0xFF, 0x36, 0x30, 0x85, 0x00, 0x56,
	0x00, 0x01, 0x61, 0xA4, 0x4A, 0x55, 0x6B, 0xFA, 0x02, 0x56, 0xFF, 0xFE,
	0x61, 0xBC, 0x4A, 0x01, 0x66, 0xEC, 0x02, 0x56, 0xFF, 0xFD, 0x02, 0x56,
	0xFF, 0xF9, 0x61, 0xAE, 0x4A, 0x01, 0x66, 0xD8, 0x70, 0x01, 0x4A, 0xFA,
	0x25, 0xB8, 0xA7, 0xB4, 0x2A, 0xBD
};
