/*
 *  $Id$
 */
#ifndef TU_V_VV4L2PP_H
#define TU_V_VV4L2PP_H

#include "TU/v/CmdPane.h"
#include "TU/V4L2CameraArray.h"

namespace TU
{
namespace v
{
/************************************************************************
*  global functions							*
************************************************************************/
MenuDef*	createFormatMenu(const V4L2Camera& camera)		;
bool		selectROI(V4L2Camera& camera, u_int id,
			  size_t& u0, size_t& v0,
			  size_t& width, size_t& height, Window& window);
CmdDef*		createFeatureCmds(const V4L2Camera& camera,
				  size_t ncameras=1)			;
void		refreshFeatureCmds(const V4L2Camera& camera,
				   CmdPane& cmdPane)			;
void		refreshSliderCmd(const V4L2Camera& camera,
				 CmdId id, CmdPane& cmdPane)		;

template <class CAMERAS> auto
setFormat(CAMERAS&& cameras, CmdId id, CmdVal val, Window& window)
    -> typename std::enable_if<
	  std::is_convertible<
	      typename std::remove_reference<
		  decltype(*std::begin(cameras))>::type, V4L2Camera>::value,
	      bool>::type
{
    if (id == V4L2Camera::UNKNOWN_PIXEL_FORMAT)
    {
	if (TU::size(cameras))
	{
	    size_t	u0, v0, width, height;
	    selectROI(*std::begin(cameras), id, u0, v0, width, height, window);

	    for (auto& camera : cameras)
		camera.setROI(u0, v0, width, height);
	}

	return true;
    }
    else
	return setFormat(cameras, id, val);
}

inline bool
setFormat(V4L2Camera& camera, CmdId id, CmdVal val, Window& window)
{
    return setFormat(make_range(&camera, 1), id, val, window);
}

template <class CAMERAS> auto
refreshFeatureCmds(const CAMERAS& cameras, CmdPane& cmdPane)
    -> typename std::enable_if<std::is_convertible<
				   decltype(*std::begin(cameras)),
				   const V4L2Camera&>::value>::type
{
    auto	camera = std::begin(cameras);
    
    switch (TU::size(cameras))
    {
      case 0:
	return;
      case 1:
	break;
      default:
	std::advance(camera, cmdPane.getValue(V4L2CAMERA_CHOICE));
	break;
    }
    
    refreshFeatureCmds(*camera, cmdPane);
}
    
template <class CAMERAS> auto
setFeature(CAMERAS&& cameras, CmdId id, CmdVal val, CmdPane& cmdPane)
    -> typename std::enable_if<
	  std::is_convertible<
	      typename std::remove_reference<
		  decltype(*std::begin(cameras))>::type, V4L2Camera>::value,
	      bool>::type
{
    if (id == V4L2CAMERA_CHOICE)		// 選択カメラが変更されたら...
    {
	auto	camera = std::begin(cameras);
	std::advance(camera, val);
	refreshFeatureCmds(*camera, cmdPane);	// カメラの全属性をGUIに反映
	return true;
    }

    if (TU::size(cameras) > 1 &&		// カメラが複数かつ
	!cmdPane.getValue(V4L2CAMERA_ALL))	// 全カメラ操作モードでなければ...
    {
	auto	camera = std::begin(cameras);
	std::advance(camera, cmdPane.getValue(V4L2CAMERA_CHOICE));
	return setFeature(*camera, id, int(val));   // 選択カメラの属性設定
    }
    else
	return setFeature(cameras, id, int(val));   // 全カメラの属性設定
}

inline bool
setFeature(V4L2Camera& camera, CmdId id, CmdVal val, CmdPane& cmdPane)
{
    return setFeature(make_range(&camera, 1), id, val, cmdPane);
}

}	// namespace v
}	// namespace TU
#endif	// !TU_V_V4L2PP_H
