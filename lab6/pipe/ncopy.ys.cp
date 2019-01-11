#/* $begin ncopy-ys */
##################################################################
# ncopy.ys - Copy a src block of len words to dst.
# Return the number of positive words (>0) contained in src.
#
# Include your name and ID here.
# name: lijun
# ID: 516030910233
# Describe how and why you modified the baseline code.
#
##################################################################
# Do not modify this portion
# Function prologue.
# %rdi = src, %rsi = dst, %rdx = len
ncopy:

##################################################################
# You can modify this portion
	# Loop header
	iaddq $-8, %rdx
	jl Loop8end
Loop8:
	mrmovq (%rdi), %r8
	andq %r8, %r8
	mrmovq 8(%rdi), %r9
	rmmovq %r8, (%rsi)
	jle L0
	iaddq $1, %rax
L0:
	andq %r9, %r9		# val <= 0?
	mrmovq 16(%rdi), %r10
	rmmovq %r9, 8(%rsi)
	jle L1
	iaddq $1, %rax
L1:
	andq %r10, %r10
	mrmovq 24(%rdi), %r11
	rmmovq %r10, 16(%rsi)
	jle L2
	iaddq $1, %rax
L2:
	andq %r11, %r11
	mrmovq 32(%rdi), %r12
	rmmovq %r11, 24(%rsi)
	jle L3
	iaddq $1, %rax
L3:
	andq %r12, %r12
	mrmovq 40(%rdi), %r13
	rmmovq %r12, 32(%rsi)
	jle L4
	iaddq $1, %rax
L4:
	andq %r13, %r13
	mrmovq 48(%rdi), %r14
	rmmovq %r13, 40(%rsi)
	jle L5
	iaddq $1, %rax
L5:
	andq %r14, %r14
	mrmovq 56(%rdi), %rbx
	rmmovq %r14, 48(%rsi)
	jle L6
	iaddq $1, %rax
L6:
	andq %rbx, %rbx
	rmmovq %rbx, 56(%rsi)
	jle L7
	iaddq $1, %rax
L7:
	iaddq $64, %rdi		# src++
	iaddq $64, %rsi		# dst++
	iaddq $-8, %rdx		# len--
test:
	jge Loop8			# if so, goto Loop:
Loop8end:
	iaddq $8, %rdx

	iaddq $-2, %rdx
	jl Loop2end
Loop2:
	mrmovq (%rdi), %r9
	mrmovq 8(%rdi), %r10
	rmmovq %r9, (%rsi)
	rmmovq %r10, 8(%rsi)

	andq %r9, %r9
	jle L8
	iaddq $1, %rax
L8:
	andq %r10, %r10
	jle L9
	iaddq $1, %rax
L9:
	iaddq $16, %rdi
	iaddq $16, %rsi
	iaddq $-2, %rdx
test2:
	jge Loop2
Loop2end:
	iaddq $2, %rdx

	mrmovq (%rdi), %r9
	jle Done
	andq %r9, %r9
	jle L10
	iaddq $1, %rax
L10: 
	rmmovq %r9, (%rsi)
	
##################################################################
# Do not modify the following section of code
# Function epilogue.
Done:
	ret
##################################################################
# Keep the following label at the end of your function
End:
#/* $end ncopy-ys */
