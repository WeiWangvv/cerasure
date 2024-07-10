#include "vandermonde.h"
#include "galois.h"

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

#define MAX_CHECK 63		/* Size is limited by using uint64_t to represent subsets */
#define M_MAX 0x3F
#define K_MAX 0x3F
#define ROWS M_MAX
#define COLS K_MAX
int gf_invert_matrix_w(unsigned int *in_mat, unsigned int *out_mat, const int n,int w)
{
	int i, j, k;
	unsigned int temp;
    // printf("--------\n");
    // for(i=0;i<n;i++)
    // {
    //     for(j=0;j<n;j++)
    //     {
    //         printf("%d ",in_mat[i*n+j]);
    //     }
    //     printf("\n");
    // }
	// Set out_mat[] to the identity matrix
	for (i = 0; i < n * n; i++)	// memset(out_mat, 0, n*n)
		out_mat[i] = 0;

	for (i = 0; i < n; i++)
		out_mat[i * n + i] = 1;
	// Inverse
	for (i = 0; i < n; i++) {
		// Check for 0 in pivot element
		if (in_mat[i * n + i] == 0) {
			// Find a row with non-zero in current column and swap
			for (j = i + 1; j < n; j++)
				if (in_mat[j * n + i])
					break;
			if (j == n)	// Couldn't find means it's singular
				return -1;
			for (k = 0; k < n; k++) {	// Swap rows i,j
				temp = in_mat[i * n + k];
				in_mat[i * n + k] = in_mat[j * n + k];
				in_mat[j * n + k] = temp;

				temp = out_mat[i * n + k];
				out_mat[i * n + k] = out_mat[j * n + k];
				out_mat[j * n + k] = temp;
			}
		}
		temp = galois_inverse(in_mat[i * n + i],w);	// 1/pivot
        
		for (j = 0; j < n; j++) {	// Scale row i by 1/pivot
			in_mat[i * n + j] = galois_single_multiply(in_mat[i * n + j], temp,w);
			out_mat[i * n + j] = galois_single_multiply(out_mat[i * n + j], temp,w);
		}

		for (j = 0; j < n; j++) {
			if (j == i)
				continue;

			temp = in_mat[j * n + i];
			for (k = 0; k < n; k++) {
				out_mat[j * n + k] ^= galois_single_multiply(temp, out_mat[i * n + k],w);
				in_mat[j * n + k] ^= galois_single_multiply(temp, in_mat[i * n + k],w);
			}
		}
	}
	return 0;
}

static inline int min(int a, int b)
{
	if (a <= b)
		return a;
	else
		return b;
}

void gen_sub_matrix(unsigned int *out_matrix, unsigned int *in_matrix, int rows,
		    int cols, unsigned int row_indicator, unsigned int col_indicator)
{
	int i, j, r, s,dim;
    int p=rows-cols;
    dim=0;
    // printf("-------\n");
    //printf("%d\n",row_indicator);
    for(i = 0; i < cols; i++)
    {
        if (!(col_indicator & ((uint64_t) 1 << i)))
        {
            for (j = 0 ; j < cols; j++) {
                out_matrix[dim * cols + j] = in_matrix[cols * i + j];
            }
            dim++;
        }
        // printf("%d ",i);
    }
    //printf("%d\n",dim);
    for(i=0;i<p;i++)
    {
        if (row_indicator & ((uint64_t) 1 << i))
        {
            for (j = 0 ; j < cols; j++) {
                //printf("%d ",in_matrix[cols * (i+cols) + j]);
			    out_matrix[dim * cols + j] = in_matrix[cols * (i+cols) + j];
                //printf("%d ",out_matrix[dim * cols + j]);
            }
            dim++;
        }
    }
    //printf("%d\n",dim);
    //printf("%d\n",row_indicator);
	// for (i = 0, r = 0; i < rows; i++) {
    //     //printf("----%d %d %d\n",row_indicator,((uint64_t) 1 << i)),(row_indicator & ((uint64_t) 1 << i));
	// 	if (!(row_indicator & ((uint64_t) 1 << i)))
	// 		continue;

	// 	for (j = 0, s = 0; j < cols; j++) {
	// 		if (!(col_indicator & ((uint64_t) 1 << j)))
	// 			continue;
	// 		out_matrix[dim * r + s] = in_matrix[cols * i + j];
            
	// 		s++;
	// 	}
	// 	r++;
	// }
   	// for (i = 0; i < rows; i++) {
	// 	for (j = 0; j < cols; j++) {
    //         printf("%d ",in_matrix[cols * i + j]);
	// 	}
    //     printf("\n");
	// } 
    // for (i = 0; i < cols; i++) {
	// 	for (j = 0; j < cols; j++) {
    //         printf("%d ",out_matrix[cols * i + j]);
	// 	}
    //     printf("\n");
	// } 
    // printf("\n");
}

/* Gosper's Hack */
uint64_t next_subset(uint64_t * subset, uint64_t element_count, uint64_t subsize)
{
	uint64_t tmp1 = *subset & -*subset;
	uint64_t tmp2 = *subset + tmp1;
	*subset = (((*subset ^ tmp2) >> 2) / tmp1) | tmp2;
	if (*subset & (((uint64_t) 1 << element_count))) {
		/* Overflow on last subset */
		*subset = ((uint64_t) 1 << subsize) - 1;
		return 1;
	}

	return 0;
}

int are_submatrices_singular(unsigned int *vmatrix, int rows, int cols,int w)
{
    int i,j;
	unsigned int *matrix;
	unsigned int *invert_matrix;
    matrix=malloc(cols*cols*sizeof(unsigned int));
    invert_matrix=malloc(cols*cols*sizeof(unsigned int));
	uint64_t row_indicator, col_indicator, subset_init, subsize;
    //选取k 行 k列
	/* Check all square subsize x subsize submatrices of the rows x cols
	 * vmatrix for singularity*/
	for (subsize = 1; subsize <= min(rows-cols, cols); subsize++) {
		subset_init = (1 << subsize) - 1;
		col_indicator = subset_init;
        //printf("%d\n",subset_init);
        int count=0;
		do {
            //printf("%d\n",count);
            count++;
			row_indicator = subset_init;
			do {
				gen_sub_matrix(matrix, vmatrix, rows, cols, row_indicator, col_indicator);
				if (gf_invert_matrix_w(matrix, invert_matrix, cols,w))
                {
                    // printf("--------\n");
                    // for(i=0;i<cols;i++)
                    // {
                    //     for(j=0;j<cols;j++)
                    //     {
                    //         printf("%d ",invert_matrix[i*cols+j]);
                    //     }
                    //     printf("\n");
                    // }
                    return 1;
                }
			} while (next_subset(&row_indicator, rows-cols, subsize) == 0);
		} while (next_subset(&col_indicator, cols, subsize) == 0);
        // printf("%d\n",count);
	}
	return 0;
}


void gen_sub_matrix_w(unsigned int *out_matrix, int dim, unsigned int *in_matrix, int rows,
		    int cols, uint64_t row_indicator, uint64_t col_indicator)
{
	int i, j, r, s;
	for (i = 0, r = 0; i < rows; i++) {
		if (!(row_indicator & ((uint64_t) 1 << i)))
			continue;
		for (j = 0, s = 0; j < cols; j++) {
			if (!(col_indicator & ((uint64_t) 1 << j)))
				continue;
			out_matrix[dim * r + s] = in_matrix[cols * i + j];
			s++;
		}
		r++;
	}
}

int are_submatrices_singular_w(unsigned int *vmatrix, long long int rows, long long int cols,int w)
{
	unsigned int matrix[COLS * COLS];
	unsigned int invert_matrix[COLS * COLS];
	uint64_t row_indicator, col_indicator, subset_init, subsize;

	/* Check all square subsize x subsize submatrices of the rows x cols
	 * vmatrix for singularity*/
    subsize=min(rows, cols);
    subset_init = (1 << subsize) - 1;
    //printf("%lld\n",subsize);
    int number=0;
    int i=0;
    int j=0;
    col_indicator = subset_init;
    do {
        row_indicator = subset_init;
        //printf("%d %d\n",row_indicator,col_indicator);
        do {
            //printf("%d\n",number++);
            gen_sub_matrix_w(matrix, subsize, vmatrix, rows,
                        cols, row_indicator, col_indicator);
            // for(i=0;i<cols;i++)
            // {
            //     for(j=0;j<cols;j++)
            //     {
            //         printf("%d ",matrix[i*cols+j]);
            //     }
            //     printf("\n");
            // }
            if (gf_invert_matrix_w(matrix, invert_matrix, subsize,w)==-1)
                return 1;
        } while (next_subset(&row_indicator, rows, subsize) == 0);
    } while (next_subset(&col_indicator, cols, subsize) == 0);

	return 0;
}

//检查矩阵是否满足MDS性质
int check_coding_matrix_w(int k, int m, int w,unsigned int* encode_matrix)
{
    unsigned int *matrix;
    int i,j;
    matrix=malloc(k * (k+m) * sizeof(unsigned int));
	memset(matrix, 0, k * (k+m) );
	for (i = 0; i < k; i++)
    {
        matrix[k * i + i] = 1;
    }
	for (i = k; i < (m+k); i++) {
		for (j = 0; j < k; j++) {
			matrix[k * i + j] = encode_matrix[(i-k)*k+j];
		}
	}
    // for(i=0;i < (m+k); i++)
    // {
    //     for (j = 0; j < k; j++) {
	// 		printf("%d ", matrix[k * i + j]) ;
	// 	}
    //     printf("\n");
    // }
    if(are_submatrices_singular_w(matrix,k+m,k,w))
    {
        // free(matrix);
        //printf("No MDS\n");
        return 1;
    }
    // free(matrix);
    //printf(" MDS\n");
    return 0;
}

unsigned int *vandermonde_sub_coding_matrix(int k, int m, int w){
    int i,j,tmp,elem;
    unsigned int *matrix;
    matrix=malloc(sizeof(unsigned int)*k*m);
    elem=1;
    for(i=0;i<m;i++)
    {
        tmp=1;
        for(j=0;j<k;j++)
        {
            matrix[i*k+j]=tmp;
            tmp=galois_single_multiply(tmp,elem,w);
        }
        elem=galois_single_multiply(elem,2,w);
    }
    return matrix;
}

//检查矩阵是否满足MDS性质
int check_coding_matrix(int k, int m, int w,unsigned int* encode_matrix)
{
    unsigned int *matrix;
    int i,j;
    matrix=malloc(k * (k+m) * sizeof(unsigned int));
	memset(matrix, 0, k * (k+m) );
	for (i = 0; i < k; i++)
    {
        matrix[k * i + i] = 1;
    }
	for (i = k; i < (m+k); i++) {
		for (j = 0; j < k; j++) {
			matrix[k * i + j] = encode_matrix[(i-k)*k+j];
		}
	}
    if(are_submatrices_singular(matrix,k+m,k,w))
    {
        free(matrix);
        return 1;
    }
    free(matrix);
    return 0;
}
int count_one_numbers(int k, int m, int w, unsigned int* bitmatrix)
{
    int count=0;
    int i,j;
    for(i=0;i<m*w;i++)
    {
        for(j=0;j<k*w;j++)
        {
            if(bitmatrix[i*k*w+j])
            {
                count++;
            }
        }
    }
    return count;
}
unsigned int* gen_best_matrix(int k, int m, int w)
{
    unsigned int* encode_matrix_sub;
    unsigned int* encode_matrix_cauchy;
	int* encode_bitmatrix;
    int count_sub=0;
    int count_cauchy=0;
	encode_matrix_sub=vandermonde_sub_coding_matrix(k,m,w);
    encode_matrix_cauchy= cauchy_good_general_coding_matrix(k,m,w);
	int flag=check_coding_matrix_w(k,m,w,encode_matrix_sub);
	if(flag)
    {
        //printf("encode_matrix_cauchy\n");
        return encode_matrix_cauchy;
    }
	encode_bitmatrix=matrix_to_bitmatrix(k, m, w, encode_matrix_sub);
    count_sub=count_one_numbers(k,m,w,encode_bitmatrix);
    encode_bitmatrix=matrix_to_bitmatrix(k, m, w, encode_matrix_cauchy);
    count_cauchy=count_one_numbers(k,m,w,encode_bitmatrix);
    if(count_sub < count_cauchy)
    {
        //printf("encode_matrix_sub\n");
        return encode_matrix_sub;
    }
    //printf("encode_matrix_cauchy\n");
    return encode_matrix_cauchy;
}

void vandermonde_R(int n,int w,int a)
{
    int i;
    int tmp=1;
    //int *R=malloc(sizeof(int)*n);
    for(i=0;i<n-1;i++)
    {
        printf("%d,",tmp);
        tmp=galois_single_multiply(tmp,a,w);
    }
    printf("%d",tmp);
}

void vandermonde_all(w)
{
    int i;
    int n = 1;
    for(i=0;i<w;i++)
    {
        n=n*2;
    } 
    printf("{");
    for(i=1;i<n-1;i++)
    {
        printf("{");
        vandermonde_R(n,w,i);
        printf("},");
    }
    printf("{");
    vandermonde_R(n,w,i);
    printf("}");
    printf("}");
}

void count_one_R(int n,int w,int a)
{
    int i;
    int tmp=a;
    int count=0;
    int* encode_bitmatrix;
    for(i=0;i<n-1;i++)
    {
        encode_bitmatrix=matrix_to_bitmatrix(1, 1, w, &R_w4[a-1][i]);
        count+=count_one_numbers(1,1,w,encode_bitmatrix);
        printf("%d,",count);
    }
    encode_bitmatrix=matrix_to_bitmatrix(1, 1, w, &R_w4[a-1][i]);
    count+=count_one_numbers(1,1,w,encode_bitmatrix);
    printf("%d",count);
}

void count_one_R_all(int w)
{
    int i;
    int n = 1;
    for(i=0;i<w;i++)
    {
        n=n*2;
    } 
    printf("{");
    for(i=1;i<n-1;i++)
    {
        printf("{");
        count_one_R(n,w,i);
        printf("},");
    }
    printf("{");
    count_one_R(n,w,i);
    printf("}");
    printf("}");
}
int comp(const R_one* a,const R_one* b)//用来做比较的函数。
{
    return (*a).num-(*b).num;
}
//纯贪心方式
unsigned int *vandermonde_sub_coding_matrix_greedy(int k, int m, int w,int count_cauchy,int* flag){
    //int nums=100;
    int loop=0;
    int i,j,tmp,elem,index;
    int n = 1;
    for(i=0;i<w;i++)
    {
        n=n*2;
    } 
    R_one order[n];
    unsigned int *matrix;
    int* encode_bitmatrix;
    int parents[n];
    int (*p)[n];
    int (*p_ones)[n];
    index_order indexs[m];
    int count=0;
    matrix=malloc(sizeof(unsigned int)*k*m);
    for(i=0;i<n;i++)
    {
        parents[i]=0;
    }
    if(w==8)
    {
        //printf("!!8\n");
        p=R_w8;
        p_ones=R_ones_w8;
    }
    else if(w==4)
    {
        //printf("!! 4\n");
        p=R_w4;
        p_ones=R_ones_w4;
        //printf("\n%d\n",p[15][15]);
    }
    for(i=0;i<n-1;i++)
    {
        order[i].index = i;
        order[i].num = p_ones[i][k-1];
    }
    qsort(order,n-1,sizeof(R_one),comp);
    // for(i=0;i<n;i++)
    // {
    //     printf(" %d %d ",order[i].index,order[i].num );
    // }
    // printf("\n");
    for(i=0;i<m;i++)
    {
        indexs[i].index=order[i].index;
        indexs[i].order_index=i;
        //printf("%d ",indexs[i]);
        parents[indexs[i].index]=1;
    }
    //printf("\n");
    for(i=0;i<m;i++)
    {
        for(j=0;j<k;j++)
        {
            matrix[i*k+j]=p[indexs[i].index][j];
            //printf("%d ",matrix[i*k+j]);
        }
    }
    //printf("\n");
    encode_bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
    count=count_one_numbers(k,m,w,encode_bitmatrix);
    int tmp_begin_index=-1;
    int tmp_end_index=-1;
    int tmp_num;
    int tmp_min;
    while(count < count_cauchy && check_coding_matrix_w(k,m,w,matrix))
    {
        tmp_min=1<<30;
        for(i=0;i<m;i++)
        {
            j= 1 + indexs[i].order_index;
            //printf("%d %d\n",i,j);
            while( j < n-1 && parents[order[j].index]==1)
            {
                j++;
            }
            //printf("%d %d %d\n",i,j,tmp_min);
            if(j < n-1)
            {
                tmp_num=p_ones[order[j].index][k-1]-p_ones[indexs[i].index][k-1];
                if(tmp_min >= tmp_num)
                {
                    tmp_min=tmp_num;
                    tmp_begin_index=i;
                    tmp_end_index=j;
                }
            }
            //printf("%d %d %d\n",i,j,tmp_min);
        }
        //printf("!!!%d %d %d\n",tmp_begin_index,tmp_end_index,tmp_min);
        parents[indexs[tmp_begin_index].index]=0;
        parents[order[tmp_end_index].index]=1;
        //parents[tmp_begin_index]==0;
        // parents[tmp_end_index]=1;
        indexs[tmp_begin_index].order_index = tmp_end_index;
        indexs[tmp_begin_index].index=order[tmp_end_index].index;
        for(i=0;i<m;i++)
        {
            for(j=0;j<k;j++)
            {
                matrix[i*k+j]=p[indexs[i].index][j];
                //printf("%d ",p[indexs[i].index][j]);
            }
        }
        encode_bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
        count=count_one_numbers(k,m,w,encode_bitmatrix);
        loop++;
        // if(loop>nums)
        // {
        //     break;
        // }
        //printf("%d \n",loop);
        //printf("%d\n",count);
        //break;
        //进行R的替换
    }
    // printf("%d\n",loop);
    if(count >= count_cauchy )
    {
        *flag = 0;
        return matrix;
    }
    *flag=1;
    return matrix;
}

unsigned int* gen_best_matrix_all(int k, int m, int w)
{
    unsigned int* encode_matrix_sub;
    unsigned int* encode_matrix_cauchy;
    int flag=0;
	int* encode_bitmatrix_cauchy;
    int count_sub=0;
    int count_cauchy=0;
    encode_matrix_cauchy= cauchy_good_general_coding_matrix(k,m,w);
    encode_bitmatrix_cauchy=matrix_to_bitmatrix(k, m, w, encode_matrix_cauchy);
    count_cauchy=count_one_numbers(k,m,w,encode_bitmatrix_cauchy);
    encode_matrix_sub=vandermonde_sub_coding_matrix_greedy(k, m, w,count_cauchy,&flag);
    //printf("%d\n",flag);
    if(flag==1)
    {
        return encode_matrix_sub;
    }
    return encode_matrix_cauchy;
}

int comp_desc(const matrix_num_index* a,const matrix_num_index* b)//用来做比较的函数。
{
    return (*b).num-(*a).num;
}

int comp_asce(const matrix_num_index* a,const matrix_num_index* b)//用来做比较的函数。
{
    return (*a).num-(*b).num;
}

unsigned int* gen_best_matrix_from_any_element_lowtime(int k, int m, int w)
{
    unsigned int n = 1;
    unsigned int i,j,ii;
    int count_old, count_new;
    unsigned int backup;
    for(i=0;i<w;i++)
    {
        n=n*2;
    } 
    unsigned int* encode_matrix_best_base=gen_best_matrix_all(k,m,w);

    // unsigned int* matrix=cauchy_good_general_coding_matrix(k,m,w);  //cauchy is the inpunt matrix
    unsigned int* matrix=gen_best_matrix_all(k,m,w);  //output matrix of v-search is the input matrix

    int km=k*m;
    // matrix_num_index matrix_desc[km];
    // matrix_num_index backup_element_asce[n];
    matrix_num_index* matrix_desc;
    matrix_num_index* backup_element_asce;

    matrix_desc=talloc(matrix_num_index, km);
    backup_element_asce=talloc(matrix_num_index, n);
    

    int* bitmatrix;
    bitmatrix = talloc(int, k*m*w*w);
    if (!bitmatrix) return NULL;

    int* bitmatrix_best_base=matrix_to_bitmatrix(k, m, w, encode_matrix_best_base);
    // unsigned int backup_element_list[n];
    unsigned int* backup_element_list;
    backup_element_list=talloc(unsigned int, n);
    int num_of_backup_element;
    // int flag[n];
    int *flag;
    flag=talloc(int,n);
    unsigned int replaced_num;
    unsigned int linear_num;
    int row,col;
    int times;

    for(times=0;times<1;times++)
    {
    for(i=0;i<k*m;i++)
    {
        matrix_desc[i].num=num_of_ones_of_w8[matrix[i]-1];
        matrix_desc[i].index=i;
    }
    qsort(matrix_desc,k*m,sizeof(matrix_num_index),comp_desc);
    for(i=0;i<k*m;i++)
    {
        num_of_backup_element=0;
        for(j=0;j<n-1;j++)
            flag[j]=1;

        replaced_num=matrix[matrix_desc[i].index];
        for(j=1;j<n;j++)
        {
            if(num_of_ones_of_w8[j-1]>=num_of_ones_of_w8[replaced_num-1])
            {
                flag[j-1]=0;
            }
        }

        //去除线性相关元素
        for(j=0;j<m;j++)
        {
            for(ii=0;ii<k;ii++)
            {
                row=matrix_desc[i].index/k;
                col=matrix_desc[i].index-row*k;
                linear_num=galois_single_divide(galois_single_multiply(matrix[j*k+col],matrix[row*k+ii],w),matrix[j*k+ii],w);
                flag[linear_num-1]=0;
            }
        }
        

        for(j=1;j<n;j++)
        {
            if(flag[j-1]==1)
            {
                backup_element_list[num_of_backup_element]=j;
                num_of_backup_element++;
            }
        }


        for(j=0;j<num_of_backup_element;j++)
        {
            backup_element_asce[j].num=num_of_ones_of_w8[backup_element_list[j]-1];
            backup_element_asce[j].index=backup_element_list[j];
        }

        qsort(backup_element_asce,num_of_backup_element,sizeof(matrix_num_index),comp_asce);


        for(j=0;j<num_of_backup_element;j++)
        {   
            matrix[matrix_desc[i].index]=backup_element_asce[j].index;
            bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
            
            if(check_coding_matrix_w(k,m,w,matrix))
            {
                matrix[matrix_desc[i].index]=replaced_num;
            }
            else
            {
                break;
            }
        }
    }
    // count_old=count_one_numbers(k,m,w,bitmatrix_best_base);
    // bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
    // count_new=count_one_numbers(k,m,w,bitmatrix);
    // printf("%d,%d,%d,%d\n",times,count_old,count_new,count_old-count_new);
    }


    // // old matrix
    // for(i=0;i<m;i++)
    // {
    //     for(j=0;j<k;j++)
    //     {
    //         printf("%d ",encode_matrix_best_base[i*k+j]);
    //     }
    //     printf("\n");
    // }
    // //new matrix
    // for(i=0;i<m;i++)
    // {
    //     for(j=0;j<k;j++)
    //     {
    //         printf("%d ",matrix[i*k+j]);
    //     }
    //     printf("\n");
    // }

    // if(check_coding_matrix_w(k,m,w,matrix))
	// {
	// 	printf("No MDS\n");
	// 	return 1;
	// }
    // printf("MDS\n");

    return matrix;

}

unsigned int* gen_best_matrix_from_any_element_lowtime_lrc(int k, int m, int l, int w)
{
    int *matrix, *matrix_tmp, i, j, ii;
    matrix = talloc(int, k*(m+l));
    matrix_tmp = talloc(int, k*(m+1));

    matrix_tmp=gen_best_matrix_from_any_element_lowtime(k,m+1,w);

    for(i=0;i<l;i++)
    {
        for(j=0;j<k;j++)
        {
            if(j>=(i*k/l) && j<=((i+1)*k/l-1))
                matrix[i*k+j]=1;
            else
                matrix[i*k+j]=0;
        }
    }

    for(i=0;i<k*m;i++)
    {
        matrix[l*k+i]=matrix_tmp[k+i];
    }

    return matrix;
}

unsigned int* gen_best_matrix_from_any_element(int k, int m, int w)
{
    int n = 1;
    unsigned int i,j,ii;
    int count_old, count_new;
    unsigned int backup;
    for(i=0;i<w;i++)
    {
        n=n*2;
    } 
    unsigned int* encode_matrix_best_base=gen_best_matrix_all(k,m,w);
    unsigned int* matrix=gen_best_matrix_all(k,m,w);
    int *bitmatrix;
    int* bitmatrix_best_base=matrix_to_bitmatrix(k, m, w, encode_matrix_best_base);
    unsigned int backup_element_list[n];

    for(i=0;i<n;i++)
        backup_element_list[i]=i;


    int times;
    for(times=0;times<1;times++)
    {
    for(i=0;i<m;i++)
    {
        for(j=0;j<k;j++)
        {
            for(ii=1;ii<n;ii++)
            {

                bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
                count_old=count_one_numbers(k,m,w,bitmatrix);
                backup=matrix[i*k+j];
                matrix[i*k+j]=ii;
                bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
                count_new=count_one_numbers(k,m,w,bitmatrix);
                if(count_new>=count_old || check_coding_matrix_w(k,m,w,matrix))
                {
                    matrix[i*k+j]=backup;
                }
            }
        }
    }
    }

    count_old=count_one_numbers(k,m,w,bitmatrix_best_base);
    bitmatrix=matrix_to_bitmatrix(k, m, w, matrix);
    count_new=count_one_numbers(k,m,w,bitmatrix);
    printf("%d,%d,%d\n",count_old,count_new,count_old-count_new);
    //old matrix
    for(i=0;i<m;i++)
    {
        for(j=0;j<k;j++)
        {
            printf("%d ",encode_matrix_best_base[i*k+j]);
        }
        printf("\n");
    }
    //new matrix
    for(i=0;i<m;i++)
    {
        for(j=0;j<k;j++)
        {
            printf("%d ",matrix[i*k+j]);
        }
        printf("\n");
    }

    if(check_coding_matrix_w(k,m,w,matrix))
	{
		printf("No MDS\n");
		return 1;
	}
    printf("MDS\n");
    return matrix;
}

int comp_int_asce(const void * a, const void * b)//用来做比较的函数。
{
    return ( *(int*)a - *(int*)b );
}

int the_least_one_num_of_matrix_in_gf(k,m,w)
{
    int* array=malloc(sizeof(int)*255);
    int i;
    for(i=0;i<255;i++)
        array[i]=num_of_ones_of_w8[i];
    qsort(array,255,sizeof(int),comp_int_asce);
    int total=0;
    for(i=0;i<k;i++)
    {
        total+=array[i];
    }
    int lowbound=total*(m-1)+w*k;
    return lowbound;
}

void print_number_of_ones_of_elements(w)
{
    int n = 1;
    int i;
    int* bitmatrix;
    int element[1];
    int num;
    for(i=0;i<w;i++)
    {
        n=n*2;
    }
    printf("{");
    for(i=1;i<n;i++)
    {
        element[0]=i;
        bitmatrix=matrix_to_bitmatrix(1,1,w,element);
        num=count_one_numbers(1,1,w,bitmatrix);
        printf("%d,",num);
    }
    printf("}\n");
}

int **smart_bitmatrix_to_schedule(int k, int m, int w, int *bitmatrix)
{
  int **operations;
  int op;
  int i, j;
  int *diff, *from, *b1, *flink, *blink;
  int *ptr, no, row;
  int optodo;
  int bestrow = 0, bestdiff, top;

/*   printf("Scheduling:\n\n");
  jerasure_print_bitmatrix(bitmatrix, m*w, k*w, w); */

  operations = talloc(int *, k*m*w*w+1);
  if (!operations) return NULL;
  op = 0;
  
  diff = talloc(int, m*w);
  if (!diff) {
    free(operations);
    return NULL;
  }
  from = talloc(int, m*w);
  if (!from) {
    free(operations);
    free(diff);
    return NULL;
  }
  flink = talloc(int, m*w);
  if (!flink) {
    free(operations);
    free(diff);
    free(from);
    return NULL;
  }
  blink = talloc(int, m*w);
  if (!blink) {
    free(operations);
    free(diff);
    free(from);
    free(flink);
    return NULL;
  }

  ptr = bitmatrix;

  bestdiff = k*w+1;
  top = 0;
  for (i = 0; i < m*w; i++) {
    no = 0;
    for (j = 0; j < k*w; j++) {
      no += *ptr;
      ptr++;
    }
    diff[i] = no;
    from[i] = -1;
    flink[i] = i+1;
    blink[i] = i-1;
    if (no < bestdiff) {
      bestdiff = no;
      bestrow = i;
    }
  }

  flink[m*w-1] = -1;
  
  while (top != -1) {
    row = bestrow;
    /* printf("Doing row %d - %d from %d\n", row, diff[row], from[row]);  */

    if (blink[row] == -1) {
      top = flink[row];
      if (top != -1) blink[top] = -1;
    } else {
      flink[blink[row]] = flink[row];
      if (flink[row] != -1) {
        blink[flink[row]] = blink[row];
      }
    }

    ptr = bitmatrix + row*k*w;
    if (from[row] == -1) {
      optodo = 0;
      for (j = 0; j < k*w; j++) {
        if (ptr[j]) {
          operations[op] = talloc(int, 5);
          if (!operations[op]) goto error;
          operations[op][4] = optodo;
          operations[op][0] = j/w;
          operations[op][1] = j%w;
          operations[op][2] = k+row/w;
          operations[op][3] = row%w;
          optodo = 1;
          op++;
        }
      }
    } else {
      operations[op] = talloc(int, 5);
      if (!operations[op]) goto error;
      operations[op][4] = 0;
      operations[op][0] = k+from[row]/w;
      operations[op][1] = from[row]%w;
      operations[op][2] = k+row/w;
      operations[op][3] = row%w;
      op++;
      b1 = bitmatrix + from[row]*k*w;
      for (j = 0; j < k*w; j++) {
        if (ptr[j] ^ b1[j]) {
          operations[op] = talloc(int, 5);
          if (!operations[op]) goto error;
          operations[op][4] = 1;
          operations[op][0] = j/w;
          operations[op][1] = j%w;
          operations[op][2] = k+row/w;
          operations[op][3] = row%w;
          optodo = 1;
          op++;
        }
      }
    }
    bestdiff = k*w+1;
    for (i = top; i != -1; i = flink[i]) {
      no = 1;
      b1 = bitmatrix + i*k*w;
      for (j = 0; j < k*w; j++) no += (ptr[j] ^ b1[j]);
      if (no < diff[i]) {
        from[i] = row;
        diff[i] = no;
      }
      if (diff[i] < bestdiff) {
        bestdiff = diff[i];
        bestrow = i;
      }
    }
  }
  
  operations[op] = talloc(int, 5);
  if (!operations[op]) goto error;
  operations[op][0] = -1;
  free(from);
  free(diff);
  free(blink);
  free(flink);

  return operations;

error:
  for (i = 0; i <= op; i++) {
    free(operations[op]);
  }
  free(operations);
  free(from);
  free(diff);
  free(blink);
  free(flink);
  return NULL;
}

int count_xor_num_of_smart_schedule(int** operations)
{
    int op;
    int count=0;
    for (op = 0; operations[op][0] >= 0; op++) {
        if(operations[op][4]==1)
            count++;
    }
    return count;
}


void print_smart_schedule(int** operations)
{
    int op;
    for (op = 0; operations[op][0] >= 0; op++) {
        printf("%d,",operations[op][0]);
        printf("%d,",operations[op][1]);
        printf("%d,",operations[op][2]);
        printf("%d,",operations[op][3]);
        printf("%d\n",operations[op][4]);
    }
}

int **bitmatrix_to_schedule(int k, int p, int w, int *bitmatrix)
{
	int **operations;
	int oper_index, oper_count;
	int i, j;

	operations=malloc(sizeof(int *) * (p*w+1));
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
		operations[i]=malloc(sizeof(int)*(2*oper_count+3));
		operations[i][0]=oper_count;
		oper_index++;
		operations[i][oper_index]=k+i/w;
		oper_index++;
		operations[i][oper_index]=i%w;
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
	operations[i]=malloc(sizeof(int));
	operations[p*w][0]=-1;

	return operations;	
}

int **bitmatrix_to_smart_schedule(int k, int p, int w, int *bitmatrix)
{
	int** smart_operations=smart_bitmatrix_to_schedule(k,p,w,bitmatrix);
    // print_smart_schedule(smart_operations);
	int **operations;
	int i, j, op,a;

	operations=malloc(sizeof(int *) * (p*w+1));
	if (!operations) 
		return NULL;
	int *oper_count;
	oper_count=malloc(sizeof(int)*p*w);
    for(i=0;i<p*w;i++)
        oper_count[i]=0;
	int oper_count_index=-1;

	for (op = 0; smart_operations[op][0] >= 0; op++) 
	{
		if(smart_operations[op][4]==0)
		{
			oper_count_index++;
			oper_count[oper_count_index]=1;
		}
		else 
			oper_count[oper_count_index]++;
	}
    // printf("%d\n",oper_count_index);
	op=0;
	int oper_index=0;

	for(i=0;i<p*w;i++)
    {
        //如果某一行是全0，那么这一行对应的校验包不需要计算，第一个元素设为-1
        if(oper_count[i]==0)
        {
            operations[i]=malloc(sizeof(int));
	        operations[i][0]=-1;
            for(j=0;j<p*w;j++)
            {
                int is_all_zero=1;
                for(a=0;a<i;a++)
                {
                    if(j==(operations[a][1]-k)*w+operations[a][2])
                    {
                        is_all_zero=0;
                        break;
                    }
                }
                if(is_all_zero)
                {
                    operations[i][1]=j/w+k;
                    operations[i][2]=j%w;
                    break; 
                }
            }
            continue;
        }
		operations[i]=malloc(sizeof(int)*(2*oper_count[i]+3));
		operations[i][oper_index]=oper_count[i];
		oper_index++;
		operations[i][oper_index]=smart_operations[op][2];
		oper_index++;
		operations[i][oper_index]=smart_operations[op][3];
		oper_index++;
		while(oper_index<2*oper_count[i]+3){
			operations[i][oper_index]=smart_operations[op][0];
			oper_index++;
			operations[i][oper_index]=smart_operations[op][1];
			oper_index++;
			op++;
		}
		oper_index=0;
	}
	operations[p*w]=malloc(sizeof(int));
	operations[p*w][0]=-1;

    return operations;
}

int** find_best_split(int k, int p, int w, int group_num, int *bitmatrix)
{
    int* sub_bitmatrix;
    int** bitmatrix_2d;
    int i,j,x,y,z;
    int* flag;
    int* same_element_num;
    int** group;
    int tmp_num;
    int count_of_smart_operations=0;

    bitmatrix_2d=talloc(int *, p*w);
    for(i=0;i<p*w;i++)
        bitmatrix_2d[i]=talloc(int,k*w);

    flag=talloc(int,k*w);
    same_element_num=talloc(int,k*w);

    group=talloc(int *, group_num);
    for(i=0;i<group_num;i++)
        group[i]=talloc(int,k*w/group_num);

    for(i=0;i<p*w;i++)
    {
        for(j=0;j<k*w;j++)
        {
            bitmatrix_2d[i][j]=bitmatrix[i*k*w+j];
        }
    }

    for(i=0;i<k*w;i++)
    {
        flag[i]=0;
    }

    for(i=0;i<group_num;i++)
    {
        for(j=0;j<k*w;j++)
        {
            if(flag[j]==0)
            {
                group[i][0]=j;
                tmp_num=1;
                flag[j]=1;
                break;
            }
        }

        while(tmp_num<k*w/group_num)
        {
            for(x=0;x<k*w;x++)
                same_element_num[x]=0;

            for(x=0;x<k*w;x++)
            {
                for(y=0;y<tmp_num;y++)
                {
                    for(z=0;z<p*w;z++)
                    {
                        if(bitmatrix_2d[z][x]==bitmatrix_2d[z][group[i][y]])
                        {
                            same_element_num[x]++;
                        }
                    }
                }
            }

            int max=0;
            for(x=0;x<k*w;x++)
            {
                if(same_element_num[x]>max && flag[x]==0)
                    max=same_element_num[x];
            }

            for(x=0;x<k*w;x++)
            {
                if(tmp_num==k*w/group_num)
                    break;
                else if(same_element_num[x]==max && flag[x]==0)
                {
                    group[i][tmp_num]=x;
                    flag[x]=1;
                    tmp_num++;
                }
            }
        }
    }

    // for(i=0;i<group_num;i++)
    // {
    //     for(j=0;j<k*w/group_num;j++)
    //     {
    //         printf("%d ",group[i][j]);
    //     }
    //     printf("\n");
    // }

    int **smart_operations;
    smart_operations = talloc(int *, k*p*w*w/group_num+1);
    sub_bitmatrix = talloc(int, k*p*w*w/group_num);
    if (!smart_operations) return NULL;
    int total=0;

    for(i=0;i<group_num;i++)
    {
        for(j=0;j<p*w;j++)
        {
            for(x=0;x<k*w/group_num;x++)
            {
                sub_bitmatrix[j*k*w/group_num+x]=bitmatrix_2d[j][group[i][x]];
            }
        }
        smart_operations=smart_bitmatrix_to_schedule(k/group_num,p,w,sub_bitmatrix);
		count_of_smart_operations=count_xor_num_of_smart_schedule(smart_operations);
        total+=count_of_smart_operations;
        // printf("%d\n",count_of_smart_operations);
    }

    total+=(group_num-1)*p*w;
    // printf("%d\n",total);

    //验证分组正确性，是否每个列均被分到一个组中
    int* group_1d;
    group_1d=talloc(int,k*w);
    for(i=0;i<group_num;i++)
    {
        for(j=0;j<k*w/group_num;j++)
        {
            group_1d[i*k*w/group_num+j]=group[i][j];
        }
    }
    qsort(group_1d,k*w,sizeof(int),comp_int_asce);
    for(i=0;i<k*w;i++)
    {
        // printf("%d ",group_1d[i]);
        if(group_1d[i]!=i)
            printf("error\n");
    }


    return group;
}

int*** bitmatrix_to_smart_schedule_split(int k,int p, int w,int group_num,int *bitmatrix)
{
	int ***schedule;
	int i,j,x;
	schedule=(int **)malloc(sizeof(int**)*(group_num));
    int **group=find_best_split(k,p,w,group_num,bitmatrix);
    int *sub_bitmatrix = talloc(int, k*p*w*w/group_num);
    int **sub_schedule;

    for(i=0;i<group_num;i++)
    {
        for(j=0;j<p*w;j++)
        {
            for(x=0;x<k*w/group_num;x++)
            {
                sub_bitmatrix[j*k*w/group_num+x]=bitmatrix[j*k*w+group[i][x]];
            }
        }
        sub_schedule=bitmatrix_to_smart_schedule(k/group_num,p,w,sub_bitmatrix);
        schedule[i]=(int *)malloc(sizeof(int *) * (p*w+1));

        // j=0;
        // while(sub_schedule[j][0]!=-1)
        // {
        //     for(x=0;x<=2*sub_schedule[j][0]+2;x++)
        //     {
        //         printf("%d ",sub_schedule[j][x]);
        //     }
        //     j++;
        //     printf("\n");
        // }

        for(j=0;j<p*w;j++)
        {
            schedule[i][j]=(int )malloc(sizeof(int) * (2*sub_schedule[j][0]+3));
            schedule[i][j][0]=sub_schedule[j][0];
            schedule[i][j][1]=sub_schedule[j][1]+(k-k/group_num);
            schedule[i][j][2]=sub_schedule[j][2];
            x=3;
            while(x<2*sub_schedule[j][0]+3)
            {
                if(sub_schedule[j][x]<k/group_num)  //计算包含数据块
                {
                    int column_in_sub_bitmatrix=sub_schedule[j][x]*w+sub_schedule[j][x+1];
                    int column_in_whole_bitmatrix=group[i][column_in_sub_bitmatrix];
                    schedule[i][j][x]=column_in_whole_bitmatrix/w;
                    x++;
                    schedule[i][j][x]=column_in_whole_bitmatrix%w;
                    x++;
                }
                else  //计算包含某个校验块
                {
                    schedule[i][j][x]=sub_schedule[j][x]+(k-k/group_num);
                    x++;
                    schedule[i][j][x]=sub_schedule[j][x];
                    x++;
                }
            }
        }
        schedule[i][p*w]=(int )malloc(sizeof(int)); 
        schedule[i][p*w][0]=-1;
    }
    return schedule;
}
