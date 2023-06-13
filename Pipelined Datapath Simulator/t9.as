	lw	0	1	one
	lw	0	2	zero
	lw	0	3	four
loop	beq	2	3	end
	add	1	2	2
	beq	0	0	loop
end	halt
one	.fill	1
zero	.fill	0
four	.fill	4