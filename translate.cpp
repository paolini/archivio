#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <map>

bool link=true;
int verbose=10;


char description[][40]={
  "COGNOME", //0
  "NOME",
  "SESSO",
  "DATA di NASCITA",
  "PAESE di NASCITA",
  "PROV. di NASCITA", //5
  "PAESE di RESIDENZA",
  "PROV. di RESIDENZA",
  "INDIRIZZO",
  "No CIVICO",
  "CODICE FISCALE",
  "TELEFONO",
  "PRATICHE", //12
  "DISEGNI",  //13
  "NOTE"};    //14

string nonull(string s) {
  if (s.size()==0) return string("\\N");
  else return s;
};

class Nome {
public:
  int id;
  int connections;
  string field[15];
public:
  Nome() {};
  Nome(FILE *in,int n);
  void writeShort(ostream &out) const;
  void writeLong(ostream &out) const;
  void save(ostream &out) const;
  static void convert(string &s);
};

class Pratica {
public:
  string id;
  string mod;
  string description;
  string luogo;
  Pratica() {};
  //  Pratica(string the_mod, string the_description, string the_luogo=""):
  //    mod(the_mod), description(the_description), 
  //   luogo(the_luogo) {};
  void writeLong(ostream &out) const;
  void save(ostream &out) const;
};

class Link {
public:
  int nome;
  string pratica;
  Link() {};
  Link(int n, string s): nome(n), pratica(s) {};
  void save(ostream& out) const {
    out<<nome<<"\t"<<pratica<<"\n";};
};

map<int,Nome> clienti;
map<string,Pratica> pratiche;
vector<Link> collegamenti;

void Nome::convert(string &s) {
  static unsigned char from[]={135,151,149,133,248,138,141,0};
  static unsigned char to[]="çùòà°èì";
  int i;
  for (i=0;i<s.length();i++) {
    unsigned char c=s[i];
    if (c>127) {
      int j;
      for (j=0;from[j] && from[j]!=c;j++);
      if (from[j]) {
	//	cerr<<" -> ";
	s[i]=to[j];
	//	cerr<<s<<"\n";
      } else {
	cerr<<"convert: "<<s<<"\n";
	cerr<<"*** char code: "<<int(c)<<"\n";
      }
    }
  }
};

bool extract(string &s,const Nome &nome, int id, string P) {
  while (s.length()!=0 && isspace(s[0])) s.erase(0,1);
  if (s.length()==0) return false;
  if (!isdigit(s[0])) {
    throw runtime_error("cannot parse pratica, digit expected");
  }
  int i;
  Pratica pratica;
  for (i=0;i<s.length() && isdigit(s[i]);++i);
  if (i>4) {
    throw runtime_error("id too long ("+s);
  }
  pratica.id=string(s,0,i);
  for (int k=0;k<4-i;++k)
    pratica.id.insert(0,string("0"));
  assert(pratica.id.length()==4);
  int j=i;
  for (;i<s.length() && isupper(s[i]);++i) {
    if (!(s[i]=='L' || s[i]=='M')) {
      cerr<<"warning: id character "<<s[i]<<" strange in "<<id<<" "
	  <<P<<pratica.id<<"\n";
    }
  }
  if (i<s.length() && s[i]!=' ' && s[i]!='-' && s[i]!=',') {
    if (verbose>0)
      cerr<<"warning: space expected in "
	  <<id<<" "<<P<<pratica.id<<" ("<<s<<")\n";
  }
  assert(j<=s.length());
  pratica.mod=string(s,j,i-j);
  assert(pratica.mod.length()==i-j);
  //  cout<<"found id "<<P<<pratica.id<<" in "<<s<<"\n";
  s.erase(0,i);
  while (s.length()>0 && isspace(s[0])) s.erase(0,1);
  i=0;
  do {
    for (;i<s.length() && s[i]!=',';i++);
    if (i==s.length()) {
      while (isspace(s[s.length()-1])) s.erase(s.length()-1,1);
      pratica.description=s;
      s="";
      break;
    } else {
      int j;
      for (j=i+1;j<s.length() && isspace(s[j]);++j);
      if (j<s.length() && !isdigit(s[j])) {
	if (verbose>0)
	  cerr<<"warning, comma without number found in "
	      <<id<<" "<<P<<pratica.id<<" ("<<
	      s<<")\n";
	
	i++;
      } else {
	int j;
	for (j=i;j>0 && isspace(s[j-1]);--j);
	pratica.description=string(s,0,j);
	s.erase(0,i+1);
	break;
      }
    }
  } while (true);
  //cout<<"found description: "<<pratica.description<<"\n";
  pratica.id.insert(0,P);

  // controlla se la pratica c'era già
  {
    map<string,Pratica>::iterator j=pratiche.find(pratica.id);
    //    for (j=0;j<pratiche.size() && pratiche[j].id!=pratica.id;j++);
    if (j!=pratiche.end()) { // c'e` gia`
      int k;
      for (k=0;k<j->second.description.length() &&
	     k<pratica.description.length() && 
	     j->second.description[k]==pratica.description[k];k++);
      if (pratica.description.length()==k || 
	  !isalnum(pratica.description[k+1]))
	pratica.description.erase(0,k);
      if (pratica.description.length() && isspace(pratica.description[0]))
	pratica.description.erase(0,1);
      j->second.description.insert(0,pratica.description + " ");
    } else { // nuova
      pratiche[pratica.id]=pratica;
    }
  }
  
  collegamenti.push_back(Link(id,pratica.id));
  return true;
}

void LoadNome(FILE *in,int id) {
  char s[300];
  char *p;
  Nome my;
  my.id=id;
  my.connections=0;
  if (verbose>20)
    cerr<<"Loading "<<id<<"\n";
  for (int j=0;j<15;++j) {
    for (p=s;(*p=fgetc(in))!=13;p++);
    *p=0;
    my.field[j]=s;
    // converti le lettere accentate:
    Nome::convert(my.field[j]);
  }

  // linka
  if (link) {
    string P="P";
    for (int j=12;j<14;j++) {
      string s=my.field[j];
      try {
	while (extract(s,my,id,P));
      } catch (runtime_error e) {
	cerr<<"Error while linking "<<id<<": "<<e.what()<<"\n";
      }
      P="D";
    } 
  }

  // connect
  map<int,Nome>::iterator j;
  for (j=clienti.begin();j!=clienti.end();++j) {
    if (j->second.field[0]==my.field[0] &&
	j->second.field[1]==my.field[1] &&
	//my.field[1].length()>0 &&
	j->second.field[14]==my.field[14]) {
      int k;
      for (k=2;k<12;k++) {
	if (!(j->second.field[k]==my.field[k]
	      || j->second.field[k].length()==0
	      || my.field[k].length()==0)) {
	  cerr<<"different fields in connection "<<id<<" -> "<<
	    j->first<<"\n";
	  break;
	}
      }
      if (k==12) { // connetti
	for (k=2;k<12;k++) {
	  if (j->second.field[k].length()==0) j->second.field[k]=my.field[k];
	}
	for (k=0;k<collegamenti.size();k++)
	  if (collegamenti[k].nome==id) collegamenti[k].nome=j->first;
	id=j->first;
	j->second.connections++;
	for (k=12;k<14;k++) {
	  j->second.field[k].insert(0,", ");
	  j->second.field[k].insert(0,my.field[k]);
	}
	break;
      } 	
    }
  }
  if (j==clienti.end()) {
      clienti[id]=my;
  }
}

void Nome::writeShort(ostream &out) const {
  for (int i=0;i<15;i++) {
    if (field[i].length()) {
      cout<<"F"<<i<<": "<<field[i]<<" ";
    }
  }
  cout<<"\n";
};

void Nome::writeLong(ostream &out) const {
  out<<"NOMINATIVO "<<id<<" connections: "<<connections<<"\n";
  for (int i=0;i<15;i++) {
    for (int j=0;j<18-strlen(description[i]);j++) out<<" ";
    out<<description[i]<<": "<<field[i]<<"\n";
  }
  out<<"Links: ";
  for (int i=0;i<collegamenti.size();++i) {
    if (collegamenti[i].nome==id) {
      out<<collegamenti[i].pratica<<" ";
    }
  }
  out<<"\n\n";
};

void Nome::save(ostream &out) const {
  out<<id;
  for (int i=0;i<15;++i) {
    out<<"\t";
    if (i==12) i=14;
    if (i==3)  {//data di nascita
      if (field[i].size()==0) 
	out<<"\\N";
      else if (field[i].size()==8) {
	out<<"19"<<string(field[i],6,2)<<"-"
	   <<string(field[i],3,2)<<"-"
	   <<string(field[i],0,2);
      } else {
	out<<field[i];
	cerr<<"data "<<field[i]<<" non la capisco!\n";
      }
    } else
      //  out<<nonull(field[i]);
      out<<field[i];
  }
  out<<"\n";
}

void Pratica::writeLong(ostream& out) const {
  out<<"PRATICA "<<id<<" mod "<<mod<<"\n"
     <<"  description: "<<description<<"\n"
     <<"  links: ";
  for (int i=0;i<collegamenti.size();++i) {
    if (collegamenti[i].pratica==id) {
      out<<collegamenti[i].nome<<" ";
    }
  }
  out<<"\n\n";
};

void Pratica::save(ostream& out) const {
  out<<id<<"\t"<<mod<<"\t"<<nonull(description)<<"\t"<<nonull(luogo)<<"\n";
};

void eraseReferences(void) {
  int count=0;
  for (int i=0;i<collegamenti.size();i++) {
    // if (i%100==0)
    //      cerr<<i*100/collegamenti.size()<<"%\n";
    map<int,Nome>::iterator j=clienti.find(collegamenti[i].nome);
    map<string,Pratica>::iterator k=pratiche.find(collegamenti[i].pratica);
    if (j!=clienti.end() && k!=pratiche.end()) {
      for (int l=0;l<2;l++) {
	int d;
	while(j->second.field[l].size()>2 && 
	      (d=k->second.description.find(j->second.field[l]))
	      !=string::npos) {
	  count++;
	  k->second.description.erase(d,j->second.field[l].length());
	  if (d>0 && k->second.description[d-1]=='-')
	    k->second.description.erase(d-1,1);
	}
      }
    } else {
      if (j==clienti.end()) cerr<<"ERRORE non trovo cliente "<<
			       collegamenti[i].nome<<"\n";
      else cerr<<"ERRORE non trovo pratica "<<
	     collegamenti[i].pratica<<"\n";
      abort();
    }
  }
  cerr<<"Erased "<<count<<" references\n";
}

main(int argc, char *argv[]) {
  try{
    for (int i=1;i<argc;++i) {
      if (!strcmp(argv[i],"cliente")) {
	if (i+1<argc) {
	  if (!strcmp(argv[i+1],"all")) {
	    // scrive tutti i nominativi in formato .txt    
	    for (map<int,Nome>::iterator j=clienti.begin();
		 j!=clienti.end();++j) {
	      j->second.writeLong(cout);
	    }
	  } else {
	    int n=atoi(argv[i+1]);
	    clienti[n].writeLong(cout);
	  }
	  i++;
	}
      } else if (!strcmp(argv[i],"pratica")) {
	if (i+1>=argc) continue;
	string s=argv[i+1];
	i++;
	map<string,Pratica>::iterator j;
	for (j=pratiche.begin();j!=pratiche.end();++j) {
	  if (j->first == s || s=="all") 
	    j->second.writeLong(cout);
	}
      } else if (!strcmp(argv[i],"nolink")) {
	link=false;
      } else if (!strcmp(argv[i],"save")) {
	{
	  ofstream out("clienti.dat");
	  for (map<int,Nome>::iterator j=clienti.begin();j!=clienti.end();++j) 
	    j->second.save(out);
	}
	{
	  ofstream out("pratiche.dat");
	  for (map<string,Pratica>::iterator j=pratiche.begin();j!=pratiche.end();
	       ++j)
	    j->second.save(out);
	}
	{
	  ofstream out("links.dat");
	  for (vector<Link>::iterator j=collegamenti.begin();j!=collegamenti.end();
	       ++j)
	    j->save(out);
	}
      } else if (!strcmp(argv[i],"max")) {
	{
	  int max[15];
	  for (int i=0;i<15;++i) max[i]=0;
	  map<int,Nome>::iterator j;
	  for (j=clienti.begin();j!=clienti.end();++j) {
	    for (int i=0;i<15;++i)
	      if (j->second.field[i].size()>max[i]) 
		max[i]=j->second.field[i].size();
	  }
	  for (int i=0;i<15;++i)
	    cout<<"["<<i<<"]="<<max[i]<<" ";
	  cout<<"\n";
	}
	{
	  int max1=0,max2=0;
	  map<string,Pratica>::iterator j;
	  for (j=pratiche.begin();j!=pratiche.end();++j) {
	    if (j->second.description.size()> max1) 
	      max1=j->second.description.size();
	    if (j->second.luogo.size()>max2)
	      max2=j->second.luogo.size();
	  }
	  cout<<max1<<" "<<max2<<"\n";
	}
      } else if (!strcmp(argv[i],"load")) {
	unsigned int len=0;
	unsigned int xx=0;
	int j;
	char field[15][300];
	char *p;
	FILE *in;
	i++;
	in=fopen(argv[i],"rb");
	if (in==NULL) {
	  cerr<<"cannot open file "<<argv[i]<<"\n";
	  abort();
	}
	
	fread(&len,1,4,in);  
	fread(&xx,1,4,in);
	fprintf(stderr,"len: %d  xx=: %d\n",len,xx);
	
	// Carica nominativi in memoria
	
	cerr<<"Loading "<<len<<" nomi\n";
	
	for (int i=0;i<len;++i) {
	  LoadNome(in,i+1);
	}      
	
	eraseReferences();
	
	cerr<<"clienti: "<<clienti.size()<<" pratiche: "<<pratiche.size()
	    <<" links: "<<collegamenti.size()<<"\n";
      }
    }
  } catch (runtime_error e) {
    cerr<<"ERRORE: "<<e.what()<<"\n";
  } catch (logic_error e) {
    cerr<<"ERRORE: "<<e.what()<<"\n";
  } catch (exception e) {
    cerr<<"ERRORE! \n";
  }
}
