start	lw	0	6	pos1
	sw	5	7	Stack
	add	5	6	5
	sw	5	1	Stack
	add	5	6	5
	sw	5	2	Stack
	add	5	6	5
	sw	5	4	Stack
	add	5	6	5
	beq	0	2	return
	beq	1	2	return
	lw	0	6	neg1	
	add	1	6	1
	lw	0	4	Caddr	
	jalr	4	7
	lw	0	6	neg1
	add	2	6	2
	sw	5	3	Stack
	lw	0	6	pos1
	add	5	6	5
	jalr	4	7
	lw	0	6	neg1
	add	5	6	5
	lw	5	4	Stack
	add	4	3	3
	beq	0	0	regres
return	lw	0	6	pos1
	add	0	6	3
regres	lw	0	6	neg1
	add	5	6	5	
	lw	5	4	Stack
	add	5	6	5
	lw	5	2	Stack
	add	5	6	5
	lw	5	1	Stack
	add	5	6	5
	lw	5	7	Stack	
	jalr	7	6
Caddr	.fill	start
pos1	.fill	1
neg1	.fill	-1
