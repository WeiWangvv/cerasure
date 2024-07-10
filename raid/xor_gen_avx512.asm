;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  Copyright(c) 2011-2017 Intel Corporation All rights reserved.
;
;  Redistribution and use in source and binary forms, with or without
;  modification, are permitted provided that the following conditions
;  are met:
;    * Redistributions of source code must retain the above copyright
;      notice, this list of conditions and the following disclaimer.
;    * Redistributions in binary form must reproduce the above copyright
;      notice, this list of conditions and the following disclaimer in
;      the documentation and/or other materials provided with the
;      distribution.
;    * Neither the name of Intel Corporation nor the names of its
;      contributors may be used to endorse or promote products derived
;      from this software without specific prior written permission.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
;  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
;  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
;  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
;  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Optimized xor of N source vectors using AVX512
;;; int xor_gen_avx512(int vects, int len, void **array)

;;; Generates xor parity vector from N (vects-1) sources in array of pointers
;;; (**array).  Last pointer is the dest.
;;; Vectors must be aligned to 32 bytes.  Length can be any value.

%include "reg_sizes.asm"

%ifdef HAVE_AS_KNOWS_AVX512

%ifidn __OUTPUT_FORMAT__, elf64
 %define arg0  rdi
 %define arg1  rsi
 %define arg2  rdx
 %define arg3  rcx
 %define arg4  r8
 %define arg5  r9
 %define tmp   r11

%define tmp2  r10
%define tmp3  r12     ; must be saved and restored
 ;%define tmp3  arg4
 %define func(x) x: endbranch
 %define return rax
   %macro FUNC_SAVE 0
	push	r12
 %endmacro
 %macro FUNC_RESTORE 0
	pop	r12
 %endmacro

;  %define FUNC_SAVE
;  %define FUNC_RESTORE

%elifidn __OUTPUT_FORMAT__, win64
 %define arg0  rcx
 %define arg1  rdx
 %define arg2  r8
 %define arg3  r9
 %define tmp   r11
 %define tmp3  r10
 %define func(x) proc_frame x
 %define return rax
 %define stack_size  2*16 + 8 	;must be an odd multiple of 8

 %macro FUNC_SAVE 0
	alloc_stack	stack_size
	vmovdqu	[rsp + 0*16], xmm6
	vmovdqu	[rsp + 1*16], xmm7
	end_prolog
 %endmacro
 %macro FUNC_RESTORE 0
	vmovdqu	xmm6, [rsp + 0*16]
	vmovdqu	xmm7, [rsp + 1*316]
	add	rsp, stack_size
 %endmacro

%endif	;output formats


%define common arg0
%define p1 arg1
%define p2 arg2
%define len arg3
%define src arg4
%define dest arg5

%define ptr tmp
%define vec_i tmp2
%define tmp2.b al
%define pos tmp3
%define PS 8

%define NO_NT_LDST
;; Use Non-temporal load/stor
%ifdef NO_NT_LDST
 %define XLDR vmovdqu8
 %define XSTR vmovdqu8
%else
 %define XLDR vmovntdqa
 %define XSTR vmovntdq
%endif



default rel
[bits 64]

section .text

;=======================XOR for PGC matching============================
align 16
mk_global  xor_gen_avx512, function
func(xor_gen_avx512)

	FUNC_SAVE
	cmp	len, 0
	je	return_pass
	sal common, 3    ;common *= PS
	sal p1, 3
	add p1, common
	sal p2, 3
	add p2, p1
	; test	len, (128-1)		;Check alignment of length
	; jnz	len_not_aligned


len_aligned_128bytes:
	sub	len, 128
	mov	pos, 0
	cmp common, 0
	je loop128_without_common
	;jmp loop128_without_common

loop128_with_common:
	xor vec_i, vec_i
	mov ptr, [src+vec_i]
	add vec_i, PS
	XLDR 	zmm0, [ptr+pos]
	XLDR	zmm1, [ptr+pos+64]	
	;XLDR	zmm2, [ptr+pos+(2*64)]
	;XLDR	zmm3, [ptr+pos+(3*64)]

next_vect_common:
	mov ptr, [src+vec_i]
	add vec_i, PS
	XLDR 	zmm4, [ptr+pos]
	XLDR	zmm5, [ptr+pos+64]	
	;XLDR	zmm6, [ptr+pos+(2*64)]
	;XLDR	zmm7, [ptr+pos+(3*64)]
	vpxorq	zmm0, zmm0, zmm4	
	vpxorq	zmm1, zmm1, zmm5
	;vpxorq	zmm2, zmm2, zmm6
	;vpxorq	zmm3, zmm3, zmm7

	cmp vec_i, common
	jl next_vect_common

	vmovdqu8 zmm8,  zmm0  ;keep common xor in zmm8-9 for parity 2
	vmovdqu8 zmm9,  zmm1
	;vmovdqu8 zmm10, zmm2
	;vmovdqu8 zmm11, zmm3

	mov vec_i, common

	cmp p1, common
	je next_vect_parity2
next_vect_parity1:
	mov ptr, [src+vec_i]
	add vec_i, PS
	XLDR 	zmm4, [ptr+pos]
	XLDR	zmm5, [ptr+pos+64]	
	;XLDR	zmm6, [ptr+pos+(2*64)]
	;XLDR	zmm7, [ptr+pos+(3*64)]
	vpxorq	zmm0, zmm0, zmm4	
	vpxorq	zmm1, zmm1, zmm5
	;vpxorq	zmm2, zmm2, zmm6
	;vpxorq	zmm3, zmm3, zmm7

	cmp vec_i, p1
	jl next_vect_parity1

	mov ptr, [dest]
	XSTR	[ptr+pos], zmm0		;Write parity1 xor vector
	XSTR	[ptr+pos+(1*64)], zmm1
	;XSTR	[ptr+pos+(2*64)], zmm2
	;XSTR	[ptr+pos+(3*64)], zmm3

	mov vec_i, p1

	cmp p2, p1
	je next_128B
next_vect_parity2:
	mov ptr, [src+vec_i]
	add vec_i, PS
	XLDR 	zmm4, [ptr+pos]
	XLDR	zmm5, [ptr+pos+64]	
	;XLDR	zmm6, [ptr+pos+(2*64)]
	;XLDR	zmm7, [ptr+pos+(3*64)]
	vpxorq	zmm8, zmm8, zmm4	
	vpxorq	zmm9, zmm9, zmm5
	;vpxorq	zmm10, zmm10, zmm6
	;vpxorq	zmm11, zmm11, zmm7

	cmp vec_i, p2
	jl next_vect_parity2

	mov ptr, [dest+PS]
	XSTR	[ptr+pos], zmm8		;Write parity1 xor vector
	XSTR	[ptr+pos+(1*64)], zmm9
	;XSTR	[ptr+pos+(2*64)], zmm10
	;XSTR	[ptr+pos+(3*64)], zmm11

next_128B:
	add	pos, 128
	cmp	pos, len
	jle	loop128_with_common

	jmp return_pass

loop128_without_common:
	;mov vec_i, 0
	xor vec_i, vec_i		
	mov	ptr, [src+vec_i]	
	add vec_i, PS			
	XLDR	zmm0, [ptr+pos]	
	XLDR	zmm1, [ptr+pos+64]	
	;XLDR	zmm2, [ptr+pos+(2*64)]
	;XLDR	zmm3, [ptr+pos+(3*64)]

next_vect:
	mov 	ptr, [src+vec_i]
	add vec_i, PS
	XLDR	zmm4, [ptr+pos]		
	XLDR	zmm5, [ptr+pos+64]
	;XLDR	zmm6, [ptr+pos+(2*64)]
	;XLDR	zmm7, [ptr+pos+(3*64)]
	vpxorq	zmm0, zmm0, zmm4	
	vpxorq	zmm1, zmm1, zmm5
	;vpxorq	zmm2, zmm2, zmm6
	;vpxorq	zmm3, zmm3, zmm7

	cmp vec_i, p1
	jl	next_vect	

	mov	ptr, [dest]	
	XSTR	[ptr+pos], zmm0		
	XSTR	[ptr+pos+(1*64)], zmm1
	;XSTR	[ptr+pos+(2*64)], zmm2
	;XSTR	[ptr+pos+(3*64)], zmm3

	add	pos, 128
	cmp	pos, len
	jle	loop128_without_common


return_pass:
	FUNC_RESTORE
	mov	return, 0
	ret
;=======================XOR for PGC matching============================

; align 16
; mk_global  xor_gen_avx512, function
; func(xor_gen_avx512)
; 	FUNC_SAVE
; 	sub	vec, 2			;Keep as offset to last source
; 	jng	return_fail		;Must have at least 2 sources
; 	cmp	len, 0
; 	je	return_pass
; 	test	len, (128-1)		;Check alignment of length
; 	jnz	len_not_aligned

; len_aligned_128bytes:
; 	sub	len, 128
; 	mov	pos, 0

; loop128:
; 	mov	tmp, vec		;Back to last vector
; 	mov	tmp2, [arg2+vec*PS]	;Fetch last pointer in array
; 	sub	tmp, 1			;Next vect
; 	XLDR	zmm0, [tmp2+pos]	;Start with end of array in last vector
; 	XLDR	zmm1, [tmp2+pos+64]	;Keep xor parity in xmm0-7

; next_vect:
; 	mov 	ptr, [arg2+tmp*PS]
; 	sub	tmp, 1
; 	XLDR	zmm4, [ptr+pos]		;Get next vector (source)
; 	XLDR	zmm5, [ptr+pos+64]
; 	vpxorq	zmm0, zmm0, zmm4	;Add to xor parity
; 	vpxorq	zmm1, zmm1, zmm5
; 	jge	next_vect		;Loop for each source

; 	mov	ptr, [arg2+PS+vec*PS]	;Address of parity vector
; 	XSTR	[ptr+pos], zmm0		;Write parity xor vector
; 	XSTR	[ptr+pos+64], zmm1
; 	add	pos, 128
; 	cmp	pos, len
; 	jle	loop128

; return_pass:
; 	FUNC_RESTORE
; 	mov	return, 0
; 	ret


;;; Do one byte at a time for no alignment case
; loop_1byte:
; 	mov	tmp, vec		;Back to last vector
; 	mov 	ptr, [arg2+vec*PS] 	;Fetch last pointer in array
; 	mov	tmp2.b, [ptr+len-1]	;Get array n
; 	sub	tmp, 1
; nextvect_1byte:
; 	mov 	ptr, [arg2+tmp*PS]
; 	xor	tmp2.b, [ptr+len-1]
; 	sub	tmp, 1
; 	jge	nextvect_1byte

; 	mov	tmp, vec
; 	add	tmp, 1		  	;Add back to point to last vec
; 	mov	ptr, [arg2+tmp*PS]
; 	mov	[ptr+len-1], tmp2.b 	;Write parity
; 	sub	len, 1
; 	test	len, (PS-1)
; 	jnz	loop_1byte

; 	cmp	len, 0
; 	je	return_pass
; 	test	len, (128-1)		;If not 0 and 128bit aligned
; 	jz	len_aligned_128bytes	; then do aligned case. len = y * 128

; 	;; else we are 8-byte aligned so fall through to recheck


; 	;; Unaligned length cases
; len_not_aligned:
; 	test	len, (PS-1)
; 	jne	loop_1byte
; 	mov	tmp3, len
; 	and	tmp3, (128-1)		;Do the unaligned bytes 8 at a time

; 	;; Run backwards 8 bytes at a time for (tmp3) bytes
; loop8_bytes:
; 	mov	tmp, vec		;Back to last vector
; 	mov 	ptr, [arg2+vec*PS] 	;Fetch last pointer in array
; 	mov	tmp2, [ptr+len-PS]	;Get array n
; 	sub	tmp, 1
; nextvect_8bytes:
; 	mov 	ptr, [arg2+tmp*PS] 	;Get pointer to next vector
; 	xor	tmp2, [ptr+len-PS]
; 	sub	tmp, 1
; 	jge	nextvect_8bytes	;Loop for each source

; 	mov	tmp, vec
; 	add	tmp, 1		  	;Add back to point to last vec
; 	mov	ptr, [arg2+tmp*PS]
; 	mov	[ptr+len-PS], tmp2	;Write parity
; 	sub	len, PS
; 	sub	tmp3, PS
; 	jg	loop8_bytes

; 	cmp	len, 128		;Now len is aligned to 128B
; 	jge	len_aligned_128bytes	;We can do the rest aligned

; 	cmp	len, 0
; 	je	return_pass

return_fail:
	FUNC_RESTORE
	mov	return, 1
	ret

endproc_frame

%endif  ; ifdef HAVE_AS_KNOWS_AVX512
