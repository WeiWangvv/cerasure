#include "slp.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//

int **rustslp_to_schedule(int k, int p, int w, int *slp)
{
    int **schedule;
    int i=0;
    int j=0;
    int index=0;
    int rows=0;
    while(slp[i]!=-2)
    {
        if(slp[i]==-1)
        {
            rows++;
        }
        i++;
    }
    schedule=malloc(sizeof(int *) * (rows+1));
    schedule[0]=malloc(sizeof(int) * (2*k*w+3));
    i=0;
    j=1;
    while(slp[index]!=-2)
    {
        if(slp[index]==-1)
        {
            schedule[i][0]=j-2;
            //printf("\n %d %d \n",i,schedule[i][0]);
            schedule[i][2*j-1]=-1;
            i++;
            schedule[i]=malloc(sizeof(int) * (2*k*w+3));
            j=1;
        }
        else
        {
            schedule[i][2*j-1]=slp[index]/w;
            schedule[i][2*j]=slp[index]%w;
            j++;
        }
        index++;
    }
    schedule[i][0]=-1;
    index=0;
    return schedule;
}


int*** bitmatrix_to_schedule_split(int k,int p, int w,int n,int *bitmarix)
{
	int ***schedule;
	int i;
	schedule=(int **)malloc(sizeof(int**)*(n+1));
	for(i=0;i<n;i++)
	{
		int offset=i*(k*w/n);
		schedule[i]=(int *)malloc(sizeof(int *) * (p*w+1));
		int j,a,count;
		j=0,a=0;
		for(j=0;j<p*w;j++)
		{
			schedule[i][j]=(int )malloc(sizeof(int *) * (2*k*w+3));
			count=0;
			schedule[i][j][1] = j/w+k;
			schedule[i][j][2] = j%w;
			for(a=0;a<k*w/n;a++)
			{
				if(bitmarix[j*k*w+a+offset]==1)
				{
					schedule[i][j][count*2+3]= (a+offset)/w;
					schedule[i][j][count*2+4]= (a+offset)%w;
					//printf("%d %d ",schedule[i][j][count*2+3],schedule[i][j][count*2+4]);
					count++;
				}
			}
			//printf("%d\n",count);
			schedule[i][j][0]=count;
			//printf("%d\n",count);
		}
		schedule[i][j]=malloc(sizeof(int *) * (2*k*w+3));
		schedule[i][j][0]=-1;
	}
	return schedule;
}

int max_var(int *slp)
{
    int i=0;
    int value=0;
    while(slp[i]!=-2)
    {
        if(slp[i] > value)
        {
            value=slp[i];
        }
        i++;
    }
    return value;
}

int rows_of_schedule(int **schedule)
{
	int index=0;
    while(schedule[index][0]!=-1)
    {
        index++;
    }
    return index;
}
// void** tmp_var(int k, int p, int w, int *slp,int packsize,int value)
// {
//     int i;
//     void* tmp_var[value/w+1];
//     for(i=0;i<value/w+1;i++)
//     {
//         void *buf;
//         posix_memalign(&buf, 32, packsize * w);
//         tmp_var[i] = buf;
//     }
//     return tmp_var;
// }

void encode_base(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize)
{
	int i,j;
    u8 *copy_ptr[p*w+1];
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    //printf("%d\n",value/w);
    //printf("!!!\n");
    // printf("%d\n",copy_ptr[i-2]);
	//int xor_bytes=0;
    //slp_encode_once(k, p, w, schedule, copy_ptr, packetsize);
	for(i=0;i<len;i += packetsize*w)
	{
		encode_base_once(k, p, w, schedule, copy_ptr ,packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k+p;j++)
		{
			copy_ptr[j] += (packetsize*w);
		}
	}
}

void encode_base_once(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize)
{
    void **data_buffs;
	void **parity_buffs;
	int j;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *));
	int index=0;

	while(schedule[index][0] >= 0){
        for(j=0;j<schedule[index][0];j++)
        {
            data_buffs[j] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize ;
            //printf("%d %d ",schedule[index][2*j+3],schedule[index][2*j+4]);
        }
        parity_buffs[0]=copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
        //printf("%d %d \n",schedule[index][1],schedule[index][2]);
        xor_nodeforest(schedule[index][0],packetsize, data_buffs, parity_buffs); 
		index++;
	}
}


void encode_base_deforestation(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize)
{
	int i,j;
    u8 *copy_ptr[p*w+1];
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    //printf("%d\n",value/w);
    //printf("!!!\n");
    // printf("%d\n",copy_ptr[i-2]);
	//int xor_bytes=0;
    //slp_encode_once(k, p, w, schedule, copy_ptr, packetsize);
	for(i=0;i<len;i += packetsize*w)
	{
		encode_base_once_deforestation(k, p, w, schedule, copy_ptr ,packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k+p;j++)
		{
			copy_ptr[j] += (packetsize*w);
		}
	}
}

void encode_base_once_deforestation(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize)
{
    void **data_buffs;
	void **parity_buffs;
	int j;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *));
	int index=0;

	while(schedule[index][0] >= 0){
        for(j=0;j<schedule[index][0];j++)
        {
            data_buffs[j] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize ;
            //printf("%d %d ",schedule[index][2*j+3],schedule[index][2*j+4]);
        }
        parity_buffs[0]=copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
        //printf("%d %d \n",schedule[index][1],schedule[index][2]);
        xor_gen(schedule[index][0],packetsize, data_buffs, parity_buffs);
		index++;
	}
}

void slp_encode(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value)
{
	int i,j;
    u8 *copy_ptr[value/w+1];
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    for(i=0;i<value/w-p-k+1;i++)
    {
        copy_ptr[i+k+p] = tmp[i];
    }
    //printf("%d\n",value/w);
    //printf("!!!\n");
    // printf("%d\n",copy_ptr[i-2]);
	//int xor_bytes=0;
    //slp_encode_once(k, p, w, schedule, copy_ptr, packetsize);
	for(i=0;i<len;i += packetsize*w)
	{
		slp_encode_once(k, p, w, schedule, copy_ptr ,packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k+p;j++)
		{
			copy_ptr[j] += (packetsize*w);
		}
	}
}

void slp_encode_once(int k, int p, int w, int **schedule,u8 **copy_ptr,int packetsize)
{
    void **data_buffs;
	void **parity_buffs;
	int j;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *));
	int index=0;

	while(schedule[index][0] >= 0){
        for(j=0;j<schedule[index][0];j++)
        {
            data_buffs[j] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize ;
            //printf("%d %d ",schedule[index][2*j+3],schedule[index][2*j+4]);
        }
        parity_buffs[0]=copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
        //printf("%d %d \n",schedule[index][1],schedule[index][2]);
        xor_gen(schedule[index][0],packetsize, data_buffs, parity_buffs);
		index++;
	}
}


void slp_encode_better(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value)
{
	int i,j;
    u8 *copy_ptr[value/w+1];
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    for(i=0;i<value/w-p-k+1;i++)
    {
        copy_ptr[i+k+p] = tmp[i];
    }
    //printf("%d\n",value/w);
    //printf("!!!\n");
    // printf("%d\n",copy_ptr[i-2]);
	//int xor_bytes=0;
    //slp_encode_once(k, p, w, schedule, copy_ptr, packetsize);
    void **data_buffs;
	void **parity_buffs;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *));

	for(i=0;i<len;i+=packetsize*w)
	{
		slp_encode_better_once(k, p, w, schedule, copy_ptr,data_buffs,parity_buffs,packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k+p;j++)
		{
			copy_ptr[j] += (packetsize*w);
		}
	}
}

void slp_encode_better_once(int k, int p, int w, int **schedule,u8 **copy_ptr,void **data_buffs,void **parity_buffs,int packetsize)
{
    // void **data_buffs;
	// void **parity_buffs;
	// data_buffs=malloc(sizeof(void *) * (2*k*w));
	// parity_buffs=malloc(sizeof(void *));
    int j;
	int index=0;

	while(schedule[index][0] >= 0){
        for(j=0;j<schedule[index][0];j++)
        {
            data_buffs[j] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize ;
            //printf("%d %d ",schedule[index][2*j+3],schedule[index][2*j+4]);
        }
        parity_buffs[0]=copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
        //printf("%d %d \n",schedule[index][1],schedule[index][2]);
        xor_gen(schedule[index][0],packetsize, data_buffs, parity_buffs);
		index++;
	}
}
void slp_encode_best(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,int len, int packetsize,int value)
{
	int i,j;
    u8 *copy_ptr[value/w+1];

    //构造指针加法
    long *add_offset;
    long *buf;
	if (posix_memalign(&buf, 32, p*w*k*w*sizeof(long))) {
		return 1;
	}
	add_offset = buf;
    long add_value= w * packetsize;
	for(i=0;i<p*w*k*w;i++)
	{
		add_offset[i]= add_value;
	}
    
    //构造头指针
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    for(i=0;i<value/w-p-k+1;i++)
    {
        copy_ptr[i+k+p] = tmp[i];
    }

    int rows=rows_of_schedule(schedule);
	void **indexs;
	void **data_buffs[rows];
	void **parity_buffs[rows];
    //printf("%d\n",rows);
	// for(i=0;i<rows;i++)
	// {
	// 	posix_memalign(&buf, 32, k*w*sizeof(long));
	// 	data_buffs[i]=buf;
	// 	parity_buffs[i]=malloc(sizeof(void *));
	// }
	posix_memalign(&buf, 32, p*w*k*w*sizeof(long));
	indexs=buf;
	int ptr_index=0;
	//int xor_bytes=0;
	int index=0;
    //printf("!!!\n");
    while(schedule[index][0] >= 0){
        data_buffs[index]=&indexs[ptr_index];
        for(j=0;j<schedule[index][0];j++)
        {
            if(schedule[index][2*j+3] >= (k+p) )
            {
               add_offset[ptr_index] = 0;
            }
            indexs[ptr_index++] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize;
        }
        parity_buffs[index]=&indexs[ptr_index];
        if(schedule[index][1] >= (k+p) )
        {
            add_offset[ptr_index] = 0;
        }
        indexs[ptr_index++] = copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
		index++;
	}
    //printf("!!!\n");
    //printf("%d\n",index);
	int loop=mceil(ptr_index,8)*64;
	for(i=0;i<len;i+=add_value)
	{
		for(j=0;j < index;j++)
		{
            //printf("%d\n",schedule[j][1]);
			xor_gen(schedule[j][0], packetsize, data_buffs[j] , parity_buffs[j]);
		}
        //printf("!!\n");
		buff_add(loop,add_offset,indexs);
	}
    //printf("!!!\n");
}

void slp_encode_best_finally(int k, int p, int w, int **schedule, u8 **src, u8 **dest,int len, int packetsize)
{
	int i,j;
    u8 *copy_ptr[k+p];

    //构造指针加法
    long *add_offset;
    long *buf;
	if (posix_memalign(&buf, 32, 16*sizeof(long))) {
		return 1;
	}
	add_offset = buf;
    long add_value= w * packetsize;
	for(i=0;i<16;i++)
	{
		add_offset[i]= add_value;
	}
    
    //构造头指针
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}

	void **indexs;
	void **data_buffs[p*w];
	void **parity_buffs[p*w];
    //printf("%d\n",rows);
	for(i=0;i<p*w;i++)
	{
		posix_memalign(&buf, 32, k*w*sizeof(long));
		data_buffs[i]=buf;
		parity_buffs[i]=malloc(sizeof(void *));
	}
	posix_memalign(&buf, 32, p*w*k*w*sizeof(long));
	indexs=buf;
	int ptr_index=0;
	//int xor_bytes=0;
	int index=0;

    while(schedule[index][0] >= 0){
        data_buffs[index]=&indexs[ptr_index];
        for(j=0;j<schedule[index][0];j++)
        {
            indexs[ptr_index++] = copy_ptr[schedule[index][2*j+3]] + schedule[index][2*j+4] * packetsize;
        }
        parity_buffs[index]=&indexs[ptr_index];
        indexs[ptr_index++] = copy_ptr[schedule[index][1]] + schedule[index][2]*packetsize;
		index++;
	}

	int loop=mceil(ptr_index,8)*64;
	for(i=0;i<len;i+=add_value)
	{
		for(j=0;j < index;j++)
		{
			xor_gen(schedule[j][0], packetsize, data_buffs[j] , parity_buffs[j]);
		}
		buff_finally(loop,add_offset,indexs);
	}

}


void slp_encode_better_finally(int k, int p, int w, int **schedule, u8 **src, u8 **dest, u8** tmp,void **data_buffs,void **parity_buffs,int len, int packetsize,int value)
{
	int i,j;
    u8 *copy_ptr[value/w+1];
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}
    for(i=0;i<value/w-p-k+1;i++)
    {
        copy_ptr[i+k+p] = tmp[i];
    }
    //printf("%d\n",value/w);
    //printf("!!!\n");
    // printf("%d\n",copy_ptr[i-2]);
	//int xor_bytes=0;
    //slp_encode_once(k, p, w, schedule, copy_ptr, packetsize);

	for(i=0;i<len;i+=packetsize*w)
	{
		slp_encode_better_once(k, p, w, schedule, copy_ptr,data_buffs,parity_buffs,packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k+p;j++)
		{
			copy_ptr[j] += (packetsize*w);
		}
	}
}

void  slp_encode_best_spilt(int k, int p, int w, int ***operations, u8 **src, u8 **dest,int n,int len, int packetsize)
{
	int i,j,a;
    u8 *copy_ptr[k+p];

    //构造指针加法
    long *add_offset;
    long *buf;
	if (posix_memalign(&buf, 32, 16*sizeof(long))) {
		return 1;
	}
	add_offset = buf;
    long add_value= w * packetsize;
	for(i=0;i<16;i++)
	{
		add_offset[i]= add_value;
	}
    
    //构造头指针
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}

	// void **data_indexs;
	// void **parity_indexs;

	void **indexs;
	void **data_buffs[n][p*w];
	void **parity_buffs[n][p*w];
	int oper_group_indexs[n];

	for(i=0;i<n;i++)
	{
		for(j=0;j<p*w;j++)
		{
			//posix_memalign(&buf, 32, 2*k*w*sizeof(long));
			//data_buffs[i]=buf;
			data_buffs[i][j]=malloc(k*w*sizeof(long)/n);
			parity_buffs[i][j]=malloc(sizeof(long) * 2);
		}
	}
	posix_memalign(&buf, 32,p*w*k*w*sizeof(long));
	indexs=buf;
	int ptr_index=0;
	int index;
	for(i=0;i<n;i++)
	{
		index=0;
		while(operations[i][index][0] >= 0){
			data_buffs[i][index]=&indexs[ptr_index];
		
			for(j=0;j<operations[i][index][0];j++)
			{
				indexs[ptr_index++] = copy_ptr[operations[i][index][2*j+3]] + operations[i][index][2*j+4] * packetsize;
			}
			if(i==0)
			{
				parity_buffs[i][index]=&indexs[ptr_index];
				indexs[ptr_index++] = copy_ptr[operations[i][index][1]] + operations[i][index][2]*packetsize;
			}else
			{
				parity_buffs[i][index]=parity_buffs[0][index];
			}
			index++;
		}	
	}	
	// // //int xor_bytes=0;
	int loop=mceil(ptr_index,8)*64;
	for(i=0;i<len;i+=add_value)
	{
		for(a=0;a < p*w;a++)
		{
			split_genf(operations[0][a][0], packetsize, data_buffs[0][a], parity_buffs[0][a]);
		}

		for(j=1;j<n;j++)
		{	
			//split_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
			for(a=0;a < p*w;a++)
			{
				split_gen(operations[j][a][0], packetsize, data_buffs[j][a], parity_buffs[j][a]);
			}
			// xor_gen(p*w, packetsize, data_buffs[j], parity_buffs[j]);
		}
        //printf("!!\n");
		buff_finally(loop,add_offset,indexs);
	}

}



void  slp_encode_best_spilt_with_smart_schedule(int k, int p, int w, int ***operations, u8 **src, u8 **dest,u8 *** tmp_parity,int n,int len, int packetsize)
{
	int i,j,a;
    u8 *copy_ptr[k+p];

    //构造指针加法
    long *add_offset;
    long *buf;
	if (posix_memalign(&buf, 32, 16*sizeof(long))) {
		return 1;
	}
	add_offset = buf;
    long add_value= w * packetsize;
	for(i=0;i<16;i++)
	{
		add_offset[i]= add_value;
	}
    
    //构造头指针
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}


	void **indexs;
	void **data_buffs[n][p*w];
	void **parity_buffs[n][p*w];
	int oper_group_indexs[n];

	for(i=0;i<n;i++)
	{
		for(j=0;j<p*w;j++)
		{
			data_buffs[i][j]=malloc(k*w*sizeof(long)/n);
			parity_buffs[i][j]=malloc(sizeof(long) * 2);
		}
	}

	posix_memalign(&buf, 32,p*w*k*w*sizeof(long));
	indexs=buf;
	int ptr_index=0;
	int index;
	for(i=0;i<n;i++)
	{
		index=0;
		while(operations[i][index][0] >= 0){
			data_buffs[i][index]=&indexs[ptr_index];

			if(operations[i][index][3]<k)
				indexs[ptr_index++] = copy_ptr[operations[i][index][3]] + operations[i][index][4] * packetsize;
			for(j=1;j<operations[i][index][0];j++)
			{
				indexs[ptr_index++] = copy_ptr[operations[i][index][2*j+3]] + operations[i][index][2*j+4] * packetsize;
			}
			index++;
		}	
	}

	for(index=0;index<p*w;index++){
		parity_buffs[0][index]=&indexs[ptr_index];
		indexs[ptr_index++] = copy_ptr[k+index/w] + (index%w)*packetsize;
	}	

		

	// // //int xor_bytes=0;
	int loop=mceil(ptr_index,8)*64;
	// printf("!!ww\n");
	for(i=0;i<len;i+=add_value)
	{
		// for(a=0;a < p*w;a++)
		// {
		// 	split_genf(operations[0][a][0], packetsize, data_buffs[0][a], parity_buffs[0][a]);
		// }

		for(j=0;j<n;j++)
		{	
			//split_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
			// printf("!!ww\n");

			a=0;
			while (operations[j][a][0] >=0)
			{
				int parity_packet_index=(operations[j][a][1]-k)*w+operations[j][a][2];
				if(operations[j][a][3]<k)
				{
					split_genf(operations[j][a][0], packetsize, data_buffs[j][a], &tmp_parity[parity_packet_index][j]);
				}
				else
				{
					int source=(operations[j][a][3]-k)*w+operations[j][a][4];
					split_smart(operations[j][a][0]-1, packetsize, data_buffs[j][a],&tmp_parity[source][j], &tmp_parity[parity_packet_index][j]);
				}
				a++;	
			}

			while(a<p*w)
			{
				int dest=(operations[j][a][1]-k)*w+operations[j][a][2];
				memset(tmp_parity[dest][j],0,packetsize);
				a++;
			}
		}
		for(a=0;a<p*w;a++)
		{
			// if(n==1)
			// {
			// 	memcpy(parity_buffs[0][a][0],tmp_parity[a][0],packetsize);
			// }
			// else
				split_genf(n, packetsize, tmp_parity[a], parity_buffs[0][a]);
		}
		buff_finally(loop,add_offset,indexs);
	}
}

/*
void  slp_encode_best_spilt_with_smart_schedule(int k, int p, int w, int ***operations, u8 **src, u8 **dest,int n,int len, int packetsize)
{
	int i,j,a;
    u8 *copy_ptr[k+p];

    //构造指针加法
    long *add_offset;
    long *buf;
	if (posix_memalign(&buf, 32, 16*sizeof(long))) {
		return 1;
	}
	add_offset = buf;
    long add_value= w * packetsize;
	for(i=0;i<16;i++)
	{
		add_offset[i]= add_value;
	}
    
    //构造头指针
	for(i=0;i<k;i++)
	{
		copy_ptr[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		copy_ptr[i+k] = dest[i];
	}

	// void **data_indexs;
	// void **parity_indexs;

	void **indexs;
	void **data_buffs[n][p*w];
	void **parity_buffs[n][p*w];
	int oper_group_indexs[n];

	for(i=0;i<n;i++)
	{
		for(j=0;j<p*w;j++)
		{
			//posix_memalign(&buf, 32, 2*k*w*sizeof(long));
			//data_buffs[i]=buf;
			data_buffs[i][j]=malloc(k*w*sizeof(long)/n);
			parity_buffs[i][j]=malloc(sizeof(long) * 2);
		}
	}
	posix_memalign(&buf, 32,p*w*k*w*sizeof(long));
	indexs=buf;
	int ptr_index=0;
	int index;
	for(i=0;i<n;i++)
	{
		index=0;
		while(operations[i][index][0] >= 0){
			data_buffs[i][index]=&indexs[ptr_index];
		
			for(j=0;j<operations[i][index][0];j++)
			{
				indexs[ptr_index++] = copy_ptr[operations[i][index][2*j+3]] + operations[i][index][2*j+4] * packetsize;
			}
			parity_buffs[i][index]=&indexs[ptr_index];
			indexs[ptr_index++] = copy_ptr[operations[i][index][1]] + operations[i][index][2]*packetsize;			
			index++;
		}	
	}
	// while(operations[i][index][0] >= 0){
	// 	index=0;	
	// 	for(i=0;i<n;i++)
	// 	{
	// 		parity_buffs[i][index]=&indexs[ptr_index];
	// 		indexs[ptr_index++] = copy_ptr[operations[i][index][1]] + operations[i][index][2]*packetsize;
	// 	}
	// 	index++;
	// }
	
	// // //int xor_bytes=0;
	int loop=mceil(ptr_index,8)*64;
	for(i=0;i<len;i+=add_value)
	{
		for(a=0;a < p*w;a++)
		{
			split_genf(operations[0][a][0], packetsize, data_buffs[0][a], parity_buffs[0][a]);
		}

		for(j=1;j<n;j++)
		{	
			//split_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
			for(a=0;a < p*w;a++)
			{
				split_genf(operations[j][a][0], packetsize, data_buffs[j][a], parity_buffs[j][a]);
			}
			// xor_gen(p*w, packetsize, data_buffs[j], parity_buffs[j]);
		}

		for(j=0;j<p*w;j++)
		{
			for(a=0;a<n;a++)
				split_gen(n,packetsize,parity_buffs[j][a],parity_buffs[j][a]);
		}
        //printf("!!\n");
		buff_finally(loop,add_offset,indexs);
	}

}
*/