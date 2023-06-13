start	lw	2	3	hello
	jalr	3	5
five	add	1	2	5
hello	add	3	5	1
	beq	1	1	five	//negative number
	beq	2	4	-5
	halt
	noop
	.fill	hello
label	.fill	label
