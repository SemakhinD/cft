
#ifndef _TYPES_H_
#define _TYPES_H_

#define BOOL int
#define TRUE 1
#define FALSE 0

typedef struct StatData { 
    long  id; 
    int   count; 
    float cost; 
    unsigned int primary:1; 
    unsigned int mode:3; 
} StatData;

#endif /* _TYPES_H_ */