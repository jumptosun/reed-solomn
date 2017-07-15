#ifndef RS_CONFIG_HPP
#define RS_CONFIG_HPP

#include <stdio.h>

// whether turn on the thread safety
#undef RS_CONFIG_OPTION_THREAD_SAFETY
// change the log funtion if need
#define rs_log(msg, ...) printf(msg, ##__VA_ARGS__)


#endif // RS_CONFIG_HPP
