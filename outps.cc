#include <stdio.h>
#include <cstring>
#include <string>

#include "mysql.hh"


char campo[16][4096];
void strupr(char *s) {
  for (;*s;++s)
    *s=toupper(*s);
}

int is(int n)
	{
	return(campo[n][0]);
	}
void strins(char *pos)
	{
	char *end;
	for(end=strchr(pos,0);end>=pos;end--)
		*(end+1)=*end;
	*pos=' ';
	}
void codeset(char *s)
	{
	strupr(s);
	if (*s)
		{
		strins(s+3);
		strins(s+7);
		strins(s+13);
		}
	}
void parstrip(char *s)
	{
	for (;*s!='\0';s++)
		{
		 if (*s=='(' || *s==')')
			{
			strins(s);
			*s='\\';
			s++;
			}
		}
	}
static char tab1[]={'`','\'','\"','^','~',',','\0'};
static char tab2[]={"aeiouAEIOUnNcC"};
static char *tab3[]={"grave","acute","dieresis","circumflex","tilde","cedilla",""};

// tab0[i][j] -> tab2[i]tab3[j]

// SQL codes (UNICODE??)
static char *tab4[]={"?", //a
		     "??",  //e
		     "?", //i
		     "?", //o
		     "?", //u
		     "?", //A
		     "??", //E
		     "?", //I
		     "?", //O
		     "?", //U
		     
		     "", //n
		     "", //N
		     "     ?", //c
		     "     ?", //C
		     0};

//DOS codes (?)
static char *tab0[]={"?????",
		     "????",
		     "????",
		     "?????",
		     "????",
		     "?????",
		     "Ԑ??",
		     "????",
		     "?????",
		     "????",
		     "?????",
		     "?????",
		     "??????",
		     "??????",NULL};
char *ps_octal(unsigned char n)
{
  static char s[4];
  s[2]=n%8+'0';
  n/=8;
  s[1]=n%8+'0';
  n/=8;
  s[0]=n%8+'0';
  s[3]='\0';
  return s;
}

void init(FILE *prn)
	{
	int i,j;
	fprintf(prn,"%%!PS-PostScript generated by arcps.c by Manu\n");
	fprintf(prn,"/cm {72 2.54 div mul} def\n");
	if (true) { //reencode fonts
	  fprintf(prn,
		  "/reencsmalldict 14 dict def\n"
		  "/ReEncodeSmall\n"
		  "{reencsmalldict begin\n"
		  "/newcodesandnames exch def\n"
		  "/newfontname exch def\n"
		  "/basefontname exch def\n"
		  "/basefontdict basefontname findfont def\n"
		  "/newfont basefontdict maxlength dict def\n"
		  "basefontdict\n"
		  "{exch dup /FID ne {dup /Encoding eq\n"
		  "{exch dup length array copy\n"
		  "newfont 3 1 roll put }\n"
		  "{exch newfont 3 1 roll put}\n"
 		  "ifelse\n"
		  "}{pop pop }\n"
		  "ifelse }forall\n"
		  "newfont /FontName newfontname put\n"
		  "newcodesandnames aload pop\n"
		  "newcodesandnames length 2 idiv\n"
		  "{newfont /Encoding get 3 1 roll put}\n"
		  "repeat\n"
		  "newfontname newfont definefont pop\n"
		  "end\n"
		  "}def\n");
	  fprintf(prn,"/scandvec[\n");
	  for (i=0;tab4[i]!=NULL;i++)
	    for (j=0;tab4[i][j]!='\0';j++)
	      if (tab4[i][j]!='?' && tab4[i][j]!=' ')
		fprintf(prn,"8#%s /%c%s\n",ps_octal(tab4[i][j]),tab2[i],tab3[j]);
	  fprintf(prn,"] def\n");
	  fprintf(prn,
		  "/Times-Roman /T0 scandvec ReEncodeSmall\n"
		  "/Times-Italic /T1 scandvec ReEncodeSmall\n"
		  "/Times-Bold /T2 scandvec ReEncodeSmall\n");
	  fprintf(prn,"/FN /T0 findfont 10 scalefont def\n");
	  fprintf(prn,"/FB /T2 findfont 10 scalefont def\n");
	  fprintf(prn,"/FI /T1 findfont 10 scalefont def\n");
	  fprintf(prn,"/FH /T2 findfont 1 cm scalefont def\n");
	} else {
	  fprintf(prn,"/FN /Times-Roman findfont 10 scalefont def\n");
	  fprintf(prn,"/FB /Times-Bold findfont 10 scalefont def\n");
	  fprintf(prn,"/FI /Times-Italic findfont 10 scalefont def\n");
	  fprintf(prn,"/FH /Times-Bold findfont 1 cm scalefont def\n");
	}
	fprintf(prn,"/bold {FB setfont} def\n");
	fprintf(prn,"/norm {FN setfont} def\n");
	fprintf(prn,"/it {FI setfont} def\n");
	fprintf(prn,"/huge {FH setfont} def\n");
	fprintf(prn,"%% x0,y0 x1,y1: margini,\n");
	fprintf(prn,"%% skip: interlinea, limbo\n");
	fprintf(prn,"/ncols 2 def\n");
	fprintf(prn,"/col 1 def\n");
	fprintf(prn,"/colwidth 9 cm def\n");
	fprintf(prn,"/left 1 cm def\n");
	fprintf(prn,"/x0 2.5 cm def /y0 26 cm def \n");
	fprintf(prn,"/x1 10 cm def /y1 2 cm def \n");
	fprintf(prn,"x0 y0 moveto /skip 10 def \n");
	fprintf(prn,"/limbo 4 skip mul def \n");
	fprintf(prn,"/xgoto {currentpoint exch pop moveto} def");
	fprintf(prn,"/margin {dup currentpoint pop le {xgoto} {pop} ifelse} def\n");
	fprintf(prn,"/ycheck {currentpoint exch pop limbo sub y1 le {newpage} if} def\n");
	fprintf(prn,"/newline {x0 currentpoint exch pop skip sub dup y1 le {newpage pop pop} {moveto} ifelse} def\n");
	fprintf(prn,"/write {dup stringwidth pop currentpoint pop add x1 ge {newline} if show} def\n");
	//	fprintf(prn,"/newpage {showpage x0 y0 moveto} def\n");
	fprintf(prn,
		"/newpage { /col col 1 add def col ncols "
		" le {colwidth 0 translate} "
		"{/col 1 def showpage "
		" left 0 translate} "
		"ifelse "
		" x0 y0 moveto"
		"} def\n");
	fprintf(prn,"left 0 translate\n");
	}
int issep(char c)
	{
	return c==' '||c==','||c==';'||c=='-'||c=='\0';
	}
int ispresep(char c)
	{
	return c=='-' || c=='\0';
	}

void writeline(FILE *prn, const char *s) {
  int i; 
  //char *p;  p=s+i;
  char c;
  while (*s!='\0') {
    for (i=1;!issep(*(s+i));i++);
    if (!ispresep(*(s+i))) i++;
    fprintf(prn,"(%.*s) write ",i,s);
    s+=i;
  }
}


unsigned char myup(unsigned char c)
{
  static int i,j;
  if (c>127) {
      for (i=0;i<5;i++)
	for (j=0;tab4[i][j]!='\0';j++)
	  if ((unsigned char)tab4[i][j]==c)
	    return tab4[i+5][j];
      return c;
    }
  else
    return toupper(c);
}
void mystrup(char *s) {
  for (;*s!='\0';s++)
    {
      *s=myup((unsigned char)*s);
    }
}

void mystrup2(char *s) {
  s[0]=myup(s[0]);
  for (s++;*s!='\0';s++)
    if (*(s-1)==' ' || *(s-1)=='\'')
      *s=myup(*s);
}

void par_filter(char *s) {
  int l=strlen(s);
  for (;*s;++s,--l) {
    if (*s=='(' || *s==')') {
      memmove(s+1,s,l+1);
      *s='\\';
      ++s; // l risulta essere giusto!
    }
  }
}

string par_filter(string s) {
  for (int i=0;i<s.size();++i) {
    if ((s[i]=='(' || s[i]==')') && (i==0 || s[i-1]!='\\')) {
      s.insert(i,"\\");
      ++i;
    }
  }
  return s;
}

void stampa(FILE *prn){
	int flag=0;
	int i;
	mystrup(campo[0]);
	mystrup(campo[1]);
	mystrup2(campo[4]);
	mystrup2(campo[6]);
	//	codeset(campo[10]);
	for (i=0;i<15;i++)
		parstrip(campo[i]);
	fprintf(prn,"\nycheck\n");
	fprintf(prn,"x0 1.0 cm sub xgoto bold (%s %s) show norm\n",campo[0],campo[1]);
	if (is(14))
		fprintf(prn,"( \\(%s\\)) show ",campo[14]);
	fprintf(prn,"(: ) show ");

	bool newline=false;
	if (is(3)||is(4)||is(6)||is(8)||is(11))	{
		flag=1;
		if (is(4)||is(3))
			{
			  //			      j=0;
fprintf(prn,"(n. ) write ");
			if (is(4))
				{
				fprintf(prn,"(a ) write ");
				writeline(prn,campo[4]);
				fprintf(prn,"( ) show");
				}
			if (is(3))
				fprintf(prn,"(il %s ) write ",campo[3]);
			}
		if (is(6))
			{
			fprintf(prn,"(res. a ) write ");
			writeline(prn,campo[6]);
			fprintf(prn,"( ) show ");
			}
		if (is(8))
			{
			writeline(prn,campo[8]);
			if (is(9))
				fprintf(prn,"(,%s) write ",campo[9]);
			fprintf(prn,"( ) show ");
			}
		if (is(11))
			fprintf(prn,"(tel.) write (%s ) write ",campo[11]);
		fprintf(prn,"newline\n");
		newline=true;
	} else fprintf(prn,"newline\n");
}

void print_fasci(FILE *prn, mysql_table &table) {
  int n=table.size();
  string lastid;
  int k=0;
  int j=0;

  init(prn);
  fprintf(prn,"90 rotate 0 -20 cm translate\n");
  double x;
  for (int i=0;i<n;++i) {
    string id=table[i][0];
    if (id!=lastid) {
      x=3.0+9.0*(k%3);
      if (k%3==0 && k>0) {
	fprintf(prn,"showpage\n\n");
	fprintf(prn,"90 rotate 0 -20 cm translate\n");
      }
      fprintf(prn,"%lf cm %lf cm moveto\n",x,18.0);
      fprintf(prn,"huge (%s) show bold \n ",id.c_str());
      fprintf(prn,"%lf cm %lf cm moveto\n",x,17.5);
      fprintf(prn,"( ) show (%s) show ( ) show (%s) show \n",
	      table[i]["descrizione"].c_str(), 
	      table[i]["luogo"].c_str());
      // cout<<"<a href='"<<myref<<"?pra="<<id<<"'>";
      // cout<<id<<"</a>"<<table[i]["mod"]<<": \n";
      // cout<<table[i]["descrizione"]<<" ("<<table[i]["luogo"]<<")\n";
      k++;
      j=0;
    }
    
    fprintf(prn,"%lf cm %lf cm moveto\n",x,17.5-1.0-j*0.5);
    fprintf(prn,"(%s) show ( ) show (%s) show\n",
	    (table[i]["cognome"]).c_str(), (table[i]["nome"]).c_str());
    
    j++;
    
    lastid=id;
  }
  //  cout<<"<br>\n";
};

int print(FILE *prn, mysql &sql, mysql_table &table, bool pag) {
  int i;
  char c;
  //  bool pag=false;  // true non funziona!
  char curletter=0;
  int nletter=0;

  init(prn);
  for(i=0;i<table.size();++i) { // itera sui nominativi
    for (int j=0;j<12;++j) {
      strcpy(campo[j],table[i][j+1].c_str());
    }
    if (table[i][4].size()==10)
      strcpy(campo[3],(table[i][4].substr(8,2)+"/"+table[i][4].substr(5,2)+
		       "/"+table[i][4].substr(0,4)).c_str());
    strcpy(campo[14],table[i]["note"].c_str());
    strcpy(campo[12],"");
    strcpy(campo[13],"");
    
    c=tolower(campo[0][0]);
    if (c!=curletter&&pag)
      {
	if (curletter)
	  fprintf(prn,"\n /col ncols def newpage ");
	//	fprintf(prn,"left 0 translate /col 1 def \n");
	fprintf(prn,"x0 x1 add 2 div 1 cm sub y0 1 cm sub moveto "
		"huge (%c) show norm newline newline\n",myup(c));
	curletter=c;
	nletter++;
      }


    stampa(prn);

    string id=table[i][0];

    // pratiche collegate a id
    mysql_table tab
      =sql.query("select pratiche.id,luogo,descrizione, "
		 "clienti.id,cognome,nome, pratiche.mod "
		 "from links as l1, links as l2,pratiche,clienti "
		 "where l1.pratica=l2.pratica "
		 "and l1.cliente='"+id+"' "
		 "and l2.cliente=clienti.id "
		 "and l1.pratica=pratiche.id "
		 "order by l1.pratica,l2.cliente");
    
    for (int j=0;j<tab.size();) { // merge pratiche
      fprintf(prn,"x0 0.5 cm sub xgoto ");
      fprintf(prn,"it ");
      string pratica=tab[j][0];      
      writeline(prn,(pratica+tab[j][6]+": ").c_str());
      fprintf(prn,"norm ");
      if (tab[j][1].size())
	writeline(prn,(par_filter(tab[j][1])+" ").c_str());
      writeline(prn,(par_filter(tab[j][2])).c_str());
      bool altri=false;
      for (;j<tab.size() && tab[j][0]==pratica;++j) {
	//altri clienti con la stessa pratica
	if (tab[j][3]==id) continue;
	if (!altri) {
	  writeline(prn," > ");
	  fprintf(prn,"it ");
	  altri=true;
	} else
	  writeline(prn,", ");
	char buf[4096];
	strcpy(buf,(tab[j][4]+" "+tab[j][5]).c_str());
	mystrup2(buf);
	writeline(prn,buf);
      }
      fprintf(prn,"norm newline\n");
    }
    
  }
  fprintf(prn,"\nshowpage\n");
  return 0;
}

void fascetta(FILE *prn, mysql &sql, string id) {
  mysql_table tab=sql.query("select cognome, nome, mod, descrizione, luogo "
			      "from pratiche, links, clienti "
			      "where clienti.id = links.cliente "
			      "and links.pratica = pratiche.id "
			      "and pratiche.id='"+id+"'");
  if (tab.size() == 0)
    throw runtime_error("La pratica "+id+" non ha clienti associati");
  /* to be continued... */
}
