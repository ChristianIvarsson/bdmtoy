# Do not expect anything in this workspace to function properly!
#

# -TODO -
#
# 0. IMPORTANT! libUSB deals with byte buffers and internal code deals with words. You _COULD_ receive odd counts
#
# 1. Anything tagged with <Mend me>
#
# 2. Endianess on the host end - It's using uint32_t deref in a few places so a simple injection of byteswap of words won't work.
#
# 3. TAP_DO_UPDATESTATUS is only using the <STATUS> word on the host end. -> Fix up the damn adapter code to actually use the <FLAG> word properly!
# - - < Should not affect old host code >
#

# - When implementing -
#
# - - Trionic needs more checks before attempting a mirror
# - - - T5 can mirror 128K up to 256 or 512. 256K up to 512
# - - - T7 can mirror 512K up to 1M
