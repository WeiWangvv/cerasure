#include"xorMatchCode.h"
//-----------------encode---------------------------------------------------------------------------------------------------/
void xor_acc(u8 *src, u8 *dest, int nbytes)
{
	void *buffs[3];

	buffs[0]=dest;
	buffs[1]=src;
	buffs[2]=dest;
	//xor_gen(3, nbytes, buffs);
}

int xor_encode_once_origin(int k, int p, int w, int *bitmatrix, u8 **src, u8 **dest, int packetsize)
{
	int copy_xor_tag[p*w];
	int i,j;
	int xor_bytes=0;
	int src_block_index, src_packet_index, dest_block_index, dest_packet_index;

	for(i=0;i<p*w;i++)
	{
		copy_xor_tag[i]=0;
	}

	for(i=0;i<k*w;i++)
	{
		for(j=0;j<p*w;j++)
		{
			if(bitmatrix[j*k*w+i]==1)
			{
				if(copy_xor_tag[j]==1)
				{
					src_block_index=i/w;
					src_packet_index=i%w;
					dest_block_index=j/w;
					dest_packet_index=j%w;

					xor_acc(src[src_block_index]+(src_packet_index)*packetsize, dest[dest_block_index]+(dest_packet_index)*packetsize, packetsize);
					xor_bytes += packetsize;
				}
				else
				{
					src_block_index=i/w;
					src_packet_index=i%w;
					dest_block_index=j/w;
					dest_packet_index=j%w;

					memcpy(dest[dest_block_index]+(dest_packet_index)*packetsize, src[src_block_index]+(src_packet_index)*packetsize, packetsize);
					copy_xor_tag[j]=1;
				}
			}
		}
	}

	return xor_bytes;
}

void xor_encode_origin(int k, int p, int w, int *bitmatrix, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	int xor_bytes=0;

	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}

	for(i=0;i<len;i+=packetsize*w)
	{	
		xor_bytes += xor_encode_once_origin(k, p, w, bitmatrix, src_copy, dest_copy, packetsize);

		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}

		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}

int **bitmatrix_to_weighted_graph(int k, int p, int w, int *bitmatrix)
{
	int **weighted_graph;
	int weight;
	int index=0;
	int i, j, q;
	weighted_graph=malloc(sizeof(int *) * 4);

	for (i = 0; i < 3; i++) {
		weighted_graph[i]=malloc(sizeof(int) * (p*w*(p*w-1)/2));			
	}
	weighted_graph[3]=malloc(sizeof(int) * 1);

	for(i=0;i<p*w-1;i++){
		for(j=i+1;j<p*w;j++){
			weight=0;
			for(q=0;q<k*w;q++){
				if(bitmatrix[i*k*w+q]==1 && bitmatrix[j*k*w+q]==1)
					weight++;
			}
			if(weight>=2){
				weighted_graph[0][index]=i;
				weighted_graph[1][index]=j;
				weighted_graph[2][index]=weight;
				index++;
			}
		}
	}

	//printf("%d\n", index);
	weighted_graph[3][0]=index;
	return weighted_graph;
}


// operations: operation count, data block id, data packet id, data block id, data packet id, ...
int **bitmatrix_to_schedule_PGC(int k, int p, int w, int *bitmatrix)
{
	int **operations;
	int oper_index, oper_count;
	int i, j;

	operations=malloc(sizeof(int *) * (p*w));
	if (!operations) 
		return NULL;
  	oper_index=0;
	oper_count=0;

	for(i=0;i<p*w;i++){
		for(j=0;j<k*w;j++){
			if(bitmatrix[i*k*w+j]){
				oper_count++;
			}
		}
		operations[i]=malloc(sizeof(int)*(2*oper_count+1));
		operations[i][0]=oper_count;
		oper_index++;
		for(j=0;j<k*w;j++){
			if(bitmatrix[i*k*w+j]){
				operations[i][oper_index]=j/w;
				oper_index++;
				operations[i][oper_index]=j%w;
				oper_index++;
			}
		}

		oper_index=0;
		oper_count=0;
	}

	return operations;	
}

int **bitmatrix_to_schedule_PGC_macthing_change(int k, int p, int w, int *bitmatrix, int *match)
{
	int **operations;
	int oper_group_index, oper_index;
	int common_xor_count, p1_xor_count, p2_xor_count;
	int i, j, l;

	int count_num=0;
	operations=malloc(sizeof(int *) * (p*w));
	if (!operations) 
		return NULL;
	common_xor_count=0;
	p1_xor_count=0;
	p2_xor_count=0;
	oper_group_index=0;
	oper_index=3;

	for(i=0;i<p*w;i++){

		//unmatched parity packet 
		if(match[i]==-1){
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]){
					p1_xor_count++;
				}
			}
			
			operations[oper_group_index]=malloc(sizeof(int)*(2*(p1_xor_count+1)+3));
			operations[oper_group_index][0]=0;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=0;

			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;

			p1_xor_count=0;
			oper_index=3;
			oper_group_index++;
		}

		//schedule two matched parity packets into the same operation		
		else if(match[i]>=0){
			l=match[i];
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					common_xor_count++;
					bitmatrix[i*k*w+j]=0;
					bitmatrix[l*k*w+j]=0;
				}
				else if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					p1_xor_count++;
				}
				else if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					p2_xor_count++;
				}
			}

			operations[oper_group_index]=malloc(sizeof(int)*(2*(common_xor_count+p1_xor_count+p2_xor_count+2)+3));
			operations[oper_group_index][0]=common_xor_count;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=p2_xor_count;

			//common XORs of two matched parity packets 
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the first parity pakcets
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the second parity pakcets
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//two matched parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;
			operations[oper_group_index][oper_index]=l/w;
			oper_index++;
			operations[oper_group_index][oper_index]=l%w;
			oper_index++;
			count_num+=common_xor_count;
			common_xor_count=0;
			p1_xor_count=0;
			p2_xor_count=0;
			oper_index=3;
			oper_group_index++;

			match[i]=-2; //匹配的两个parity packet无需再生成operations
			match[l]=-2;
			
		}
	}
	//printf("%d\n",count_num);
	//printf("!!!%d\n",oper_group_index);
	operations[oper_group_index]=malloc(sizeof(int));
	operations[oper_group_index][0]=-1;
	return operations;
}

//operations: common XOR count, parity1 remaining XOR count, parity2 remaining XOR count, data block id, data packet id, ...
int **bitmatrix_to_schedule_PGC_macthing(int k, int p, int w, int *bitmatrix, int *match)
{
	int **operations;
	int oper_group_index, oper_index;
	int common_xor_count, p1_xor_count, p2_xor_count;
	int i, j, l;

	// int count_num=0;
	operations=malloc(sizeof(int *) * (p*w));
	if (!operations) 
		return NULL;
	common_xor_count=0;
	p1_xor_count=0;
	p2_xor_count=0;
	oper_group_index=0;
	oper_index=3;

	for(i=0;i<p*w;i++){

		//unmatched parity packet 
		if(match[i]==-1){
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]){
					p1_xor_count++;
				}
			}
			
			operations[oper_group_index]=malloc(sizeof(int)*(2*(p1_xor_count+1)+3));
			operations[oper_group_index][0]=0;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=0;

			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;

			p1_xor_count=0;
			oper_index=3;
			oper_group_index++;
		}

		//schedule two matched parity packets into the same operation		
		else if(match[i]>=0){
			l=match[i];
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					common_xor_count++;
				}
				else if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					p1_xor_count++;
				}
				else if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					p2_xor_count++;
				}
			}

			operations[oper_group_index]=malloc(sizeof(int)*(2*(common_xor_count+p1_xor_count+p2_xor_count+2)+3));
			operations[oper_group_index][0]=common_xor_count;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=p2_xor_count;

			//common XORs of two matched parity packets 
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the first parity pakcets
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the second parity pakcets
			for(j=0;j<k*w;j++){
				if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//two matched parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;
			operations[oper_group_index][oper_index]=l/w;
			oper_index++;
			operations[oper_group_index][oper_index]=l%w;
			oper_index++;
			// count_num+=common_xor_count;
			common_xor_count=0;
			p1_xor_count=0;
			p2_xor_count=0;
			oper_index=3;
			oper_group_index++;

			match[i]=-2; //匹配的两个parity packet无需再生成operations
			match[l]=-2;
			
		}
	}
	// printf("%d",count_num);
	//printf("!!!%d\n",oper_group_index);
	operations[oper_group_index]=malloc(sizeof(int));
	operations[oper_group_index][0]=-1;
	return operations;
}


int **bitmatrix_to_schedule_PGC_macthing_split(int k, int p, int w, int *bitmatrix, int *match,int begin,int end)
{
	int **operations;
	int oper_group_index, oper_index;
	int common_xor_count, p1_xor_count, p2_xor_count;
	int i, j, l;

	// int count_num=0;
	operations=malloc(sizeof(int *) * (p*w));
	if (!operations) 
		return NULL;
	common_xor_count=0;
	p1_xor_count=0;
	p2_xor_count=0;
	oper_group_index=0;
	oper_index=3;

	for(i=0;i<p*w;i++){

		//unmatched parity packet 
		if(match[i]==-1){
			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]){
					p1_xor_count++;
				}
			}
			
			operations[oper_group_index]=malloc(sizeof(int)*(2*(p1_xor_count+1)+3));
			operations[oper_group_index][0]=0;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=0;

			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;

			p1_xor_count=0;
			oper_index=3;
			oper_group_index++;
		}

		//schedule two matched parity packets into the same operation		
		else if(match[i]>=0){
			l=match[i];
			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					common_xor_count++;
				}
				else if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					p1_xor_count++;
				}
				else if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					p2_xor_count++;
				}
			}

			operations[oper_group_index]=malloc(sizeof(int)*(2*(common_xor_count+p1_xor_count+p2_xor_count+2)+3));
			operations[oper_group_index][0]=common_xor_count;
			operations[oper_group_index][1]=p1_xor_count;
			operations[oper_group_index][2]=p2_xor_count;

			//common XORs of two matched parity packets 
			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the first parity pakcets
			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]==1 && bitmatrix[l*k*w+j]!=1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//remaining XORs of the second parity pakcets
			for(j=begin;j<end;j++){
				if(bitmatrix[i*k*w+j]!=1 && bitmatrix[l*k*w+j]==1){
					operations[oper_group_index][oper_index]=j/w;
					oper_index++;
					operations[oper_group_index][oper_index]=j%w;
					oper_index++;
				}
			}

			//two matched parity packet id
			operations[oper_group_index][oper_index]=i/w;
			oper_index++;
			operations[oper_group_index][oper_index]=i%w;
			oper_index++;
			operations[oper_group_index][oper_index]=l/w;
			oper_index++;
			operations[oper_group_index][oper_index]=l%w;
			oper_index++;
			// count_num+=common_xor_count;
			common_xor_count=0;
			p1_xor_count=0;
			p2_xor_count=0;
			oper_index=3;
			oper_group_index++;

			match[i]=-2; //匹配的两个parity packet无需再生成operations
			match[l]=-2;
			
		}
	}
	// printf("%d",count_num);
	//printf("!!!%d\n",oper_group_index);
	operations[oper_group_index]=malloc(sizeof(int));
	operations[oper_group_index][0]=-1;
	return operations;
}


//xor_gen(common_xor_count, p1_xor_count, p2_xor_count, data_buffs, parity_buffs)
void xor_encode_once_PGC_matching(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs)
{
	// void **data_buffs;
	// void **parity_buffs;
	// data_buffs=malloc(sizeof(void *) * (2*k*w));
	// parity_buffs=malloc(sizeof(void *) * 2);
	int j;
	int oper_group_index=0;
	//int a=0;
	//printf("!\n");
	//operations[][0], -1:end, 0:unmatched parity packet, >0:two matched parity packets
	while(operations[oper_group_index][0]>=0){
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_buffs[j]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			//printf("%d %d ",operations[oper_group_index][2*j+3],operations[oper_group_index][2*j+4]);
		}
		//printf("\n");
		if(operations[oper_group_index][0]>0){
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			j++;
			parity_buffs[1]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		else{
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}

		//xor_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
		oper_group_index++;
	}
	//xor_gen(operations[0][0], operations[0][1], operations[0][2], packetsize, data_buffs, parity_buffs);
	//printf("%d\n",a);
}

//xor_gen(common_xor_count, p1_xor_count, p2_xor_count, data_buffs, parity_buffs)
void xor_encode_once_PGC_matching_split(int k, int p, int w,int n,int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs)
{
	int j;
	// void **data_buffs;
	// void **parity_buffs;
	// data_buffs=malloc(sizeof(void *) * (2*k*w));
	// parity_buffs=malloc(sizeof(void *) * 2);
	int oper_group_index=0;
	//int a=0;
	//printf("!\n");
	//operations[][0], -1:end, 0:unmatched parity packet, >0:two matched parity packets
	while(operations[oper_group_index][0]>=0){
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_buffs[j]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		if(operations[oper_group_index][0]>0){
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			j++;
			parity_buffs[1]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		else{
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		//a++;
		split_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
		oper_group_index++;
	}
	//printf("%d\n",a);
}

void xor_encode_once_PGC_matching_split_first(int k, int p, int w,int n,int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs)
{
	int j;
	// void **data_buffs;
	// void **parity_buffs;
	// data_buffs=malloc(sizeof(void *) * (2*k*w));
	// parity_buffs=malloc(sizeof(void *) * 2);
	int oper_group_index=0;
	//int a=0;
	//printf("!\n");
	//operations[][0], -1:end, 0:unmatched parity packet, >0:two matched parity packets
	while(operations[oper_group_index][0]>=0){
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_buffs[j]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		if(operations[oper_group_index][0]>0){
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			j++;
			parity_buffs[1]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		else{
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		//a++;
		split_genf(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
		oper_group_index++;
	}
	//printf("%d\n",a);
}

void xor_encode_once_PGC_matching_asm(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize,void **data_buffs,void **parity_buffs)
{
	// void *buf;
	// posix_memalign(&buf, 8, sizeof(void *) * (2*k*w));
	// data_buffs=buf;
	// posix_memalign(&buf, 8, sizeof(void *) * 8);
	// parity_buffs=buf;
	int j=0;
	int oper_group_index=0;
	//int a=0;
	//printf("!\n");
	//operations[][0], -1:end, 0:unmatched parity packet, >0:two matched parity packets
	while(operations[oper_group_index][0]>=0){
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_buffs[j]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			//printf("%d %d ",operations[oper_group_index][2*j+3],operations[oper_group_index][2*j+4]);
		}
		//printf("\n");
		if(operations[oper_group_index][0]>0){
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			j++;
			parity_buffs[1]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		else{
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		//a++;
		//xor_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
		oper_group_index++;
	}
	//printf("%d\n",a);
}

void xor_encode_PGC_matching(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}
	void **data_buffs;
	void **parity_buffs;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *) * 2);
	//int xor_bytes=0;
	// int a;
	// int oper_group_index=0;
	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_PGC_matching(k, p, w, operations, src_copy, dest_copy, packetsize,data_buffs,parity_buffs);
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}

void xor_encode_PGC_matching_only(int k, int p, int w, int **operations, u8 **src, u8 **dest,void** data_indexs,void **parity_indexs,int len, int packetsize)
{
	if(packetsize==0 || packetsize%128!=0)
	{
		return;
	}
	int i,j;
	long *add_offset;
	long *buf;
	if (posix_memalign(&buf, 32, 4*sizeof(long))) {
		return 1;
	}
	add_offset = buf;

	for(i=0;i<4;i++)
	{
		add_offset[i]=w*packetsize;
	}
	int tmp[k*w][3];
	// void **data_indexs;
	// void **parity_indexs;
	void **data_buffs[p*w];
	void **parity_buffs[p*w];
	// for(i=0;i<p*w;i++)
	// {
	// 	//posix_memalign(&buf, 32, 2*k*w*sizeof(long));
	// 	//data_buffs[i]=buf;
	// 	data_buffs[i]=malloc(2*k*w*sizeof(long));
	// 	parity_buffs[i]=malloc(sizeof(void *) * 2);
	// }
	// posix_memalign(&buf, 32, p*w*2*k*w*sizeof(long));
	// data_indexs=buf;
	// posix_memalign(&buf, 32, 2*2*k*w*sizeof(long));
	// parity_indexs=buf;
	int data_index=0;
	int parity_index=0;
	//int xor_bytes=0;
	int oper_group_index=0;
	//初始化第一次指针
	while(operations[oper_group_index][0]>=0){
		data_buffs[oper_group_index]=&data_indexs[data_index];
		parity_buffs[oper_group_index]=&parity_indexs[parity_index];
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_indexs[data_index]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			data_index++;
		}
		if(operations[oper_group_index][0]>0){
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
			j++;
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
		}
		else{
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
		}
		oper_group_index++;
	}
	int data_loop=mceil(data_index,8)*64;
	int parity_loop=mceil(parity_index,8)*64;
	//printf("%d\n",oper_group_index);
	for(i=0;i<oper_group_index;i++)
	{
		tmp[i][0]=8*operations[i][0];
		tmp[i][1]=8*operations[i][1]+tmp[i][0];
		tmp[i][2]=8*operations[i][2]+tmp[i][1];
		// operations[i][1]=8*operations[i][1]+operations[i][0];
		// operations[i][2]=8*operations[i][2]+operations[i][1];
	}
	for(i=0;i<len;i+=packetsize*w)
	{
		for(j=0;j<oper_group_index;j++)
		{
			//xor_gen(tmp[j][0], tmp[j][1], tmp[j][2], packetsize, data_buffs[j], parity_buffs[j]);
			//xor_gen(operations[j][0], operations[j][1], operations[j][2], packetsize, data_buffs[j], parity_buffs[j]);
		}
		buff_add(data_loop,add_offset,data_indexs);
		buff_add(parity_loop,add_offset,parity_indexs);
	}
}


void xor_encode_PGC_matching_only_split(int k, int p, int w, int ***operations, u8 **src, u8 **dest,void** data_indexs,void **parity_indexs,int n,int len, int packetsize)
{
	int i,j,a;
	long *add_offset;
	long *buf;
	if (posix_memalign(&buf, 32, 4*sizeof(long))) {
		return 1;
	}
	add_offset = buf;

	for(i=0;i<4;i++)
	{
		add_offset[i]=w*packetsize;
	}

	// void **data_indexs;
	// void **parity_indexs;


	void **data_buffs[n][p*w];
	void **parity_buffs[n][p*w];
	int oper_group_indexs[n];
	for(j=0;j<n;j++)
	{
		for(i=0;i<p*w;i++)
		{
			//posix_memalign(&buf, 32, 2*k*w*sizeof(long));
			//data_buffs[i]=buf;
			data_buffs[j][i]=malloc(2*k*w/n*sizeof(long));
			parity_buffs[j][i]=malloc(sizeof(void *) * 2);
		}
	}
	// posix_memalign(&buf, 32, p*w*2*k*w*sizeof(long));
	// data_indexs=buf;
	// posix_memalign(&buf, 32, 2*2*k*w*sizeof(long));
	// parity_indexs=buf;
	int data_index=0;
	int parity_index=0;
	//int xor_bytes=0;
	for(i=0;i<n;i++)
	{
		int oper_group_index=0;
	//初始化第一次指针
		while(operations[i][oper_group_index][0]>=0){
			data_buffs[i][oper_group_index]=&data_indexs[data_index];
			parity_buffs[i][oper_group_index]=&parity_indexs[parity_index];
			for(j=0;j<operations[i][oper_group_index][0]+operations[i][oper_group_index][1]+operations[i][oper_group_index][2];j++){
				data_indexs[data_index]=src[operations[i][oper_group_index][2*j+3]]+operations[i][oper_group_index][2*j+4]*packetsize;
				data_index++;
			}
			if(operations[i][oper_group_index][0]>0){
				parity_indexs[parity_index]=dest[operations[i][oper_group_index][2*j+3]]+operations[i][oper_group_index][2*j+4]*packetsize;
				parity_index++;
				j++;
				parity_indexs[parity_index]=dest[operations[i][oper_group_index][2*j+3]]+operations[i][oper_group_index][2*j+4]*packetsize;
				parity_index++;
			}
			else{
				parity_indexs[parity_index]=dest[operations[i][oper_group_index][2*j+3]]+operations[i][oper_group_index][2*j+4]*packetsize;
				parity_index++;
			}
			oper_group_index++;
		}
		oper_group_indexs[i]=oper_group_index;
	}

	int data_loop=mceil(data_index,8)*64;
	int parity_loop=mceil(parity_index,8)*64;
	for(i=0;i<len;i+=packetsize*w)
	{

		for(a=0;a < oper_group_indexs[0];a++)
		{
			split_genf(operations[0][a][0], operations[0][a][1], operations[0][a][2], packetsize, data_buffs[0][a], parity_buffs[0][a]);
		}

		for(j=1;j<n;j++)
		{	
			//split_gen(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
			for(a=0;a < oper_group_indexs[j];a++)
			{
				split_gen(operations[j][a][0], operations[j][a][1], operations[j][a][2], packetsize, data_buffs[j][a], parity_buffs[j][a]);
			}
			//xor_gen(operations[j][0], operations[j][1], operations[j][2], packetsize, data_buffs[j], parity_buffs[j]);
		}
		buff_add(data_loop,add_offset,data_indexs);
		buff_add(parity_loop,add_offset,parity_indexs);
	}
}


void xor_encode_PGC_matching_only_spilt(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	long *add_offset;
	long *buf;
	if (posix_memalign(&buf, 32, 4*sizeof(long))) {
		return 1;
	}
	add_offset = buf;

	for(i=0;i<4;i++)
	{
		add_offset[i]=w*packetsize;
	}

	void **data_indexs;
	void **parity_indexs;
	void **data_buffs[p*w];
	void **parity_buffs[p*w];
	for(i=0;i<p*w;i++)
	{
		posix_memalign(&buf, 32, 2*k*w*sizeof(long));
		data_buffs[i]=buf;
		parity_buffs[i]=malloc(sizeof(void *) * 2);
	}
	posix_memalign(&buf, 32, p*w*2*k*w*sizeof(long));
	data_indexs=buf;
	posix_memalign(&buf, 32, 2*2*k*w*sizeof(long));
	parity_indexs=buf;
	int data_index=0;
	int parity_index=0;
	//int xor_bytes=0;
	int oper_group_index=0;
	while(operations[oper_group_index][0]>=0){
		data_buffs[oper_group_index]=&data_indexs[data_index];
		parity_buffs[oper_group_index]=&parity_indexs[parity_index];
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_indexs[data_index]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			data_index++;
		}
		if(operations[oper_group_index][0]>0){
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
			j++;
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
		}
		else{
			parity_indexs[parity_index]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			parity_index++;
		}
		oper_group_index++;
	}
	int data_loop=mceil(data_index,8)*64;
	int parity_loop=mceil(parity_index,8)*64;
	for(i=0;i<len;i+=packetsize*w)
	{
		for(j=0;j<oper_group_index;j++)
		{
			//xor_gen(operations[j][0], operations[j][1], operations[j][2], packetsize, data_buffs[j], parity_buffs[j]);
		}
		buff_add(data_loop,add_offset,data_indexs);
		buff_add(parity_loop,add_offset,parity_indexs);
	}
}


int mceil(int n,int m)
{
	if(n%m==0)
	{
		return n/m;
	}
	return n/m+1;
}

void xor_encode_PGC_matching_asm(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}
	void **data_buffs;
	void **parity_buffs;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *) * 2);
	//int xor_bytes=0;
	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_PGC_matching_asm(k, p, w, operations, src_copy, dest_copy, packetsize,data_buffs,parity_buffs);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}

void xor_encode_PGC_matching_spilt(int k, int p, int w, int n,int ***operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j,z;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}
	void **data_buffs;
	void **parity_buffs;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *) * 2);
	//int xor_bytes=0;
	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_PGC_matching_split_first(k, p, w, n , operations[0], &src_copy[0], dest_copy, packetsize,data_buffs,parity_buffs);
		for(z=1;z<n;z++)
		{
			//xor_encode_once_PGC_matching_split(k, p, w, n ,operations[0], src_copy, dest_copy, packetsize);
			//xor_encode_once_PGC_matching(k, p, w, operations[0], src_copy, dest_copy, packetsize);
			xor_encode_once_PGC_matching_split(k, p, w, n , operations[z], &src_copy[z*k/n], dest_copy, packetsize,data_buffs,parity_buffs);
		}
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}

void xor_encode_once_PGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize)
{
	void **buffs;
	int i, j;
	buffs=malloc(sizeof(void *) * (k*w+1));

	for(i=0;i<p*w;i++){
		for(j=0;j<operations[i][0];j++){
			buffs[j]=src[operations[i][2*j+1]]+operations[i][2*j+2]*packetsize;
		}
		buffs[operations[i][0]]=dest[i/w]+(i%w)*packetsize;
		//xor_gen(operations[i][0]+1, packetsize, buffs);
	}
}


void xor_encode_PGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}
	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_PGC(k, p, w, operations, src_copy, dest_copy, packetsize);
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}


//xor_gen(common_xor_count, p1_xor_count, p2_xor_count, data_buffs, parity_buffs)
void xor_encode_once_PGC_matching_origin(int k, int p, int w, int **operations, u8 **src, u8 **dest, int packetsize)
{
	void **data_buffs;
	void **parity_buffs;
	int j;
	data_buffs=malloc(sizeof(void *) * (2*k*w));
	parity_buffs=malloc(sizeof(void *) * 2);

	int oper_group_index=0;

	//operations[][0], -1:end, 0:unmatched parity packet, >0:two matched parity packets
	while(operations[oper_group_index][0]>=0){
		for(j=0;j<operations[oper_group_index][0]+operations[oper_group_index][1]+operations[oper_group_index][2];j++){
			data_buffs[j]=src[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		if(operations[oper_group_index][0]>0){
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
			j++;
			parity_buffs[1]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}
		else{
			parity_buffs[0]=dest[operations[oper_group_index][2*j+3]]+operations[oper_group_index][2*j+4]*packetsize;
		}

		split_genf(operations[oper_group_index][0], operations[oper_group_index][1], operations[oper_group_index][2], packetsize, data_buffs, parity_buffs);
		oper_group_index++;
	}
}

void xor_encode_PGC_matching_origin(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}
	//int xor_bytes=0;
	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_PGC_matching_origin(k, p, w, operations, src_copy, dest_copy, packetsize);
		//xor_bytes += xor_encode_once_schedule(operations, src, dest, packetsize);
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}

// operations: data block id, data packet id, parity block id, parity packet id, xor_copy_tag (1-xor, 0-copy) 
int **bitmatrix_to_schedule_DGC(int k, int p, int w, int *bitmatrix)
{
	int **operations;
	int oper_group_index, matrix_index;
	int i, j;
	int copy_xor_tag[p*w];

	operations=malloc(sizeof(int *) * (k*w*p*w+1));
	if (!operations) 
		return NULL;
  	oper_group_index=0;

	for(i=0;i<p*w;i++)
	{
		copy_xor_tag[i]=0;
	}

	for(i=0; i<k*w; i++){
		matrix_index = i;
		for(j =0; j<p*w; j++){
			if(bitmatrix[matrix_index]){
				operations[oper_group_index]=malloc(sizeof(int)*5);
				operations[oper_group_index][0]=i/w;
				operations[oper_group_index][1]=i%w;
				operations[oper_group_index][2]=j/w;
				operations[oper_group_index][3]=j%w;
				if(copy_xor_tag[j]==1){
					operations[oper_group_index][4]=1;
				}
				else{
					operations[oper_group_index][4]=0;
					copy_xor_tag[j]=1;
				}
				oper_group_index++;
			}
			matrix_index+=k*w;
		}
	}
	operations[oper_group_index]=malloc(sizeof(int)*5);
	operations[oper_group_index][0]=-1;

	return operations;	
}

// operations: data block id, data packet id, parity block id, parity packet id, xor_copy_tag (1-xor, 0-copy) 
int **bitmatrix_to_schedule_PGC_mad(int k, int p, int w, int *bitmatrix)
{
	int **operations;
	int oper_group_index;
	int i, j;
	int copy_xor_tag[p*w];

	operations=malloc(sizeof(int *) * (k*w*p*w+1));
	if (!operations) 
		return NULL;
  	oper_group_index=0;

	for(i=0;i<p*w;i++)
	{
		copy_xor_tag[i]=0;
	}

	for(i=0; i<p*w; i++){
		for(j =0; j<k*w; j++){
			if(bitmatrix[i*k*w+j]){
				operations[oper_group_index]=malloc(sizeof(int)*5);
				operations[oper_group_index][0]=j/w;
				operations[oper_group_index][1]=j%w;
				operations[oper_group_index][2]=i/w;
				operations[oper_group_index][3]=i%w;
				if(copy_xor_tag[i]==1){
					operations[oper_group_index][4]=1;
				}
				else{
					operations[oper_group_index][4]=0;
					copy_xor_tag[i]=1;
				}
				oper_group_index++;
			}
		}
	}
	operations[oper_group_index]=malloc(sizeof(int)*5);
	operations[oper_group_index][0]=-1;

	return operations;	
}

//DGC schedule with matching pairs of data packets to reduce number of XOR's 
int **bitmatrix_to_schedule_with_matching(int k, int p, int w, int *bitmatrix, int *match)
{
	int **operations;
	int isSchedule[k*w];
	int copy_xor_tag[p*w];
	int oper_group_index, matrix_index, matrix_index_matching;
	int i, j;

	operations=malloc(sizeof(int *) * (k*w*p*w+1));
	if (!operations) 
		return NULL;
  	oper_group_index=0;

	for(i=0;i<p*w;i++)
	{
		copy_xor_tag[i]=0;
	}
	for(i=0;i<k*w;i++)
	{
		isSchedule[i]=0;
	}

	for(i=0;i<k*w;i++){
		//data packet is not matched
		if(match[i]<0){
			matrix_index = i;
			for(j =0; j<p*w; j++){
				if(bitmatrix[matrix_index]){
					operations[oper_group_index]=malloc(sizeof(int)*5);
					operations[oper_group_index][0]=i/w;
					operations[oper_group_index][1]=i%w;
					operations[oper_group_index][2]=j/w;
					operations[oper_group_index][3]=j%w;
					if(copy_xor_tag[j]==1){
						operations[oper_group_index][4]=1;
					}
					else{
						operations[oper_group_index][4]=0;
						copy_xor_tag[j]=1;
					}
					oper_group_index++;
				}
				matrix_index+=k*w;
			}
		}
		//schedule data packet and its matching data packet
		else if(isSchedule[i]==0){
			matrix_index=i;
			matrix_index_matching=match[i];
			for(j=0; j<p*w;j++){
				if(bitmatrix[matrix_index]==1 && bitmatrix[matrix_index_matching]!=1){
					operations[oper_group_index]=malloc(sizeof(int)*5);
					operations[oper_group_index][0]=i/w;
					operations[oper_group_index][1]=i%w;
					operations[oper_group_index][2]=j/w;
					operations[oper_group_index][3]=j%w;
					if(copy_xor_tag[j]==1){
						operations[oper_group_index][4]=1;
					}
					else{
						operations[oper_group_index][4]=0;
						copy_xor_tag[j]=1;
					}
					oper_group_index++;
				}
				else if(bitmatrix[matrix_index]!=1 && bitmatrix[matrix_index_matching]==1){
					operations[oper_group_index]=malloc(sizeof(int)*5);
					operations[oper_group_index][0]=match[i]/w;
					operations[oper_group_index][1]=match[i]%w;
					operations[oper_group_index][2]=j/w;
					operations[oper_group_index][3]=j%w;
					if(copy_xor_tag[j]==1){
						operations[oper_group_index][4]=1;
					}
					else{
						operations[oper_group_index][4]=0;
						copy_xor_tag[j]=1;
					}
					oper_group_index++;
				}
				matrix_index+=k*w;
				matrix_index_matching+=k*w;
			}

			//match[i]-th data packet = match[i]-th data packet XOR i-th data packet
			operations[oper_group_index]=malloc(sizeof(int)*5);
			operations[oper_group_index][0]=i/w;
			operations[oper_group_index][1]=i%w;
			operations[oper_group_index][2]=match[i]/w;
			operations[oper_group_index][3]=match[i]%w;
			operations[oper_group_index][4]=1;
			oper_group_index++;

			matrix_index=i;
			matrix_index_matching=match[i];
			for(j=0;j<p*w;j++){
				if(bitmatrix[matrix_index]==1 && bitmatrix[matrix_index_matching]==1){
					operations[oper_group_index]=malloc(sizeof(int)*5);
					operations[oper_group_index][0]=match[i]/w;
					operations[oper_group_index][1]=match[i]%w;
					operations[oper_group_index][2]=j/w;
					operations[oper_group_index][3]=j%w;
					if(copy_xor_tag[j]==1){
						operations[oper_group_index][4]=1;
					}
					else{
						operations[oper_group_index][4]=0;
						copy_xor_tag[j]=1;
					}
					oper_group_index++;
				}
				matrix_index+=k*w;
				matrix_index_matching+=k*w;
			}

			isSchedule[i]=1;
			isSchedule[match[i]]=1;	
		}

		operations[oper_group_index]=malloc(sizeof(int)*5);
		operations[oper_group_index][0]=-1;
	}

	return operations;
}


void xor_encode_once_DGC(int **operations, u8 **src, u8 **dest, int packetsize)
{
	void *buffs[2];
	int oper_group_index=0;
	int xor_bytes=0;

	while(operations[oper_group_index][0]>=0){
		buffs[0]=dest[operations[oper_group_index][2]]+operations[oper_group_index][3]*packetsize;
		buffs[1]=src[operations[oper_group_index][0]]+operations[oper_group_index][1]*packetsize;
		if(operations[oper_group_index][4]){
			//xor_gen(2,packetsize,buffs);
			xor_bytes += packetsize;
		}
		else{
			memcpy(buffs[0], buffs[1], packetsize);
		}
		oper_group_index++;
	}

}

void xor_encode_DGC(int k, int p, int w, int **operations, u8 **src, u8 **dest, int len, int packetsize)
{
	int i,j;
	u8 *src_copy[k];
	u8 *dest_copy[p];
	for(i=0;i<k;i++)
	{
		src_copy[i] = src[i];
	}
	for(i=0;i<p;i++)
	{
		dest_copy[i] = dest[i];
	}

	for(i=0;i<len;i+=packetsize*w)
	{
		xor_encode_once_DGC(operations, src_copy, dest_copy, packetsize);
		for(j=0;j<k;j++)
		{
			src_copy[j] += (packetsize*w);
		}
		for(j=0;j<p;j++)
		{
			dest_copy[j] += (packetsize*w);
		}
	}
}
//-----------------decode---------------------------------------------------------------------------------------------------/
int invert_bitmatrix(int *mat, int *inv, int rows)
{
  int cols, i, j, k;
  int tmp;
 
  cols = rows;

  k = 0;
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      inv[k] = (i == j) ? 1 : 0;
      k++;
    }
  }

  /* First -- convert into upper triangular */

  for (i = 0; i < cols; i++) {

    /* Swap rows if we have a zero i,i element.  If we can't swap, then the 
       matrix was not invertible */

    if ((mat[i*cols+i]) == 0) { 
      for (j = i+1; j < rows && (mat[j*cols+i]) == 0; j++) ;
      if (j == rows) return -1;
      for (k = 0; k < cols; k++) {
        tmp = mat[i*cols+k]; mat[i*cols+k] = mat[j*cols+k]; mat[j*cols+k] = tmp;
        tmp = inv[i*cols+k]; inv[i*cols+k] = inv[j*cols+k]; inv[j*cols+k] = tmp;
      }
    }
 
    /* Now for each j>i, add A_ji*Ai to Aj */
    for (j = i+1; j != rows; j++) {
      if (mat[j*cols+i] != 0) {
        for (k = 0; k < cols; k++) {
          mat[j*cols+k] ^= mat[i*cols+k]; 
          inv[j*cols+k] ^= inv[i*cols+k];
        }
      }
    }
  }

  /* Now the matrix is upper triangular.  Start at the top and multiply down */

  for (i = rows-1; i >= 0; i--) {
    for (j = 0; j < i; j++) {
      if (mat[j*cols+i]) {
        for (k = 0; k < cols; k++) {
          mat[j*cols+k] ^= mat[i*cols+k]; 
          inv[j*cols+k] ^= inv[i*cols+k];
        }
      }
    }
  } 
  return 0;
}
// int xor_decode_PGC_matching(int k, int p, int w, int *bitmatrix, int *eraseds,
//                             char **data_ptrs, char **coding_ptrs, int size, int packetsize)
// {
// 	int i;
// 	u8 **ptrs;
// 	int *decode_bitmatrix;
// 	int **operations;
// 	ptrs = set_up_ptrs_for_opertaions(k, p, eraseds, data_ptrs, coding_ptrs);
// 	if (ptrs == NULL) return -1;
// 	decode_bitmatrix = generate_decoding_bitmatrix(k, p, w, bitmatrix, eraseds);
// 	//schedule = generate_decoding_bitmatrix(k, p, w, bitmatrix, eraseds);
// 	int num=0;
// 	for (i = 0; eraseds[i] != -1; i++) {
//     	num++;
//   	}

// 	int **weighted_graph=bitmatrix_to_weighted_graph(k, num, w, decode_bitmatrix);

// 	int *match=max_weight_matching(num*w,weighted_graph[3][0],weighted_graph[0],weighted_graph[1],weighted_graph[2]);

// 	operations = bitmatrix_to_schedule_PGC_macthing(k, num, w, decode_bitmatrix, match);

// 	if (operations == NULL) {
// 		free(ptrs);
// 		return -1;
// 	}
	
// 	xor_encode_PGC_matching(k, num , w, operations, ptrs, &ptrs[k], size, packetsize);


// 	for (i = 0; operations[i][0] >= 0; i++) 
// 		free(operations[i]);
// 	free(operations[i]);
// 	free(operations);
// 	free(ptrs);

// 	return 0;
// }

u8 **set_up_ptrs_for_opertaions(int k, int p, int *eraseds, char **data_ptrs, char **coding_ptrs,int *recover)
{
  int ddf, cdf;
  int *erased;
  int *visted;
  char **ptrs;
  int i, j, x;

  ddf = 0;
  cdf = 0;
  for (i = 0; eraseds[i] != -1; i++) {
    if (eraseds[i] < k) ddf++; else cdf++;
  }
  
  erased = eraseds_to_erased(k, p, eraseds);
  if (erased == NULL) return NULL;

  /* Set up ptrs.  It will be as follows:

       - If data drive i has not failed, then ptrs[i] = data_ptrs[i].
       - If data drive i has failed, then ptrs[i] = coding_ptrs[j], where j is the 
            lowest unused non-failed coding drive.
       - Elements k to k+ddf-1 are data_ptrs[] of the failed data drives.
       - Elements k+ddf to k+ddf+cdf-1 are coding_ptrs[] of the failed data drives.

       The array row_ids contains the ids of ptrs.
       The array ind_to_row_ids contains the row_id of drive i.
  
       However, we're going to set row_ids and ind_to_row in a different procedure.
   */      
	ptrs = talloc(u8 *, k+p);
	if (!ptrs) {
		free(erased);
		return NULL;
	}
	/* 重新实现指针的赋值*/
	visted = talloc(int, k+p);
	for(i=0;i<k+p;i++)
	{
		visted[i]=0;
	}
	j = 0;

	for(i=0;i<k;i++)
	{
		if(recover[i]<k)
		{
			ptrs[recover[i]] = data_ptrs[recover[i]];
			visted[recover[i]]=recover[i];
		}
		else
		{
			while(visted[j])j++;
			ptrs[j]=coding_ptrs[recover[i]-k];
			visted[j]=j;
			j++;
		}
	}
	x=k;
	for (i = 0; i < k+p; i++) {
		if (erased[i]==1) 
		{
			if(erased[i]<k)
			{
				ptrs[x] = data_ptrs[i];
			}
			else
			{
				ptrs[x] = coding_ptrs[i-k];
			}
			x++;
		}
	}
 
	/* See set_up_ptrs_for_opertaions for how these are set */
//   j = k;
//   x = k;
//   for (i = 0; i < k; i++) {
//     if (erased[i] == 0) {
//       ptrs[i] = data_ptrs[i];
//     } else {
//       while (erased[j]) j++;
//       ptrs[i] = coding_ptrs[j-k];
//       j++;
//       ptrs[x] = data_ptrs[i];
//       x++;
//     }
//   }
//   for (i = k; i < k+p; i++) {
//     if (erased[i]) {
//       ptrs[x] = coding_ptrs[i-k];
//       x++;
//     }
//   }
  
  free(erased);
  return ptrs;
}

// int set_up_ids_for_operations(int k, int p, int *eraseds, int *row_ids, int *ind_to_row)
// {
//   int ddf, cdf;
//   int *erased;
//   int i, j, x;

//   ddf = 0;
//   cdf = 0;
//   for (i = 0; eraseds[i] != -1; i++) {
//     if (eraseds[i] < k) ddf++; else cdf++;
//   }
  
//   erased = eraseds_to_erased(k, p, eraseds);
//   if (erased == NULL) return -1;

//   /* See set_up_ptrs_for_opertaions for how these are set */

//   j = k;
//   x = k;
//   for (i = 0; i < k; i++) {
//     if (erased[i] == 0) {
//       row_ids[i] = i;
//       ind_to_row[i] = i;
//     } else {
//       while (erased[j]) j++;
//       row_ids[i] = j;
//       ind_to_row[j] = i;
//       j++;
//       row_ids[x] = i;
//       ind_to_row[i] = x;
//       x++;
//     }
//   }
//   for (i = k; i < k+p; i++) {
//     if (erased[i]) {
//       row_ids[x] = i;
//       ind_to_row[i] = x;
//       x++;
//     }
//   }
//   free(erased);
//   return 0;
// }
// int *eraseds_to_erased(int k, int p, int *eraseds)
// {
//   int td;
//   int t_non_erased;
//   int *erased;
//   int i;

//   td = k+p;
//   erased = talloc(int, td);
//   if (erased == NULL) return NULL;
//   t_non_erased = td;

//   for (i = 0; i < td; i++) erased[i] = 0;

//   for (i = 0; eraseds[i] != -1; i++) {
//     if (erased[eraseds[i]] == 0) {
//       erased[eraseds[i]] = 1;
//       t_non_erased--;
//       if (t_non_erased < k) {
//         free(erased);
//         return NULL;
//       }
//     }
//   }
//   return erased;
// }

int set_up_ids_for_operations(int k, int p, int *eraseds, int *row_ids, int *ind_to_row,int *recover)
{
	int ddf, cdf;
	int *erased;
	int i, j, x;

	ddf = 0;
	cdf = 0;
	for (i = 0; eraseds[i] != -1; i++) {
		if (eraseds[i] < k) ddf++; else cdf++;
	}
	
	erased = eraseds_to_erased(k, p, eraseds);
	if (erased == NULL) return -1;
	for(i=0;i<k+p;i++)
	{
		row_ids[i]=0;
	}
	/* See set_up_ptrs_for_opertaions for how these are set */

	j = 0;
	for(i=0;i<k;i++)
	{
		if(recover[i] < k)
		{
			row_ids[recover[i]]=recover[i];
			ind_to_row[recover[i]] = recover[i];
		}
		else
		{
			while(row_ids[j]) j++;
			row_ids[j]=recover[i];
			ind_to_row[j] = recover[i];
			j++;
		}
	}
	// printf("%d\n",j);
	x=k;
	for (i = 0; i < k+p; i++) {
		if (erased[i]==1) 
		{
			row_ids[x] = i;
			ind_to_row[x] = i;
			x++;
		}
	}
	// j = k;
	// x = k;
	//   for (i = 0; i < k; i++) {
	//     if (erased[i] == 0) {
	//       row_ids[i] = i;
	//       ind_to_row[i] = i;
	//     } else {
	//       while (erased[j]) j++;
	//       row_ids[i] = j;
	//       ind_to_row[j] = i;
	//       j++;
	//       row_ids[x] = i;
	//       ind_to_row[i] = x;
	//       x++;
	//     }
	//   }
	// for (i = k; i < k+p; i++) {
	// 	if (erased[i]) {
	// 	row_ids[j] = i;
	// 	ind_to_row[i] = j;
	// 	j++;
	// 	}
	// }
	free(erased);
	return 0;
}

int *eraseds_to_erased(int k, int p, int *eraseds)
{
  int td;
  int t_non_erased;
  int *erased;
  int i;

  td = k+p;
  erased = talloc(int, td);
  if (erased == NULL) return NULL;
  t_non_erased = td;

  for (i = 0; i < td; i++) erased[i] = 0;

  for (i = 0; eraseds[i] != -1; i++) {
    if (erased[eraseds[i]] == 0) {
      erased[eraseds[i]] = 1;
      t_non_erased--;
      if (t_non_erased < k) {
        free(erased);
        return NULL;
      }
    }
  }
  return erased;
}

int find_best_decode(int n,int m,int index,int num,int *survives,int *recover,int *de_recover,int k, int p, int w, int *bitmatrix, int *eraseds,int* min_num)
{
    if(num==m)
    {
		int i=0;
		int error_num=0;
		int count=0;
		while(eraseds[i]!=-1)
		{
			i++;
			error_num++;
		}
		int * decode_bitmatrix = generate_decoding_bitmatrix(k, p, w, bitmatrix, eraseds,recover);
		int **de_weighted_graph = bitmatrix_to_weighted_graph(k, error_num, w, decode_bitmatrix);
		int *de_match = max_weight_matching(error_num * w,de_weighted_graph[3][0],de_weighted_graph[0],de_weighted_graph[1],de_weighted_graph[2]);
		int ** de_operations = bitmatrix_to_schedule_PGC_macthing(k, error_num, w, decode_bitmatrix, de_match);
		int *xorMatch_num =count_xorMatch_num(k,p,w,de_operations);
		count = xorMatch_num[0];
		if(count < min_num[0])
		{
			min_num[0]= count;
			for(i=0;i<m;i++)
			{
				//printf("%d ",recover[i]);
				de_recover[i]=recover[i];
			}
			//printf("%d\n",count);
		}

        return 0;
    }
    if(index==n || num+(n-index)< m )
    {
        return 0;
    }
    recover[num]=survives[index];
    find_best_decode(n,m,index+1,num+1,survives,recover,de_recover,k,p,w,bitmatrix,eraseds,min_num);
    find_best_decode(n,m,index+1,num,survives,recover,de_recover,k,p,w,bitmatrix,eraseds,min_num);
    return 0;
}

int *generate_decoding_bitmatrix(int k, int p, int w, int *bitmatrix, int *eraseds,int *recover)
{
  int i, j, x, drive, y, index, z;
  int *decoding_matrix, *inverse, *real_decoding_matrix;
  int *ptr;
  int *row_ids;
  int *ind_to_row;
  int ddf, cdf;
  int **schedule;
  int *b1, *b2;
 
 /* First, figure out the number of data drives that have failed, and the
    number of coding drives that have failed: ddf and cdf */

  ddf = 0;
  cdf = 0;
  for (i = 0; eraseds[i] != -1; i++) {
    if (eraseds[i] < k) ddf++; else cdf++;
  }
  
  row_ids = talloc(int, k+p);
  if (!row_ids) return NULL;
  ind_to_row = talloc(int, k+p);
  if (!ind_to_row) {
    free(row_ids);
    return NULL;
  }

  if (set_up_ids_for_operations(k, p, eraseds, row_ids, ind_to_row,recover) < 0) {
    free(row_ids);
    free(ind_to_row);
    return NULL;
  }
	// for(i=0;i < k+p;i++)
	// {
	// 	printf("%d ",row_ids[i]);
	// }
	// printf("\n");
  /* Now, we're going to create one decoding matrix which is going to 
     decode everything with one call.  The hope is that the scheduler
     will do a good job.    This matrix has w*e rows, where e is the
     number of eraseds (ddf+cdf) */
  real_decoding_matrix = talloc(int, k*w*(cdf+ddf)*w);
  if (!real_decoding_matrix) {
    free(row_ids);
    free(ind_to_row);
    return NULL;
  }

  /* First, if any data drives have failed, then initialize the first
     ddf*w rows of the decoding matrix from the standard decoding
     matrix inversion */

  if (ddf > 0) {
    
    decoding_matrix = talloc(int, k*k*w*w);
    if (!decoding_matrix) {
      free(row_ids);
      free(ind_to_row);
      return NULL;
    }
    ptr = decoding_matrix;
    for (i = 0; i < k; i++) {
      if (row_ids[i] == i) {
        bzero(ptr, k*w*w*sizeof(int));
        for (x = 0; x < w; x++) {
          ptr[x+i*w+x*k*w] = 1;
        } 
      } else {
        memcpy(ptr, bitmatrix+k*w*w*(row_ids[i]-k), k*w*w*sizeof(int));
      }
      ptr += (k*w*w);
    }
    inverse = talloc(int, k*k*w*w);
    if (!inverse) {
      free(row_ids);
      free(ind_to_row);
      free(decoding_matrix);
      return NULL;
    }
	// for(i=0;i<k*w;i++)
	// {
	// 	for(j=0;j<k*w;j++)
	// 	{
	// 		printf("%d ",decoding_matrix[i*k+j]);
	// 	}
	// 	printf("\n");
	// }
    invert_bitmatrix(decoding_matrix, inverse, k*w);

/*    printf("\nMatrix to invert\n");
    jerasure_print_bitmatrix(decoding_matrix, k*w, k*w, w);
    printf("\n");
    printf("\nInverse\n");
    jerasure_print_bitmatrix(inverse, k*w, k*w, w);
    printf("\n"); */

    free(decoding_matrix);
    ptr = real_decoding_matrix;
    for (i = 0; i < ddf; i++) {
      memcpy(ptr, inverse+k*w*w*row_ids[k+i], sizeof(int)*k*w*w);
      ptr += (k*w*w);
    }
    free(inverse);
  } 

  /* Next, here comes the hard part.  For each coding node that needs
     to be decoded, you start by putting its rows of the distribution
     matrix into the decoding matrix.  If there were no failed data
     nodes, then you're done.  However, if there have been failed
     data nodes, then you need to modify the columns that correspond
     to the data nodes.  You do that by first zeroing them.  Then
     whereever there is a one in the distribution matrix, you XOR
     in the corresponding row from the failed data node's entry in
     the decoding matrix.  The whole process kind of makes my head
     spin, but it works.
   */

  for (x = 0; x < cdf; x++) {
    drive = row_ids[x+ddf+k]-k;
    ptr = real_decoding_matrix + k*w*w*(ddf+x);
    memcpy(ptr, bitmatrix+drive*k*w*w, sizeof(int)*k*w*w);

    for (i = 0; i < k; i++) {
      if (row_ids[i] != i) {
        for (j = 0; j < w; j++) {
          bzero(ptr+j*k*w+i*w, sizeof(int)*w);
        }
      }  
    }

    /* There's the yucky part */

    index = drive*k*w*w;
    for (i = 0; i < k; i++) {
      if (row_ids[i] != i) {
        b1 = real_decoding_matrix+(ind_to_row[i]-k)*k*w*w;
        for (j = 0; j < w; j++) {
          b2 = ptr + j*k*w;
          for (y = 0; y < w; y++) {
            if (bitmatrix[index+j*k*w+i*w+y]) {
              for (z = 0; z < k*w; z++) {
                b2[z] = b2[z] ^ b1[z+y*k*w];
              }
            }
          }
        }
      }  
    }
  }

  free(row_ids);
  free(ind_to_row);
  return real_decoding_matrix;
}
int count_bitmatrix_one_number(int k,int p,int w,int *bitmatrix,int begin,int end)
{
	int one_num=0;
	int i=0;
	for(i=0;i<k * w * (end-begin) * w;i++)
	{
		if(bitmatrix[i+begin*w*w*k]==1)
		{
			one_num++;
		}
	}
	//printf("%d\n",one_num);
	return one_num;
}

int* count_native_num(int k,int p,int w,int x)
{
	int *nums=malloc(sizeof(int)*3);
	nums[0]=2*x-p*w;
	nums[1]=x;
	nums[2]=x-p*w;
	//printf("%d %d %d\n",nums[0],nums[1],nums[2]);
	return nums;
}

int* count_noMatch_num(int k,int p,int w,int x)
{
	int *nums=malloc(sizeof(int)*3);
	nums[0]=x;
	nums[1]=p*w;
	nums[2]=x-p*w;
	//printf("%d %d %d\n",nums[0],nums[1],nums[2]);
	return nums;
}
int *count_xorMatch_num(int k,int p,int w,int **operations)
{
	int *nums=malloc(sizeof(int)*3);
	int oper_group_index=0;
	nums[0]=0;
	nums[1]=p*w;
	nums[2]=0;
	while(operations[oper_group_index][0]>=0){
		nums[0] = nums[0]+ operations[oper_group_index][0] + operations[oper_group_index][1] + operations[oper_group_index][2];
		nums[2] =nums[2] + operations[oper_group_index][0]+operations[oper_group_index][1] + operations[oper_group_index][2]-1;
		oper_group_index++;
	}
	return nums;
}
void *count_xorMatch_size(int k,int p,int w,int **operations)
{
	double constant=10.0/(k*w);
	int *nums=malloc(sizeof(int)*3);
	int oper_group_index=0;
	nums[0]=0;
	nums[1]=p*w;
	nums[2]=0;
	while(operations[oper_group_index][0]>=0){
		nums[0] = operations[oper_group_index][0] + operations[oper_group_index][1] + operations[oper_group_index][2];
		nums[2] = operations[oper_group_index][0]+operations[oper_group_index][1] + operations[oper_group_index][2]-1;
		printf("%lf %lf\n",nums[0]*constant,nums[2]*constant);
		oper_group_index++;
	}

}

void count_native_size_Imp(int k,int p,int w,int *bitmatrix)
{
	int i=0,index=0;
	int begin=0;
	int end=2;
	//默认是datasize为10MB
	double constant=10.0/(k*w);
	for(index=0;index<p;index++)
	{
		int x=count_bitmatrix_one_number(k,p,w,bitmatrix,index,index+1);
		//printf("%d ",x);
		int *native_num =count_native_num(k,1,w,x);
		double *native=malloc(sizeof(double)*3);
		for(i=0;i<3;i++)
		{
				//printf("%d\n",xorMatch_num[i]);
				native[i]=native_num[i]*constant;
				
				//printf("%lf %lf ",xorMatch_size[i],native_size[i]);
				printf("%lf ",native[i]);
		}
		printf("\n");
	}

	//printf("\n");
}

void count_noMatch_size_Imp(int k,int p,int w,int *bitmatrix)
{
	int i=0,index=0;
	int begin=0;
	int end=2;
	//默认是datasize为10MB
	double constant=10.0/(k*w);
	for(index=0;index<p;index++)
	{
		int x=count_bitmatrix_one_number(k,1,w,bitmatrix,index,index+1);
		printf("%d ",x);
		int *noMatch_num =count_noMatch_num(k,1,w,x);
		double *noMatch=malloc(sizeof(double)*3);
		for(i=0;i<3;i++)
		{
				//printf("%d\n",xorMatch_num[i]);
				noMatch[i]=noMatch_num[i]*constant;
				
				//printf("%lf %lf ",xorMatch_size[i],native_size[i]);
				printf("%lf ",noMatch[i]);
		}
		printf("\n");
	}

	//printf("\n");
}

double count_xorMatch_size_Imp(int k,int p,int w,int **operations,int *bitmatrix)
{
	int i=0;
	//默认是datasize为10MB
	double constant=10.0/(k*w);
	int x=count_bitmatrix_one_number(k,p,w,bitmatrix,0,p);
	int *xorMatch_num =count_xorMatch_num(k,p,w,operations);
	double *xorMatch_size=malloc(sizeof(double)*3);
	double *native_size=malloc(sizeof(double)*3);
	//printf("%d\n",x);
	//printf("%d\n",xorMatch_num[0]);
	for(i=0;i<3;i++)
	{
		xorMatch_size[i]=xorMatch_num[i]*constant;
		//printf("%lf\n",xorMatch_size[i]);
		//printf("%lf %lf ",xorMatch_size[i],native_size[i]);
	}
	return xorMatch_size[0];//printf("\n");
}
// int *generate_decoding_bitmatrix(int k, int p, int w, int *bitmatrix, int *eraseds)
// {
//   int i, j, x, drive, y, index, z;
//   int *decoding_matrix, *inverse, *real_decoding_matrix;
//   int *ptr;
//   int *row_ids;
//   int *ind_to_row;
//   int ddf, cdf;
//   int **schedule;
//   int *b1, *b2;
 
//  /* First, figure out the number of data drives that have failed, and the
//     number of coding drives that have failed: ddf and cdf */

//   ddf = 0;
//   cdf = 0;
//   for (i = 0; eraseds[i] != -1; i++) {
//     if (eraseds[i] < k) ddf++; else cdf++;
//   }
  
//   row_ids = talloc(int, k+p);
//   if (!row_ids) return NULL;
//   ind_to_row = talloc(int, k+p);
//   if (!ind_to_row) {
//     free(row_ids);
//     return NULL;
//   }

//   if (set_up_ids_for_operations(k, p, eraseds, row_ids, ind_to_row) < 0) {
//     free(row_ids);
//     free(ind_to_row);
//     return NULL;
//   }

//   /* Now, we're going to create one decoding matrix which is going to 
//      decode everything with one call.  The hope is that the scheduler
//      will do a good job.    This matrix has w*e rows, where e is the
//      number of eraseds (ddf+cdf) */
//   real_decoding_matrix = talloc(int, k*w*(cdf+ddf)*w);
//   if (!real_decoding_matrix) {
//     free(row_ids);
//     free(ind_to_row);
//     return NULL;
//   }

//   /* First, if any data drives have failed, then initialize the first
//      ddf*w rows of the decoding matrix from the standard decoding
//      matrix inversion */

//   if (ddf > 0) {
    
//     decoding_matrix = talloc(int, k*k*w*w);
//     if (!decoding_matrix) {
//       free(row_ids);
//       free(ind_to_row);
//       return NULL;
//     }
//     ptr = decoding_matrix;
//     for (i = 0; i < k; i++) {
//       if (row_ids[i] == i) {
//         bzero(ptr, k*w*w*sizeof(int));
//         for (x = 0; x < w; x++) {
//           ptr[x+i*w+x*k*w] = 1;
//         } 
//       } else {
//         memcpy(ptr, bitmatrix+k*w*w*(row_ids[i]-k), k*w*w*sizeof(int));
//       }
//       ptr += (k*w*w);
//     }
//     inverse = talloc(int, k*k*w*w);
//     if (!inverse) {
//       free(row_ids);
//       free(ind_to_row);
//       free(decoding_matrix);
//       return NULL;
//     }
//     invert_bitmatrix(decoding_matrix, inverse, k*w);

// /*    printf("\nMatrix to invert\n");
//     jerasure_print_bitmatrix(decoding_matrix, k*w, k*w, w);
//     printf("\n");
//     printf("\nInverse\n");
//     jerasure_print_bitmatrix(inverse, k*w, k*w, w);
//     printf("\n"); */

//     free(decoding_matrix);
//     ptr = real_decoding_matrix;
//     for (i = 0; i < ddf; i++) {
//       memcpy(ptr, inverse+k*w*w*row_ids[k+i], sizeof(int)*k*w*w);
//       ptr += (k*w*w);
//     }
//     free(inverse);
//   } 

//   /* Next, here comes the hard part.  For each coding node that needs
//      to be decoded, you start by putting its rows of the distribution
//      matrix into the decoding matrix.  If there were no failed data
//      nodes, then you're done.  However, if there have been failed
//      data nodes, then you need to modify the columns that correspond
//      to the data nodes.  You do that by first zeroing them.  Then
//      whereever there is a one in the distribution matrix, you XOR
//      in the corresponding row from the failed data node's entry in
//      the decoding matrix.  The whole process kind of makes my head
//      spin, but it works.
//    */

//   for (x = 0; x < cdf; x++) {
//     drive = row_ids[x+ddf+k]-k;
//     ptr = real_decoding_matrix + k*w*w*(ddf+x);
//     memcpy(ptr, bitmatrix+drive*k*w*w, sizeof(int)*k*w*w);

//     for (i = 0; i < k; i++) {
//       if (row_ids[i] != i) {
//         for (j = 0; j < w; j++) {
//           bzero(ptr+j*k*w+i*w, sizeof(int)*w);
//         }
//       }  
//     }

//     /* There's the yucky part */

//     index = drive*k*w*w;
//     for (i = 0; i < k; i++) {
//       if (row_ids[i] != i) {
//         b1 = real_decoding_matrix+(ind_to_row[i]-k)*k*w*w;
//         for (j = 0; j < w; j++) {
//           b2 = ptr + j*k*w;
//           for (y = 0; y < w; y++) {
//             if (bitmatrix[index+j*k*w+i*w+y]) {
//               for (z = 0; z < k*w; z++) {
//                 b2[z] = b2[z] ^ b1[z+y*k*w];
//               }
//             }
//           }
//         }
//       }  
//     }
//   }

//   free(row_ids);
//   free(ind_to_row);
//   return real_decoding_matrix;
// }