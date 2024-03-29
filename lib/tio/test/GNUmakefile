#-----------------------------------------------------------------------
#     $Id$
#     
#     Copyright (2008) Sandia Corporation. Under the terms of
#     Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
#     Government retains certain rights in this software.
#     
#     Hermes is free software: you can redistribute it and/or modify
#     it under the terms of the GNU Lesser General Public License as
#     published by the Free Software Foundation, either version 3 of
#     the License, or (at your option) any later version.
#     
#     Hermes is distributed in the hope that it will be useful, but
#     WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU Lesser General Public License for more details.
#     
#     You should have received a copy of the GNU Lesser General
#     Public License along with Hermes.  If not, see
#     <http://www.gnu.org/licenses/>.
#     
#-----------------------------------------------------------------------
#
#   This Makefile depends on the following environmental variables:
#
#   variable                      defined in
#   --------                      ----------
#    HERMES_ARCH_LIST   'hermes_root'/bin/hermesenv.[sh|csh]
#    HERMES_GLOBAL      'hermes_root'/bin/hermesenv.[sh|csh]
#
#--------------------------- Macro Section -----------------------------
#
TIOINC		= ..
PRODUCT = tiotest
PLATFORM = $(HERMES_SYS_TYPE)
MAKEFILE 	= GNUmakefile
SHELL 		= /bin/sh
GOOD2GO = yes

DEBUG =
CPP_OPTS =
FPP_OPTS =
F_OPTS =
BLD_SRC_EXTS = -sF -sf -sc
BLD_INC_EXTS = -iinc
LGRPFLAGS = -nmpi
WINLGRPFLAGS = -nmpi
LIBBASE   := ../..
FFLAGS_OD = $(FFLAGS_OPT)
CFLAGS_OD = $(CFLAGS_OPT)
EXT_SRCS  = nulaux.f
EXT_OBJS  = $(EXT_SRCS:.f=.o)
TIOAUXDIR = ../taux
#
include $(HERMES_GLOBAL)makeopts.$(PLATFORM)

# check for legal cross-compiling on this platform
ifeq ($(strip $(findstring :$(PLATFORM):,$(HU_SYS_LOCAL))),)
  GOOD2GO=no
  ERR_MSG=Need to use system from following list: $(subst :, ,$(HU_SYS_LOCAL))
endif

THIS_MAKE := $(PLATFORM)
#
ifneq ($(strip $(DEBUG)),)
  FFLAGS_OD = $(FFLAGS_DBG)
  CFLAGS_OD = $(CFLAGS_DBG)
  LIBBASE := $(LIBBASE)/debug
  THIS_MAKE := $(THIS_MAKE):DEBUG
endif

C_INCDIRS =
F_INCDIRS = -I.. -I../../../finclude
BLDDEPLST_ARGS =

LIBDIRNAME      = $(PLATFORM)
LIBDIR          = $(LIBBASE)/$(LIBDIRNAME)

ifneq ($(strip $(MPI)),)
  MPI_EXT = _mpi
  CPP_OPTS = -DUSE_MPI
  FPP_OPTS = -DUSE_MPI
  F_OPTS = $(HU_MACRO_DEF)USE_MPI
  FC=$(MPI_FC)
  LGRPFLAGS =
  THIS_MAKE := $(THIS_MAKE):MPI
  ifeq ($(strip $(MPI_SYS)),)
    C_INCDIRS += -I$(MPI_ROOT)/include
    F_INCDIRS += -I$(MPI_ROOT)/include
    LIBS += $(MPI_LIB)
    BLDDEPLST_ARGS = -VMPI_ROOT
  endif
endif

LIBS = -L$(LIBDIR) -ltio$(MPI_EXT)$(IDL_EXT) -lmdutil -lnserver

PRODNAME=$(PRODUCT)$(MPI_EXT)$(IDL_EXT)

MACRO_LIST_FILE = $(PRODNAME)_macro_list
DEP_LIST_FILE = $(PRODNAME)_dep_list

FFLAGS 	   = $(FFLAGS_ALL) $(FFLAGS_OD)
FPPFLAGS   = $(FPPFLAGS_SYS) $(FPP_OPTS)
FFLAGS_FPP = $(FFLAGS_SYS) $(F_OPTS)

DEP_INCDIRS = $(C_INCDIRS) $(F_INCDIRS)

%.o : %.F
	$(FC) -c $(FFLAGS) $(F_INCDIRS) $(FFLAGS_FPP) $<

%.o : %.f
	$(FC) -c $(FFLAGS) $(F_INCDIRS) $<

% : %.f
	$(FC) -o $@ $(FFLAGS) $(F_INCDIRS) $^ $(LIBS)
	rm -f $@.o

% : %.F
	$(FC) $(FFLAGS) $(FFLAGS_FPP) $(F_INCDIRS) $^ $(LIBS) -o $@
	rm -f $@.o

% : %.o
	$(FC) -o $@ $(FFLAGS) $^ $(LIBS)

%.o : $(TIOAUXDIR)/%.f
	$(FC) -c $(FFLAGS) $<

ifneq ($(strip $(wildcard $(MACRO_LIST_FILE))),)
  include $(MACRO_LIST_FILE)
endif

.INTERMEDIATE : $(MAIN_OBJS)

.PHONY : first targets DEBUG dbg MPI mpi IDL idl $(HERMES_ARCH_LIST)
.PHONY : bld_macro_list clean clobber depend nmakedepend
#
#--------------------------- Target Section -----------------------------
#

ifeq ($(strip $(GOOD2GO)),no)
first targets DEBUG dbg MPI mpi IDL idl $(HERMES_ARCH_LIST) :
	@echo $(ERR_MSG)
else
first :
	$(MAKE) PLATFORM=$(PLATFORM) DEBUG=$(DEBUG) sys_check
	$(MAKE) PLATFORM=$(PLATFORM) DEBUG=$(DEBUG) targets

targets : $(TARGETS) $(STANDALONE_TARGETS)

ifneq ($(strip $(TARGETS)),)
$(TARGETS) : $(OBJS) $(EXT_OBJS)
endif

DEBUG dbg :
	@$(MAKE) -f $(MAKEFILE) DEBUG=1  MPI=$(MPI) IDL=$(IDL)

IDL idl :
	@$(MAKE) -f $(MAKEFILE) IDL=1 DEBUG=$(DEBUG) MPI=$(MPI)

MPI mpi :
	@$(MAKE) -f $(MAKEFILE) MPI=1 DEBUG=$(DEBUG) IDL=$(IDL)

$(HERMES_ARCH_LIST) :
	@$(MAKE) DEBUG="$(DEBUG)" PLATFORM=$@
endif

sys_check :
	@if [ ! -f LAST_MAKE ]; then $(MAKE) clobber; \
	 elif ! grep ^$(THIS_MAKE)$$ LAST_MAKE >/dev/null; then \
	   $(MAKE) clobber; fi
	@$(MAKE) depend
	@echo $(THIS_MAKE) > LAST_MAKE

depend :
	@$(MAKE) -f $(MAKEFILE) bld_macro_list
	@$(MAKE) -f $(MAKEFILE) $(DEP_LIST_FILE)

$(MACRO_LIST_FILE) bld_macro_list :
	@bld_macro_list $(BLD_SRC_EXTS) $(BLD_INC_EXTS) $(LGRPFLAGS) \
                        -SSRCS -IINCS -O "" -P $(PRODNAME)
	@bld_macro_list -nmain $(BLD_SRC_EXTS) $(LGRPFLAGS) -aP $(PRODNAME)
	@bld_macro_list -mmain $(BLD_SRC_EXTS) $(LGRPFLAGS) -OMAIN_OBJS -aP \
                        $(PRODNAME)
	@bld_macro_list -mmain $(BLD_SRC_EXTS) $(LGRPFLAGS) -OTARGETS -aEP \
                        $(PRODNAME)
	@bld_macro_list -mstandalone $(BLD_SRC_EXTS) $(LGRPFLAGS) -aEd \
                        -OSTANDALONE_TARGETS $(PRODNAME)

$(DEP_LIST_FILE) : $(SRCS) $(INCS)
	@bld_dep_list $(DEP_INCDIRS) $(FPPFLAGS) $(CPPFLAGS) $(BLD_SRC_EXTS) \
	              $(BLDDEPLST_ARGS) $(LGRPFLAGS) -e0 $(PRODNAME)

nmakedepend :
	@bld_macro_list -nmain $(BLD_SRC_EXTS) $(BLD_INC_EXTS) -WP \
	               $(WINLGRPFLAGS) -SSRCS -IINCS $(PRODNAME) .nmake
	@bld_macro_list -mmain $(BLD_SRC_EXTS) -OMAIN_OBJS -WPa \
	               $(WINLGRPFLAGS) $(PRODNAME) .nmake
	@bld_macro_list -mmain -nstandalone $(BLD_SRC_EXTS) -OTARGETS -EaWP \
	                $(WINLGRPFLAGS) $(PRODUCT) .nmake
	@bld_macro_list -mstandalone $(BLD_SRC_EXTS) -OSTANDALONE_TARGETS \
	                $(WINLGRPFLAGS) -EaWd $(PRODUCT) .nmake
	@bld_dep_list $(DEP_INCDIRS) $(FPPFLAGS) $(CPPFLAGS) $(BLD_SRC_EXTS) \
	             $(BLDDEPLST_ARGS) $(WINLGRPFLAGS) -W -e0 $(PRODUCT) .nmake

clean:;	rm -f *.o core

clobber:; rm -f  *.o core LAST_MAKE $(TARGETS) $(STANDALONE_TARGETS) mtestio

#
# ------------------ Include File Dependency Section --------------------
#
ifneq ($(strip $(wildcard $(DEP_LIST_FILE))),)
  include $(DEP_LIST_FILE)
endif
