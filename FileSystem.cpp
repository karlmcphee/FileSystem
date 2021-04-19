#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <ostream>
#include <fstream>
#include <math.h>
#include <stack>
#include <stdio.h>
#include <sys/stat.h>
#include <cstdlib>
#include	<pthread.h>
#include	<semaphore.h>
#include	<unistd.h>

/* This program simulates a file system that adds to its structure based on Linux-like commands, such as mkdir, read in from a file.

@author Karl McPhee
*/
using namespace std;

int * inodearray;
#define	OCT_ALLO	0100000
#define OCT_OWN     000700
#define OCT_GROUP   000070
#define OCT_OTHERS  000007
#define OCT_DIR     040777
#define OCT_FILE    000700
sem_t mutexmod;	
sem_t mutexfree;
sem_t mutexinode;
sem_t mutexdir;
const int MAX= 30;
pthread_t tid[ MAX ]; 
int users;

struct inode{ 
	unsigned short flags; //set this...  2 bytes
	char nlinks; //1 byte
	char uid;  // 1 byte
	char gid;  // 1 byte
	char size0; // 1 byte
	unsigned short size1;  // 2 bytes
	unsigned short addr[8];  // 16 bytes
	unsigned short actime[2]; // 4 bytes
	unsigned short modtime[2];  // 4 bytes, 32 total
};

struct superblock{

	unsigned short isize;  //2 bytes
	unsigned short nfree; //2 bytesz
	unsigned short free[100]; //200 bytes
	unsigned short ninode; //2 bytes
	unsigned short inode[100]; //200 bytes
	unsigned short fsize; //2 bytes
	char flock; //1 byte
	char ilock; //1 byte
	char fmod; //1 byte
	unsigned short time[2]; //4 bytes, 415 total;
	char a[97]; //97 bytes
	
};

struct block{
	char c[512];
};
struct direntry{
	unsigned short inumber;
	char fname[14];
};
struct directory{
	struct direntry dirarray[16];
};
struct farray{
	int intarray[128];

};
struct file{
	unsigned short v;
};

struct iarray{
	struct inode inodearray[16];
};
block *  blockarray;

void fvalue(int currentblock, string sname[10], int depth, string filename, block* blockarray);

void * increment(void* arg )
{
	int id = (long) arg;
	stringstream sr;
	sr << id;
		string fname= string("user")+sr.str()+string(".txt");
		string oname = string("trace")+sr.str() +string(".txt");
		//char fname[10];
		//char oname[10];
		//strcpy(fname, fname2);
		//strcpy(oname, oname2);
ofstream tracefile(oname.c_str());
ifstream myfile2(fname.c_str());
int currentdir= 999;
if(!myfile2.is_open())
{
	cout << "User input file does not exist.";
	exit(EXIT_FAILURE);
}
string input;
while(myfile2){
	getline(myfile2, input);
	stringstream sr(input);
	string s3, s4, s5;
	sr >> s3 >> s4 >> s5;

	if(s3=="cpin")
	{
		int lnum, modnum, modnum2, blocknum, dfree, fblock, inum, inum2, blocknum3, inum3, currentblock;	
		int n = 0;
		struct stat filestatus;
		stat(s4.c_str(), &filestatus);
		int size = filestatus.st_size;
		int numblocks= (size+511)/512;
		currentblock = currentdir;
		sem_wait(&mutexinode);
		((struct superblock*) blockarray)[1].isize++;  // SEM
		modnum2=((struct superblock*) blockarray)[1].isize-1;  //SEM
                sem_post(&mutexinode);
				lnum = 2+modnum2/16;  //SEM
		modnum=modnum2%16;  //SEM
		sem_wait(&mutexmod);
		((struct iarray*) blockarray)[lnum].inodearray[modnum].flags=0;	
		((struct iarray*) blockarray)[lnum].inodearray[modnum].flags |= OCT_FILE;  //SEM2+ set 0
                ((struct iarray*) blockarray)[lnum].inodearray[modnum].flags+= 32768;
		sem_post(&mutexmod);
			((struct iarray*) blockarray)[lnum].inodearray[modnum].uid=id;
		ifstream rfile(s4.c_str(), ios::binary);
		((struct iarray*)blockarray)[lnum].inodearray[modnum].size1=size;  //SEM
		for(int i = 0; i < numblocks; i++)
		{
				sem_wait(&mutexfree);
			    dfree=((struct superblock *) blockarray)[1].nfree; //SEM
				blocknum=((struct superblock *) blockarray)[1].free[dfree];  //SEM
		((struct iarray*) blockarray)[lnum].inodearray[modnum].addr[i]=blocknum; //sets address of file for each block
			((struct superblock *) blockarray)[1].nfree--;  //SEM
				if(((struct superblock *) blockarray)[1].nfree==1)
				{
					fblock = ((struct superblock *) blockarray)[1].free[1];
					blocknum=((struct superblock *) blockarray)[1].nfree=51;
					for(int i = 0; i <51; i++)
						((struct superblock *) blockarray)[1].free[i]=((struct farray*)blockarray)[fblock].intarray[i];
				}
				sem_post(&mutexfree);
				rfile.read(((struct block*)blockarray)[blocknum].c,512);
		}
		rfile.close();
		inum=((struct directory*)blockarray)[currentdir].dirarray[0].inumber;  //
		inum2=2+inum/16;
		inum= inum%16;
		sem_wait(&mutexinode);
		((struct iarray*) blockarray)[inum2].inodearray[inum].size1++;
		inum=((struct iarray*) blockarray)[inum2].inodearray[inum].size1;
		sem_post(&mutexinode);
		if(inum%32==0)
			{
				sem_wait(&mutexfree);
			    dfree=((struct superblock *) blockarray)[1].nfree; //SEM
				blocknum3=((struct superblock *) blockarray)[1].free[dfree];  //SEM
		((struct iarray*) blockarray)[inum2].inodearray[inum].addr[inum/32]=blocknum3; //sets address of file for each block
			((struct superblock *) blockarray)[1].nfree--;  //SEM
				if(((struct superblock *) blockarray)[1].nfree==1)
				{
					fblock = ((struct superblock *) blockarray)[1].free[1];
					((struct superblock *) blockarray)[1].nfree=51;
					for(int i = 0; i <51; i++)
						((struct superblock *) blockarray)[1].free[i]=((struct farray*)blockarray)[fblock].intarray[i];
				}
				sem_post(&mutexfree);
			}
		inum3= inum/32;
		inum=inum%32;
		blocknum3=((struct iarray*)blockarray)[lnum].inodearray[modnum].addr[inum3];
		strcpy(((struct directory*) blockarray)[currentblock].dirarray[inum].fname,s5.c_str());
		((struct directory*) blockarray)[currentblock].dirarray[inum].inumber=modnum2; //allocate inode number stored in directory
	    tracefile << "cpin " << s4.c_str() << " " << s5.c_str() << "\n";
		tracefile << "size of the file is " << size << "\n";
		tracefile << s5.c_str() << " is allocated" << numblocks << "block(s).\n\n";
	}
else if(s3=="cd")		
	{
		int lnum, modnum, tadr, lnum2, modnum2, dsize, currblock;
		modnum2=((struct directory*)blockarray)[currentdir].dirarray[0].inumber;
		lnum=2+modnum2/16;
		modnum2=modnum2%16;
		dsize=((struct iarray*) blockarray)[lnum].inodearray[modnum2].size1;  //number of entries
		dsize=dsize/32;
		for(int k = 0; k <= dsize; k++)
		{
			currblock=((struct iarray*) blockarray)[lnum].inodearray[modnum2].addr[k];
		for(int i = 0; i < 32; i++)
		{
			if(strcmp(((struct directory*)blockarray)[currblock].dirarray[i].fname,s4.c_str())==0)
			{
				modnum=((struct directory*)blockarray)[currblock].dirarray[i].inumber;//inumber of array to switch to
					lnum=2+modnum/32;
				modnum = modnum%32;
				currentdir=((struct iarray*)blockarray)[lnum].inodearray[modnum].addr[0];
		tracefile << "Current directory changed to" << s4.c_str() << "\n\n";
		i = 33;
		k = 100;
			}
		}
	}
	}
	else if(s3=="cpout")
	{
		int modnum, lnum, tsize, tadr, numblocks, currentblock, lnum2, modnum2, numentries, tmod, tsize2;
		lnum2 = ((struct directory*)blockarray)[currentdir].dirarray[0].inumber;
		modnum2=lnum2%16;
		lnum2=2+lnum2/16;
		numblocks=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].size1+1;
		numentries= numblocks%16;
		numblocks= numblocks/16;
		currentblock= ((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[0];
		for(int p = 0; p <= numblocks; p++)
		{
			currentblock= ((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[p];
		for(int i = 0; i <= numentries; i++)
		{
			if(strcmp(((struct directory*)blockarray)[currentblock].dirarray[i].fname,s4.c_str())==0)
			{
				lnum=((struct directory*)blockarray)[currentblock].dirarray[i].inumber;
					modnum=2+lnum/32;
				lnum = lnum%32;
				tadr=((struct iarray*)blockarray)[modnum].inodearray[lnum].addr[0];
				tsize=((struct iarray*)blockarray)[modnum].inodearray[lnum].size1;
				s5=s5+string(".txt");
				ofstream ofile(s5.c_str(), ios::binary);
				tmod= tsize/512;
				tsize2=tsize;
				for(int s = 0; s <= tmod; s++)
					{
						tadr=((struct iarray*)blockarray)[modnum].inodearray[lnum].addr[s];
						if(tsize>512)
							tsize=512;
						for(int k = 0; k < tsize; k++)
						{
							ofile<< ((struct block*)blockarray)[tadr].c[k];;
						}
						tsize2=tsize2-512;
						tsize=tsize2;
					}
				tadr=((struct iarray*)blockarray)[modnum].inodearray[lnum].addr[0];
				ofile.close();
				tmod=tmod+1;
					tracefile<< "cpout " << s4 << " " << s5 << "\n";
					tracefile << "The file was printed from: " << tmod << " blocks.\n\n"; 
		i= numentries+1;
		p=numblocks+1;
			}
		}
		}

	}
	else if(s3== "mkdir")
	{
		int modnum, lnum, dfree, blocknum, blocknum2, fblock, lnum2, modnum2, currentblock, isize, ssize, blocknum3;
		sem_wait(&mutexinode);
		((struct superblock*) blockarray)[1].isize++;  //created the inode in the superblock  SEM
		ssize=((struct superblock*) blockarray)[1].isize-1;
		sem_post(&mutexinode);
		lnum = 2+ssize/16;
		modnum=ssize%16;
		sem_wait(&mutexmod);
		((struct iarray*) blockarray)[lnum].inodearray[modnum].flags=0;
			((struct iarray*) blockarray)[lnum].inodearray[modnum].flags |= OCT_DIR;
			((struct iarray*) blockarray)[lnum].inodearray[modnum].flags += 32768;  //flags set...
			((struct iarray*) blockarray)[lnum].inodearray[modnum].size1=1;
			sem_post(&mutexmod);
			sem_wait(&mutexfree);
				dfree=((struct superblock *) blockarray)[1].nfree;
			blocknum=((struct superblock *) blockarray)[1].free[dfree];//new allocated block
			((struct superblock *) blockarray)[1].nfree--;
			{
				if(((struct superblock *) blockarray)[1].nfree==1)
				{
					fblock = ((struct superblock *) blockarray)[1].free[1];
						blocknum=((struct superblock *) blockarray)[1].nfree=51;
					for(int i = 0; i <=51; i++)
						((struct superblock *) blockarray)[1].free[i]=((struct farray*)blockarray)[fblock].intarray[i];
				}
			}
			sem_post(&mutexfree);
			((struct iarray*) blockarray)[lnum].inodearray[modnum].addr[0]=blocknum;
			strcpy(((struct directory*) blockarray)[blocknum].dirarray[0].fname, ".");
			((struct directory*) blockarray)[blocknum].dirarray[1].inumber= ((struct directory*) blockarray)[currentdir].dirarray[0].inumber;//sets 1 to parent's array
			modnum=((struct directory*) blockarray)[currentdir].dirarray[0].inumber;//inumber of parent array
			lnum=2+modnum/16;								   
			modnum=modnum%16;  //
			sem_wait(&mutexinode);			
			((struct iarray*) blockarray)[lnum].inodearray[modnum].size1++;  //inc size of current array PUT DIR INCREASE HERE
			isize=((struct iarray*) blockarray)[lnum].inodearray[modnum].size1;//size of current array
			lnum2=isize/32;	 //blocks of current array 
			modnum2=isize%32;//mod of current array (storage)
			if(isize%32==0)
			{
				sem_wait(&mutexfree);
			    dfree=((struct superblock *) blockarray)[1].nfree; //SEM
				blocknum3=((struct superblock *) blockarray)[1].free[dfree];  //SEM
		((struct iarray*) blockarray)[lnum].inodearray[modnum].addr[lnum2]=blocknum3; //sets address of file for each block
			((struct superblock *) blockarray)[1].nfree--;  //SEM
				if(((struct superblock *) blockarray)[1].nfree==1)
				{
					fblock = ((struct superblock *) blockarray)[1].free[1];
					blocknum3=((struct superblock *) blockarray)[1].nfree=51;
					for(int i = 0; i <51; i++)
						((struct superblock *) blockarray)[1].free[i]=((struct farray*)blockarray)[fblock].intarray[i];
				}
				sem_post(&mutexfree);
			}
			sem_post(&mutexinode);
			blocknum2=((struct iarray*) blockarray)[lnum].inodearray[modnum].addr[lnum2];//block where array change will be allocated
			((struct directory*) blockarray)[blocknum].dirarray[0].inumber=ssize;
			strcpy(((struct directory*) blockarray)[blocknum].dirarray[1].fname, "..");
			((struct directory*) blockarray)[blocknum2].dirarray[modnum2].inumber=ssize;//sets parent's names
			strcpy(((struct directory*) blockarray)[blocknum2].dirarray[modnum2].fname, s4.c_str());
		tracefile << "mkdir command executed and directory " << s4 << " created.\n\n";
	}
	else if(s3=="chmod")
	{
		int oct = 010000;
		char * ic;
		oct=strtol(s4.c_str(),&ic, 8);
		int lnum,modnum, lnum2, modnum2, dsize, currblock;
		modnum2=((struct directory*)blockarray)[currentdir].dirarray[0].inumber;
		lnum=2+modnum2/16;
		modnum=modnum2%16;
		dsize=((struct iarray*) blockarray)[lnum].inodearray[modnum].size1;
		dsize=dsize/32;
		for(int k = 0; k <= dsize; k++)
		{
			currblock=((struct iarray*) blockarray)[lnum].inodearray[modnum].addr[k];
		for(int i = 0; i < 32; i++)
		{
			if(strcmp(((struct directory*)blockarray)[currblock].dirarray[i].fname, s5.c_str())==0)
			{
				lnum=((struct directory*)blockarray)[currentdir].dirarray[i].inumber;
				modnum=2+lnum/16;
				lnum = lnum%16;
				sem_wait(&mutexmod);
			((struct iarray*)blockarray)[modnum].inodearray[lnum].flags=0;
			((struct iarray*)blockarray)[modnum].inodearray[lnum].flags|=oct;
			((struct iarray*)blockarray)[modnum].inodearray[lnum].flags += 32768;
			sem_post(&mutexmod);
				tracefile << "chmod command successful." << s5 << "'s mode changed to " << s4 << ".\n\n";
			i=32;
			k=dsize+1;
			}
		}
		}
	}
	else if(s3=="exit")
		break;
	else if(s3=="ls")
		{
			cout << "The contents of this directory are: \n";
		int lnum2, modnum2, numblocks, lnum, modnum, currentblock;
			lnum2 = ((struct directory*)blockarray)[currentdir].dirarray[0].inumber;//inumber of current dir
		modnum2=lnum2%16;
		lnum2=2+lnum2/16;
		numblocks=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].size1; //number of entries
		currentblock=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[0];  //starting block
		modnum=numblocks%32;
		lnum=numblocks/32;
		for(int i = 0; i < lnum; i++)
		{
			for(int k = 0; k < 32; k++)
			{
				cout << ((struct directory*) blockarray)[currentblock].dirarray[k].fname;
				cout << "\n";
				currentblock=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[i+1];
			}
		}
		for(int i =0; i <= modnum; i++)
			cout << ((struct directory*) blockarray)[currentblock].dirarray[i].fname << "\n";
	}
	else if(s3=="mv")
	{
			int lnum2, modnum2, numblocks, lnum, modnum, inum, currentblock;
			lnum2 = ((struct directory*)blockarray)[currentdir].dirarray[0].inumber;
		modnum2=lnum2%16;
		lnum2=2+lnum2/16;
		numblocks=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].size1;
		currentblock=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[0];
		lnum=numblocks/32;
		for(int i = 0; i <= lnum; i++)
		{
			for(int k = 0; k < 32; k++)
			{
					if(strcmp(s4.c_str(),((struct directory*)blockarray)[currentblock].dirarray[k].fname)==0)
				{
					strcpy(((struct directory*)blockarray)[currentblock].dirarray[k].fname, s5.c_str());
				}
			}
			currentblock=((struct iarray*) blockarray)[lnum2].inodearray[modnum2].addr[i+1];
		}
	}
	else if(s3=="pwd")
	{
		int inum, lnum, modnum, dnum, dsize, modnum2;
		string path="";
		int currentblock=currentdir;
		while(((struct directory*)blockarray)[currentblock].dirarray[0].inumber!=0)	
		{
			dnum=((struct directory*)blockarray)[currentblock].dirarray[0].inumber;
			inum=((struct directory*)blockarray)[currentblock].dirarray[1].inumber;
			lnum= 2+inum/16;
			modnum= inum%16;
			currentblock=((struct iarray*)blockarray)[lnum].inodearray[modnum].addr[0];//go up directory
			dsize=((struct iarray*)blockarray)[lnum].inodearray[modnum].size1;
			modnum2=dsize%16;
			dsize=dsize/16;
			for(int i = 0; i <=dsize; i++)
			{
				currentblock=((struct iarray*)blockarray)[lnum].inodearray[modnum].addr[i];
				for(int k = 0; k < 32; k++)
				{
					if(((struct directory*)blockarray)[currentblock].dirarray[k].inumber==dnum)
					{
						path=string("/")+string(((struct directory*)blockarray)[currentblock].dirarray[k].fname)+path;
						k=33;
						i=dsize+1;
					}
				}
			}
	}
		path = string("Absolute file path: root")+path;
				cout << path;
	}
	else if(s3=="find")
	{
	int currentblock;
		string fname[10];
		fname[0]= "/";
		int depth = 1;
		currentblock= currentdir;
		fvalue(currentblock, fname, depth, s4, blockarray);

}

}
myfile2.close();
tracefile.close();
return 0;
}

int main(int argc, char *argv[])
{
blockarray = new block[1000];
string fname, desc;
int currentdir = 999;
bool test, ongoing;
string fileName=argv[1];
int no_threads= atoi(argv[2]);
users = no_threads;
fstream myfile(fileName.c_str(), ios::binary);
sem_init( &mutexinode, 0, 1 );
sem_init( &mutexfree, 0, 1 );
sem_init( &mutexmod, 0, 1 );
sem_init( &mutexdir, 0, 1 );
if(myfile.good())
{
for(int i =0;i<1000;i++)
	myfile.read(blockarray[i].c,512);
myfile.close();
int test = ((struct iarray*) blockarray)[2].inodearray[0].size1;
}
else
{
	int l=42;
	int m = 950;
	((superblock*) blockarray)[1].isize = 1;
	((struct superblock *) blockarray)[1].nfree=50;

	for(int i = 2; i <= 41; i++)
	{
		for(int k =0; k<16; k++)
		{
			((struct iarray*) blockarray)[i].inodearray[k].flags&=0;
		}
	}
	((struct iarray*) blockarray)[2].inodearray[0].size1=1;
	((struct iarray*) blockarray)[2].inodearray[0].addr[0]= 999;	
	for(int i = 42; i <= 60; i++)
	{
		l=i+1;
		((struct farray*)blockarray)[i].intarray[0]=l;
		((struct farray*)blockarray)[i].intarray[1]=50;
		for(int k = 2; k<=51; k++)
		{
			((struct farray*)blockarray)[i].intarray[k]=m;
			m++;
			if(i==60 && k==51)
			{
				m=m-50;
				((struct farray*)blockarray)[i].intarray[1]=57;
				for(int l = 51; l< 58; l++)
				{
					m--;
					((struct farray*)blockarray)[i].intarray[l]=m;
				}
			}
		}
		m=m-100;
		if(m<42)
			m=42;
	}
	for(int i = 0; i <52; i++)
		((struct superblock *) blockarray)[1].free[i]=((struct farray*)blockarray)[42].intarray[i];
	((struct iarray*) blockarray)[2].inodearray[0].flags|=OCT_DIR;
	((struct iarray*) blockarray)[2].inodearray[0].flags= ((struct iarray*) blockarray)[2].inodearray[0].flags+32768;
	((struct iarray*) blockarray)[2].inodearray[0].addr[0]=999;
	((struct directory*) blockarray)[999].dirarray[0].inumber=0;

}

	((struct farray*)blockarray)[60].intarray[0]=0;
	((struct directory*)blockarray)[currentdir].dirarray[1].inumber=0;
	strcpy(((struct directory*)blockarray)[currentdir].dirarray[1].fname,"..");
	((struct directory*)blockarray)[currentdir].dirarray[0].inumber=0;
	strcpy(((struct directory*)blockarray)[currentdir].dirarray[0].fname,".");
	for(int i = 1; i <= no_threads; i++ ) 
		pthread_create( &tid[ i ], NULL, increment, (void*) i );

for( int i = 1; i <= no_threads; i++ ) 
		pthread_join( tid[ i ], NULL );
ofstream wrfile("disk.txt", ios::binary);
	for(int i = 0; i <1000;i++)
	for(int k = 0; k < 512; k++)
			wrfile<< (((struct block*)blockarray)[i].c[k]);
	wrfile.close();
	//Shutting down

	return NULL;
}

void fvalue(int currentblock, string fpath[10], int depth, string filename, block* blockarray)
		{
			int inum, modnum, lnum, size, modnum2, lnum2, modnum3, m;
			//fpath[d]=((struct directory*)blockarray)[currentblock].dirarray[1].fname;
			inum=((struct directory*)blockarray)[currentblock].dirarray[0].inumber;
			modnum= inum%16;
			lnum= inum/16+2;
			fpath[0]="/";
			size=((struct iarray*)blockarray)[lnum].inodearray[modnum].size1;//dir size
			modnum2=size%32;
			lnum2=size/32;
			for(int i =0; i<=lnum2; i++)
			{
				currentblock=((struct iarray*)blockarray)[lnum].inodearray[modnum].addr[i];
				m = 32;
				if(i==lnum2)
					m=modnum2;
				for(int k = 2; k<= modnum2; k++)
				{
					inum=((struct directory*)blockarray)[currentblock].dirarray[k].inumber;
					modnum3=inum%16;
					inum=inum/16+2;
					if(((struct iarray*)blockarray)[inum].inodearray[modnum3].flags==49663)
					{
						fpath[depth]=string(((struct directory*)blockarray)[currentblock].dirarray[k].fname)+"/";
						fvalue(((struct iarray*)blockarray)[inum].inodearray[modnum3].addr[0],fpath,depth+1,filename, blockarray);
					}
					else if(strcmp(((struct directory*)blockarray)[currentblock].dirarray[k].fname,filename.c_str())==0)
					{
						for(int l = 0; l <= depth; l++)
						{
							cout << fpath[i];
						cout << filename;
						}
					}
				}
			
				}
}
