/*
 * SDL_Lesson.c
 *
 *  Created on: Aug 12, 2014
 *      Author: clarck
 */
#include <jni.h>
#include <android/native_window_jni.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/mem.h>
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_events.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

#include <android/log.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "test/player_audio.h"
#include "queue.h"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"ERROR: ", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"INFO: ", __VA_ARGS__)

AVFrame wanted_frame;
int quit = 0;  //是否退出的标志
PacketQueue audioq;

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf,
                       int buf_size) {
    LOGI("maintest 音频解码开始");
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    AVFrame *frame = av_frame_alloc();
    int len1, data_size = 0;

    SwrContext *swr_ctx = swr_alloc();

    for (;;) {
        while (audio_pkt_size > 0) {
            int got_frame = 0;
            LOGI("maintest 音频解码至 got_frame start");
            len1 = avcodec_decode_audio4(aCodecCtx, frame, &got_frame, &pkt);
            LOGI("maintest 音频解码至 got_frame end");
            if (len1 < 0) {
                /* if error, skip frame */
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            LOGI("maintest 音频解码至 got_frame");
            if (got_frame) {
                //chnanels: 通道数量, 仅用于音频
                //channel_layout: 通道布局。
                //多音频通道的流，一个通道布局可以具体描述其配置情况.通道布局这个概念不懂。
                //大概指的是单声道(mono)，立体声道（stereo), 四声道之类的吧？
                //详见源码及：https://xdsnet.gitbooks.io/other-doc-cn-ffmpeg/content/ffmpeg-doc-cn-07
                // .html#%E9%80%9A%E9%81%93%E5%B8%83%E5%B1%80
                if (frame->channels > 0 && frame->channel_layout == 0)
                {
                    //获取默认布局，默认应该了stereo吧？
                    frame->channel_layout = (uint64_t) av_get_default_channel_layout(frame->channels);
                }
                else if (frame->channels == 0 && frame->channel_layout > 0)
                {
                    frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);
                }
                //重采样设置参数-------------start
                //输入的采样格式
                enum AVSampleFormat in_sample_fmt = aCodecCtx->sample_fmt;
                //输出采样格式16bit PCM
                enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
                //输入采样率
                int in_sample_rate = aCodecCtx->sample_rate;
                //输出采样率
                int out_sample_rate = in_sample_rate;
                //获取输入的声道布局
                //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
                //av_get_default_channel_layout(codecCtx->channels);
                uint64_t in_ch_layout = aCodecCtx->channel_layout;
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

                LOGI("maintest 音频解码至 swr_alloc_set_opts end");
                //初始化
                if ( swr_init(swr_ctx) < 0|| swr_ctx == NULL )
                {
                    LOGE("swr_init error\n");
                    break;
                }

                // 计算转换后的sample个数 a * b / c
                int64_t dst_nb_samples = av_rescale_rnd(
                        swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                        frame->sample_rate,
                        frame->sample_rate,
                        AV_ROUND_INF);


                // 转换，返回值为转换后的sample个数
                int nb = swr_convert(swr_ctx,
                                     &audio_buf, dst_nb_samples,
                                     (const uint8_t**)frame->data,
                                     frame->nb_samples);

                //根据布局获取声道数
                int out_channels = av_get_channel_layout_nb_channels(out_ch_layout);
                //数据的大小
                data_size = out_channels * nb * av_get_bytes_per_sample(out_sample_fmt);
                LOGI("decode data_size = %d\n", data_size);

            }
            if (data_size <= 0) {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }
        if (pkt.data)
            //av_packet_free();
            av_free_packet(&pkt);

        if (quit) {
            return -1;
        }

        if (packet_queue_get(&audioq, &pkt, 1) < 0) {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
    return 0;
}
void audio_callback(void *userdata, Uint8 *stream, int len) {
    LOGI("maintest 音频解码回调");
    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;

    static uint8_t audio_buff[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    SDL_memset(stream, 0, len);

    if(quit == 1 || quit == -1)
    {
        SDL_PauseAudio(0);
        memset(audio_buff, 0, audio_buf_size);
        SDL_MixAudio(stream, audio_buff + audio_buf_index, len, 0);
        return;
    }

 // LOGI("pkt nums: %d    queue size: %d\n", playerState->audioq.nb_packets, playerState->audioq.size);
    while (len > 0)// 想设备发送长度为len的数据
    {
        if (audio_buf_index >= audio_buf_size) // 缓冲区中无数据
        {
            // 从packet中解码数据
            audio_size = audio_decode_frame(aCodecCtx, audio_buff, sizeof(audio_buff));
            LOGI("audio_size:%d",audio_size);
            if (audio_size < 0) // 没有解码到数据或出错，填充0
            {
                audio_buf_size = 0;
                memset(audio_buff, 0, audio_buf_size);
            }
            else
                audio_buf_size = audio_size;

            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index; // 缓冲区中剩下的数据长度
        if (len1 > len) // 向设备发送的数据长度为len
            len1 = len;

        LOGI("-----------SDL_MixAudio");
        SDL_MixAudio(stream, audio_buff + audio_buf_index, len, SDL_MIX_MAXVOLUME);

        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int main(int argc, char *argv[]) {
    char *file_path = argv[1];
    LOGI("file_path:%s", file_path);

    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    AVPacket *packet;
    uint8_t *out_buffer;

    AVCodecContext *aCodecCtx;
    AVCodec *aCodec;

    SDL_Texture *bmp = NULL;
    SDL_Window *screen = NULL;
    SDL_Rect rect;
    SDL_Event event;

    SDL_AudioSpec wanted_spec, spec;

    static struct SwsContext *img_convert_ctx;

    int videoStream, audioStream, i, numBytes;
    int ret, got_picture;

    av_register_all();
    pFormatCtx = avformat_alloc_context();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOGE("Could not initialize SDL - %s. \n", SDL_GetError());
        exit(1);
    }

    if (avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0) {
        LOGE("can't open the file. \n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Could't find stream infomation.\n");
        return -1;
    }

    videoStream = 1;
    audioStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO
            && audioStream < 0) {
            audioStream = i;
        }
    }

    LOGI("videoStream:%d", videoStream);
    if (videoStream == -1) {
        LOGE("Didn't find a video stream.\n");
        return -1;
    }

    if (audioStream == -1) {
        LOGE("Didn't find a audio stream.\n");
        return -1;
    }

    aCodecCtx = pFormatCtx->streams[audioStream]->codec;
    // Set audio settings from codec info
    wanted_spec.freq = aCodecCtx->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = aCodecCtx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = aCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
        LOGE("SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }
    //设置参数，供解码时候用, swr_alloc_set_opts的in部分参数
    wanted_frame.format         = AV_SAMPLE_FMT_S16;
    wanted_frame.sample_rate    = spec.freq;
    wanted_frame.channel_layout = av_get_default_channel_layout(spec.channels);
    wanted_frame.channels       = spec.channels;

    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
    if (!aCodec) {
        LOGE("Unsupported codec!\n");
        return -1;
    }
    avcodec_open2(aCodecCtx, aCodec, NULL);

    // audio_st = pFormatCtx->streams[index]
    packet_queue_init(&audioq);
    SDL_PauseAudio(0);

    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (pCodec == NULL) {
        LOGE("Codec not found.\n");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    //---------------------------init sdl---------------------------//

    screen = SDL_CreateWindow("My Player Window", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, pCodecCtx->width, pCodecCtx->height,
                              SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);

    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);

    bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
                            SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

    //-------------------------------------------------------------//

    numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                  pCodecCtx->height);
    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,
                   pCodecCtx->width, pCodecCtx->height);

    rect.x = 0;
    rect.y = 0;
    rect.w = pCodecCtx->width;
    rect.h = pCodecCtx->height;

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket));
    av_new_packet(packet, y_size);

    av_dump_format(pFormatCtx, 0, file_path, 0);

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoStream) {

            //int getPacketCode = avcodec_send_packet(pCodecCtx, packet);

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,
                                        packet);

            if (ret < 0) {
                LOGE("decode error.\n");
                return -1;
            }

            LOGI("got_picture:%d", got_picture);
            if (got_picture) {
                img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                                 pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                                 AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
                sws_scale(img_convert_ctx,
                          (uint8_t const * const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
                          pFrameYUV->linesize);
                sws_freeContext(img_convert_ctx);
                ////iPitch 计算yuv一行数据占的字节数
                SDL_UpdateYUVTexture(bmp, &rect,
                                     pFrameYUV->data[0], pFrameYUV->linesize[0],
                                     pFrameYUV->data[1], pFrameYUV->linesize[1],
                                     pFrameYUV->data[2], pFrameYUV->linesize[2]);
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, bmp, &rect, &rect);
                SDL_RenderPresent(renderer);

                //设置每秒25帧，1000/25 = 40
                SDL_Delay(25);

            }
            av_free_packet(packet);
        } else if (packet->stream_index == audioStream) {
            LOGI("maintest 准备播放音频");
            packet_queue_put(&audioq, packet);
        } else {
            av_free_packet(packet);
        }

        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
    }
    SDL_DestroyTexture(bmp);

    av_free(out_buffer);
    av_free(pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

