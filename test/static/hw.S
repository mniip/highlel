.data
string:
	.ascii "Hello, world\n"
	len = . - string

.text
.global _start
_start:
	mov $1, %rax
	mov $1, %rdi
	mov $string, %rsi
	mov $len, %rdx
	syscall
	mov $60, %rax
	mov $0, %rdi
	syscall
