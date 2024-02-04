.global appEntry

# Vectors. You need to do some more in the linker script to get this going since you normally want text.entry to be first.
# .section .text.vectors

# App entry point.
# This must be first
.section .text.entry

appEntry:

    # Set sp to entrypoint + 7fc
    lea.l    appEntry    , a1
    lea.l    0x7fc(a1)   , sp

    bsr.b    someTestFunc

# Enter bdm mode
    bgnd


# Place ordinary code after this point (You don't have to declare this in other files if you were to write a multi-file project)
.section .text

someTestFunc:
    rts
