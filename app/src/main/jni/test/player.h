/*=========================================================================\
* Copyright(C)2016 Chudai.
*
* File name    : player_v1.3.h
* Version      : v1.0.0
* Author       : i.sshe
* Github       : github.com/isshe
* Date         : 2016/10/06
* Description  :
* Function list: 1.
*                2.
*                3.
* History      :
\*=========================================================================*/

#ifndef PLAYER_H_
#define PLAYER_H_

#ifdef __cplusplus
extern "C"{
#endif

/*=========================================================================*\
 * #include#                                                               *
\*=========================================================================*/

#include "packet_queue.h"
#include "frame_queue.h"
#include "../include/SDL_thread.h"
#include "../include/SDL_config_windows.h"
#include "../include/SDL_video.h"
#include "../include/SDL_render.h"

/*=========================================================================*\
 * #define#                                                                *
\*=========================================================================*/
#define MAX_AUDIO_FRAME_SIZE    192000  //1 second of 48khz 32bit audio
#define SDL_AUDIO_BUFFER_SIZE   1024    //

#define FILE_NAME               "/home/isshe/Music/WavinFlag.aac"
#define ERR_STREAM              stderr
#define OUT_SAMPLE_RATE         44100
#define OUT_STREAM              stdout
#define WINDOW_W                640
#define WINDOW_H                320
#define ISSHE_REFRESH_EVENT 			(SDL_USEREVENT + 1)
#define BREAK_EVENT   			(SDL_USEREVENT + 2)
#define MAX_AUDIO_QUEUE_SIZE 	128
#define MAX_VIDEO_QUEUE_SIZE 	64
#define VIDEO_FRAME_QUEUE_CAPACITY 1

#define SYNC_THRESHOLD 			(0.01)
#define NOSYNC_THRESHOLD 		(10.0)
/*=========================================================================*\
 * #enum#                                                                  *
\*=========================================================================*/

/*=========================================================================*\
 * #struct#                                                                *
\*=========================================================================*/

//这是一个统筹的结构
typedef struct PlayerState
{
     //公共
     AVFormatContext    *pformat_ctx;
     char               filename[1024];
     int                quit;
     SDL_Thread         *audio_decode_tid;
     SDL_Thread         *audio_tid;
     SDL_Thread         *video_decode_tid;
     SDL_Thread         *video_tid;

     //音频
     int                audio_stream_index;
     AVStream           *paudio_stream;
     AVCodecContext     *paudio_codec_ctx;
     AVCodec            *paudio_codec;
     PacketQueue        audio_packet_queue;
     uint8_t            audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
     unsigned int       audio_buf_size;
     unsigned int       audio_buf_index;
//     AVFrame            wanted_frame;       //
     AVPacket           packet;
     uint8_t            *pkt_data;
     int                pkt_size;

     //视频
     int                video_stream_index;
     AVStream           *pvideo_stream;
     AVCodecContext     *pvideo_codec_ctx;
     AVCodec            *pvideo_codec;
     PacketQueue        video_packet_queue;
     FrameQueue 		video_frame_queue;
     uint8_t            *video_buf;
     unsigned int       video_buf_size;
     unsigned int       video_buf_index;
     struct SwsContext  *psws_ctx;

     int                pixel_w;
     int                pixel_h;
     int                window_w;
     int                window_h;

     SDL_Window         *pwindow;
     SDL_Renderer       *prenderer;
     SDL_Texture        *ptexture;
     SDL_Rect           sdl_rect;
     AVPixelFormat      pixfmt;
     AVFrame            frame;
     AVFrame            out_frame;

     //同步相关
     double             audio_clock;
     double             video_clock;
     double 			pre_frame_pts; 			//前一帧显示时间
     double 			cur_frame_pkt_pts; 		//当前帧在packet中标记的pts
     double 			pre_cur_frame_delay; 	//当前帧和前一帧的延时，前面两个相减的结果
     double 			cur_frame_pts; 			//packet.pts
     uint32_t			delay;
     
     double             frame_timer;
     double 			test;
}PlayerState;

/*=========================================================================*\
 * #function#                                                              *
\*=========================================================================*/


/*=====================================================================\
* Function   (名称): player_state_init()
* Description(功能): 初始化
* Called By  (被调): 1.
* Call_B file(被文): 1.
* Calls list (调用): 1.
* Calls file (调文): 1.
* Input      (输入): 1.
* Output     (输出): 1.
* Return     (返回):
*         success  :
*         failure  :
* Change log (修改): 1.
* Others     (其他): 1.
\*=====================================================================*/
void player_state_init(PlayerState *ps);



/*=====================================================================\
* Function   (名称): decode_thread
* Description(功能): 解码
* Called By  (被调): 1.
* Call_B file(被文): 1.
* Calls list (调用): 1.
* Calls file (调文): 1.
* Input      (输入): 1.
* Output     (输出): 1.
* Return     (返回):
*         success  :
*         failure  :
* Change log (修改): 1.
* Others     (其他): 1.
\*=====================================================================*/
int decode_thread(void *arg);
/*=====================================================================\
* Function   (名称): prepare_common()
* Description(功能): 准备
* Called By  (被调): 1.
* Call_B file(被文): 1.
* Calls list (调用): 1.
* Calls file (调文): 1.
* Input      (输入): 1.
* Output     (输出): 1.
* Return     (返回):
*         success  :
*         failure  :
* Change log (修改): 1.
* Others     (其他): 1.
\*=====================================================================*/
int prepare_common(PlayerState *ps);


#ifdef __cplusplus
}
#endif

#endif

