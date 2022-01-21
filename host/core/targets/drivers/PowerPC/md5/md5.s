# r3: Pointer
# r4: Number of bytes

# Return
# r6: A
# r7: B
# r8: C
# r9: D

# Expect it to thrash every other register known to man so make sure to backup what you hold dear...
# Length MUST be in multiples of 64 and it will _NOT_ finalize the hash (ie append length)
# Reason for this is that I try not to use the stack. Do it on the host, all you need is the hash of the flash and total length

    # Setting up A, B, C and D..
    lis     r6,            0x6745
    lis     r7,            0xefcd
    lis     r8,            0x98ba
    lis     r9,            0x1032
    ori     r6,      r6,   0x2301
    ori     r7,      r7,   0xab89
    ori     r8,      r8,   0xdcfe
    ori     r9,      r9,   0x5476
    li     r11,                 0

LoopOver:

    # Backing up A, B, C and D
    mr     r12,                r6
    mr     r13,                r7
    mr     r14,                r8
    mr     r15,                r9


    # # # # # STEP  0 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    55147

    # F  = D ^ (B & (C ^ D))
    andc   r10,     r15,     r13
    and    r16,     r14,     r13
    or     r10,     r10,     r16

    # A = A + data[00] + K + F
    addi   r12,     r12,   -23432
    li     r11,                  4
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  7) + B
    rotlwi r12,     r12,        7
    add    r12,     r13,     r12


    # # # # # STEP  1 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    59592

    # F  = C ^ (A & (B ^ C))
    andc   r10,     r14,     r12
    and    r16,     r13,     r12
    or     r10,     r10,     r16

    # D = D + data[04] + K + F
    addi   r15,     r15,   -18602
    li     r11,                  8
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 12) + A
    rotlwi r15,     r15,       12
    add    r15,     r12,     r15


    # # # # # STEP  2 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,     9248

    # F  = B ^ (D & (A ^ B))
    andc   r10,     r13,     r15
    and    r16,     r12,     r15
    or     r10,     r10,     r16

    # C = C + data[08] + K + F
    addi   r14,     r14,    28891
    li     r11,                 12
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 17) + D
    rotlwi r14,     r14,       17
    add    r14,     r15,     r14


    # # # # # STEP  3 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    49598

    # F  = A ^ (C & (D ^ A))
    andc   r10,     r12,     r14
    and    r16,     r15,     r14
    or     r10,     r10,     r16

    # B = B + data[12] + K + F
    addi   r13,     r13,   -12562
    li     r11,                 16
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 22) + C
    rotlwi r13,     r13,       22
    add    r13,     r14,     r13


    # # # # # STEP  4 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    62844

    # F  = D ^ (B & (C ^ D))
    andc   r10,     r15,     r13
    and    r16,     r14,     r13
    or     r10,     r10,     r16

    # A = A + data[16] + K + F
    addi   r12,     r12,     4015
    li     r11,                 20
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  7) + B
    rotlwi r12,     r12,        7
    add    r12,     r13,     r12


    # # # # # STEP  5 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    18312

    # F  = C ^ (A & (B ^ C))
    andc   r10,     r14,     r12
    and    r16,     r13,     r12
    or     r10,     r10,     r16

    # D = D + data[20] + K + F
    addi   r15,     r15,   -14806
    li     r11,                 24
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 12) + A
    rotlwi r15,     r15,       12
    add    r15,     r12,     r15


    # # # # # STEP  6 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    43056

    # F  = B ^ (D & (A ^ B))
    andc   r10,     r13,     r15
    and    r16,     r12,     r15
    or     r10,     r10,     r16

    # C = C + data[24] + K + F
    addi   r14,     r14,    17939
    li     r11,                 28
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 17) + D
    rotlwi r14,     r14,       17
    add    r14,     r15,     r14


    # # # # # STEP  7 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    64839

    # F  = A ^ (C & (D ^ A))
    andc   r10,     r12,     r14
    and    r16,     r15,     r14
    or     r10,     r10,     r16

    # B = B + data[28] + K + F
    addi   r13,     r13,   -27391
    li     r11,                 32
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 22) + C
    rotlwi r13,     r13,       22
    add    r13,     r14,     r13


    # # # # # STEP  8 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    27009

    # F  = D ^ (B & (C ^ D))
    andc   r10,     r15,     r13
    and    r16,     r14,     r13
    or     r10,     r10,     r16

    # A = A + data[32] + K + F
    addi   r12,     r12,   -26408
    li     r11,                 36
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  7) + B
    rotlwi r12,     r12,        7
    add    r12,     r13,     r12


    # # # # # STEP  9 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    35653

    # F  = C ^ (A & (B ^ C))
    andc   r10,     r14,     r12
    and    r16,     r13,     r12
    or     r10,     r10,     r16

    # D = D + data[36] + K + F
    addi   r15,     r15,    -2129
    li     r11,                 40
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 12) + A
    rotlwi r15,     r15,       12
    add    r15,     r12,     r15


    # # # # # STEP 10 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    65535

    # F  = B ^ (D & (A ^ B))
    andc   r10,     r13,     r15
    and    r16,     r12,     r15
    or     r10,     r10,     r16

    # C = C + data[40] + K + F
    addi   r14,     r14,    23473
    li     r11,                 44
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 17) + D
    rotlwi r14,     r14,       17
    add    r14,     r15,     r14


    # # # # # STEP 11 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    35165

    # F  = A ^ (C & (D ^ A))
    andc   r10,     r12,     r14
    and    r16,     r15,     r14
    or     r10,     r10,     r16

    # B = B + data[44] + K + F
    addi   r13,     r13,   -10306
    li     r11,                 48
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 22) + C
    rotlwi r13,     r13,       22
    add    r13,     r14,     r13


    # # # # # STEP 12 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    27536

    # F  = D ^ (B & (C ^ D))
    andc   r10,     r15,     r13
    and    r16,     r14,     r13
    or     r10,     r10,     r16

    # A = A + data[48] + K + F
    addi   r12,     r12,     4386
    li     r11,                 52
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  7) + B
    rotlwi r12,     r12,        7
    add    r12,     r13,     r12


    # # # # # STEP 13 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    64920

    # F  = C ^ (A & (B ^ C))
    andc   r10,     r14,     r12
    and    r16,     r13,     r12
    or     r10,     r10,     r16

    # D = D + data[52] + K + F
    addi   r15,     r15,    29075
    li     r11,                 56
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 12) + A
    rotlwi r15,     r15,       12
    add    r15,     r12,     r15


    # # # # # STEP 14 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    42617

    # F  = B ^ (D & (A ^ B))
    andc   r10,     r13,     r15
    and    r16,     r12,     r15
    or     r10,     r10,     r16

    # C = C + data[56] + K + F
    addi   r14,     r14,    17294
    li     r11,                 60
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 17) + D
    rotlwi r14,     r14,       17
    add    r14,     r15,     r14


    # # # # # STEP 15 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    18868

    # F  = A ^ (C & (D ^ A))
    andc   r10,     r12,     r14
    and    r16,     r15,     r14
    or     r10,     r10,     r16

    # B = B + data[60] + K + F
    addi   r13,     r13,     2081
    li     r11,                  4
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 22) + C
    rotlwi r13,     r13,       22
    add    r13,     r14,     r13


    # # # # # STEP 16 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    63006

    # F  = C ^ (D & (B ^ C))
    andc   r10,     r14,     r15
    and    r16,     r13,     r15
    or     r10,     r10,     r16

    # A = A + data[04] + K + F
    addi   r12,     r12,     9570
    li     r11,                 24
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  5) + B
    rotlwi r12,     r12,        5
    add    r12,     r13,     r12


    # # # # # STEP 17 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    49217

    # F  = B ^ (C & (A ^ B))
    andc   r10,     r13,     r14
    and    r16,     r12,     r14
    or     r10,     r10,     r16

    # D = D + data[24] + K + F
    addi   r15,     r15,   -19648
    li     r11,                 44
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D <<  9) + A
    rotlwi r15,     r15,        9
    add    r15,     r12,     r15


    # # # # # STEP 18 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,     9822

    # F  = A ^ (B & (D ^ A))
    andc   r10,     r12,     r13
    and    r16,     r15,     r13
    or     r10,     r10,     r16

    # C = C + data[44] + K + F
    addi   r14,     r14,    23121
    li     r11,                  0
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 14) + D
    rotlwi r14,     r14,       14
    add    r14,     r15,     r14


    # # # # # STEP 19 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    59831

    # F  = D ^ (A & (C ^ D))
    andc   r10,     r15,     r12
    and    r16,     r14,     r12
    or     r10,     r10,     r16

    # B = B + data[00] + K + F
    addi   r13,     r13,   -14422
    li     r11,                 20
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 20) + C
    rotlwi r13,     r13,       20
    add    r13,     r14,     r13


    # # # # # STEP 20 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    54831

    # F  = C ^ (D & (B ^ C))
    andc   r10,     r14,     r15
    and    r16,     r13,     r15
    or     r10,     r10,     r16

    # A = A + data[20] + K + F
    addi   r12,     r12,     4189
    li     r11,                 40
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  5) + B
    rotlwi r12,     r12,        5
    add    r12,     r13,     r12


    # # # # # STEP 21 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,      580

    # F  = B ^ (C & (A ^ B))
    andc   r10,     r13,     r14
    and    r16,     r12,     r14
    or     r10,     r10,     r16

    # D = D + data[40] + K + F
    addi   r15,     r15,     5203
    li     r11,                 60
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D <<  9) + A
    rotlwi r15,     r15,        9
    add    r15,     r12,     r15


    # # # # # STEP 22 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    55458

    # F  = A ^ (B & (D ^ A))
    andc   r10,     r12,     r13
    and    r16,     r15,     r13
    or     r10,     r10,     r16

    # C = C + data[60] + K + F
    addi   r14,     r14,    -6527
    li     r11,                 16
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 14) + D
    rotlwi r14,     r14,       14
    add    r14,     r15,     r14


    # # # # # STEP 23 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    59348

    # F  = D ^ (A & (C ^ D))
    andc   r10,     r15,     r12
    and    r16,     r14,     r12
    or     r10,     r10,     r16

    # B = B + data[16] + K + F
    addi   r13,     r13,    -1080
    li     r11,                 36
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 20) + C
    rotlwi r13,     r13,       20
    add    r13,     r14,     r13


    # # # # # STEP 24 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,     8674

    # F  = C ^ (D & (B ^ C))
    andc   r10,     r14,     r15
    and    r16,     r13,     r15
    or     r10,     r10,     r16

    # A = A + data[36] + K + F
    addi   r12,     r12,   -12826
    li     r11,                 56
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  5) + B
    rotlwi r12,     r12,        5
    add    r12,     r13,     r12


    # # # # # STEP 25 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    49975

    # F  = B ^ (C & (A ^ B))
    andc   r10,     r13,     r14
    and    r16,     r12,     r14
    or     r10,     r10,     r16

    # D = D + data[56] + K + F
    addi   r15,     r15,     2006
    li     r11,                 12
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D <<  9) + A
    rotlwi r15,     r15,        9
    add    r15,     r12,     r15


    # # # # # STEP 26 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    62677

    # F  = A ^ (B & (D ^ A))
    andc   r10,     r12,     r13
    and    r16,     r15,     r13
    or     r10,     r10,     r16

    # C = C + data[12] + K + F
    addi   r14,     r14,     3463
    li     r11,                 32
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 14) + D
    rotlwi r14,     r14,       14
    add    r14,     r15,     r14


    # # # # # STEP 27 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    17754

    # F  = D ^ (A & (C ^ D))
    andc   r10,     r15,     r12
    and    r16,     r14,     r12
    or     r10,     r10,     r16

    # B = B + data[32] + K + F
    addi   r13,     r13,     5357
    li     r11,                 52
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 20) + C
    rotlwi r13,     r13,       20
    add    r13,     r14,     r13


    # # # # # STEP 28 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    43492

    # F  = C ^ (D & (B ^ C))
    andc   r10,     r14,     r15
    and    r16,     r13,     r15
    or     r10,     r10,     r16

    # A = A + data[52] + K + F
    addi   r12,     r12,    -5883
    li     r11,                  8
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  5) + B
    rotlwi r12,     r12,        5
    add    r12,     r13,     r12


    # # # # # STEP 29 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    64752

    # F  = B ^ (C & (A ^ B))
    andc   r10,     r13,     r14
    and    r16,     r12,     r14
    or     r10,     r10,     r16

    # D = D + data[08] + K + F
    addi   r15,     r15,   -23560
    li     r11,                 28
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D <<  9) + A
    rotlwi r15,     r15,        9
    add    r15,     r12,     r15


    # # # # # STEP 30 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    26479

    # F  = A ^ (B & (D ^ A))
    andc   r10,     r12,     r13
    and    r16,     r15,     r13
    or     r10,     r10,     r16

    # C = C + data[28] + K + F
    addi   r14,     r14,      729
    li     r11,                 48
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 14) + D
    rotlwi r14,     r14,       14
    add    r14,     r15,     r14


    # # # # # STEP 31 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    36138

    # F  = D ^ (A & (C ^ D))
    andc   r10,     r15,     r12
    and    r16,     r14,     r12
    or     r10,     r10,     r16

    # B = B + data[48] + K + F
    addi   r13,     r13,    19594
    li     r11,                 20
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 20) + C
    rotlwi r13,     r13,       20
    add    r13,     r14,     r13


    # # # # # STEP 32 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    65530

    # F  = B ^ C ^ D
    xor    r10,     r14,     r15
    xor    r10,     r10,     r13

    # A = A + data[20] + K + F
    addi   r12,     r12,    14658
    li     r11,                 32
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  4) + B
    rotlwi r12,     r12,        4
    add    r12,     r13,     r12


    # # # # # STEP 33 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    34674

    # F  = A ^ B ^ C
    xor    r10,     r13,     r14
    xor    r10,     r10,     r12

    # D = D + data[32] + K + F
    addi   r15,     r15,    -2431
    li     r11,                 44
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 11) + A
    rotlwi r15,     r15,       11
    add    r15,     r12,     r15


    # # # # # STEP 34 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    28061

    # F  = D ^ A ^ B
    xor    r10,     r12,     r13
    xor    r10,     r10,     r15

    # C = C + data[44] + K + F
    addi   r14,     r14,    24866
    li     r11,                 56
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 16) + D
    rotlwi r14,     r14,       16
    add    r14,     r15,     r14


    # # # # # STEP 35 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    64997

    # F  = C ^ D ^ A
    xor    r10,     r15,     r12
    xor    r10,     r10,     r14

    # B = B + data[56] + K + F
    addi   r13,     r13,    14348
    li     r11,                  4
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 23) + C
    rotlwi r13,     r13,       23
    add    r13,     r14,     r13


    # # # # # STEP 36 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    42175

    # F  = B ^ C ^ D
    xor    r10,     r14,     r15
    xor    r10,     r10,     r13

    # A = A + data[04] + K + F
    addi   r12,     r12,    -5564
    li     r11,                 16
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  4) + B
    rotlwi r12,     r12,        4
    add    r12,     r13,     r12


    # # # # # STEP 37 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    19423

    # F  = A ^ B ^ C
    xor    r10,     r13,     r14
    xor    r10,     r10,     r12

    # D = D + data[16] + K + F
    addi   r15,     r15,   -12375
    li     r11,                 28
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 11) + A
    rotlwi r15,     r15,       11
    add    r15,     r12,     r15


    # # # # # STEP 38 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    63163

    # F  = D ^ A ^ B
    xor    r10,     r12,     r13
    xor    r10,     r10,     r15

    # C = C + data[28] + K + F
    addi   r14,     r14,    19296
    li     r11,                 40
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 16) + D
    rotlwi r14,     r14,       16
    add    r14,     r15,     r14


    # # # # # STEP 39 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    48832

    # F  = C ^ D ^ A
    xor    r10,     r15,     r12
    xor    r10,     r10,     r14

    # B = B + data[40] + K + F
    addi   r13,     r13,   -17296
    li     r11,                 52
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 23) + C
    rotlwi r13,     r13,       23
    add    r13,     r14,     r13


    # # # # # STEP 40 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    10395

    # F  = B ^ C ^ D
    xor    r10,     r14,     r15
    xor    r10,     r10,     r13

    # A = A + data[52] + K + F
    addi   r12,     r12,    32454
    li     r11,                  0
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  4) + B
    rotlwi r12,     r12,        4
    add    r12,     r13,     r12


    # # # # # STEP 41 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    60065

    # F  = A ^ B ^ C
    xor    r10,     r13,     r14
    xor    r10,     r10,     r12

    # D = D + data[00] + K + F
    addi   r15,     r15,    10234
    li     r11,                 12
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 11) + A
    rotlwi r15,     r15,       11
    add    r15,     r12,     r15


    # # # # # STEP 42 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    54511

    # F  = D ^ A ^ B
    xor    r10,     r12,     r13
    xor    r10,     r10,     r15

    # C = C + data[12] + K + F
    addi   r14,     r14,    12421
    li     r11,                 24
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 16) + D
    rotlwi r14,     r14,       16
    add    r14,     r15,     r14


    # # # # # STEP 43 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,     1160

    # F  = C ^ D ^ A
    xor    r10,     r15,     r12
    xor    r10,     r10,     r14

    # B = B + data[24] + K + F
    addi   r13,     r13,     7429
    li     r11,                 36
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 23) + C
    rotlwi r13,     r13,       23
    add    r13,     r14,     r13


    # # # # # STEP 44 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    55765

    # F  = B ^ C ^ D
    xor    r10,     r14,     r15
    xor    r10,     r10,     r13

    # A = A + data[36] + K + F
    addi   r12,     r12,   -12231
    li     r11,                 48
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  4) + B
    rotlwi r12,     r12,        4
    add    r12,     r13,     r12


    # # # # # STEP 45 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    59100

    # F  = A ^ B ^ C
    xor    r10,     r13,     r14
    xor    r10,     r10,     r12

    # D = D + data[48] + K + F
    addi   r15,     r15,   -26139
    li     r11,                 60
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 11) + A
    rotlwi r15,     r15,       11
    add    r15,     r12,     r15


    # # # # # STEP 46 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,     8098

    # F  = D ^ A ^ B
    xor    r10,     r12,     r13
    xor    r10,     r10,     r15

    # C = C + data[60] + K + F
    addi   r14,     r14,    31992
    li     r11,                  8
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 16) + D
    rotlwi r14,     r14,       16
    add    r14,     r15,     r14


    # # # # # STEP 47 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    50348

    # F  = C ^ D ^ A
    xor    r10,     r15,     r12
    xor    r10,     r10,     r14

    # B = B + data[08] + K + F
    addi   r13,     r13,    22117
    li     r11,                  0
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 23) + C
    rotlwi r13,     r13,       23
    add    r13,     r14,     r13


    # # # # # STEP 48 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    62505

    # F  = C ^ (B | (~D))
    orc    r10,     r13,     r15
    xor    r10,     r10,     r14

    # A = A + data[00] + K + F
    addi   r12,     r12,     8772
    li     r11,                 28
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  6) + B
    rotlwi r12,     r12,        6
    add    r12,     r13,     r12


    # # # # # STEP 49 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    17195

    # F  = B ^ (A | (~C))
    orc    r10,     r12,     r14
    xor    r10,     r10,     r13

    # D = D + data[28] + K + F
    addi   r15,     r15,     -105
    li     r11,                 56
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 10) + A
    rotlwi r15,     r15,       10
    add    r15,     r12,     r15


    # # # # # STEP 50 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    43924

    # F  = A ^ (D | (~B))
    orc    r10,     r15,     r13
    xor    r10,     r10,     r12

    # C = C + data[56] + K + F
    addi   r14,     r14,     9127
    li     r11,                 20
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 15) + D
    rotlwi r14,     r14,       15
    add    r14,     r15,     r14


    # # # # # STEP 51 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    64660

    # F  = D ^ (C | (~A))
    orc    r10,     r14,     r12
    xor    r10,     r10,     r15

    # B = B + data[20] + K + F
    addi   r13,     r13,   -24519
    li     r11,                 48
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 21) + C
    rotlwi r13,     r13,       21
    add    r13,     r14,     r13


    # # # # # STEP 52 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    25947

    # F  = C ^ (B | (~D))
    orc    r10,     r13,     r15
    xor    r10,     r10,     r14

    # A = A + data[48] + K + F
    addi   r12,     r12,    22979
    li     r11,                 12
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  6) + B
    rotlwi r12,     r12,        6
    add    r12,     r13,     r12


    # # # # # STEP 53 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    36621

    # F  = B ^ (A | (~C))
    orc    r10,     r12,     r14
    xor    r10,     r10,     r13

    # D = D + data[12] + K + F
    addi   r15,     r15,   -13166
    li     r11,                 40
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 10) + A
    rotlwi r15,     r15,       10
    add    r15,     r12,     r15


    # # # # # STEP 54 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    65520

    # F  = A ^ (D | (~B))
    orc    r10,     r15,     r13
    xor    r10,     r10,     r12

    # C = C + data[40] + K + F
    addi   r14,     r14,    -2947
    li     r11,                  4
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 15) + D
    rotlwi r14,     r14,       15
    add    r14,     r15,     r14


    # # # # # STEP 55 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    34180

    # F  = D ^ (C | (~A))
    orc    r10,     r14,     r12
    xor    r10,     r10,     r15

    # B = B + data[04] + K + F
    addi   r13,     r13,    24017
    li     r11,                 32
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 21) + C
    rotlwi r13,     r13,       21
    add    r13,     r14,     r13


    # # # # # STEP 56 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    28584

    # F  = C ^ (B | (~D))
    orc    r10,     r13,     r15
    xor    r10,     r10,     r14

    # A = A + data[32] + K + F
    addi   r12,     r12,    32335
    li     r11,                 60
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  6) + B
    rotlwi r12,     r12,        6
    add    r12,     r13,     r12


    # # # # # STEP 57 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    65069

    # F  = B ^ (A | (~C))
    orc    r10,     r12,     r14
    xor    r10,     r10,     r13

    # D = D + data[60] + K + F
    addi   r15,     r15,    -6432
    li     r11,                 24
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 10) + A
    rotlwi r15,     r15,       10
    add    r15,     r12,     r15


    # # # # # STEP 58 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    41729

    # F  = A ^ (D | (~B))
    orc    r10,     r15,     r13
    xor    r10,     r10,     r12

    # C = C + data[24] + K + F
    addi   r14,     r14,    17172
    li     r11,                 52
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 15) + D
    rotlwi r14,     r14,       15
    add    r14,     r15,     r14


    # # # # # STEP 59 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    19976

    # F  = D ^ (C | (~A))
    orc    r10,     r14,     r12
    xor    r10,     r10,     r15

    # B = B + data[52] + K + F
    addi   r13,     r13,     4513
    li     r11,                 16
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 21) + C
    rotlwi r13,     r13,       21
    add    r13,     r14,     r13


    # # # # # STEP 60 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r12,     r12,    63315

    # F  = C ^ (B | (~D))
    orc    r10,     r13,     r15
    xor    r10,     r10,     r14

    # A = A + data[16] + K + F
    addi   r12,     r12,    32386
    li     r11,                 44
    add    r12,     r12,     r17
    add    r12,     r12,     r10

    # A  = (A <<  6) + B
    rotlwi r12,     r12,        6
    add    r12,     r13,     r12


    # # # # # STEP 61 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r15,     r15,    48443

    # F  = B ^ (A | (~C))
    orc    r10,     r12,     r14
    xor    r10,     r10,     r13

    # D = D + data[44] + K + F
    addi   r15,     r15,    -3531
    li     r11,                  8
    add    r15,     r15,     r17
    add    r15,     r15,     r10

    # D  = (D << 10) + A
    rotlwi r15,     r15,       10
    add    r15,     r12,     r15


    # # # # # STEP 62 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r14,     r14,    10968

    # F  = A ^ (D | (~B))
    orc    r10,     r15,     r13
    xor    r10,     r10,     r12

    # C = C + data[08] + K + F
    addi   r14,     r14,   -11589
    li     r11,                 36
    add    r14,     r14,     r17
    add    r14,     r14,     r10

    # C  = (C << 15) + D
    rotlwi r14,     r14,       15
    add    r14,     r15,     r14


    # # # # # STEP 63 # # # # #

    lwbrx  r17,     r11,      r3
    addis  r13,     r13,    60295

    # F  = D ^ (C | (~A))
    orc    r10,     r14,     r12
    xor    r10,     r10,     r15

    # B = B + data[36] + K + F
    addi   r13,     r13,   -11375
    li     r11,                  0
    add    r13,     r13,     r17
    add    r13,     r13,     r10

    # B  = (B << 21) + C
    rotlwi r13,     r13,       21
    add    r13,     r14,     r13


    # Updating A, B, C and D
    add     r6,      r6,     r12
    add     r7,      r7,     r13
    add     r8,      r8,     r14
    add     r9,      r9,     r15

    addi    r3,      r3,       64
    addic.  r4,      r4,      -64
    bne                    LoopOver

    trap
    nop
    nop
    nop
    nop
    nop
