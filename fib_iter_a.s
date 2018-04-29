    .global fib_iter_a
    .func fib_iter_a

/* r0 = n */

fib_iter_a:
    sub sp, sp, #4
    str r4, [sp]
    mov r1, #0
    mov r2, #1
    mov r3, #2            /* r3 = i */
    cmp r0, #2            /* if n < 2 */
    blt done

loop:
    cmp r3, r0            /* i < n */
    bgt done2
    add r4, r1, r2        /* r4 = r1 + r2 */
    mov r1, r2            /* r1 = r2 */
    mov r2, r4            /* r2 = r4 */
    add r3, r3, #1        /* i++ */
    b loop

done:
    cmp r0, #0
    moveq r0, #0          /* n == 0 */
    cmp r0, #1
    moveq r0, #1          /* n == 1 */
    bx lr

done2:
    mov r0, r4            /* r4 = nth fibonacci sequence value */
    ldr r4, [sp]
    add sp, sp, #4
    bx lr
