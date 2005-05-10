#
#  $Id: Makefile,v 1.4 2005-05-10 02:51:14 ueshiba Exp $
#
#################################
#  User customizable macros	#
#################################
DEST		= $(BINDIR)
INCDIRS		= -I$(HOME)/include -I$(X11HOME)/include

PROGRAM		= ogltest
LIBS		= -lTUOgl++ -lTUv++ -lTUTools++ \
		  -lGLU -lGL -lXaw3d -lXt -lXext -lXmu -lX11

CPPFLAGS	= -DUseXaw
CFLAGS		= -g
CCFLAGS		= -g
LDFLAGS		= -L$(LIBDIR) -L$(X11HOME)/lib
LINKER		= $(CCC)
ifeq ($(CCC), icc)
#  CCFLAGS	= -cxxlib-icc -O3 -parallel
  CCFLAGS	= -cxxlib-icc -g
  LDFLAGS      += -cxxlib-icc
  LINKER	= icpc
endif

#########################
#  Macros set by mkmf	#
#########################
SUFFIX		= .cc:sC
EXTHDRS		= /home/ueshiba/include/TU/Array++.h \
		/home/ueshiba/include/TU/Geometry++.h \
		/home/ueshiba/include/TU/Image++.h \
		/home/ueshiba/include/TU/List++.h \
		/home/ueshiba/include/TU/Manip.h \
		/home/ueshiba/include/TU/Vector++.h \
		/home/ueshiba/include/TU/types.h \
		/home/ueshiba/include/TU/v/App.h \
		/home/ueshiba/include/TU/v/CanvasPane.h \
		/home/ueshiba/include/TU/v/CanvasPaneDC.h \
		/home/ueshiba/include/TU/v/CanvasPaneDC3.h \
		/home/ueshiba/include/TU/v/CmdPane.h \
		/home/ueshiba/include/TU/v/CmdWindow.h \
		/home/ueshiba/include/TU/v/Colormap.h \
		/home/ueshiba/include/TU/v/DC.h \
		/home/ueshiba/include/TU/v/DC3.h \
		/home/ueshiba/include/TU/v/Menu.h \
		/home/ueshiba/include/TU/v/OglDC.h \
		/home/ueshiba/include/TU/v/TUv++.h \
		/home/ueshiba/include/TU/v/Widget-Xaw.h \
		/home/ueshiba/include/TU/v/XDC.h
HDRS		=
SRCS		= main.cc
OBJS		= main.o

#########################
#  Macros used by RCS	#
#########################
REV		= $(shell echo $Revision: 1.4 $	|		\
		  sed 's/evision://'		|		\
		  awk -F"."					\
		  '{						\
		      for (count = 1; count < NF; count++)	\
			  printf("%d.", $$count);		\
		      printf("%d", $$count + 1);		\
		  }')

include $(PROJECT)/lib/p.mk
###
main.o: /home/ueshiba/include/TU/v/App.h \
	/home/ueshiba/include/TU/v/TUv++.h \
	/home/ueshiba/include/TU/v/Colormap.h \
	/home/ueshiba/include/TU/Image++.h \
	/home/ueshiba/include/TU/Vector++.h \
	/home/ueshiba/include/TU/Array++.h /home/ueshiba/include/TU/types.h \
	/home/ueshiba/include/TU/List++.h \
	/home/ueshiba/include/TU/Geometry++.h \
	/home/ueshiba/include/TU/v/Widget-Xaw.h \
	/home/ueshiba/include/TU/v/CmdPane.h \
	/home/ueshiba/include/TU/v/CmdWindow.h \
	/home/ueshiba/include/TU/v/OglDC.h \
	/home/ueshiba/include/TU/v/CanvasPaneDC3.h \
	/home/ueshiba/include/TU/v/CanvasPaneDC.h \
	/home/ueshiba/include/TU/v/XDC.h /home/ueshiba/include/TU/v/DC.h \
	/home/ueshiba/include/TU/Manip.h \
	/home/ueshiba/include/TU/v/CanvasPane.h \
	/home/ueshiba/include/TU/v/Menu.h /home/ueshiba/include/TU/v/DC3.h
