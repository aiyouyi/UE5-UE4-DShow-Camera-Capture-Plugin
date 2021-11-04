// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "libyuv/libyuv.h"
/**
 * 
 */
class LIBYUV_API LibYUVWrapper
{
public:
	LibYUVWrapper();
	~LibYUVWrapper();

	static int LibYUVConvertToARGB(const uint8_t* sample,
		size_t sample_size,
		uint8_t* dst_argb,
		int dst_stride_argb,
		int crop_x,
		int crop_y,
		int src_width,
		int src_height,
		int crop_width,
		int crop_height,
		libyuv::RotationMode rotation,
		uint32_t fourcc);
};