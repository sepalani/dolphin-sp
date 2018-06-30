	.file	"real_dsp.cpp"
	.section	".text"
	.align 2
	.type	_ZL15dsp_irq_handlermPv, @function
_ZL15dsp_irq_handlermPv:
.LFB76:
	.cfi_startproc
	lis 10,0xcc00
	ori 10,10,0x500a
	lhz 9,0(10)
	andi. 9,9,0xff57
	ori 9,9,0x80
	sth 9,0(10)
	blr
	.cfi_endproc
.LFE76:
	.size	_ZL15dsp_irq_handlermPv, .-_ZL15dsp_irq_handlermPv
	.align 2
	.globl _ZN7RealDSP5ResetEv
	.type	_ZN7RealDSP5ResetEv, @function
_ZN7RealDSP5ResetEv:
.LFB78:
	.cfi_startproc
	lis 9,0xcc00
	ori 9,9,0x500a
	lhz 8,0(9)
	mr 10,9
	andi. 8,8,0xf757
	ori 8,8,0x800
	sth 8,0(9)
	lhz 8,0(9)
	andi. 8,8,0xff53
	sth 8,0(9)
	lhz 8,0(9)
	ori 8,8,0x1
	sth 8,0(9)
.L3:
	lhz 9,0(10)
	andi. 8,9,0x1
	bne+ 0,.L3
	lis 9,0xcc00
	li 10,99
	ori 9,9,0x5012
	sth 10,0(9)
	blr
	.cfi_endproc
.LFE78:
	.size	_ZN7RealDSP5ResetEv, .-_ZN7RealDSP5ResetEv
	.section	.text._ZN7RealDSPD2Ev,"axG",@progbits,_ZN7RealDSPD5Ev,comdat
	.align 2
	.weak	_ZN7RealDSPD2Ev
	.type	_ZN7RealDSPD2Ev, @function
_ZN7RealDSPD2Ev:
.LFB82:
	.cfi_startproc
	blr
	.cfi_endproc
.LFE82:
	.size	_ZN7RealDSPD2Ev, .-_ZN7RealDSPD2Ev
	.weak	_ZN7RealDSPD1Ev
	.set	_ZN7RealDSPD1Ev,_ZN7RealDSPD2Ev
	.section	".text"
	.align 2
	.globl _ZN7RealDSP4InitEv
	.type	_ZN7RealDSP4InitEv, @function
_ZN7RealDSP4InitEv:
.LFB77:
	.cfi_startproc
	mflr 0
	stwu 1,-16(1)
	.cfi_def_cfa_offset 16
	.cfi_register 65, 0
	lis 10,0xcc00
	stw 30,8(1)
	ori 10,10,0x500a
	stw 0,20(1)
	.cfi_offset 30, -8
	.cfi_offset 65, 4
	li 30,0
	stw 31,12(1)
	.cfi_offset 31, -4
	lhz 9,0(10)
	andi. 9,9,0xf757
	ori 9,9,0x800
	sth 9,0(10)
	lhz 9,0(10)
	andi. 9,9,0xff53
	sth 9,0(10)
	mr 10,30
	mr 9,30
 # 30 "s:/Projects/dolphin_sp/Source/DSPSpy/real_dsp.cpp" 1
	mfmsr 10
rlwinm 9,10,0,17,15
mtmsr 9
extrwi 10,10,1,16
 # 0 "" 2
	lis 4,_ZL15dsp_irq_handlermPv@ha
	li 5,0
	la 4,_ZL15dsp_irq_handlermPv@l(4)
	li 3,7
	mr 31,10
	bl IRQ_Request
 # 32 "s:/Projects/dolphin_sp/Source/DSPSpy/real_dsp.cpp" 1
	    cmpwi 31,0
    beq 1f
    mfmsr 30
    ori 30,30,0x8000
    mtmsr 30
1:
 # 0 "" 2
	lwz 0,20(1)
	lwz 30,8(1)
	mtlr 0
	.cfi_restore 65
	lwz 31,12(1)
	addi 1,1,16
	.cfi_restore 31
	.cfi_restore 30
	.cfi_def_cfa_offset 0
	blr
	.cfi_endproc
.LFE77:
	.size	_ZN7RealDSP4InitEv, .-_ZN7RealDSP4InitEv
	.align 2
	.globl _ZN7RealDSP11CheckMailToEv
	.type	_ZN7RealDSP11CheckMailToEv, @function
_ZN7RealDSP11CheckMailToEv:
.LFB79:
	.cfi_startproc
	b DSP_CheckMailTo
	.cfi_endproc
.LFE79:
	.size	_ZN7RealDSP11CheckMailToEv, .-_ZN7RealDSP11CheckMailToEv
	.align 2
	.globl _ZN7RealDSP10SendMailToEm
	.type	_ZN7RealDSP10SendMailToEm, @function
_ZN7RealDSP10SendMailToEm:
.LFB80:
	.cfi_startproc
	mr 3,4
	b DSP_SendMailTo
	.cfi_endproc
.LFE80:
	.size	_ZN7RealDSP10SendMailToEm, .-_ZN7RealDSP10SendMailToEm
	.section	.text._ZN7RealDSPD0Ev,"axG",@progbits,_ZN7RealDSPD5Ev,comdat
	.align 2
	.weak	_ZN7RealDSPD0Ev
	.type	_ZN7RealDSPD0Ev, @function
_ZN7RealDSPD0Ev:
.LFB84:
	.cfi_startproc
	b _ZdlPv
	.cfi_endproc
.LFE84:
	.size	_ZN7RealDSPD0Ev, .-_ZN7RealDSPD0Ev
	.weak	_ZTS4IDSP
	.section	.sdata._ZTS4IDSP,"awG",@progbits,_ZTS4IDSP,comdat
	.align 2
	.type	_ZTS4IDSP, @object
	.size	_ZTS4IDSP, 6
_ZTS4IDSP:
	.string	"4IDSP"
	.weak	_ZTI4IDSP
	.section	.sdata._ZTI4IDSP,"awG",@progbits,_ZTI4IDSP,comdat
	.align 2
	.type	_ZTI4IDSP, @object
	.size	_ZTI4IDSP, 8
_ZTI4IDSP:
	.long	_ZTVN10__cxxabiv117__class_type_infoE+8
	.long	_ZTS4IDSP
	.weak	_ZTS7RealDSP
	.section	.rodata._ZTS7RealDSP,"aG",@progbits,_ZTS7RealDSP,comdat
	.align 2
	.type	_ZTS7RealDSP, @object
	.size	_ZTS7RealDSP, 9
_ZTS7RealDSP:
	.string	"7RealDSP"
	.weak	_ZTI7RealDSP
	.section	.rodata._ZTI7RealDSP,"aG",@progbits,_ZTI7RealDSP,comdat
	.align 2
	.type	_ZTI7RealDSP, @object
	.size	_ZTI7RealDSP, 12
_ZTI7RealDSP:
	.long	_ZTVN10__cxxabiv120__si_class_type_infoE+8
	.long	_ZTS7RealDSP
	.long	_ZTI4IDSP
	.weak	_ZTV7RealDSP
	.section	.rodata._ZTV7RealDSP,"aG",@progbits,_ZTV7RealDSP,comdat
	.align 2
	.type	_ZTV7RealDSP, @object
	.size	_ZTV7RealDSP, 32
_ZTV7RealDSP:
	.long	0
	.long	_ZTI7RealDSP
	.long	_ZN7RealDSPD1Ev
	.long	_ZN7RealDSPD0Ev
	.long	_ZN7RealDSP4InitEv
	.long	_ZN7RealDSP5ResetEv
	.long	_ZN7RealDSP11CheckMailToEv
	.long	_ZN7RealDSP10SendMailToEm
	.ident	"GCC: (devkitPPC release 29-1) 6.3.0"
