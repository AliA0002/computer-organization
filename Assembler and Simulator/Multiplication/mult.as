start	lw	0	2	mcand
	lw	0	3	mplier
	beq	4	0	setmax
setmax	lw	0	4	maxval
	beq	5	0	setmak
setmak	lw	0	5	mask
check	beq	3	0	finish
check2	beq	2	0	finish
begin	nor	3	5	6
	add	0	1	7
	beq	6	0	mult
	beq	0	0	cont
mult	add	7	2	1
	add	0	0	7
cont	lw	0	6	neg1
	add	4	6	4
	beq	4	0	finish
	lw	0	6	pos1
	add	2	2	2
	add	5	5	5
	add	5	6	5
	beq	0	0	begin
finish	add	0	0	7	
	halt
mcand	.fill	1103
mplier	.fill	7043
pos1	.fill	1
neg1	.fill	-1
mask	.fill	-2
maxval	.fill	15