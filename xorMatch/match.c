//
// Created by nty on 2022/7/11.
//
#include "match.h"
#define maxn 620
//int maxn=620; //内存开点数的1.5倍
const int inf=0x3f3f3f3f;

node graph[maxn][maxn];
int n,m,nx,timeflag,weight[maxn],match[maxn],slack[maxn];
int st[maxn],pa[maxn],flower_from[maxn][maxn],flag[maxn],visit[maxn];
int *flower[maxn];
Queue *q;

typedef struct vector_data vector_data;

struct vector_data {
    vec_size_t alloc;				// stores the number of bytes allocated
    vec_size_t length;
    char buff[]; // use char to store bytes of an unknown type
};

vector_data* vector_alloc(vec_size_t alloc, vec_size_t size) {
    vector_data* v_data = malloc(sizeof(vector_data) + alloc * size);
    v_data->alloc = alloc;
    return v_data;
}

vector_data* vector_get_data(vector vec) {
    return &((vector_data*)vec)[-1];
}

vector vector_create(void) {
    vector_data* v = malloc(sizeof(vector_data));
    v->alloc = 0;
    v->length = 0;

    return &v->buff;
}

void vector_free(vector vec) {
    free(vector_get_data(vec));
}

vec_size_t vector_size(vector vec) {
    return vector_get_data(vec)->length;
}

vec_size_t vector_get_alloc(vector vec) {
    return vector_get_data(vec)->alloc;
}

vector_data* vector_realloc(vector_data* v_data, vec_type_t type_size) {
    vec_size_t new_alloc = (v_data->alloc == 0) ? 1 : v_data->alloc * 2;
    vector_data* new_v_data = realloc(v_data, sizeof(vector_data) + new_alloc * type_size);
    new_v_data->alloc = new_alloc;
    return new_v_data;
}

bool vector_has_space(vector_data* v_data) {
    return v_data->alloc - v_data->length > 0;
}

void* _vector_add(vector* vec_addr, vec_type_t type_size) {
    vector_data* v_data = vector_get_data(*vec_addr);

    if (!vector_has_space(v_data)) {
        v_data = vector_realloc(v_data, type_size);
        *vec_addr = v_data->buff;
    }

    return (void*)&v_data->buff[type_size * v_data->length++];
}

void* _vector_insert(vector* vec_addr, vec_type_t type_size, vec_size_t pos) {
    vector_data* v_data = vector_get_data(*vec_addr);

    vec_size_t new_length = v_data->length + 1;

    // make sure there is enough room for the new element
    if (!vector_has_space(v_data)) {
        v_data = vector_realloc(v_data, type_size);
    }
    memmove(&v_data->buff[(pos+1) * type_size],
            &v_data->buff[pos * type_size],
            (v_data->length - pos) * type_size); // move trailing elements

    v_data->length = new_length;

    return &v_data->buff[pos * type_size];
}

void _vector_erase(vector* vec_addr, vec_type_t type_size, vec_size_t pos, vec_size_t len) {
    vector_data* v_data = vector_get_data(vec_addr);
    // anyone who puts in a bad index can face the consequences on their own
    memmove(&v_data->buff[pos * type_size],
            &v_data->buff[(pos+len) * type_size],
            (v_data->length - pos - len) * type_size);

    v_data->length -= len;
}

void _vector_remove(vector* vec_addr, vec_type_t type_size, vec_size_t pos) {
    _vector_erase(vec_addr, type_size, pos, 1);
}

// =======================Queue=======================
Queue *initQueue() {
    // printf("===22===\n");
    Queue *q = (Queue *) malloc(sizeof(Queue));
    // Queue q;

    // printf("===24===\n");
    if (!q) {
        printf("initQueue failed!\n");
        return NULL;
    }
    q->front = -1;
    q->rear = -1;
    q->size = 0;
    // printf("===33===\n");
    return q;
}

void clearQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
    q->size = 0;
}

int isQueueFull(Queue *q) {
    return (q->size == MAXSIZE);
}

void pushQueue(Queue *q, int item) {
    if (isQueueFull(q)) {
        printf("queue full!\n");
        return;
    }
    q->rear++;
    q->rear %= MAXSIZE;
    q->size++;
    q->data[q->rear] = item;
}

int isQueueEmpty(Queue *q) {
    return (q->size == 0);
}

int popQueue(Queue *q) {
    if (isQueueEmpty(q)) {
        printf("empty queue!\n");
        return ERROR;
    }
    q->front++;
    q->front %= MAXSIZE; //0 1 2 3 4 5
    q->size--;
    return q->data[q->front];
}

int frontQueue(Queue *q) {
    if (isQueueEmpty(q)) {
        printf("empty queue!\n");
        return ERROR;
    }
    return q->data[q->front + 1];
}

void printQueue(Queue *q) {
    if (isQueueEmpty(q)) {
        printf("empty queue!\n");
        return;
    }
    printf("print queue:\n");
    int index = q->front;
    int i;
    for (i = 0; i < q->size; i++) {
        index++;
        index %= MAXSIZE;
        printf("%d ", q->data[index]);
    }
    printf("\n");
}
// =======================Queue=======================

void reverse(int *vec, int begin_pos, int end_pos) {
    int tmp[end_pos - begin_pos];
    for (int i = 0; i < end_pos - begin_pos; i++) {
        tmp[i] = vec[begin_pos + i];
    }
    for (int i = 0; i < end_pos - begin_pos; i++) {
        vec[begin_pos + i] = tmp[end_pos - begin_pos - i - 1];
    }
}

void rotate(int *vec, int begin_pos, int middle_pos, int end_pos) {
    int tmp[end_pos - begin_pos];
    int i;
    for (i = 0; i < end_pos - middle_pos; i++) {
        tmp[i] = vec[i + middle_pos];
    }
    for (i = 0; i < middle_pos - begin_pos; i++) {
        tmp[i + end_pos - middle_pos] = vec[i + begin_pos];
    }
    for (int i = begin_pos; i < end_pos; i++) {
        vec[i] = tmp[i - begin_pos];
    }
}

int find(int *vec, int begin_pos, int end_pos, int vec_element) {
    for (int i = begin_pos; i < end_pos; i++) {
        if (vec[i] == vec_element)
            return i;
    }
    return end_pos;
}

void initGraph()
{
    for(int i=0;i<maxn;i++)
    {
        flower[i]=vector_create();
    }
    q=initQueue();
}

int dist(node e)
{
    return weight[e.x]+weight[e.y]-graph[e.x][e.y].z*2;
}

void update_slack(int x,int y)
{
    if(!slack[y]||dist(graph[x][y])<dist(graph[slack[y]][y]))
        slack[y]=x;
}

void set_slack(int y)
{
    slack[y]=0;
    for(int x=1;x<=n;x++)
    {
        if(graph[x][y].z>0&&st[x]!=y&&flag[st[x]]==0)
            update_slack(x,y);
    }
}

void q_push(int x)
{
    if(x<=n) return pushQueue(q,x);
    for(int i=0;i<vector_size(flower[x]);i++) q_push(flower[x][i]);
}

void set_st(int x,int b)
{
    st[x]=b;
    if(x<=n) return;
    for(int i=0;i<vector_size(flower[x]);i++) set_st(flower[x][i],b);
}

int get_pr(int b,int xr)
{
    int pr=find(flower[b],0,vector_size(flower[b]),xr);
    if(pr%2==1)
    {
        reverse(flower[b],1,vector_size(flower[b]));
        return vector_size(flower[b])-pr;
    }
    else return pr;
}

void set_match(int x,int y)
{
    match[x]=graph[x][y].y;
    if(x<=n) return;
    node e=graph[x][y];
    int xr=flower_from[x][e.x],pr =get_pr(x,xr);
    for(int i=0;i<pr;i++)
        set_match(flower[x][i],flower[x][i^1]);

    set_match(xr,y);
    rotate(flower[x],0,pr,vector_size(flower[x]));
}

void augment(int x,int y)
{
    int xnv=st[match[x]];
    set_match(x,y);
    if(!xnv) return;
    set_match(xnv,st[pa[xnv]]);
    augment(st[pa[xnv]],xnv);
}

void swap(int *x, int *y)
{
    int tmp=*x;
    *x=*y;
    *y=tmp;
}

int get_lca(int x,int y)
{
    //printf("time:%d",time);
    int tmp;
    for(++timeflag;x||y;swap(&x,&y))
    {
        //printf("%d",time);
        if(x==0) continue;
        if(visit[x]==timeflag) return x;
        visit[x]=timeflag;
        x=st[match[x]];
        if(x) x=st[pa[x]];
    }
    return 0;
}

void add_blossom(int x,int lca,int y)
{
    int b=n+1;
    while(b<=nx&&st[b]) b++;
    if(b>nx) nx++;
    weight[b]=0,flag[b]=0;
    match[b]=match[lca];
    vector_erase(flower[b],0,vector_size(flower[b]));
    vector_add(&flower[b],lca);

    for(int i=x,j;i!=lca;i=st[pa[j]])
    {
        vector_add(&flower[b],i);
        j=st[match[i]];
        vector_add(&flower[b],j);
        q_push(j);
    }
    reverse(flower[b],1,vector_size(flower[b]));
    for(int i=y,j;i!=lca;i=st[pa[j]])
    {
        vector_add(&flower[b],i);
        j=st[match[i]];
        vector_add(&flower[b],j);
        q_push(j);
    }

    set_st(b,b);
    for(int i=1;i<=nx;i++)
    {
        graph[b][i].z=graph[i][b].z=0;
    }
    for(int i=1;i<=n;i++)
    {
        flower_from[b][i]=0;
    }
    for(int i=0;i<vector_size(flower[b]);i++)
    {
        int xs=flower[b][i];
        for(int i=1;i<=nx;i++)
        {
            if(graph[b][i].z==0||dist(graph[xs][i])<dist(graph[b][i]))
            {
                graph[b][i]=graph[xs][i],graph[i][b]=graph[i][xs];
            }
        }
        for(int i=1;i<=n;i++)
        {
            if(flower_from[xs][i])
            {
                flower_from[b][i]=xs;
            }
        }
    }
    set_slack(b);
}

void expand_blossom(int b)
{
    for(int i=0;i<vector_size(flower[b]);i++ )
    {
        set_st(flower[b][i],flower[b][i]);
    }
    int xr=flower_from[b][graph[b][pa[b]].x],pr=get_pr(b,xr);
    for(int i=0;i<pr;i+=2)
    {
        int xs=flower[b][i],xns=flower[b][i+1];
        pa[xs]=graph[xns][xs].x;
        flag[xs]=1,flag[xns]=0;
        slack[xs]=0,set_slack(xns);
        q_push(xns);
    }
    flag[xr]=1,pa[xr]=pa[b];
    for(int i=pr+1;i<vector_size(flower[b]);i++)
    {
        int xs=flower[b][i];
        flag[xs]=-1,set_slack(xs);
    }
    st[b] = 0;
}

bool on_found_edge(node e)
{
    int x=st[e.x],y=st[e.y];
    if(flag[y]==-1)
    {
        pa[y]=e.x,flag[y]=1;
        int nu=st[match[y]];
        slack[y]=slack[nu]=0;
        flag[nu]=0,q_push(nu);
    }
    else if(flag[y]==0)
    {
        //printf("line 342");
        int lca=get_lca(x,y);
        if (!lca)
        {
            return augment(x,y),augment(y,x),true;
        }
        else
        {
            //printf("line 351");
            add_blossom(x,lca,y);
        }
    }
    return false;
}

int min(int a, int b)
{
    if(a>b)
        return b;
    else
        return a;
}

int max(int a, int b)
{
    if(a>=b)
        return a;
    else
        return b;
}

bool matching(void)
{
    //flag表示是否被匹配，以及奇点还是偶点
    memset(flag,-1,sizeof(flag));
    //slack交错树的权值
    memset(slack,0,sizeof(slack));
    //把未匹配的点加入队列，st代表该点后面和谁连接，pa代表前面跟谁连接
    clearQueue(q);
    for(int x=1;x<=nx;x++)
    {
        if(st[x]==x&&!match[x])
        {
            pa[x]=0,flag[x]=0,q_push(x);
        }
    }

    if(isQueueEmpty(q)) return false;
    //开始进行匹配算法
    while(1)
    {
        while(q->size)
        {
            int x=frontQueue(q);
            popQueue(q);
            if(flag[st[x]]==1) continue;
            for(int y=1;y<=n;y++)
                if(graph[x][y].z>0&&st[x]!=st[y])
                {
                    if(dist(graph[x][y])==0)
                    {
                        //printf("line385");
                        if(on_found_edge(graph[x][y]))
                        {
                            //printf("wwww");
                            return true;
                        }
                        //printf("line391");
                    }
                    else
                    {
                        //更新权值
                        update_slack(x,st[y]);
                    }
                }
        }
        int d=inf;
        for(int b=n+1;b<=nx;b++)
        {
            if(st[b]==b&&flag[b]==1)
            {
                d=min(d,weight[b]/2);
            }
        }
        for(int x=1;x<=nx;x++)
            if(st[x]==x&&slack[x])
            {
                if(flag[x]==-1)
                {
                    d=min(d,dist(graph[slack[x]][x]));
                }
                else if(flag[x]==0)
                {
                    d=min(d,dist(graph[slack[x]][x])/2);
                }
            }
        for(int x=1;x<=n;x++)
        {
            if(flag[st[x]]==0)
            {
                if(weight[x]<=d) return false;
                weight[x]-=d;
            }
            else if(flag[st[x]]==1)
            {
                weight[x]+=d;
            }
        }
        for(int b=n+1;b<=nx;b++)
            if(st[b]==b)
            {
                if(flag[st[b]]==0) weight[b]+=d*2;
                else if (flag[st[b]]==1) weight[b]-=d*2;
            }
        clearQueue(q);
        for(int x=1;x<=nx;x++)
            if(st[x]==x&&slack[x]&&st[slack[x]]!=x&&dist(graph[slack[x]][x])==0)
                if(on_found_edge(graph[slack[x]][x]))
                {
                    //printf("wang");
                    return true;
                }
        for(int b=n+1;b<=nx;b++)
            if(st[b]==b&&flag[b]==1&&weight[b]==0) expand_blossom(b);
    }
    return false;
}

ll weight_blossom(void)
{
    //ll* total_weight_matches=(ll *)malloc(sizeof(ll));
    memset(match,0,sizeof(match));
    nx=n;
    int n_matches=0;
    ll tot_weight=0;
    for(int x=0;x<=n;x++)
    {
        st[x]=x;
        //flower应该是以邻接表的形式存储
        vector_erase(flower[x],0,vector_size(flower[x]));
    }
    int w_max=0;
    for(int x=1;x<=n;x++)
        for(int y=1;y<=n;y++)
        {
            //flower_from记录花根
            //w_max记录边的最大权
            flower_from[x][y]=(x==y?x:0);
            w_max=max(w_max,graph[x][y].z);
        }
    for(int x=1;x<=n;x++)
    {
        //x节点的权值
        weight[x]=w_max;
    }
    //printf("wa");
    while(matching())
    {
        //printf("wangwei");
        ++n_matches;
    }
    for(int x=1;x<=n;x++)
    {
        if(match[x]&&match[x]<x)
            tot_weight+=(ll)graph[x][match[x]].z;
    }
    //total_weight_matches[0]=tot_weight;
    //total_weight_matches[1]=n_matches;
    //return total_weight_matches;
    return tot_weight;
}
//----------------------max Matching---------------------

int *max_weight_matching(int nodeNum,int edgeNum,int *x,int *y,int *z)
{
    n=nodeNum;
    m=edgeNum;
    int *ans =(int *)malloc(maxn);
    int count=0;
    //生成flower的二维数组,用邻接表存储图
    initGraph();
    //为每个节点赋值,x,y节点标号，z权值,进行初始化
    for(int x=1;x<=n;x++)
        for(int y=1;y<=n;y++ )
        {
            graph[x][y].x=x;
            graph[x][y].y=y;
            graph[x][y].z=0;
        }
    //这个地方做了一个数组的转换处理
    // for(int i=1;i<=m;i++ )
    // {
    //     //scanf ("%d%d%d",&x,&y,&z);
    //     graph[x[i]][y[i]].z=graph[y[i]][x[i]].z=z[i];
    // }
    for(int i=1;i<=m;i++ )
    {
        //scanf ("%d%d%d",&x,&y,&z);
        graph[x[i-1]+1][y[i-1]+1].z=graph[y[i-1]+1][x[i-1]+1].z=z[i-1];
    }
    count = weight_blossom();
    //printf ("%lld\n",weight_blossom()[1]);
    // for(int i=1;i<=n;i++)
    //     match[i]-=1;
    for(int i=1;i<=n;i++)
    {
        ans[i-1]=match[i]-1;
    }
    return ans;
}