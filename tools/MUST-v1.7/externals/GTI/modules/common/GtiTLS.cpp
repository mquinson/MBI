#include "GtiTLS.h"
#include <mutex>

static __thread int tid=-1;
static std::mutex _m;
static int new_tid = 0;
int getGtiTid() {
    if (tid<0){
        std::lock_guard<std::mutex> lock(_m);
        tid=new_tid++;
    }
    return tid;
}

