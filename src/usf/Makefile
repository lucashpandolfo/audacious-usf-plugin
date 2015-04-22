PLUGIN = usf${PLUGIN_SUFFIX}

SRCS = audio_hle_main.cc \
       audio_ucode1.cc \
       audio_ucode2.cc \
       audio_ucode3.cc \
       audio_ucode3mp3.cc \
       cpu.cc \
       dma.cc \
       exception.cc \
       interpreter_cpu.cc \
       interpreter_ops.cc \
       main.cc \
       memory.cc \
       pif.cc \
       psftag.cc \
       recompiler_cpu.cc \
       recompiler_fpu_ops.cc \
       recompiler_ops.cc \
       registers.cc \
       rsp.cc \
       rsp_interpreter_cpu.cc \
       rsp_mmx.cc \
       rsp_recompiler_analysis.cc \
       rsp_recompiler_cpu.cc \
       rsp_recompiler_ops.cc \
       rsp_sse.cc \
       rsp_x86.cc \
       tlb.cc \
       usf.cc \
       x86.cc \
       x86_fpu.cc \
       plugin.cc

include ../../buildsys.mk
include ../../extra.mk

plugindir := ${plugindir}/${INPUT_PLUGIN_DIR}

CFLAGS += ${PLUGIN_CFLAGS} -O0
CXXFLAGS += ${PLUGIN_CFLAGS} -O0
CPPFLAGS += ${PLUGIN_CPPFLAGS} ${GLIB_CFLAGS}  -I../.. -Ispu/ -I.
LIBS += ${GLIB_LIBS}  -lz -lm -lstdc++

