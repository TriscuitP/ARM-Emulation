    .global find_str_a
    .func find_str_a

/* r0 = *s */
/* r1 = *sub */

find_str_a:
    sub sp, sp, #4
    str r4, [sp]
    sub sp, sp, #4
    str r5, [sp]
    sub sp, sp, #4
    str r6, [sp]

    mov r2, #0                   /* r2 = i */
    mov r3, #0                   /* r3 = j */
    mov r4, #0                   /* r4 = count */

while:
    ldrb r5, [r0, r2]            /* r5 = s[i] */
    ldrb r6, [r1, r3]            /* r6 = sub[j] */
    cmp r5, #0                   /* if(s[i] != '\0') */
    beq next
    cmp r6, #0                   /* if(sub[j] != '\0') */
    beq next
    cmp r5, r6
    bne if                       /* if(s[i] != sub[j]) */
    add r2, r2, #1               /* i++ */
    add r3, r3, #1               /* j++ */
    add r4, r4, #1               /* count++ */
    b while

if:
    add r2, r2, #1               /* i++ */
    mov r3, #0                   /* j = 0 */
    add r4, r4, #1               /* count++ */
    b while

next:
    cmp r6, #0                   /* if(sub[j] == '\0') */
    beq done                     
    mov r0, #0
    sub r0, r0, #1               /* return -1 */
    bx lr

done:
    sub r0, r4, r3               /* r0 = count - j */

    ldr r6, [sp]
    add sp, sp, #4
    ldr r5, [sp]
    add sp, sp, #4
    ldr r4, [sp]
    add sp, sp, #4
    bx lr

    
