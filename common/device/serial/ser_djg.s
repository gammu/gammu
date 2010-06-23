# Some sources from SVAsync (c) 1996, 1997, Samuel Vincent
# 7337 Carioca Ct, Rohnert Park, Ca 94928
# "you may freely use it in your programs without paying me anything"

	.file   "isr.S"
.text
.align 4
.globl _SVAsyncProtISR
_SVAsyncProtISR:
	pushl %eax
	pushl %edx
	pushw %ds
	movw %cs:___djgpp_ds_alias, %ds
top:        
	movw _LSR,%dx
	inb %dx, %al
	testb $1,%al
	je byebye

	movw _RDR,%dx
	inb %dx, %al
	movl _RecHead,%edx
	incl _RecHead
	movb %al, _RecBuffer(%edx)
	cmpl $32768,_RecHead
	jb inrange
	movl $0,_RecHead
inrange:
	jmp top

byebye:        
	popw %ds
	popl %edx
	
	movb $32,%al
	outb %al, $32
	
	popl %eax
	iret
