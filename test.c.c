#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	FILE *file;
	int i,j;

	file = fopen("test.txt","w");
	if(file==NULL) {
		return 0;
	}

	for(i=0; i < 4000; i++){
		for(j=0; j < 50000 ; j++) {
			fptuc("s",file);
		}
	}

	fflush(file);
	fclose(file);
	return 0;
}
	
