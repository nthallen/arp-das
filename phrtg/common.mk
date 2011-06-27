# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.


ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=$(PROJECT_ROOT)/src/Usemsg



#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/src
#===== POST_BUILD - extra steps to do after building the image.
define POST_BUILD
phabbind $(BUILDNAME) $(PHAB_MODULES)
phabbind $(BUILDNAME) $(PHAB_MODULES)
phabbind $(BUILDNAME) $(PHAB_MODULES)
endef
#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(QNX_TARGET)/usr/local/include /usr/pkg/include
#===== EXTRA_LIBVPATH - a space-separated list of directories to search for library files.
EXTRA_LIBVPATH+=$(QNX_TARGET)/usr/local/lib /usr/pkg/lib
#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=fftw3f nort tm
#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-Wl,-rpath -Wl,/usr/local/lib
#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-w2 -O0
include $(MKFILES_ROOT)/qmacros.mk
include $(QNX_INTERNAL)
postbuild:
	$(POST_BUILD)
include $(MKFILES_ROOT)/qtargets.mk
OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
