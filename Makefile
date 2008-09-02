#
#  $Id: Makefile,v 1.19 2008-09-02 05:15:32 ueshiba Exp $
#
#################################
#  User customizable macros	#
#################################
DEST		= $(LIBDIR)
INCDIR		= $(HOME)/include/TU/v
INCDIRS		= -I. -I$(HOME)/include -I$(X11HOME)/include

NAME		= $(shell basename $(PWD))

CPPFLAGS	=
CFLAGS		= -g
ifeq ($(CCC), icpc)
  CFLAGS	= -O3
  ifeq ($(OSTYPE), darwin)
    CPPFLAGS   += -DSSE3 -axP -ip -parallel
  else
    CPPFLAGS   += -DSSE2 -xN -ip
  endif
endif
CCFLAGS		= $(CFLAGS)

LINKER		= $(CCC)

#########################
#  Macros set by mkmf	#
#########################
SUFFIX		= .cc:sC
EXTHDRS		= TU/v/OglDC.h
HDRS		= OglDC.h
SRCS		= OglDC.cc
OBJS		= OglDC.o

#########################
#  Macros used by RCS	#
#########################
REV		= $(shell echo $Revision: 1.19 $	|		\
		  sed 's/evision://'		|		\
		  awk -F"."					\
		  '{						\
		      for (count = 1; count < NF; count++)	\
			  printf("%d.", $$count);		\
		      printf("%d", $$count + 1);		\
		  }')

include $(PROJECT)/lib/l.mk
###
OglDC.o: TU/v/OglDC.h
