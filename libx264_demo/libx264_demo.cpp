#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "x264.h"
}

#pragma comment(lib, "libx264.lib")

typedef unsigned char uchar;

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		printf("usage : app [input] [width] [height] [output]\n");
		return 0;
	}

	const char* input = argv[1];
	const char* out264 = argv[4];
	const int width = atoi(argv[2]);
	const int height = atoi(argv[3]);

	FILE *fin = 0, *fout = 0;
	fopen_s(&fin, input, "rb");
	fopen_s(&fout, out264, "wb");

	if (fin == 0 || fout == 0)
	{
		printf("openf file failed\n");
		return -1;
	}

	x264_param_t param;
	x264_param_default_preset(&param, "fast", "zerolatency");
	//x264_param_default(&param);

	param.i_width = width;
	param.i_height = height;
	param.i_bframe = 0;
	param.i_threads = 1;
	param.i_sync_lookahead = 0;
	//	param.b_deblocking_filter = 1;
	param.b_cabac = 1;
	param.i_fps_num = 25;
	param.i_fps_den = 1;
	param.i_level_idc = 30;
	param.i_keyint_min = param.i_fps_num;
	param.i_keyint_max = param.i_fps_num * 2;

	param.analyse.i_subpel_refine = 5;
	param.analyse.i_me_method = X264_ME_HEX;
	param.analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
	param.analyse.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;

	if ((param.analyse.inter | param.analyse.intra) & X264_ANALYSE_I8x8)
	{
		param.analyse.b_transform_8x8 = 1;
	}

	param.rc.i_lookahead = 0;
	param.rc.i_bitrate = 500;
	param.rc.i_vbv_max_bitrate = param.rc.i_bitrate;
	param.rc.i_vbv_buffer_size = param.rc.i_bitrate;
	param.rc.f_vbv_buffer_init = 0.7f;
	param.rc.b_mb_tree = 0;
	//	param.rc.i_qp_min = 2;
	//	param.rc.i_qp_max = 31;
	//	param.rc.f_qcompress = 0.5f;
	//	param.rc.i_qp_constant = 0;
	param.rc.i_rc_method = X264_RC_ABR;
	param.rc.f_rf_constant = 8.5f;
	param.rc.f_rf_constant_max = 20.0f;
	param.rc.f_rate_tolerance = 0.1f;
	param.rc.i_aq_mode = X264_AQ_AUTOVARIANCE;
	param.rc.f_aq_strength = 0.5f;
	param.b_repeat_headers = 0;
	//	param.b_annexb = 0;
#ifdef _DEBUG
	//	param.analyse.b_psnr = true;
	//	param.analyse.b_ssim = true;
	param.i_log_level = X264_LOG_INFO;
#endif

	x264_t* h264 = x264_encoder_open(&param);
	if (h264 == NULL)
	{
		printf("x264 open failed\n");
		return -1;
	}

	x264_picture_t pic_in;
	x264_picture_t pic_out;

	x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);
	x264_picture_init(&pic_out);

	int bufferSize = width * height * 3 / 2;
	uchar* buffer = (uchar*)malloc(bufferSize);

	pic_in.img.i_csp = X264_CSP_I420;
	pic_in.img.i_plane = 3;
	pic_in.img.plane[0] = buffer;
	pic_in.img.plane[1] = buffer + width * height;
	pic_in.img.plane[2] = buffer + width * height * 5 / 4;
	pic_in.img.i_stride[0] = width;
	pic_in.img.i_stride[1] = width / 2;
	pic_in.img.i_stride[2] = width / 2;
	pic_in.i_pts = 0;

	int frameCount = 0;
	int encodeFrameCount = 0;
	int frameSize = 0;
	int i_nal = 0;
	x264_nal_t* p_nal = NULL;

	//从头部信息里面获取PPS、SPS等
	x264_encoder_headers(h264, &p_nal, &i_nal);

	for (int i = 0; i < i_nal; i++)
	{
		fwrite(p_nal[i].p_payload, 1, p_nal[i].i_payload, fout);
	}

	while (!feof(fin))
	{
		int bytes = fread(buffer, 1, bufferSize, fin);
		if (bytes != bufferSize)
		{
			break;
		}

		if (frameCount++ % 8 == 0)
		{
			pic_in.i_type = X264_TYPE_IDR;
		}
		else
		{
			pic_in.i_type = X264_TYPE_AUTO;
		}

		frameSize = x264_encoder_encode(h264, &p_nal, &i_nal, &pic_in, &pic_out);

		if (frameSize < 0)
		{
			printf("x264 encode failed\n");
			break;
		}

		if (frameSize > 0)
		{
			for (int i = 0; i < i_nal; i++)
			{
				fwrite(p_nal[i].p_payload, 1, p_nal[i].i_payload, fout);
			}

			encodeFrameCount++;
		}

		pic_in.i_pts++;
	}

	while (frameSize = x264_encoder_encode(h264, &p_nal, &i_nal, NULL, &pic_out))
	{
		for (int i = 0; i < i_nal; i++)
		{
			fwrite(p_nal[i].p_payload, 1, p_nal[i].i_payload, fout);
		}

		encodeFrameCount++;
	}

	free(buffer);
	x264_encoder_close(h264);
	fclose(fin);
	fclose(fout);

	printf("read video frame = %d\n", frameCount);
	printf("encode video frame = %d\n", encodeFrameCount);

	system("pause");
	return 0;
}