	.file	"hptw.c"
	.text
.Ltext0:
	.comm	p,8,8
	.section	.rodata
.LC0:
	.string	"%d, %p\n"
	.text
	.globl	createlist
	.type	createlist, @function
createlist:
.LFB2:
	.file 1 "hptw.c"
	.loc 1 14 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	.loc 1 18 0
	movq	$0, -16(%rbp)
	.loc 1 19 0
	movl	$0, -20(%rbp)
	.loc 1 20 0
	jmp	.L2
.L3:
	.loc 1 21 0
	movl	$16, %edi
	call	malloc
	movq	%rax, -8(%rbp)
	.loc 1 22 0
	movl	$4, %edi
	call	malloc
	movq	%rax, %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, 8(%rax)
	.loc 1 23 0
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movl	-20(%rbp), %edx
	movl	%edx, (%rax)
	.loc 1 24 0
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, (%rax)
	.loc 1 25 0
	movq	-8(%rbp), %rax
	movq	%rax, -16(%rbp)
	.loc 1 26 0
	movq	-8(%rbp), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	.loc 1 27 0
	addl	$1, -20(%rbp)
.L2:
	.loc 1 20 0
	cmpl	$9, -20(%rbp)
	jbe	.L3
	.loc 1 29 0
	movq	-16(%rbp), %rax
	.loc 1 30 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	createlist, .-createlist
	.section	.rodata
.LC1:
	.string	"%d\n"
	.text
	.globl	printlist
	.type	printlist, @function
printlist:
.LFB3:
	.loc 1 32 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	.loc 1 34 0
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	.loc 1 35 0
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	jmp	.L6
.L7:
	.loc 1 37 0 discriminator 3
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movl	(%rax), %edx
	addl	$1, %edx
	movl	%edx, (%rax)
	.loc 1 38 0 discriminator 3
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movl	(%rax), %eax
	movl	%eax, %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	.loc 1 35 0 discriminator 3
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
.L6:
	.loc 1 35 0 is_stmt 0 discriminator 1
	cmpq	$0, -8(%rbp)
	jne	.L7
	.loc 1 40 0 is_stmt 1
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	printlist, .-printlist
	.section	.rodata
.LC2:
	.string	"creating"
.LC3:
	.string	"geting"
.LC4:
	.string	"end"
	.text
	.globl	main
	.type	main, @function
main:
.LFB4:
	.loc 1 42 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	.loc 1 44 0
	movl	$.LC2, %edi
	call	puts
	.loc 1 45 0
	call	createlist
	movq	%rax, -8(%rbp)
	.loc 1 46 0
	movl	$.LC3, %edi
	call	puts
	.loc 1 47 0
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	printlist
	.loc 1 48 0
	movl	$.LC4, %edi
	call	puts
	.loc 1 49 0
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	main, .-main
.Letext0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x1a4
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF18
	.byte	0xc
	.long	.LASF19
	.long	.LASF20
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF7
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF8
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF9
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF10
	.uleb128 0x4
	.string	"pat"
	.byte	0x4
	.byte	0x1
	.byte	0x3
	.long	0x9a
	.uleb128 0x5
	.long	.LASF12
	.byte	0x1
	.byte	0x4
	.long	0x42
	.byte	0
	.byte	0
	.uleb128 0x6
	.long	.LASF11
	.byte	0x10
	.byte	0x1
	.byte	0x6
	.long	0xbf
	.uleb128 0x5
	.long	.LASF13
	.byte	0x1
	.byte	0x7
	.long	0xbf
	.byte	0
	.uleb128 0x7
	.string	"pat"
	.byte	0x1
	.byte	0x8
	.long	0xc5
	.byte	0x8
	.byte	0
	.uleb128 0x8
	.byte	0x8
	.long	0x9a
	.uleb128 0x8
	.byte	0x8
	.long	0x81
	.uleb128 0x9
	.long	.LASF14
	.byte	0x1
	.byte	0x9
	.long	0x9a
	.uleb128 0x9
	.long	.LASF15
	.byte	0x1
	.byte	0xb
	.long	0xe1
	.uleb128 0x8
	.byte	0x8
	.long	0xcb
	.uleb128 0xa
	.long	.LASF21
	.byte	0x1
	.byte	0xd
	.long	0xd6
	.quad	.LFB2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x130
	.uleb128 0xb
	.string	"ch"
	.byte	0x1
	.byte	0xf
	.long	0x42
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0xc
	.long	.LASF16
	.byte	0x1
	.byte	0x10
	.long	0xd6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0xb
	.string	"p"
	.byte	0x1
	.byte	0x11
	.long	0xe1
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0xd
	.long	.LASF22
	.byte	0x1
	.byte	0x1f
	.quad	.LFB3
	.quad	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.long	0x168
	.uleb128 0xe
	.long	.LASF16
	.byte	0x1
	.byte	0x1f
	.long	0xd6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0xb
	.string	"p"
	.byte	0x1
	.byte	0x21
	.long	0xe1
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0xf
	.long	.LASF23
	.byte	0x1
	.byte	0x29
	.quad	.LFB4
	.quad	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x194
	.uleb128 0xc
	.long	.LASF17
	.byte	0x1
	.byte	0x2b
	.long	0xd6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x10
	.string	"p"
	.byte	0x1
	.byte	0xc
	.long	0xe1
	.uleb128 0x9
	.byte	0x3
	.quad	p
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF9:
	.string	"long long int"
.LASF3:
	.string	"unsigned int"
.LASF23:
	.string	"main"
.LASF16:
	.string	"head"
.LASF18:
	.string	"GNU C11 5.4.0 20160609 -mtune=generic -march=x86-64 -g -O0 -fstack-protector-strong"
.LASF0:
	.string	"long unsigned int"
.LASF10:
	.string	"long long unsigned int"
.LASF14:
	.string	"listnode"
.LASF8:
	.string	"char"
.LASF22:
	.string	"printlist"
.LASF1:
	.string	"unsigned char"
.LASF12:
	.string	"data"
.LASF6:
	.string	"long int"
.LASF20:
	.string	"/home/zhangqianlong/Speculative_Snoop/sniper_sim/HPTW"
.LASF19:
	.string	"hptw.c"
.LASF11:
	.string	"node"
.LASF15:
	.string	"linklist"
.LASF2:
	.string	"short unsigned int"
.LASF4:
	.string	"signed char"
.LASF17:
	.string	"list"
.LASF7:
	.string	"sizetype"
.LASF5:
	.string	"short int"
.LASF21:
	.string	"createlist"
.LASF13:
	.string	"next"
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
