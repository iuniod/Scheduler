#include <stdio.h>
#include <stdlib.h>

#include "so_scheduler.h"

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io) {
    return 0;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority) {
    return 0;
}

DECL_PREFIX int so_wait(unsigned int io) {
    return 0;
}

DECL_PREFIX int so_signal(unsigned int io) {
    return 0;
}

DECL_PREFIX void so_exec(void) {

}

DECL_PREFIX void so_end(void) {
    
}