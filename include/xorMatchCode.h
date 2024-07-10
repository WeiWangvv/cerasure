//
// Created by nty on 2022/7/8.
//
#ifndef ISA_L_XOR_MATCH_CODE_H
#define ISA_L_XOR_MATCH_CODE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "raid.h"
#include "cauchy.h"
#include "galois.h"
#include "match.h"
//#include "types.h"

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

typedef unsigned char u8;

//----------------decode---------------------------------------------------------------------------------------------------/
int xor_decode_PGC_matching(int k, int p, int w, int *bitmatrix, int *eraseds,
                            char **data_ptrs, char **coding_ptrs, int size, int packetsize);
//static char **set_up_ptrs_for_opertaions(int k, int p, int *eraseds, char **data_ptrs, char **coding_ptrs);
u8 **set_up_ptrs_for_opertaions(int k, int p, int *eraseds, char **data_ptrs, char **coding_ptrs,int* recover);
//static int set_up_ids_for_operations(int k, int p, int *eraseds, int *row_ids, int *ind_to_row);
int set_up_ids_for_operations(int k, int p, int *eraseds, int *row_ids, int *ind_to_row,int *recover);
int *eraseds_to_erased(int k, int p, int *eraseds);
int *generate_decoding_bitmatrix(int k, int p, int w, int *bitmatrix, int *eraseds,int *recover);

int invert_bitmatrix(int *mat, int *inv, int rows);
//-----------------encode---------------------------------------------------------------------------------------------------/
void xor_acc(u8 *src, u8 *dest, int nbytes);

int xor_encode_once_origin(int k, int p, int w, int *bitmatrix, u8 **src, u8 **dest, int packetsize);

void xor_encode_origin(int k, int p, int w, int *bitmatrix, u8 **src, u8 **dest, int len, int packetsize);

int **bitmatrix_to_weighted_graph(int k, int p, int w, int *bitmatrix);


// operations: operation count, data block id, data packet id, data block id, data packet id, ...
int **bitmatrix_to_schedule_PGC(int k, int p, int w, int *bitmatrix);
int **bitmatrix_to_schedule_PGC_macthing_change(int k, int p, int w, int *bitmatrix, int *match);


//operations: common XOR count, parity1 remaining XOR count, parity2 remaining XOR count, data block id, data packet id, ...
int **bitmatrix_to_schedule_PGC_macthing(int k, int p, int w, int *bitmatrix, int *match);

//xor_gen(common_xor_count, p1_xor_count, p2_xor_count, data_buffs, parity_buffs)
void xor_encode_once_PGC_matching(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs);

void xor_encode_PGC_matching(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize);

void xor_encode_once_PGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize);


void xor_encode_PGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize);

int find_best_decode(int n,int m,int index,int num,int *survives,int *recover,int *de_recover,int k, int p, int w, int *bitmatrix, int *eraseds,int* min_num);


// operations: data block id, data packet id, parity block id, parity packet id, xor_copy_tag (1-xor, 0-copy) 
int **bitmatrix_to_schedule_DGC(int k, int p, int w, int *bitmatrix);

// operations: data block id, data packet id, parity block id, parity packet id, xor_copy_tag (1-xor, 0-copy) 
int **bitmatrix_to_schedule_PGC_mad(int k, int p, int w, int *bitmatrix);

//DGC schedule with matching pairs of data packets to reduce number of XOR's 
int **bitmatrix_to_schedule_with_matching(int k, int p, int w, int *bitmatrix, int *match);


void xor_encode_once_DGC(int **operations, u8 **src, u8 **dest, int packetsize);

void xor_encode_DGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize);

int count_bitmatrix_one_number(int k,int p,int w,int *bitmatrix,int begein,int end);

int* count_native_num(int k,int p,int w,int x);
int *count_xorMatch_num(int k,int p,int w,int **operations);
double count_xorMatch_size_Imp(int k,int p,int w,int **operations,int *bitmatrix);
int* count_noMatch_num(int k,int p,int w,int x);
void count_noMatch_size_Imp(int k,int p,int w,int *bitmatrix);
void count_native_size_Imp(int k,int p,int w,int *bitmatrix);
void *count_xorMatch_size(int k,int p,int w,int **operations);
void xor_encode_PGC_matching_asm(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize);
void xor_encode_once_PGC_matching_asm(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs);
void xor_encode_PGC_matching_spilt(int k, int p, int w, int n,int ***operations, u8 **src, u8 **dest, int len, int packetsize);
void xor_encode_once_PGC_matching_split(int k, int p, int w,int n,int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs);
void xor_encode_once_PGC_matching_split_first(int k, int p, int w,int n,int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs);
void xor_encode_PGC_matching_only(int k, int p, int w, int **operations, u8 **src, u8 **dest,void** data_indexs,void **parity_indexs,int len, int packetsize);
void xor_encode_PGC_matching_only_split(int k, int p, int w, int ***operations, u8 **src, u8 **dest,void** data_indexs,void **parity_indexs,int n,int len, int packetsize);

int **bitmatrix_to_schedule_PGC_macthing_split(int k, int p, int w, int *bitmatrix, int *match,int begin,int end);
int mceil(int n,int m);
//xor_gen(common_xor_count, p1_xor_count, p2_xor_count, data_buffs, parity_buffs)
void xor_encode_once_PGC_matching_origin(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize);
void xor_encode_PGC_matching_origin(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize);

#endif


