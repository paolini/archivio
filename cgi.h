/*
 * cgi.h: manu-fatto (1999-2002)
 * 
 * handles cgi requests
 *
 */

#ifndef _CGI_H_
#define _CGI_H_

#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

struct CgiError: public std::runtime_error {
  CgiError(const std::string& msg): runtime_error("Cgi Error: "+msg) {};};

class CgiItem;

class Cgi {
private:
  enum {GET,POST,MULTI} method;
  bool index_query;
  bool empty_query;
  size_t length;
  std::string boundary;
  std::string QueryString;
  size_t QueryPos;
  static char x2c(char *what);
  static char digit(int n);
  char *buf;
  static bool strchomp(std::string &s, const std::string& with);
  void GetMultiItem(CgiItem *item);
  char ReadChar(void);
  int Expect(const char *what);
  std::string GetBounded(void);
public:
  static void unescape_url(std::string &url);
  static int escapanda(char c);
  static std::string escape_html(std::string s);
  static std::string escape_url(std::string s);
  static std::string GetEnv(const std::string &name);
  std::string ReadUntil(const char *end);
  Cgi();
  ~Cgi();
  bool GetItem(CgiItem &item);
  bool empty() const {return empty_query;};
};

struct CgiItem {
  std::string name;
  std::string value;
  size_t value_length;
  std::string filename;
  bool good;
  CgiItem() {
    good=true;
  };
  CgiItem(Cgi &cgi) {
    good=cgi.GetItem(*this);
  };
  operator bool() const {return good;};
};

#endif
