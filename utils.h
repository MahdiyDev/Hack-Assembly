#pragma once

char* decimal_to_binary(int n, unsigned int bus_size);
char* bin_to_hex(char* bin, unsigned int bus_size);

#ifdef UTILS_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>

char* decimal_to_binary(int n, unsigned int bus_size) {
	char* bin;
	int i = 0, b;

	if (!(bin = (char*)malloc(sizeof(char) * bus_size))) {
		fprintf(stderr, "cannot alocate memory!\n");
		return NULL;
	}

	for (b = 14; b >= 0; b--) {
	    int bit = n >> b;

	    if (bit & 1) {
	    	bin[i] = '1';
	    }
	    else {
	     	bin[i] = '0';
	    }
	    i++;
  	}
  	return bin;
}

char* bin_to_hex(char* bin, unsigned int bus_size) {
    char* buffer = (char*)malloc(sizeof(char)*bus_size);
    char *a = bin;
    int num = 0;
    do {
        int b = *a=='1'?1:0;
        num = (num<<1)|b;
        a++;
    } while (*a);
    snprintf(buffer, bus_size, "%.4X", num);
    return buffer;
}
#endif // UTILS_IMPLEMENTATION
