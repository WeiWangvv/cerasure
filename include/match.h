//
// Created by nty on 2022/7/8.
//

#ifndef ISA_L_MATCH_H
#define ISA_L_MATCH_H
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
int timeflag;
typedef void* vector; // you can't use this to store vectors, it's just used internally as a generic type
typedef size_t vec_size_t; // stores the number of elements
typedef unsigned char vec_type_t; // stores the number of bytes for a type

typedef int*	vec_int;
typedef char*	vec_char;

#define MAXSIZE 3000 //存储数据元素的最大个数
#define ERROR -99    //ElementType的特殊值，标志错误
#define ll long long
#define llu unsigned ll
//----------------------max Matching---------------------
typedef struct {
    int front; //记录队列头元素位置
    int rear;  //记录队列尾元素位置
    int size;  //存储数据元素的个数
    int data[MAXSIZE];
} Queue;
typedef struct
{
    int x,y,z;
}node;

#ifndef _MSC_VER

// shortcut defines

// vec_addr is a vector* (aka type**)
#define vector_add_asg(vec_addr)			((typeof(*vec_addr))(_vector_add((vector*)vec_addr, sizeof(typeof(**vec_addr)))))
#define vector_insert_asg(vec_addr, pos)	((typeof(*vec_addr))(_vector_insert((vector*)vec_addr, sizeof(typeof(**vec_addr)), pos)))

#define vector_add(vec_addr, value)			(*vector_add_asg(vec_addr) = value)
#define vector_insert(vec_addr, pos, value)	(*vector_insert_asg(vec_addr, pos) = value)

// vec is a vector (aka type*)
#define vector_erase(vec, pos, len)			(_vector_erase((vector*)vec, sizeof(typeof(*vec)), pos, len))
#define vector_remove(vec, pos)				(_vector_remove((vector*)vec, sizeof(typeof(*vec)), pos))

#else

// shortcut defines

// vec is a vector* (aka type**)
#define vector_add_asg(vec_addr, type)				((type*)_vector_add((vector*)vec_addr, sizeof(type)))
#define vector_insert_asg(vec_addr, type, pos)		((type*)_vector_insert((vector*)vec_addr, sizeof(type), pos))

#define vector_add(vec_addr, type, value)			(*vector_add_asg(vec_addr, type) = value)
#define vector_insert(vec_addr, type, pos, value)	(*vector_insert_asg(vec_addr, type, pos) = value)

// vec is a vector (aka type*)
#define vector_erase(vec, type, pos, len)			(_vector_erase((vector*)vec, sizeof(type), pos, len))
#define vector_remove(vec, type, pos)				(_vector_remove((vector*)vec, sizeof(type), pos))

#endif

vector vector_create(void);

void vector_free(vector vec);

void* _vector_add(vector* vec_addr, vec_type_t type_size);

void* _vector_insert(vector* vec_addr, vec_type_t type_size, vec_size_t pos);

void _vector_erase(vector* vec_addr, vec_type_t type_size, vec_size_t pos, vec_size_t len);

void _vector_remove(vector* vec_addr, vec_type_t type_size, vec_size_t pos);

vec_size_t vector_size(vector vec);

vec_size_t vector_get_alloc(vector vec);


Queue *initQueue() ;

void clearQueue(Queue *q) ;

int isQueueFull(Queue *q) ;

void pushQueue(Queue *q, int item) ;

int isQueueEmpty(Queue *q) ;

int popQueue(Queue *q) ;

int frontQueue(Queue *q) ;

void printQueue(Queue *q) ;
// =======================Queue=======================

void reverse(int *vec, int begin_pos, int end_pos) ;

void rotate(int *vec, int begin_pos, int middle_pos, int end_pos) ;

int find(int *vec, int begin_pos, int end_pos, int vec_element) ;

void initGraph();

int dist(node e);

void update_slack(int x,int y);

void set_slack(int y);

void q_push(int x);

void set_st(int x,int b);

int get_pr(int b,int xr);

void set_match(int x,int y);

void augment(int x,int y);

void swap(int *x, int *y);

int get_lca(int x,int y);

void add_blossom(int x,int lca,int y);

void expand_blossom(int b);

bool on_found_edge(node e);

int min(int a, int b);

int max(int a, int b);

bool matching(void);

ll weight_blossom(void);

int *max_weight_matching(int nodeNum,int edgeNum,int *x,int *y,int *z);

#endif /* vec_h */


