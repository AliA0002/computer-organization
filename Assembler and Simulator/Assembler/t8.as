first	lw	0	1	-10
second	add	1	2	3
third	beq	0	0	first
fourth	nor	1	2	3
fifth	sw	2	4	sevent
sixth	lw	2	1	fifth
sevent	halt
	noop
	.fill 25
	.fill first
	.fill third
	.fill -5002
