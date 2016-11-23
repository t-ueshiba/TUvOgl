/*
 *  平成9-19年（独）産業技術総合研究所 著作権所有
 *  
 *  創作者：植芝俊夫
 *
 *  本プログラムは（独）産業技術総合研究所の職員である植芝俊夫が創作し，
 *  （独）産業技術総合研究所が著作権を所有する秘密情報です．創作者によ
 *  る許可なしに本プログラムを使用，複製，改変，第三者へ開示する等の著
 *  作権を侵害する行為を禁止します．
 *  
 *  このプログラムによって生じるいかなる損害に対しても，著作権所有者お
 *  よび創作者は責任を負いません。
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
 *  $Id: OglDC.h,v 1.1 2012-09-15 05:02:27 ueshiba Exp $
 */
#ifndef __TU_V_OGLDC_H
#define __TU_V_OGLDC_H

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
	  size_t width=0, size_t height=0, float zoom=1)		;
    virtual		~OglDC()					;
    
    virtual DC&	setSize(size_t width, size_t height, float zoom)	;
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
#endif	// !__TU_V_OGLDC_H
