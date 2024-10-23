#ifndef __NODE_H__
#define __NODE_H__

#include "structs.h"

status_t init_node();
status_t connect();
status_t disconnect();
status_t send();
status_t receive();

#endif // __NODE_H__