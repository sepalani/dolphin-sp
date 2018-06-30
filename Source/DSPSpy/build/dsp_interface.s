	.file	"dsp_interface.cpp"
	.section	".text"
	.align 2
	.globl _ZN4IDSP8SendTaskEPvttt
	.type	_ZN4IDSP8SendTaskEPvttt, @function
_ZN4IDSP8SendTaskEPvttt:
.LFB65:
	.cfi_startproc
	mflr 0
	stwu 1,-32(1)
	.cfi_def_cfa_offset 32
	.cfi_register 65, 0
	stw 27,12(1)
	.cfi_offset 27, -20
	mr 27,4
	stw 0,36(1)
	stw 28,16(1)
	.cfi_offset 65, 4
	.cfi_offset 28, -16
	mr 28,5
	stw 29,20(1)
	.cfi_offset 29, -12
	mr 29,6
	stw 30,24(1)
	.cfi_offset 30, -8
	mr 30,7
	stw 31,28(1)
	.cfi_offset 31, -4
	mr 31,3
.L2:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L2
	lwz 9,0(31)
	lis 4,0x80f3
	ori 4,4,0xa001
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L3:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L3
	lwz 9,0(31)
	mr 4,27
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L4:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L4
	lwz 9,0(31)
	lis 4,0x80f3
	ori 4,4,0xc002
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L5:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L5
	lwz 9,0(31)
	mr 4,28
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L6:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L6
	lwz 9,0(31)
	lis 4,0x80f3
	ori 4,4,0xa002
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L7:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L7
	lwz 9,0(31)
	mr 4,29
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L8:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L8
	lwz 9,0(31)
	lis 4,0x80f3
	ori 4,4,0xb002
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L9:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L9
	lwz 9,0(31)
	li 4,0
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L10:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L10
	lwz 9,0(31)
	lis 4,0x80f3
	ori 4,4,0xd001
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L11:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L11
	lwz 9,0(31)
	mr 4,30
	mr 3,31
	lwz 9,20(9)
	mtctr 9
	bctrl
.L12:
	lwz 9,0(31)
	mr 3,31
	lwz 9,16(9)
	mtctr 9
	bctrl
	cmpwi 7,3,0
	bne+ 7,.L12
	lwz 0,36(1)
	lwz 27,12(1)
	mtlr 0
	.cfi_restore 65
	lwz 28,16(1)
	lwz 29,20(1)
	lwz 30,24(1)
	lwz 31,28(1)
	addi 1,1,32
	.cfi_restore 31
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 28
	.cfi_restore 27
	.cfi_def_cfa_offset 0
	blr
	.cfi_endproc
.LFE65:
	.size	_ZN4IDSP8SendTaskEPvttt, .-_ZN4IDSP8SendTaskEPvttt
	.ident	"GCC: (devkitPPC release 29-1) 6.3.0"
