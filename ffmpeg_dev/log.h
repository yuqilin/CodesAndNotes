#ifndef XLOG_H
#define XLOG_H

int xlog_init();

int xlog_uninit();

int xlog(const char* format, ...);


#define LOG xlog

#endif