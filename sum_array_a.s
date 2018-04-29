    .global sum_array_a
    .func sum_array_a

/* r0 = array address */
/* r1 = n */

sum_array_a:
    sub sp, sp, #4
    str r4, [sp]
    mov r2, #0                         /* int i */
    mov r3, #0                         /* int sum */

loop:
    cmp r2, r1                         /* i < n */
    beq done          
    ldr r4, [r0]                       /* array[i] */
    add r3, r3, r4                     /* sum += array[i] */
    add r0, r0, #4
    add r2, r2, #1                     /* i++ */
    b loop

done:
    ldr r4, [sp]
    add sp, sp, #4
    mov r0, r3                         /* return sum */
    bx lr
    