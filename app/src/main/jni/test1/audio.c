/*=========================================================================\
 * * Copyright(C)2016 Chudai.
*
* File name    : audio.c
* Version      : v1.0.0
* Author       : 初代
* Date         : 2016/10/06
* Description  :
* Function list: 1.
*                2.
*                3.
* History      :
\*=========================================================================*/

/*-----------------------------------------------------------*
 * 头文件                                                    *
 *-----------------------------------------------------------*/
#define __STDC_CONSTANT_MACROS      //ffmpeg要求


#include "player.h"
#include "audio.h"
#include <android/log.h>
//使用NDK的log
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"ERROR: ", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"INFO: ", __VA_ARGS__)
/*======================================================================\
* Author     (作者): i.sshe
* Date       (日期): 2016/10/06
* Others     (其他): 准备audio
\*=======================================================================*/
int prepare_audio(PlayerState *ps)
{

	ps->paudio_stream = ps->pformat_ctx->streams[ps->audio_stream_index];
	ps->paudio_codec_ctx = 	ps->paudio_stream->codec;
	ps->paudio_codec = avcodec_find_decoder(ps->paudio_codec_ctx->codec_id);
	if (ps->paudio_codec == NULL)
    {
        LOGE("Couldn't find audio decoder!!!!!!!\n");
         return -1;
    }
	//初始化AVCondecContext，以及进行一些处理工作。
	avcodec_open2(ps->paudio_codec_ctx, ps->paudio_codec, NULL);
    return 0;
}


/*======================================================================\
* Author     (作者): i.sshe
* Date       (日期): 2016/10/06
* Others     (其他): 播放声音
\*=======================================================================*/
int play_audio(PlayerState *ps)
{
    SDL_AudioSpec      wanted_spec;
//    SDL_AudioSpec      spec;
    wanted_spec.freq      = ps->paudio_codec_ctx->sample_rate;
    wanted_spec.format    = AUDIO_S16SYS;
    wanted_spec.channels  = ps->paudio_codec_ctx->channels;
    wanted_spec.silence   = 0;
    wanted_spec.samples   = 1024;     //
    wanted_spec.callback  = audio_callback;
    wanted_spec.userdata  = ps; // ps->paudio_codec_ctx;

    //打开音频设备
    //运行callback
    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        LOGE("Couldn't open audio device\n");
        return -1;
    }

    SDL_PauseAudio(0);

    return 0;
}


/*======================================================================\
* Author     (作者): i.sshe
* Date       (日期): 2016/10/06
* Others     (其他): 音频回调函数，打开设备的时候会开线程调用
\*=======================================================================*/
void audio_callback(void *userdata, uint8_t *stream, int len)
{
    PlayerState *ps = (PlayerState *)userdata;
    int 		send_data_size = 0;
    int 		audio_size = 0;

	if (ps->quit == 1)
	{
		exit(-1);
	}


    SDL_memset(stream, 0, len);


    while(len > 0)
    {
         if (ps->audio_buf_index >= ps->audio_buf_size)
         {
             //数据已经全部发送，再去取
             audio_size = audio_decode_frame(ps);

             if (audio_size < 0) {
                  //错误则静音一下
                  ps->audio_buf_size = 1024;
                  memset(ps->audio_buf, 0, ps->audio_buf_size); //
             }
             else
             {
                 ps->audio_buf_size = audio_size;       //解码这么多
             }
             //回到缓冲区头部
             ps->audio_buf_index = 0;
         }

         send_data_size = ps->audio_buf_size - ps->audio_buf_index;
         if (len < send_data_size)
         {
             send_data_size = len;
         }

         //第2个参数也有写len的，不过觉得发实际的对一些吧？
         //再看
         SDL_MixAudio(stream,
         			(uint8_t *)ps->audio_buf + ps->audio_buf_index,
                 	send_data_size, SDL_MIX_MAXVOLUME);

         len -= send_data_size;
         stream += send_data_size;
         ps->audio_buf_index += send_data_size;
    }
}


//对于音频来说，一个packet里面，可能含有多帧(frame)数据。
int audio_decode_frame(PlayerState *ps)
{
     uint8_t 			*audio_buf = ps->audio_buf;
     AVPacket           packet;
     AVFrame            *pframe;
     enum AVSampleFormat     dst_sample_fmt;
     uint64_t           dst_channel_layout;
     uint64_t           dst_nb_samples;
     int                convert_len;
     SwrContext 		*swr_ctx = swr_alloc();
     int                data_size;
	 int 				ret = 0;
     pframe = av_frame_alloc();

     if (ps->quit == 1)
         return -1;

     if (packet_queue_get(&ps->audio_packet_queue, &packet, 1) < 0)
     {
          LOGE("Get queue packet error\n");
          return -1;
     }
     //获取音频当前时间
     if (packet.pts != AV_NOPTS_VALUE)
     {
          ps->audio_clock = packet.pts * av_q2d(ps->paudio_stream->time_base);
     }

     ret = avcodec_send_packet(ps->paudio_codec_ctx, &packet);
     if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
     {
          LOGE( "send decode packet error\n");
          return -1;
     }

     ret = avcodec_receive_frame(ps->paudio_codec_ctx, pframe);
     if (ret < 0 && ret != AVERROR_EOF)
     {
          LOGE( "receive decode frame error\n");
          return -1;
     }

	 if (packet.pts != AV_NOPTS_VALUE)
	 {
	 	 //ps->audio_clock = packet.pts
	 		//	*av_q2d(ps->paudio_stream->time_base);
         //LOGI( "音频里：ps->audio_clock = %lf\n", ps->audio_clock);
	 }

     //设置通道数和channel_layout
     if (pframe->channels > 0 && pframe->channel_layout == 0)
     {
         pframe->channel_layout = av_get_default_channel_layout(pframe->channels);
     }
     else if (pframe->channels == 0 && pframe->channel_layout > 0)
     {
          pframe->channels = av_get_channel_layout_nb_channels(pframe->channel_layout);
     }


	 //以下设置转换参数并转换,设置参数应该只设置一次就可以了，再实验
     dst_sample_fmt     = AV_SAMPLE_FMT_S16;
     dst_channel_layout = (uint64_t) av_get_default_channel_layout(pframe->channels);

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = ps->paudio_codec_ctx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = ps->paudio_codec_ctx->sample_rate;
    //输出采样率
    int out_sample_rate = in_sample_rate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = ps->paudio_codec_ctx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    LOGI("maintest 音频解码至 swr_alloc_set_opts");
    swr_alloc_set_opts(swr_ctx,
                       out_ch_layout,
                       out_sample_fmt,
                       out_sample_rate,
                       in_ch_layout,
                       in_sample_fmt,
                       in_sample_rate,
                       0, NULL);

     //设置转换参数
/*     swr_alloc_set_opts(swr_ctx, dst_channel_layout, dst_sample_fmt,
             pframe->sample_rate, pframe->channel_layout,
             (enum AVSampleFormat)pframe->format, pframe->sample_rate, 0, NULL);*/

     if (swr_ctx == NULL|| swr_init(swr_ctx) < 0)
     {
          LOGE( "swr set open or swr init error\n");
          return -1;
     }


     //计算转换后的sample个数 a * b / c， 使用这个函数是为了不overflow
     //AVRounding 舍入
	 //也可以dst_nb_samples = 192000代替。
     dst_nb_samples = (uint64_t) av_rescale_rnd(
              swr_get_delay(swr_ctx, pframe->sample_rate) + pframe->nb_samples,
                  pframe->sample_rate, pframe->sample_rate, AV_ROUND_INF);

     //前面几个都是为了这个函数的参数。
	 convert_len = swr_convert(swr_ctx, &audio_buf, dst_nb_samples,
	 		 (const uint8_t **)pframe->data, pframe->nb_samples);

     data_size = convert_len * pframe->channels * av_get_bytes_per_sample(dst_sample_fmt);

     av_frame_free(&pframe);
     swr_free(&swr_ctx);            //
     LOGI("decode data_size = %d\n", data_size);
     return data_size;
}



