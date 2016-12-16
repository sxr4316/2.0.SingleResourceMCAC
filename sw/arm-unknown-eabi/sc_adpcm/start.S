	.section .text
	.globl	start 
_start:               
        /* Switch to User Mode */
        /* and unset interrupt mask bits */
        mov     r0, #0x00000002
        teqp    pc, r0 
 
        @ Enable the cache
        mov     r0,  #0xffffffff
	mcr	15, 0, r0, cr3, cr0, 0   @ cacheable area
        mov     r0,  #1
	mcr	15, 0, r0, cr2, cr0, 0   @ cache enable
        
        @ initialize the stack pointer
        ldr     sp, AdrStack

        @ jump to main
        .extern main
        bl      main
                
        b       _testpass
irq_service:
	stmfd sp!, {r0-r12, lr}
	.extern irq_interrupt
	bl irq_interrupt
	ldmfd sp!, {r0-r12, lr}
	subs pc, lr, #4
/* stack at top of ddr3 memory space */
AdrStack: .word  0x8000000

        .section .bss
        .word  0x0    
/* ========================================================================= */
/* ========================================================================= */
        

