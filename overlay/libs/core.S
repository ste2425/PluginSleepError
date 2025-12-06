	.set noreorder

#include "pspstub.s"

	STUB_START "core",0x40090000,0x00010005
	STUB_FUNC  0x4CD09ED4,CoreWait
	STUB_END
