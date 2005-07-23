INCLUDES = -I..

STRIP_NON_ASM = sh $(top_srcdir)/strip_non_asm_libtool_args.sh

.asm.lo:
	$(LIBTOOL) --tag=CC --mode=compile $(STRIP_NON_ASM) $(NASM) -f elf -o $@ $<

noinst_LTLIBRARIES = libSPCimpl.la

libSPCimpl_la_SOURCES = \
	SPCimpl.c \
	SPCimpl.h \
	SPC700.asm \
	SPC.ni \
	regs.ni \
	SPCaddr.ni \
	SPCmacro.ni \
	SPCops.ni \
	cycles.ni \
	misc.ni \
	PPU.ni
