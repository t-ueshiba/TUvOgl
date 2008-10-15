#
#  $Id: Makefile,v 1.22 2008-10-15 00:50:39 ueshiba Exp $
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
EXTHDRS		= /home/ueshiba/include/TU/Array++.h \
		/home/ueshiba/include/TU/Geometry++.h \
		/home/ueshiba/include/TU/Image++.h \
		/home/ueshiba/include/TU/List.h \
		/home/ueshiba/include/TU/Manip.h \
		/home/ueshiba/include/TU/Normalize.h \
		/home/ueshiba/include/TU/Vector++.h \
		/home/ueshiba/include/TU/types.h \
		/home/ueshiba/include/TU/v/CanvasPane.h \
		/home/ueshiba/include/TU/v/CanvasPaneDC.h \
		/home/ueshiba/include/TU/v/CanvasPaneDC3.h \
		/home/ueshiba/include/TU/v/Colormap.h \
		/home/ueshiba/include/TU/v/DC.h \
		/home/ueshiba/include/TU/v/DC3.h \
		/home/ueshiba/include/TU/v/Menu.h \
		/home/ueshiba/include/TU/v/TUv++.h \
		/home/ueshiba/include/TU/v/Widget-Xaw.h \
		/home/ueshiba/include/TU/v/XDC.h \
		TU/v/OglDC.h
HDRS		= OglDC.h
SRCS		= OglDC.cc
OBJS		= OglDC.o

#########################
#  Macros used by RCS	#
#########################
REV		= $(shell echo $Revision: 1.22 $	|		\
		  sed 's/evision://'		|		\
		  awk -F"."					\
		  '{						\
		      for (count = 1; count < NF; count++)	\
			  printf("%d.", $$count);		\
		      printf("%d", $$count + 1);		\
		  }')

include $(PROJECT)/lib/l.mk
###
OglDC.o: TU/v/OglDC.h /home/ueshiba/include/TU/v/CanvasPaneDC3.h \
	/home/ueshiba/include/TU/v/CanvasPaneDC.h \
	/home/ueshiba/include/TU/v/XDC.h /home/ueshiba/include/TU/v/DC.h \
	/home/ueshiba/include/TU/Geometry++.h \
	/home/ueshiba/include/TU/Vector++.h \
	/home/ueshiba/include/TU/Array++.h /home/ueshiba/include/TU/types.h \
	/home/ueshiba/include/TU/Normalize.h \
	/home/ueshiba/include/TU/Image++.h /home/ueshiba/include/TU/Manip.h \
	/home/ueshiba/include/TU/v/Colormap.h \
	/home/ueshiba/include/TU/v/CanvasPane.h \
	/home/ueshiba/include/TU/v/TUv++.h /home/ueshiba/include/TU/List.h \
	/home/ueshiba/include/TU/v/Widget-Xaw.h \
	/home/ueshiba/include/TU/v/Menu.h /home/ueshiba/include/TU/v/DC3.h
