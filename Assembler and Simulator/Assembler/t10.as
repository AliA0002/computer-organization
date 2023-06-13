start	lw	2	4	-24
	beq	2	4	start
	lw	5	1	fill
	sw	4	1	-1000
	beq	4	1	start
	lw	2	2	neg1
	halt
fill	.fill	-30
neg1	.fill	-1