/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(t_divide, T_DIVIDE)
TRAPHANDLER_NOEC(t_debug, T_DEBUG)
TRAPHANDLER_NOEC(t_nmi, T_NMI)
TRAPHANDLER_NOEC(t_brkpt, T_BRKPT)
TRAPHANDLER_NOEC(t_oflow, T_OFLOW)
TRAPHANDLER_NOEC(t_bound, T_BOUND)
TRAPHANDLER_NOEC(t_illop, T_ILLOP)
TRAPHANDLER_NOEC(t_device, T_DEVICE)
TRAPHANDLER(t_dblflt, T_DBLFLT)
TRAPHANDLER_NOEC(t_coproc, 9)
TRAPHANDLER(t_tss, T_TSS)
TRAPHANDLER(t_segnp, T_SEGNP)
TRAPHANDLER(t_stack, T_STACK)
TRAPHANDLER(t_gpflt, T_GPFLT)
TRAPHANDLER(t_pgflt, T_PGFLT)
TRAPHANDLER_NOEC(t_res, 15)
TRAPHANDLER_NOEC(t_fperr, T_FPERR)
TRAPHANDLER(t_align, T_ALIGN)
TRAPHANDLER_NOEC(t_mchk, T_MCHK)
TRAPHANDLER_NOEC(t_simderr, T_SIMDERR)
TRAPHANDLER_NOEC(t_d20, 20)
TRAPHANDLER_NOEC(t_d21, 21)
TRAPHANDLER_NOEC(t_d22, 22)
TRAPHANDLER_NOEC(t_d23, 23)
TRAPHANDLER_NOEC(t_d24, 24)
TRAPHANDLER_NOEC(t_d25, 25)
TRAPHANDLER_NOEC(t_d26, 26)
TRAPHANDLER_NOEC(t_d27, 27)
TRAPHANDLER_NOEC(t_d28, 28)
TRAPHANDLER_NOEC(t_d29, 29)
TRAPHANDLER_NOEC(t_d30, 30)
TRAPHANDLER_NOEC(t_d31, 31)
TRAPHANDLER_NOEC(t_irq0, 32)
TRAPHANDLER_NOEC(t_irq1, 33)
TRAPHANDLER_NOEC(t_irq2, 34)
TRAPHANDLER_NOEC(t_irq3, 35)
TRAPHANDLER_NOEC(t_irq4, 36)
TRAPHANDLER_NOEC(t_irq5, 37)
TRAPHANDLER_NOEC(t_irq6, 38)
TRAPHANDLER_NOEC(t_irq7, 39)
TRAPHANDLER_NOEC(t_irq8, 40)
TRAPHANDLER_NOEC(t_irq9, 41)
TRAPHANDLER_NOEC(t_irq10, 42)
TRAPHANDLER_NOEC(t_irq11, 43)
TRAPHANDLER_NOEC(t_irq12, 44)
TRAPHANDLER_NOEC(t_irq13, 45)
TRAPHANDLER_NOEC(t_irq14, 46)
TRAPHANDLER_NOEC(t_irq15, 47)
TRAPHANDLER_NOEC(t_syscall, T_SYSCALL)

/*
 * Lab 3: Your code here for _alltraps
 */

.globl _alltraps
_alltraps:
	pushl %ds
	pushl %es
	pushal
	movl  $GD_KD, %eax
	movw  %ax, %ds
	movw  %ax, %es
	pushl %esp
	call trap

.data
.globl TrapName
TrapName :
	.long t_divide
	.long t_debug
	.long t_nmi
	.long t_brkpt
	.long t_oflow
	.long t_bound
	.long t_illop
	.long t_device
	.long t_dblflt
	.long t_coproc
	.long t_tss
	.long t_segnp
	.long t_stack
	.long t_gpflt
	.long t_pgflt
	.long t_res
	.long t_fperr
	.long t_align
	.long t_mchk
	.long t_simderr
	.long t_d20
	.long t_d21
	.long t_d22
	.long t_d23
	.long t_d24
	.long t_d25
	.long t_d26
	.long t_d27
	.long t_d28
	.long t_d29
	.long t_d30
	.long t_d31
	.long t_irq0
	.long t_irq1
	.long t_irq2
	.long t_irq3
	.long t_irq4
	.long t_irq5
	.long t_irq6
	.long t_irq7
	.long t_irq8
	.long t_irq9
	.long t_irq10
	.long t_irq11
	.long t_irq12
	.long t_irq13
	.long t_irq14
	.long t_irq14
	.long t_syscall
