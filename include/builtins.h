// builtins.h
#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>

void mycd(const std::string& path);
void mypwd();
void myecho(const std::string& text);
void myexit();
void myclear();
void myls();
void mycat(const std::string& filename);
void mydate();
void mymkdir(const std::string& dirname);
void mytime();
void myhelp();
void mymv(const std::string& src, const std::string& dest);
void mycp(const std::string& src, const std::string& dest);
void myrmdir(const std::string& dirname);
void mytouch(const std::string& filename);
void myrm(const std::string& filename);

#endif
