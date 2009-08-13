/*
 *  $BJ?@.(B9-19$BG/!JFH!K;:6H5;=QAm9g8&5f=j(B $BCx:n8"=jM-(B
 *  
 *  $BAO:n<T!'?"<G=SIW(B
 *
 *  $BK\%W%m%0%i%`$O!JFH!K;:6H5;=QAm9g8&5f=j$N?&0w$G$"$k?"<G=SIW$,AO:n$7!$(B
 *  $B!JFH!K;:6H5;=QAm9g8&5f=j$,Cx:n8"$r=jM-$9$kHkL)>pJs$G$9!%AO:n<T$K$h(B
 *  $B$k5v2D$J$7$KK\%W%m%0%i%`$r;HMQ!$J#@=!$2~JQ!$Bh;0<T$X3+<($9$kEy$NCx(B
 *  $B:n8"$r?/32$9$k9T0Y$r6X;_$7$^$9!%(B
 *  
 *  $B$3$N%W%m%0%i%`$K$h$C$F@8$8$k$$$+$J$kB;32$KBP$7$F$b!$Cx:n8"=jM-<T$*(B
 *  $B$h$SAO:n<T$O@UG$$rIi$$$^$;$s!#(B
 *
 *  Copyright 1997-2007.
 *  National Institute of Advanced Industrial Science and Technology (AIST)
 *
 *  Creator: Toshio UESHIBA
 *
 *  [AIST Confidential and all rights reserved.]
 *  This program is confidential. Any using, copying, changing or
 *  giving any information concerning with this program to others
 *  without permission by the creator are strictly prohibited.
 *
 *  [No Warranty.]
 *  The copyright holders or the creator are not responsible for any
 *  damages in the use of this program.
 *  
 *  $Id: OglDC.h,v 1.7 2009-08-13 23:03:09 ueshiba Exp $
 */
#ifndef __TUvOglDC_h
#define __TUvOglDC_h

#include "TU/v/CanvasPaneDC3.h"
#include <GL/glx.h>
#include <GL/glu.h>

namespace TU
{
namespace v
{
/************************************************************************
*  class OglDC								*
************************************************************************/
class OglDC : public CanvasPaneDC3
{
  public:
    OglDC(CanvasPane& parentCanvasPane,
	  u_int width=0, u_int height=0, u_int mul=1, u_int div=1)	;
    virtual		~OglDC()					;
    
    virtual DC&	setSize(u_int width, u_int height,
			u_int mul,   u_int div)				;
    virtual DC3&	setInternal(int	   u0,	 int	v0,
				    double ku,	 double kv,
				    double near, double far=0.0)	;
    virtual DC3&	setExternal(const Point3d& t,
				    const Matrix33d& Rt)		;
    virtual const DC3&	getInternal(int&    u0,	  int&	  v0,
				    double& ku,	  double& kv,
				    double& near, double& far)	const	;
    virtual const DC3&	getExternal(Point3d& t, Matrix33d& Rt)	const	;
    virtual DC3&	translate(double d)				;
    virtual DC3&	rotate(double angle)				;

    GLUnurbsObj*	nurbsRenderer()		{return _nurbsRenderer;}

    void		swapBuffers()				const	;
    template <class T>
    Image<T>		getImage()				const	;
    
  protected:
    virtual void	initializeGraphics()				;

  private:
    OglDC&		setViewport()					;
    void		makeCurrent()				const	;
    
    GLXContext		_ctx;			// rendering context
    GLUnurbsObj* const	_nurbsRenderer;		// renderer of NURBS
};

inline void
OglDC::makeCurrent() const
{
    glXMakeCurrent(colormap().display(), drawable(), _ctx);
}
 
}
}
#endif	// !__TUvOglDC_h
