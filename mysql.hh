#ifndef MYSQL_HH
#define MYSQL_HH

#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>

#include <mysql.h>

using namespace std;

class sql_error: public runtime_error {
public:
  sql_error(string name): runtime_error(name) {};
};

class mysql_row;

class mysql_table {
private:
  int my_ncol;
  int my_nrow;
  vector<string> names; //ncol headers
  vector<string> values; //ncol x nrow values
  string the_info;

  static void error(MYSQL *sql) {throw sql_error(string(mysql_error(sql)));};
  
  mysql_table(MYSQL *sql, const char *query) {
    if (mysql_query(sql,query)!=0) 
      error(sql);
    {
      const char *s=mysql_info(sql);
      if (s) the_info=s;
    }
    MYSQL_RES *res;
    res=mysql_use_result(sql);
    if (res==0 && mysql_errno(sql)!=0) error(sql);
    my_ncol=mysql_field_count(sql);
    if (my_ncol>0) {
      for (int i=0;i<my_ncol;++i) { 
	MYSQL_FIELD *f=mysql_fetch_field(res);
	names.push_back(string(f->name));
      }
      MYSQL_ROW row;
      my_nrow=-1;
      for(my_nrow=0;row=mysql_fetch_row(res);my_nrow++) {
	for (int i=0;i<my_ncol;++i) {
	  if (row[i])
	    values.push_back(string(row[i]));
	  else
	    values.push_back(string());
	}
      } 
    }
    mysql_free_result(res);
  };

  friend class mysql;

public:
  mysql_table() {};
  int size() const {return my_nrow;};
  int ncols() const {return my_ncol;};
  int col(string name) const {
    int i;
    for (i=0;i<my_ncol && names[i]!=name;++i);
    if (i<my_ncol) return i;
    else throw sql_error("campo sconosciuto "+name);
  }
  string val(int row,int col) const {return values[row*my_ncol+col];}
  string name(int col) const {return names[col];};
  inline mysql_row operator[](int n) const;
  string info() const {return the_info;};
};

class mysql_row {
private:
  const mysql_table *tab;
  int n;
public:
  mysql_row(const mysql_table *t, int nrow): tab(t), n(nrow){};
  string operator[](int k) {return tab->val(n,k);}
  string operator[](const string &name) {return (*this)[tab->col(name)];}
  int size() const {return tab->ncols();};
  operator map<string,string>() const {
    map<string,string> res;
    for (int i=0;i<tab->size();++i)
      res[tab->name(i)]=tab->val(n,i);
    return res;
  };
};

mysql_row mysql_table::operator[](int n) const {return mysql_row(this,n);};

class mysql {
public:
  MYSQL *sql;
public:
  void error() {throw sql_error(string(mysql_error(sql)));};
  
  mysql(const char *db=0, const char *user=0, 
	const char *host=0, const char *passwd=0,
	unsigned int port=0) {
    sql=mysql_init(0);
    if (!sql) throw sql_error(string("mysql_init failed"));
    if (!mysql_real_connect(sql,host,user,passwd,db,port,
			    0,0))
      error();
  };
  
  ~mysql() {
    mysql_close(sql);
  };
  
  mysql_table query(const char *s) {
    return(mysql_table(sql,s));
  };

  mysql_table query(const string &s) {
    return(mysql_table(sql,s.c_str()));
  };


  static string escape(string s) {
    for (int i=0;i<s.size();++i) {
      if (s[i]=='\'') {
	s.insert(i,"\\");
	++i;
      }
    }
    return s;
  };
  
};

#endif
