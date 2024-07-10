#pragma once

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char u8;
extern int **rustslp_to_schedule(int k, int p, int w, int *slp);

extern void encode_base(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize);

extern void encode_base_once(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize);

extern void encode_base_deforestation(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize);

extern void encode_base_once_deforestation(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize);

extern void slp_encode(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value);
extern void slp_encode_once(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize);
extern void slp_encode_better(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value);

extern void slp_encode_better_once(int k, int p, int w, int **schedule,u8 **copy_ptr,void **data_buffs,void **parity_buffs,int packetsize);
extern int max_var(int *slp);
extern void **tmp_var(int k, int p, int w, int *slp,int packsize,int value);
extern void slp_encode_better_finally(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,void **data_buffs,void **parity_buffs,int len, int packetsize,int value);
extern void slp_encode_best_finally(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize);
extern void slp_encode_best_spilt(int k, int p, int w, int ***operations, u8 **src, u8 **dest,int n,int len, int packetsize);
extern int*** bitmatrix_to_schedule_split(int k,int p, int w,int n,int *bitmarix);
extern void * slp_encode_best_finally_threads(void *p);
// extern void slp_encode_only(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value);

// extern void slp_encode_once_only(int k, int p, int w, int **schedule, u8 **src, u8 **dest,u8* tmp ,int packetsize,void ** data_buffs,void ** parity_buffs);
typedef struct {
	int k;
	int p;
	int w;
	int **schedule;
	u8 **src;
	u8 **dest;
	int len; 
	int packetsize;
	int value;
	int begin;
	int n;
	int thread_num;
}ptInfo;
#ifdef __cplusplus
}
#endif

