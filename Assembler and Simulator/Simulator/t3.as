	lw	0	2	twel
hello	jalr	2	4
	add	1	2	3
	noop	
	lw	0	2	hello
	beq	0	0	end
	nor	2	5	2
	noop
end	halt
twel	.fill	4