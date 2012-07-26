#include <stddef.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

#include "xlinked_list.h"

/* A simple test of the xlinked_list datastructure */
int main(int argc, char *argv[]){
    xlinked_list< const char * > *xll = NULL;
    const char *removed = NULL;

    const size_t exp_size = 3;
    const char *item1 = "Hello, World!";
    const char *item2 = "Goodbye, World!";
    const char *item3 = "Hello, Again!!";
    const char *item4 = "Woah!";

    xll = new xlinked_list< const char * >();

    xll->push_back(item2);
    xll->push_front(item1);
    xll->push_front(item4);
    removed = xll->front();
    xll->pop_front();
    xll->push_back(item3);

    
    const size_t real_size = xll->size();
    if(exp_size != real_size){
        error(-1, 0, "Expected a list of size %zu, but instead found one of size %zu\n", exp_size, real_size);
    }

    if(strncmp(item4, removed, strlen(item4))){
        error(-1, 0, "Expected the removed element to be %s, but instead found %s\n", item4, removed);
    }

    for(xlinked_list< const char * >::iterator iter = xll->begin(); iter != xll->end(); ++iter){
        printf("%s\n", *iter);
    }

    printf("\n");

    for(xlinked_list< const char * >::iterator iter = xll->rbegin(); iter != xll->rend(); ++iter){
        printf("%s\n", *iter);
    }

    printf("\n");

    xll->reverse();
    for(xlinked_list< const char * >::iterator iter = xll->begin(); iter != xll->end(); ++iter){
        printf("%s\n", *iter);
    }

    delete xll;

    return 0;
}
