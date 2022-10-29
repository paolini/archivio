#include <iostream>
#include <cctype>
#include <cassert>
#include "cgi.h"

using namespace std;

char Cgi::x2c(char *what) {
    char digit;  
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
  };

char Cgi::digit(int n) {
  return n>=10? 'A'+(n-10):'0'+n;
};

string Cgi::escape_html(string s) {
  static const char *ent[]={"&amp;","\"quot;","'apos;","<lt;",">gt;",0};
  for (size_t x=0;x<s.size();++x) {
    for (int i=0;ent[i];++i) {
      if (s[x]==ent[i][0]) {
	s[x]='&';
	s.insert(x+1,ent[i]+1);
	x+=strlen(ent[i]);
	break;
      }
    }
  }
  return s;
}

void Cgi::unescape_url(string &url) {
  size_t x,y;  
  for(x=0,y=0;y<url.size();++x,++y) {
    if (url[y]=='+') url[x]=' ';
    else if ((url[x] = url[y]) == '%') {
      url[x] = x2c(&url[y+1]);
      y+=2;
    }
  }
  url.erase(x,url.npos);//    url[x] = '\0';
}

int Cgi::escapanda(char c) {
  return ((unsigned char)c>127 || c=='+' || c=='&' || c==';' || c=='%');
};

string Cgi::escape_url(string s) {
  size_t i;
  char c;
  for (i=0;i<s.size();i++) {
    if (escapanda(c=s[i])) {
      s[i++]='%';
      s.insert(i++,1,digit(c/16)); //s=Mstrinschar(s,i++,digit(c/16));
      s.insert(i,1,digit(c%16)); //s=Mstrinschar(s,i,digit(c%16));
    } else if (s[i]==' ') s[i]='+';
  }
  return s;
};

string Cgi::GetEnv(const string &name) {
  char *p;
  p=getenv(name.c_str());
  if (p) return string(p);
  else return string("");
};

char Cgi::ReadChar(void) {
  char buf[2];
  if (length==0) return 0;
  length--;
  switch(method) {
  case GET:
    return QueryString[QueryPos++];
  default: /*POST o MULTI*/
    fread(buf,1,1,stdin);
    if (buf[0]==13) {length--;fread(buf,1,1,stdin);}
    return buf[0];
  }
};

string Cgi::ReadUntil(const char *end) {
  char c;
  string r;
  while ((c=ReadChar()) && !strchr(end,c)) {
      r+=c;
  }
  return r;
};

int Cgi::Expect(const char *what) {
  for(;*what && ReadChar()==*what;what++);
  if (*what) 
    throw CgiError(string("Something else expected: ")+what);
  return 1;
}

Cgi::Cgi() {
  buf=0;
  string p;
  assert(sizeof(size_t)==sizeof(unsigned int));
  sscanf(GetEnv("CONTENT_LENGTH").c_str(),"%u",&length);
  p=GetEnv("REQUEST_METHOD");
  if (p=="POST")
    method=POST;
  else if (p=="GET") {
    method=GET;
    QueryString=GetEnv("QUERY_STRING");
      QueryPos=0;
      length=QueryString.size();
  }
  else throw CgiError("unknown request method");
  p=GetEnv("CONTENT_TYPE");
  if (strchomp(p,"multipart/form-data; boundary=")) {
    boundary=p;
    boundary.insert(0,"--");
    if (method!=POST) throw CgiError("POST required for multipart");
    method=MULTI;
    if (!Expect(boundary.c_str()) || !Expect("\n")) 
      throw CgiError("Cannot decode multipart request");
    buf=new char[boundary.size()];
  }
  index_query= (method==GET && QueryString.size() && 
		QueryString.find('=')!=string::npos);
  empty_query=(length==0);
};

Cgi::~Cgi() {
  delete buf;
}

/*legge stdin fino a boundary*/

//#define boundary(i) (((i)<2)?'-':boundary[(i)-2])

string Cgi::GetBounded(void) {
  int l=boundary.size();
  
  int i,j=0;
  int from=0;
  string res;
  
  fflush(stderr);
  
  while (from<l && fread(buf+from,sizeof(char),l-from,stdin)) {
    length-=l-from;
    /* buf[0--from-1] e' uguale a boundary[0--from-1] */
    for (i=0;i<l;i++) {
      for (j=(i==0?from:0);buf[i+j]==boundary[j] && i+j<l;j++);
      if (i+j==l) break;
      j=0;
    }
    /*fino a i NON c'e' boundary*/
    if (i>0) {
      res.append(buf,i);
    }
    if (i<l)
      memmove(buf,buf+i,l-i);
    from=j;
  }
  i=ReadChar();
  if (i=='-')
    Expect("-\n");
  else if (i!='\n')
    throw CgiError("\\n expected!"); 
  return res;
};

bool Cgi::strchomp(string &p, const string &with) {
  string::size_type i;
  if (p.size()<with.size()) return false;
  for (i=0;i<with.size() && p[i]==with[i];++i);
  if (i!=with.size()) return false;
  p.erase(0,i);
  return true;
};

void Cgi::GetMultiItem(CgiItem *item) {
  fflush(stderr);
  
  item->name=item->filename="";
  item->value=""; item->value_length=0;
  while (1) {
    string buf=ReadUntil("\n");
    if (buf.size()==0) break;
    if (strchomp(buf,"Content-Disposition: ")) {
      while (buf.size()) {
	while (isspace(buf[0])) buf.erase(0,1);
	cout<<"buf="<<buf<<"\n";
	string::size_type p;
	p=buf.find(';');
	string field=string(buf,0,p);
	if (p==string::npos) buf.erase(0,p);
	else buf.erase(0,p+1);
	
	p=field.find('=');
	string name=string(field,0,p);
	cout<<"name="<<name<<"\n";
	if (p==string::npos) field.erase(0,p);
	else field.erase(0,p+1);
	if (field[0]=='"') {
	  field.erase(0,1);
	  if (field[field.size()-1]!='"') 
	    throw CgiError("Cannot decode multipart... \" expected");
	  field.erase(field.size()-1,field.size());
	}
	cout<<"field="<<field<<"\n";
	if (name=="name") 
	  item->name=field;
	else if (name=="filename")
	  item->filename=field;
      }
    }
  }
  item->value=GetBounded();
  if (item->value.size()>=2) 
    item->value.erase(item->value.end()-2,item->value.end());
};

bool Cgi::GetItem(CgiItem &item) {
  //  cout<<"length="<<length<<"\n";
  if (length==0) return false;
  if (method==MULTI) {
    /*    if (length<=4) return 0;*/
    GetMultiItem(&item);
  } else {
    item.name=ReadUntil("=");
    unescape_url(item.name);
    item.value=ReadUntil("&");
    unescape_url(item.value);
    item.value_length=item.value.size();
    item.filename="";
  }
  return true;
};

