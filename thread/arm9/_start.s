.arm
.global _createThread

_createThread:
	push {r0-r12 , lr}
	mov r0, #0x3F       @ thread priority
	ldr r1, =thread     @ thread_addr
	mov r2, #0x0        @ arg
	ldr r3, =0x08000C00 @ StackTop
	ldr r4, =0xFFFFFFFE @ Affinity mask (-2)
	svc 0x8             @ svcCreateThread
	pop {r0-r12 , lr}
    
	ldr r0, =0x080E3408
	ldr pc, =0x0808519C