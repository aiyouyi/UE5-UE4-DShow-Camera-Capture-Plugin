// Fill out your copyright notice in the Description page of Project Settings.


#include "LibYUVWrapper.h"

LibYUVWrapper::LibYUVWrapper()
{
}

LibYUVWrapper::~LibYUVWrapper()
{
}

int LibYUVWrapper::LibYUVConvertToARGB(const uint8_t* sample, size_t sample_size, uint8_t* dst_argb, int dst_stride_argb, int crop_x, int crop_y, int src_width, int src_height, int crop_width, int crop_height, libyuv::RotationMode rotation, uint32_t fourcc)
{
	return libyuv::ConvertToARGB(sample, sample_size, dst_argb, dst_stride_argb, crop_x, crop_y, src_width, src_height, crop_width, crop_height, rotation, fourcc);
}