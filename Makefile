#
#  $Id: Makefile,v 1.12 2007-09-30 23:30:09 ueshiba Exp $
#
#################################
#  User customizable macros	#
#################################
DEST		= $(LIBDIR)
INCDIR		= $(HOME)/include/TU/v
INCDIRS		= -I$(HOME)/include -I$(X11HOME)/include

NAME		= $(shell basename $(PWD))

CPPFLAGS	= 
CFLAGS		= -O -g
CCFLAGS		= -O -g
ifeq ($(CCC), icpc)
  CCFLAGS	= -O3 -parallel
endif
LDFLAGS		= $(CCFLAGS)
LINKER		= $(CCC)

#########################
#  Macros set by mkmf	#
#########################
SUFFIX		= .cc:sC
EXTHDRS		= /Users/ueshiba/include/TU/Array++.h \
		/Users/ueshiba/include/TU/Geometry++.h \
		/Users/ueshiba/include/TU/Image++.cc \
		/Users/ueshiba/include/TU/Image++.h \
		/Users/ueshiba/include/TU/List++.h \
		/Users/ueshiba/include/TU/Manip.h \
		/Users/ueshiba/include/TU/Minimize++.h \
		/Users/ueshiba/include/TU/Vector++.h \
		/Users/ueshiba/include/TU/mmInstructions.h \
		/Users/ueshiba/include/TU/types.h \
		/Users/ueshiba/include/TU/utility.h \
		/Users/ueshiba/include/TU/v/CanvasPane.h \
		/Users/ueshiba/include/TU/v/CanvasPaneDC.h \
		/Users/ueshiba/include/TU/v/CanvasPaneDC3.h \
		/Users/ueshiba/include/TU/v/Colormap.h \
		/Users/ueshiba/include/TU/v/DC.h \
		/Users/ueshiba/include/TU/v/DC3.h \
		/Users/ueshiba/include/TU/v/Menu.h \
		/Users/ueshiba/include/TU/v/TUv++.h \
		/Users/ueshiba/include/TU/v/Widget-Xaw.h \
		/Users/ueshiba/include/TU/v/XDC.h \
		TU/v/OglDC.h
HDRS		= OglDC.h
SRCS		= OglDC.cc
OBJS		= OglDC.o

#########################
#  Macros used by RCS	#
#########################
REV		= $(shell echo $Revision: 1.12 $	|		\
		  sed 's/evision://'		|		\
		  awk -F"."					\
		  '{						\
		      for (count = 1; count < NF; count++)	\
			  printf("%d.", $$count);		\
		      printf("%d", $$count + 1);		\
		  }')

include $(PROJECT)/lib/l.mk
###
OglDC.o: TU/v/OglDC.h /Users/ueshiba/include/TU/v/CanvasPaneDC3.h \
	/Users/ueshiba/include/TU/v/CanvasPaneDC.h \
	/Users/ueshiba/include/TU/v/XDC.h /Users/ueshiba/include/TU/v/DC.h \
	/Users/ueshiba/include/TU/Geometry++.h \
	/Users/ueshiba/include/TU/utility.h \
	/Users/ueshiba/include/TU/Minimize++.h \
	/Users/ueshiba/include/TU/Vector++.h \
	/Users/ueshiba/include/TU/Array++.h /Users/ueshiba/include/TU/types.h \
	/Users/ueshiba/include/TU/Image++.h /Users/ueshiba/include/TU/Manip.h \
	/Users/ueshiba/include/TU/v/Colormap.h \
	/Users/ueshiba/include/TU/v/CanvasPane.h \
	/Users/ueshiba/include/TU/v/TUv++.h \
	/Users/ueshiba/include/TU/List++.h \
	/Users/ueshiba/include/TU/v/Widget-Xaw.h \
	/Users/ueshiba/include/TU/v/Menu.h /Users/ueshiba/include/TU/v/DC3.h \
	/Users/ueshiba/include/TU/Image++.cc \
	/Users/ueshiba/include/TU/mmInstructions.h
