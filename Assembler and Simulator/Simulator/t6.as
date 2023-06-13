	lw	0	3	neg1
	lw	0	2	three
	add	2	3	5
start	add	5	3	5
	beq	0	5	end
	beq	0	0	start
end	halt
three	.fill	3
neg1	.fill	-1