.text

.global run
run:
        push {r0,r1,r2,r3,r4,r5,r6,r7}
        ldr r0,=#0x84026214

        ldr r2,=#0x10000000
        str r2,[r0]
        ldr r0,=#0x84033F98
        mov r2,#0

.loop:
        mov r3,#0xBB
        str r3,[r0]
        add r2,#1
        cmp r2,#0x40
        bne .loop
