starts	lw	1	2	3
	add	1	2	3
	add	1	2	3
	beq	3	5	starts
five	add	3	5	6
	sw	4	1	five
	lw	2	1	starts
	noop
	.fill	five
	.fill	starts
	.fill	23152
