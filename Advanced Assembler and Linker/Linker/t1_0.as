	lw	0	1	2
Start	sw	1	2	Start
	lw	2	1	File
	beq	0	0	1
Label	.fill	Start
	.fill	File
	.fill	Hello
	.fill	Stack