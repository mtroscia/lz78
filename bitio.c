#include "bitio.h"

struct bitio{
	FILE* f; //pointer to the file
	uint64_t data; // local buffer
	u_int wp; //write index pointer 
	u_int rp; //read index pointer
	u_int mode; //0=>read  1=>write 
};


struct bitio* bit_open(const char* name,u_int mode)
{
	struct bitio* b;
	
	//check consistency of data
	if(name==NULL || name[0]=='\0' || mode>1){
		errno=EINVAL;
		return NULL;			
	}

	b=calloc(1,sizeof(struct bitio));
	if(b==NULL){
		errno=ENOMEM;
		return NULL;	
	}

	b->f=fopen(name, (mode==0)?"r":"w");
	
	if(b->f==NULL){
		errno=ENOENT;	
		free(b);
		return NULL;	
	}
	
	b->mode=mode;
	
	/************************si potrebbero omettere*********/
	b->wp=0;
	b->rp=0;
	/*******************************************************/
	
	return b;
}


int bit_close(struct bitio* b)
{
	int ret=0;	
	if(b==NULL){
		errno=EINVAL;
		return -1;	
	}

	if(b->mode==1 && b->wp>0){
		/********************DIFFERENT FROM LESSON***************/
		if(fwrite((void*)&b->data, 1, ((b->wp)+7)/8,  b->f)<=0){
			ret=-1;		
		}	
	}
	
	fclose(b->f);
	bzero(b, sizeof(*b));
	free(b);
	return ret;
}


int bit_write(struct bitio * b, u_int size, uint64_t data)
{
	int space;
	if(b==NULL || b->mode!=1 || size>64){
		errno=EINVAL;
		return -1;	
	}

	if(size==0){
		//no bit to copy
		return 0;	
	}
	
	space=64-(b->wp); //space available in the buffer
	data&=(1UL<<size)-1; //pick the rightmost size bits

	if(size<=space){
		b->data|=data<<b->wp;	//copy the block into the buffer from wp on
		b->wp+=size;
	}
	else{
		/********************DIFFERENT FROM LESSON***************/
		data&=(1UL<<space)-1; 	//pick the rightmost space bits		
		b->data|=data<<b->wp;	//copy the block into the buffer from wp on

		if(fwrite((void*)&b->data, 1, 8, b->f)<=0){		//empty b->data
			errno=ENOSPC;
			return -1;		
		}
			
		b->data=data>>space;	//copy in the buffer the remaining bits of the block of data (size-space bits)
		b->wp=size-space;	
	}
	return 0;
}


int bit_read(struct bitio* b,u_int size,uint64_t *data)
{
	int space;
	if(b==NULL || b-> mode!=0 || size>64){
		errno=EINVAL;
		return -1;
	}
	
	*data=0;		//clear all bits of data buffer
	space=(int)(b->wp)-(int)(b->rp);
	if(size==0){
		return 0; // no more bits to read	
	}
	
	if(size<=space){
		*data=((b->data)>>b->rp)&((1UL<<size)-1);
		b->rp+=size;
		return size;	
	}
	
	else{
		*data=(b->data>>(b->rp)); //no need to mask: pick all unread bits
		int ret = fread(&(b->data), 1, 8, b->f);
		if (ret<=0) {
			errno=ENODATA;
			return -1;
		}
				
		b->wp=ret*8;
		if(b->wp>=size-space){ 
			/*size-space is the remaining number of bits to read: 
			if b->wp>=size-space, we can read the remaining bits and we can add them to the data buffer*/
			
			/********************DIFFERENT FROM LESSON***************/
			*data^=*data; //clear all bits of data buffer
			*data|=b->data<<space;
			
			/********************DIFFERENT FROM LESSON***************/
			if(size<64)
				*data&=(uint64_t)((1UL<<size)-1); //only if size<64, otherwise we go out of UL and we mask all

			b->rp=size-space;
			return size-space;
		}
		else{
			*data|=b->data<<space;
			*data&=(1UL<<(b->wp+space))-1;
			b->rp=b->wp;//all buffer has been read
			return space+b->wp;		
		}
	}
}