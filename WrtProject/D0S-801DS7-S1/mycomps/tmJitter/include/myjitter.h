#ifndef MYJITTER_H_
#define MYJITTER_H_

#ifdef __cplusplus
extern "C"{
#endif    

#pragma pack(1)
typedef struct wrt_rtp_header
{

    unsigned short cc:4;
    unsigned short extbit:1;
    unsigned short padbit:1;
    unsigned short version:2;
    unsigned short paytype:7;
    unsigned short markbit:1;
    unsigned short seq_number;
    unsigned long timestamp;
    unsigned long ssrc;
} wrt_rtp_header_t;

#pragma pack()


void InitMyJitter();
void ExitMyJitter();
void PutMyJitter( wrt_rtp_header_t* p);
wrt_rtp_header_t* GetMyJitter();


//////////////////////////////////////////////
/*
void InitMyJitter2();
void ExitMyJitter2();
int PutMyJitter2(wrt_rtp_header_t* p);

int GetMyJitter2(unsigned char** data);
*/
#ifdef __cplusplus
}
#endif

#endif