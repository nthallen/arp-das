# This is required for building under Neutrino.
# For some reason, the dependencies don't work right, but
# it compiles if you give the target explicitly on the
# command line:
#   make -fnative.mk /home/nort/BUILD/QNX6/phrtg/x86/o-g/phrtg_g
PROJECT_ROOT=/home/nort/BUILD/QNX6/phrtg
BUILDNAME=phrtg_g
include Makefile

