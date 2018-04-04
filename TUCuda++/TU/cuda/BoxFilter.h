/*
 *  $Id: BoxFilter.h 1962 2016-03-22 02:56:32Z ueshiba $
 */
/*!
  \file		BoxFilter.h
  \brief	boxフィルタの定義と実装
*/ 
#ifndef TU_CUDA_BOXFILTER_H
#define TU_CUDA_BOXFILTER_H

#include "TU/Profiler.h"
#include "TU/cuda/Array++.h"
#include "TU/cuda/algorithm.h"

namespace TU
{
namespace cuda
{
#if defined(__NVCC__)
namespace device
{
/************************************************************************
*  __global__ functions							*
************************************************************************/
//! スレッドブロックの縦方向にフィルタを適用する
/*!
  sliding windowを使用するが threadIdx.y が0のスレッドしか仕事をしないので
  ウィンドウ幅が大きいときのみ高効率．また，結果は転置して格納される．
  \param in		入力2次元配列の左上隅を指す反復子
  \param colO		出力2次元配列の左上隅を指す反復子
  \param winSize	boxフィルタのウィンドウの行幅(高さ)
  \param strideI	入力2次元配列の行を1つ進めるためにインクリメントするべき要素数
  \param strideO	出力2次元配列の行を1つ進めるためにインクリメントするべき要素数
*/
template <class FILTER, class IN, class OUT>
__global__ void
box_filter(IN in, OUT out, int winSize, int strideI, int strideO)
{
    using value_type  =	typename FILTER::value_type;
    
    constexpr auto	BlockDim   = FILTER::BlockDim;
    constexpr auto	WinSizeMax = FILTER::WinSizeMax;
    
    __shared__ value_type	in_s[BlockDim + WinSizeMax - 1][BlockDim + 1];
    __shared__ value_type	out_s[BlockDim][BlockDim + 1];

    const auto	x0 = __mul24(blockIdx.x, blockDim.x);	// ブロック左上隅
    const auto	y0 = __mul24(blockIdx.y, blockDim.y);	// ブロック左上隅

    loadTileV(in + __mul24(y0, strideI) + x0, strideI, in_s, winSize - 1);
    __syncthreads();
    
    if (threadIdx.y == 0)
    {
      // 各列を並列に縦方向積算
	out_s[0][threadIdx.x] = in_s[0][threadIdx.x];
	for (int y = 1; y != winSize; ++y)
	    out_s[0][threadIdx.x] += in_s[y][threadIdx.x];

	for (int y = 1; y != blockDim.y; ++y)
	    out_s[y][threadIdx.x]
		= out_s[y-1][threadIdx.x]
		+ in_s[y-1+winSize][threadIdx.x] - in_s[y-1][threadIdx.x];
    }
    __syncthreads();

  // 結果を転置して格納
    if (blockDim.x == blockDim.y)
	out[__mul24(x0 + threadIdx.y, strideO) + y0 + threadIdx.x] =
	    out_s[threadIdx.x][threadIdx.y];
    else
	out[__mul24(x0 + threadIdx.x, strideO) + y0 + threadIdx.y] =
	    out_s[threadIdx.y][threadIdx.x];
}

template <class FILTER, class IN, class OUT, class OP> __global__ void
box_filterH(IN inL, IN inR, int ncols, OUT out, OP op, int winSizeH,
	    int strideL, int strideR, int strideXD, int strideD)
{
    using value_type  =	typename FILTER::value_type;
    
    constexpr auto	BlockDimX  = FILTER::BlockDimX;
    constexpr auto	BlockDimY  = FILTER::BlockDimY;
    constexpr auto	WinSizeMax = FILTER::WinSizeMax;

    __shared__ value_type	val_s[WinSizeMax][BlockDimY][BlockDimX + 1];

    const auto	d = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;	// 視差
    const auto	y = __mul24(blockIdx.y, blockDim.y) + threadIdx.y;	// 行

    inL += __mul24(y, strideL);
    inR += (__mul24(y, strideR)  + d);
    out += (__mul24(y, strideXD) + d);

  // 最初のwinSize画素分の相違度を計算してvalに積算
    auto	val = (val_s[0][threadIdx.y][threadIdx.x] = op(*inL, *inR));
    for (int i = 0; ++i != winSizeH; )
	val += (val_s[i][threadIdx.y][threadIdx.x] = op(*++inL, *++inR));
    *out = val;
    
  // 逐次的にvalを更新して出力
    for (int i = 0; --ncols; )
    {
	val -= val_s[i][threadIdx.y][threadIdx.x];
	*(out += strideD) = (val += (val_s[i][threadIdx.y][threadIdx.x]
				     = op(*++inL, *++inR)));
	if (++i == winSizeH)
	    i = 0;
    }
}

template <class FILTER, class IN> __global__ void
box_filterV(IN in, int nrows, int winSizeV, int strideXD, int strideD)
{
    using value_type  =	typename FILTER::value_type;
    
    constexpr auto	BlockDimX  = FILTER::BlockDimX;
    constexpr auto	BlockDimY  = FILTER::BlockDimY;
    constexpr auto	WinSizeMax = FILTER::WinSizeMax;

    __shared__ value_type	in_s[WinSizeMax][BlockDimY][BlockDimX + 1];

    const auto	d = __mul24(blockIdx.x, blockDim.x) + threadIdx.x;	// 視差
    const auto	x = __mul24(blockIdx.y, blockDim.y) + threadIdx.y;	// 列

    in += (__mul24(x, strideD) + d);

    auto	out = in;
    auto	val = (in_s[0][threadIdx.y][threadIdx.x] = *in);
    for (int i = 0; ++i < winSizeV; )
  	val += (in_s[i][threadIdx.y][threadIdx.x] = *(in += strideXD));
    *out = val;
    
    for (int i = 0; --nrows; )
    {
	val -= in_s[i][threadIdx.y][threadIdx.x];
	*(out += strideXD)
	    = (val += (in_s[i][threadIdx.y][threadIdx.x] = *(in += strideXD)));

	if (++i == winSizeV)
	    i = 0;
    }
}

template <class FILTER, class IN, class OUT> __global__ void
box_filterV(IN in, int nrows, OUT out, int winSizeV,
	    int strideXD, int strideD, int strideYX_O, int strideX_O)
{
    using value_type  =	typename FILTER::value_type;
    
    constexpr auto	BlockDim   = FILTER::BlockDim;
    constexpr auto	WinSizeMax = FILTER::WinSizeMax;

    __shared__ value_type	in_s[WinSizeMax][BlockDim][BlockDim + 1];
    __shared__ value_type	out_s[BlockDimY][BlockDimX + 1];

    const auto	d0 = __mul24(blockIdx.x, blockDim.x);	// 視差
    const auto	x0 = __mul24(blockIdx.y, blockDim.y);	// 列

    in  += (__mul24(x0 + threadIdx.y, strideD) + d0 + threadIdx.x);
    out += (__mul24(d0,		   strideYX_O) + x0);

    out_s[threadIdx.y][threadIdx.x] = (in_s[0][threadIdx.y][threadIdx.x] = *in);
    for (int i = 0; ++i < winSizeV; )
	out_s[threadIdx.y][threadIdx.x]
	    += (in_s[i][threadIdx.y][threadIdx.x] = *(in += strideXD));
    __syncthreads();
    if (blockDim.x == blockDim.y)
	out[__mul24(threadIdx.y, strideYX_O) + threadIdx.x]
	    = out_s[threadIdx.x][threadIdx.y];
    else
	out[__mul24(threadIdx.x, strideYX_O) + threadIdx.y]
	    = out_s[threadIdx.y][threadIdx.x];

    for (int i = 0; --nrows; )
    {
	out_s[threadIdx.y][threadIdx.x] -=  in_s[i][threadIdx.y][threadIdx.x];
	out_s[threadIdx.y][threadIdx.x] += (in_s[i][threadIdx.y][threadIdx.x]
					    = *(in += strideXD));
	__syncthreads();

	out += strideX_O;

	if (blockDim.x == blockDim.y)
	    out[__mul24(threadIdx.y, strideYX_O) + threadIdx.x]
		= out_s[threadIdx.x][threadIdx.y];
	else
	    out[__mul24(threadIdx.x, strideYX_O) + threadIdx.y]
		= out_s[threadIdx.y][threadIdx.x];

	if (++i == winSizeV)
	    i = 0;
    }
}
}	// namespace device
#endif	// __NVCC__
    
/************************************************************************
*  class BoxFilter2<T, CLOCK, WMAX>					*
************************************************************************/
//! CUDAによる2次元boxフィルタを表すクラス
template <class T, class CLOCK=void, size_t WMAX=23>
class BoxFilter2 : public Profiler<CLOCK>
{
  private:
    using profiler_t	= Profiler<CLOCK>;

  public:
    using value_type	= T;
    
    constexpr static size_t	WinSizeMax = WMAX;
    constexpr static size_t	BlockDim   = 16;
    constexpr static size_t	BlockDimX  = 32;
    constexpr static size_t	BlockDimY  = 16;
    
  public:
  //! CUDAによる2次元boxフィルタを生成する．
  /*!
    \param winSizeV	boxフィルタのウィンドウの行幅(高さ)
    \param winSizeH	boxフィルタのウィンドウの列幅(幅)
   */	
		BoxFilter2(size_t winSizeV, size_t winSizeH)
		    :profiler_t(3), _winSizeV(winSizeV), _winSizeH(winSizeH)
		{
		    if (_winSizeV > WinSizeMax || _winSizeH > WinSizeMax)
			throw std::runtime_error("Too large window size!");
		}

  //! boxフィルタのウィンドウの高さを返す．
  /*!
    \return	boxフィルタのウィンドウの高さ
   */
    size_t	winSizeV()		const	{ return _winSizeV; }

  //! boxフィルタのウィンドウの幅を返す．
  /*!
    \return	boxフィルタのウィンドウの幅
   */
    size_t	winSizeH()		const	{ return _winSizeH; }

  //! boxフィルタのウィンドウの高さを設定する．
  /*!
    \param winSizeV	boxフィルタのウィンドウの高さ
    \return		このboxフィルタ
   */
    BoxFilter2&	setWinSizeV(size_t winSize)
		{
		    _winSizeV = winSize;
		    return *this;
		}

  //! boxフィルタのウィンドウの幅を設定する．
  /*!
    \param winSizeH	boxフィルタのウィンドウの幅
    \return		このboxフィルタ
   */
    BoxFilter2&	setWinSizeH(size_t winSize)
		{
		    _winSizeH = winSize;
		    return *this;
		}
    
  //! 与えられた高さを持つ入力データ列に対する出力データ列の高さを返す．
  /*!
    \param inSizeV	入力データ列の高さ
    \return		出力データ列の高さ
   */
    size_t	outSizeV(size_t inSize)	const	{return inSize + 1 - _winSizeV;}
    
  //! 与えられた幅を持つ入力データ列に対する出力データ列の幅を返す．
  /*!
    \param inSizeH	入力データ列の幅
    \return		出力データ列の幅
   */
    size_t	outSizeH(size_t inSize)	const	{return inSize + 1 - _winSizeH;}

  //! 与えられた高さを持つ入力データ列に対する出力データ列の高さを返す．
  /*!
    \param inSizeV	入力データ列の高さ
    \return		出力データ列の高さ
   */
    size_t	offsetV()		const	{return _winSizeV/2;}
    
  //! 与えられた幅を持つ入力データ列に対する出力データ列の幅を返す．
  /*!
    \param inSizeH	入力データ列の幅
    \return		出力データ列の幅
   */
    size_t	offsetH()		const	{return _winSizeH/2;}

  //! 与えられた2次元配列とこのフィルタの畳み込みを行う
  /*!
    \param row	入力2次元データ配列の先頭行を指す反復子
    \param rowe	入力2次元データ配列の末尾の次の行を指す反復子
    \param rowO	出力2次元データ配列の先頭行を指す反復子
  */
    template <class ROW, class ROW_O>
    void	convolve(ROW row, ROW rowe,
			 ROW_O rowO, bool shift=false)		const	;

  //! 2組の2次元配列間の相違度とこのフィルタの畳み込みを行う
  /*!
    \param rowL			左入力2次元データ配列の先頭行を指す反復子
    \param rowLe		左入力2次元データ配列の末尾の次の行を指す反復子
    \param rowR			右入力2次元データ配列の先頭行を指す反復子
    \param rowO			出力2次元相違度配列の先頭行を指す反復子
    \param op			左右の画素間の相違度
    \param disparitySearchWidth	視差の探索幅
  */
    template <class ROW, class ROW_O, class OP>
    void	convolve(ROW rowL, ROW rowLe, ROW rowR, ROW_O rowO,
			 OP op, size_t disparitySearchWidth)	const	;
    
  private:
    size_t		_winSizeV;
    size_t		_winSizeH;
    mutable Array2<T>	_buf;
    mutable Array3<T>	_buf3;
};

#if defined(__NVCC__)
template <class T, class CLOCK, size_t WMAX>
template <class ROW, class ROW_O> void
BoxFilter2<T, CLOCK, WMAX>::convolve(ROW row, ROW rowe,
				     ROW_O rowO, bool shift) const
{
    profiler_t::start(0);
    
    auto	nrows = std::distance(row, rowe);
    if (nrows < _winSizeV)
	return;
    
    auto	ncols = std::distance(std::cbegin(*row), std::cend(*row));
    if (ncols < _winSizeH)
	return;

    ncols = outSizeH(ncols);
    _buf.resize(ncols, nrows);

    const auto	strideI = stride(row);
    const auto	strideO = stride(rowO);
    const auto	strideB = _buf.stride();

  // ---- 縦方向積算 ----
    profiler_t::start(1);
  // 左上
    dim3	threads(BlockDim, BlockDim);
    dim3	blocks(ncols/threads.x, nrows/threads.y);
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(*row),
						std::begin(_buf[0]),
						_winSizeV,
						strideI, strideB);
  // 右上
    const auto	x = blocks.x*threads.x;
    threads.x = ncols - x;
    blocks.x  = 1;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(*row) + x,
						std::begin(_buf[x]),
						_winSizeV,
						strideI, strideB);
  // 左下
    auto	y = blocks.y*threads.y;
    std::advance(row, y);
    threads.x = BlockDim;
    blocks.x  = ncols/threads.x;
    threads.y = nrows - y;
    blocks.y  = 1;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(*row),
						std::begin(_buf[0]) + y,
						_winSizeV,
						strideI, strideB);
  // 右下
    threads.x = ncols - x;
    blocks.x  = 1;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(*row) + x,
						std::begin(_buf[x]) + y,
						_winSizeV,
						strideI, strideB);

  // ---- 横方向積算 ----
    size_t	dx = 0;
    if (shift)
    {
	rowO += offsetV();
	dx    = offsetH();
    }
    
    cudaThreadSynchronize();
    profiler_t::start(2);
    nrows = outSizeH(nrows);
  // 左上
    threads.x = BlockDim;
    blocks.x  = nrows/threads.x;
    threads.y = BlockDim;
    blocks.y  = ncols/threads.y;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(_buf[0]),
						std::begin(*rowO) + dx,
						_winSizeH,
						strideB, strideO);
  // 左下
    threads.y = ncols - x;
    blocks.y  = 1;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(_buf[x]),
						std::begin(*rowO) + x + dx,
						_winSizeH,
						strideB, strideO);
  // 右上
    y	      = blocks.x*threads.x;
    std::advance(rowO, y);
    threads.x = nrows - y;
    blocks.x  = 1;
    threads.y = BlockDim;
    blocks.y  = ncols/threads.y;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(_buf[0]) + y,
						std::begin(*rowO) + dx,
						_winSizeH,
						strideB, strideO);
  // 右下
    threads.y = ncols - x;
    blocks.y  = 1;
    device::box_filter<BoxFilter2><<<blocks, threads>>>(
						std::cbegin(_buf[x]) + y,
						std::begin(*rowO) + x + dx,
						_winSizeH,
						strideB, strideO);
    cudaThreadSynchronize();
    profiler_t::nextFrame();
}

template <class T, class CLOCK, size_t WMAX>
template <class ROW, class ROW_O, class OP> void
BoxFilter2<T, CLOCK, WMAX>::convolve(ROW rowL, ROW rowLe, ROW rowR, ROW_O rowO,
				     OP op, size_t disparitySearchWidth) const
{
    profiler_t::start(0);
    
    auto	nrows = std::distance(rowL, rowLe);
    if (nrows < _winSizeV)
	return;
    
    auto	ncols = std::distance(std::cbegin(*rowL), std::cend(*rowL));
    if (ncols < _winSizeH)
	return;

    _buf3.resize(nrows, ncols, disparitySearchWidth);
    
    const auto	strideL    = stride(rowL);
    const auto	strideR    = stride(rowR);
    const auto	strideD	   = _buf3.stride();
    const auto	strideXD   = _buf3.size<1>()*strideD;
    const auto	strideX_O  = stride(std::cbegin(*rowO));
    const auto	strideYX_O = nrows*strideX_O;

  // ---- 横方向積算 ----
    profiler_t::start(1);
    ncols = outSizeH(ncols);
  // 視差左半かつ画像上半
    dim3	threads(BlockDimX, BlockDimY);
    dim3	blocks(disparitySearchWidth/threads.x, nrows/threads.y);
    device::box_filterH<BoxFilter2><<<blocks, threads>>>(
				std::cbegin(*rowL),
				std::cbegin(*rowR),
				ncols,
#ifdef DISPARITY_MAJOR
				std::begin(_buf3[0][0]),
#else
				std::begin(*std::begin(*rowO)),
#endif
				op, _winSizeH,
				strideL, strideR, strideXD, strideD);
  // 視差右半かつ画像上半
    const auto	d = blocks.x*threads.x;
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterH<BoxFilter2><<<blocks, threads>>>(
				std::cbegin(*rowL),
				std::cbegin(*rowR) + d,
				ncols,
#ifdef DISPARITY_MAJOR
				std::begin(_buf3[0][0]) + d,
#else
				std::begin(*std::begin(*rowO)) + d,
#endif
				op, _winSizeH,
				strideL, strideR, strideXD, strideD);
  // 視差左半かつ画像下半
    threads.x = BlockDimX;
    blocks.x  = disparitySearchWidth/threads.x;
    const auto	y = blocks.y*threads.y;
    threads.y = disparitySearchWidth - d;
    blocks.y  = 1;
    device::box_filterH<BoxFilter2><<<blocks, threads>>>(
				std::cbegin(*(rowL + y)),
				std::cbegin(*(rowR + y)),
				ncols,
#ifdef DISPARITY_MAJOR
				std::begin(_buf3[y][0]),
#else
				std::begin(*std::begin(*(rowO + y))),
#endif
				op, _winSizeH,
				strideL, strideR, strideXD, strideD);
  // 視差右半かつ画像下半
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterH<BoxFilter2><<<blocks, threads>>>(
				std::cbegin(*(rowL + y)) + d,
				std::cbegin(*(rowR + y)) + d,
				ncols,
#ifdef DISPARITY_MAJOR
				std::begin(_buf3[y][0]) + d,
#else
				std::begin(*std::begin(*(rowO + y))) + d,
#endif
				op, _winSizeH,
				strideL, strideR, strideXD, strideD);
  // ---- 縦方向積算 ----
    cudaThreadSynchronize();
    profiler_t::start(2);
#ifdef DISPARITY_MAJOR
  // 視差左半かつ画像左半
    nrows     = outSizeV(nrows);
    threads.x = BlockDim;
    blocks.x  = disparitySearchWidth/threads.x;
    threads.y = BlockDim;
    blocks.y  = ncols/threads.y;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(_buf3[0][0]),
				nrows,
				std::begin(*std::begin(*rowO)),
				_winSizeV,
				strideXD, strideD, strideYX_O, strideX_O);
  // 視差右半かつ画像左半
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(_buf3[0][0]) + d,
				nrows,
				std::begin(*std::begin(*(rowO + d))),
				_winSizeV,
				strideXD, strideD, strideYX_O, strideX_O);
  // 視差左半かつ画像右半
    threads.x = BlockDim;
    blocks.x  = disparitySearchWidth/threads.x;
    const auto	x = blocks.y*threads.y;
    threads.y = ncols - x;
    blocks.y  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(_buf3[0][x]),
				nrows,
				std::begin(*std::begin(*rowO)) + x,
				_winSizeV,
				strideXD, strideD, strideYX_O, strideX_O);
  // 視差右半かつ画像右半
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(_buf3[0][x]) + d,
				nrows,
				std::begin(*std::begin(*(rowO + d))) + x,
				_winSizeV,
				strideXD, strideD, strideYX_O, strideX_O);
#else
  // 視差左半かつ画像左半
    nrows     = outSizeV(nrows);
    threads.x = BlockDimX;
    blocks.x  = disparitySearchWidth/threads.x;
    threads.y = BlockDimY;
    blocks.y  = ncols/threads.y;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(*std::begin(*rowO)),
				nrows, _winSizeV, strideXD, strideD);
  // 視差右半かつ画像左半
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(*std::begin(*(rowO + d))),
				nrows, _winSizeV, strideXD, strideD);
  // 視差左半かつ画像右半
    threads.x = BlockDimX;
    blocks.x  = disparitySearchWidth/threads.x;
    const auto	x = blocks.y*threads.y;
    threads.y = ncols - x;
    blocks.y  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(*(std::begin(*rowO) + x)),
				nrows, _winSizeV, strideXD, strideD);
  // 視差右半かつ画像右半
    threads.x = disparitySearchWidth - d;
    blocks.x  = 1;
    device::box_filterV<BoxFilter2><<<blocks, threads>>>(
				std::begin(*(std::begin(*rowO) + x)) + d,
				nrows, _winSizeV, strideXD, strideD);
#endif
    cudaThreadSynchronize();
    profiler_t::nextFrame();
}
#endif	// __NVCC__
}	// namespace cuda
}	// namespace TU
#endif	// !TU_CUDA_BOXFILTER_H
