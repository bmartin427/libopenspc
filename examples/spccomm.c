/************************************************************************

		Copyright (c) 2003 Brad Martin.

This file is part of the OpenSPC example program set.

OpenSPC is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

OpenSPC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenSPC; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



spccomm.c: Example program showing communication with the SPC during
execution.  Primarily for debugging purposes as it requires a specific
program to be loaded to the SPC to communicate with.  Sound output is
handled in the same way as ospcplay.

 ************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "openspc.h"

#define BUFSIZE 128000
#define QUICK_CYC 64

int main(int argc,char **argv)
{
	FILE *echo;
	int fd,i,j;
	off_t size;
	void *ptr;
	if(argc<2)
	{
		printf("Please specify a filename!\n");
		exit(1);
	}
	fd=open(argv[1],O_RDONLY);
	if(fd<0)
	{
		printf("Unable to open file '%s'!\n",argv[1]);
		exit(1);
	}
	size=lseek(fd,0,SEEK_END);
	ptr=mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	close(fd);
	if(ptr==MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	if((fd=OSPC_Init(ptr,size)))
	{
		fprintf(stderr,"OSPC_Init returned %d!\n",fd);
		exit(1);
	}
	ptr=malloc(BUFSIZE);
	if(ptr==NULL)
	{
		fprintf(stderr,"Unable to allocate buffer!\n");
		exit(1);
	}
	while(1)
	{
		size=OSPC_Run(-1,ptr,BUFSIZE);	/* Initial 'ding' */
		write(STDOUT_FILENO,ptr,size);
		fprintf(stderr,"Press enter to start test...\n");
		getchar();
		fprintf(stderr,"Test started...\n");
		OSPC_WritePort0(1);	/* Trigger a new test */
		size=OSPC_Run(-1,ptr,BUFSIZE);
		write(STDOUT_FILENO,ptr,size);
		OSPC_WritePort2(0x42);
		OSPC_WritePort3(0x05);	/* Write an arbitrary test address */
		do	/* And wait for SPC to acknowledge it */
		{
			size=OSPC_Run(QUICK_CYC,ptr,BUFSIZE);
			write(STDOUT_FILENO,ptr,size);
		}
		while((OSPC_ReadPort2()!=0x42)||(OSPC_ReadPort3()!=0x05));
		fprintf(stderr,"Test complete.  Downloading results...\n");
		echo=fopen("echo.out","w");
		for(i=0;i<0x1E;i++)
		{
			for(j=0;j<0x100;j++)
			{
fprintf(stderr,"0x%04X/0x1FFF\r",(i<<8)|j);
fflush(stderr);
				OSPC_WritePort2(j);
				OSPC_WritePort3(i);	/* Write address */
				do	/* And wait for SPC to acknowledge */
				{
					size=OSPC_Run(QUICK_CYC,ptr,BUFSIZE);
					write(STDOUT_FILENO,ptr,size);
				}
				while(((unsigned char)OSPC_ReadPort2()!=j)||
				  ((unsigned char)OSPC_ReadPort3()!=i));
				putc(OSPC_ReadPort0(),echo);
				putc(OSPC_ReadPort1(),echo);
			}
		}
		fclose(echo);
		fprintf(stderr,"Results obtained.\n");
	}
	return 0;
}
