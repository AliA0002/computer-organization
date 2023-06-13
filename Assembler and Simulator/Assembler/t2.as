label	sw	0	3	no
	lw	1	3	4
five	jalr	5	5
	beq	3	5	label
no	nor	2	5	1
	add	1	3	7
	halt
	noop
	.fill	label
	.fill	five
	.fill	-16
	.fill	23
