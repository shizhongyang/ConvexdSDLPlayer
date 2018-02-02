//
// Created by TT on 2018-02-02.
//

#include "include/libavcodec/avcodec.h"


typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size; //长度
    SDL_mutex *mutex; //锁
    SDL_cond *cond;   //条件变量
} PacketQueue;


void packet_queue_init(PacketQueue *q);

int packet_queue_put(PacketQueue *q, AVPacket *pkt);

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);

int getQueueSize(PacketQueue *q) ;