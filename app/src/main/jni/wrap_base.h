/*=========================================================================\
* Copyright(C)2016 Chudai.
*
* File name    : wrap_base.h
* Version      : v1.0.0
* Author       : i.sshe
* Github       : github.com/isshe
* Date         : 2016/10/03
* Description  :
* Function list: 1.
*                2.
*                3.
* History      :
\*=========================================================================*/

#ifndef WRAP_BASE_H_
#define WRAP_BASE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*=========================================================================*\
 * #include#                                                               *
\*=========================================================================*/
#include <include/libavcodec/avcodec.h>
#include <include/libswresample/swresample.h>
#include <include/SDL.h>
#include "include/libavformat/avformat.h"

/*=========================================================================*\
 * #define#                                                                *
\*=========================================================================*/

/*=========================================================================*\
 * #enum#                                                                  *
\*=========================================================================*/

/*=========================================================================*\
 * #struct#                                                                *
\*=========================================================================*/

/*=========================================================================*\
 * #function#                                                              *
\*=========================================================================*/


/*=====================================================================\
* Function   (名称): find_stream_index()
* Description(功能): 查找视频流和音频流
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
int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int *audio_stream);



/*=====================================================================\
* Function   (名称): get_file_name()
* Description(功能): 获取文件名字
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
void get_file_name(char *filename, int argc, char *argv[]);
#ifdef __cplusplus
}
#endif

#endif

