/*
 *  $Id: createFormatMenu.cc,v 1.1 2009-07-28 00:00:48 ueshiba Exp $
 */
#include <vector>
#include <list>
#include <sstream>
#include <boost/foreach.hpp>
#include "TU/v/vV4L2++.h"

namespace TU
{
namespace v
{
/************************************************************************
*  global functions							*
************************************************************************/
MenuDef*
createFormatMenu(const V4L2Camera& camera)
{
    static std::vector<MenuDef>			pixelFormatMenus;
    static std::list<std::vector<MenuDef> >	frameSizeMenusList;
    static std::list<std::vector<std::string> >	frameSizeLabelsList;
    static const char*				setROILabel = "set ROI";
    
    BOOST_FOREACH (auto pixelFormat, camera.availablePixelFormats())
    {
      // この画素フォーマットがサポートする各フレームサイズに対応するメニュー項目を作る．
	frameSizeMenusList.push_back(std::vector<MenuDef>());
	auto&	frameSizeMenus = frameSizeMenusList.back();
	frameSizeLabelsList.push_back(std::vector<std::string>());
	auto&	frameSizeLabels = frameSizeLabelsList.back();
	BOOST_FOREACH (const auto& frameSize,
		       camera.availableFrameSizes(pixelFormat))
	{
	  // このフレームサイズに対応するメニュー項目を作る．
	    std::ostringstream	s;
	    s << frameSize;
	    frameSizeLabels.push_back(s.str());

	    frameSizeMenus.push_back(
		MenuDef(frameSizeLabels.back().c_str(),
			frameSizeMenus.size(),
			(camera.pixelFormat() == pixelFormat	  &&
			 frameSize.width.involves(camera.width()) &&
			 frameSize.height.involves(camera.height()))));
	}
	frameSizeMenus.push_back(MenuDef());	// End of MenuDef

      // この画素フォーマットに対応するメニュー項目を作る．
	pixelFormatMenus.push_back(MenuDef(camera.getName(pixelFormat).c_str(),
					   pixelFormat, true,
					   &frameSizeMenus.front()));
    }

  // ROIを指定する項目を作る．
    pixelFormatMenus.push_back(MenuDef(setROILabel,
				       V4L2Camera::UNKNOWN_PIXEL_FORMAT,
				       false));

    pixelFormatMenus.push_back(MenuDef());	// End of MenuDef


    return &pixelFormatMenus.front();
}
 
}	// namespace v
}	// namespace TU
