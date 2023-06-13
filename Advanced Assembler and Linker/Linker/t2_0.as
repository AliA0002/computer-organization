Start	add	1	2	3
	noop
	lw	2	4	start
	sw	2	4	Test2
	lw	4	5	hello
start	noop
hello	.fill	Start
	.fill	File3
	.fill	Test2
	.fill	hello