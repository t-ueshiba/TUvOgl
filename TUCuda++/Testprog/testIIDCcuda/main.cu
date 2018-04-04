/*
 *  $Id: main.cc,v 1.1 2012-08-30 00:13:51 ueshiba Exp $
 */
#include <cstdlib>
#include <signal.h>
#include <sys/time.h>
#include <iomanip>
#include <string>
#include <fstream>
#include <stdexcept>
#include "TU/IIDCCameraArray.h"
#include "TU/cuda/Array++.h"

namespace TU
{
namespace cuda
{
template <class T> void
interpolate(const Array2<T>& d_image0,
	    const Array2<T>& d_image1, Array2<T>& d_image2);
}

/************************************************************************
*  static functions							*
************************************************************************/
static bool	active = true;

static void
handler(int sig)
{
    using namespace	std;
    
    cerr << "Signal [" << sig << "] caught!" << endl;
    active = false;
}

template <class T> static void
doJob(IIDCCameraArray& cameras)
{
    using namespace	std;
    
  // Set signal handler.
    signal(SIGINT,  handler);
    signal(SIGPIPE, handler);

  // 1�ե졼�ढ����β������Ȥ��Υե����ޥåȤ���ϡ�
    Array<Image<T> >	images(cameras.size() + 1);
    cout << 'M' << images.size() << endl;
    for (int i = 0; i < images.size(); ++i)
    {
	images[i].resize(cameras[0].height(), cameras[0].width());
	images[i].saveHeader(cout);
    }

  // �ǥХ��������γ���
    cuda::Array2<T>	d_images[3];
    
  // �������Ϥγ��ϡ�
    for (size_t i = 0; i < cameras.size(); ++i)
	cameras[i].continuousShot(true);

    int		nframes = 0;
    timeval	start;
    while (active)
    {
	if (nframes == 10)
	{
	    timeval      end;
	    gettimeofday(&end, NULL);
	    double	interval = (end.tv_sec  - start.tv_sec) +
				   (end.tv_usec - start.tv_usec) / 1.0e6;
	    cerr << nframes / interval << " frames/sec" << endl;
	    nframes = 0;
	}
	if (nframes++ == 0)
	    gettimeofday(&start, NULL);

	for (size_t i = 0; i < cameras.size(); ++i)
	    cameras[i].snap();				// ����
	for (size_t i = 0; i < cameras.size(); ++i)
	    cameras[i] >> images[i];			// �絭���ؤ�ž��
	for (size_t i = 0; i < cameras.size(); ++i)
	    d_images[i] = images[i];			// �ǥХ����ؤ�ž��

	cuda::interpolate(d_images[0],
			  d_images[1], d_images[2]);	// CUDA�ˤ�����
	images[2] = d_images[2];			// �ۥ��Ȥؤ�ž��

	for (size_t i = 0; i < images.size(); ++i)
	    if (!images[i].saveData(cout))		// stdout�ؤν���
		active = false;
    }

  // �������Ϥ���ߡ�
    for (size_t i = 0; i < cameras.size(); ++i)
	cameras[i].continuousShot(false);
}

}
/************************************************************************
*  Global fucntions							*
************************************************************************/
int
main(int argc, char *argv[])
{
    using namespace	std;
    using namespace	TU;
    
    const char*		cameraName = IIDCCameraArray::DEFAULT_CAMERA_NAME;
    IIDCCamera::Speed	speed	   = IIDCCamera::SPD_400M;
    extern char*	optarg;
    for (int c; (c = getopt(argc, argv, "c:Bn:")) != EOF; )
	switch (c)
	{
	  case 'c':
	    cameraName = optarg;
	    break;
	  case 'B':
	    speed = IIDCCamera::SPD_800M;
	    break;
	}
    
    try
    {
      // IIDC�����Υ����ץ�
	IIDCCameraArray	cameras;
	cameras.restore(cameraName, speed);
	
	if (cameras.size() == 0)
	    return 0;

	for (size_t i = 0; i < cameras.size(); ++i)
	    cerr << "camera " << i << ": uniqId = "
		 << hex << setw(16) << setfill('0')
		 << cameras[i].globalUniqueId() << dec << endl;

      // �����Υ���ץ���Ƚ��ϡ�
	doJob<u_char>(cameras);
    }
    catch (exception& err)
    {
	cerr << err.what() << endl;
    }

    return 0;
}
