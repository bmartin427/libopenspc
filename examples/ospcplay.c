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



ospcplay.c: Example, rudimentary SPC player.  Sound output goes to stdout,
which means a pipe is required in order to play the output.  Output format
is 16-bit stereo 32kHz raw PCM.  Can handle multiple files on the command
line: they will be played in shuffled order.

 ************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openspc.h>

#define BUFSIZE 32000
#undef USE_MMAP    /* Nonportable, apparently */

#define Process(b,s) write(STDOUT_FILENO,b,s);

int main(int argc,char **argv)
{
	int fd,f,mmapped=1,*list;
	off_t size;
	void *ptr,*buf;
	char c;
	
	if(argc<2)
	{
		printf("Please specify at least one filename to play!\n");
		exit(1);
	}
	list=malloc((argc-1)*sizeof(int));
	list[0]=1;
	srand(time(NULL));
	for(f=2;f<argc;f++)
	{
		fd=rand()%f;
		list[f-1]=list[fd];
		list[fd]=f;
	}
	buf=malloc(BUFSIZE);
	fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);
	fprintf(stderr,"Press RETURN to change songs\n");
	for(f=0;f<argc-1;f++)
	{
		fprintf(stderr,"Now playing '%s'...\n",argv[list[f]]);
		fd=open(argv[list[f]],O_RDONLY);
		if(fd<0)
		{
			fprintf(stderr,"\nUnable to open file '%s'!\n",argv[1]);
			continue;
		}
		size=lseek(fd,0,SEEK_END);

#ifdef USE_MMAP   /* Nonportable, apparently */
        ptr=mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
		if(ptr==MAP_FAILED)
#endif
		{
            lseek(fd,0,SEEK_SET);
            ptr=malloc(size);
            read(fd,ptr,size);
            mmapped=0;
        }
  
        close(fd);        
		if((fd=OSPC_Init(ptr,size)))
		{
			fprintf(stderr,"\nOSPC_Init returned %d!\n",fd);
			continue;
		}
#ifdef USE_MMAP
		if(mmapped)
		    munmap(ptr,size);
		else
#endif
            free(ptr);

		while((read(STDIN_FILENO,&c,1)<=0)||(c!='\n'))
		{
			size=OSPC_Run(-1,buf,BUFSIZE);
			Process(buf,size);
		}
	}
	return 0;
}
