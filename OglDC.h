/*
 *  $Id: OglDC.h,v 1.3 2005-02-16 07:46:44 ueshiba Exp $
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
	     u_int width=0, u_int height=0)				;
    virtual		~OglDC()					;
    
    virtual DC&	setSize(u_int width, u_int height,
				u_int mul,   u_int div)			;
    virtual DC3&	setInternal(int	   u0,	 int	v0,
				    double ku,	 double kv,
				    double near, double far)		;
    virtual DC3&	setExternal(const Vector<double>& t,
				    const Matrix<double>& Rt)		;
    virtual const DC3&	getInternal(int&    u0,	  int&	  v0,
				    double& ku,	  double& kv,
				    double& near, double& far)	const	;
    virtual const DC3&	getExternal(Vector<double>& t,
				    Matrix<double>& Rt)	const	;
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
