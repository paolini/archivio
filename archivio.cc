#include <iostream>
#include <stdexcept>
#include <ctype.h>
#include <cassert>

#include <mysql.h>

#include "mysql.hh"
#include "cgi.h"
#include "outps.hh"

#ifndef VERSION
#define VERSION "?.?"
#endif

#ifndef DATA
#define DATA "??.??.??"
#endif

/* 
   TABELLE DATABASE mysql
*/

/*  creazione tabella pratiche

  create table pratiche (
  id CHAR(5) NOT NULL,
  mod CHAR(2),
  descrizione VARCHAR(160),
  luogo VARCHAR(40),
  PRIMARY KEY (id)
  );
  
  load data local infile "pratiche.dat" into table pratiche;

*/

/*  creazione tabella clienti

create table clienti (
        id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
        cognome VARCHAR(40) NOT NULL,
        nome VARCHAR(40),
        sesso CHAR(1),
        data_nascita DATE,
        paese_nascita VARCHAR(40),
        prov_nascita CHAR(2),
        paese_residenza VARCHAR(40),
        prov_residenza CHAR(2),
        indirizzo VARCHAR(80),
        no_civico VARCHAR(10),
        codice_fiscale CHAR(16),
        telefono VARCHAR(40),
        note VARCHAR(160),
        INDEX name (cognome,nome)
        );

load data local infile "clienti.dat" into table clienti;

*/

/* creazione tabella links

create table links (
        cliente INT NOT NULL,
        pratica CHAR(5) NOT NULL,
        KEY (cliente),
	KEY (pratica)
        );

load data local infile "links.dat" into table links;

 */

char *myref;

char *field[]={"cognome","nome","sesso","data_nascita","paese_nascita",
	       "prov_nascita","paese_residenza","prov_residenza","indirizzo",
	       "no_civico", "codice_fiscale","telefono","note",0};
int width[]={20,20,1,10,20,2,20,2,30,3,16,20,20};

char *field2[]={"mod","descrizione","luogo",0};
int width2[]={2,30,20};


string upper(string s) {
  for (int i=0;i<s.size();++i) {
    s[i]=toupper(s[i]);
  }
  return s;
}

string data(const string &s) {
  return string(s,6,2)+"."+string(s,4,2)+"."+string(s,0,4);
}

void write_table(const mysql_table &table) {
  cout<<"<table border=1 bgcolor='lightgreen'><tr>";
  for (int j=0;j<table.ncols();++j) 
    cout<<"<th>"<<table.name(j)<<"</th>";
  cout<<"</tr>\n";
  for (int i=0;i<table.size();++i) {
    if (i%2)
      cout<<"<tr bgcolor='pink'>";
    else
      cout<<"<tr bgcolor='yellow'>";
    for (int j=0;j<table.ncols();++j) {
      if (table.name(j)=="id" || table.name(j)=="pratica"
	  || table.name(j)=="cliente") {
	if (isdigit(table.val(i,j)[0]))
	  cout<<"<td><a href='"<<myref<<"?id="<<table.val(i,j)<<"'>"
	      <<table.val(i,j)<<"</a></td>";
	else
	  cout<<"<td><a href='"<<myref<<"?pra="<<table.val(i,j)<<"'>"
	      <<table.val(i,j)<<"</a></td>";
      } else
	cout<<"<td>"<<table.val(i,j)<<"</td>";
    }
    cout<<"</tr>\n";
  }
  cout<<"</table>\n";
};

void write_timestamp(string a) {
  if (a!="") {
    string date=string(a,0,8);
    cout<<"<p>Ultima modifica in data: <a href='"<<myref<<"?fromdate="<<date<<"&todate="<<date<<"'>"<<data(a)<<"</a></p>\n";
  }
}

void write_id(string id) {
  mysql sql("archivio");
  mysql_table table=
    sql.query("select * from clienti where id='"+id+"'");
  if (table.size()!=1) 
    throw runtime_error("ID "+id+" non ha una corrispondenza!"); 
  mysql_row row=table[0];
  id=table[0]["id"];
  cout<<"<h2>Dati personali nominativo #"<<id<<"</h2>\n";
  cout<<"<strong>"<<upper(row["cognome"])<<", "<<upper(row["nome"])<<"</strong>\n";
  string a,b;
  a=row["data_nascita"];
  b=row["paese_nascita"];
  if (a!="" || b!="") {
    string s=row["sesso"];
    if (s=="m")
      cout<<"nato ";
    else if (s=="f")
      cout<<"nata ";
    else 
      cout<<"nato/a ";
  }
  if (b!="")
    cout<<"a "<<upper(b)<<" ";
  if (a!="") { 
    cout<<"il "<<string(a,8,2)<<"."<<
      string(a,5,2)<<"."<<string(a,0,4)<<" ";
  }
  a=row["paese_residenza"];
  b=row["indirizzo"];
  if (a!="" || b!="") 
    cout<<"residente ";
  if (a!="") {
    cout<<"a "<<upper(a)<<" ";
    string s=row["prov_residenza"];
    if (s!="")
      cout<<"("<<upper(s)<<")"<<" ";
  }
  if (b!="") {
    cout<<"in "<<upper(b)<<" ";
    string s=row["no_civico"];
    cout<<"n. "<<upper(s)<<" ";
  }
  a=row["codice_fiscale"];
  if (a!="")
    cout<<"cod. fiscale "<<upper(a)<<" ";
  a=row["telefono"];
  if (a!="")
    cout<<"tel. "<<a<<" ";
  a=row["note"];
  if (a!="")
    cout<<"note: "<<upper(a)<<" ";
  

  write_timestamp(row["timestamp"]);

  cout<<"<p>Puoi <a href='"<<myref<<"?edit=&id="<<id<<"'>modificare</a> "
    "i dati di questo nominativo. "
    "Puoi anche <a href='"<<myref<<"?delete="<<id<<"'>cancellare</a> "
    "questo nominativo.</p>\n";

  mysql sql2("archivio");
  string s="select "
    "pratiche.id,mod,descrizione,luogo,l2.cliente,cognome,nome "
    "from pratiche,links as l1,links as l2,clienti "
    "where l1.cliente='"+id+"' and l1.pratica=pratiche.id "
    "and l2.pratica=pratiche.id and l2.cliente=clienti.id "
    "order by l1.pratica ";
  cout<<"<!-- query: "<<s<<" -->\n";
  table=sql2.query(s);
  char cur='X';
  for (int i=0;i<table.size();) {
    row=table[i];
    //scrivi i dati della pratica
    {
      a=row["id"];
      if (cur!=a[0]) {
	if (a[0]=='D')
	  cout<<"<h2>Disegni</h2>\n";
	else if (a[0]=='P')
	  cout<<"<h2>Pratiche</h2>\n";
	cur=a[0];
      }
      cout<<"<a href='"<<myref<<"?pra="<<row["id"]<<"'><strong>"
	  <<row["id"]<<"</strong></a>"<<row["mod"]<<": ";
      cout<<row["descrizione"]<<" ";
      a=row["luogo"];
      if (a!="")
	cout<<"localit&agrave; "<<a<<" ";
    }
    //scrivi altri clienti di questa pratica
    {
      bool first=true;
      assert(table.name(0)=="id");
      int j;
      for (j=i;j<table.size() && table.val(i,0)==table.val(j,0);j++) {
	row=table[j];
	if (row["cliente"]==id) continue;
	if (first) cout<<"[";
	else cout<<" - ";
	first=false;
	cout<<"<a href='"<<myref<<"?id="<<row["cliente"]
	    <<"'>"<<row["cognome"]<<", "<<row["nome"]<<"</a>";
      }
      i=j;
      if (!first)
	cout<<"] ";
    }
    cout<<"<br>\n";
  }
}

void write_pra(string id) {
  int nclienti=0;
  mysql sql("archivio");
  mysql_table table
    =sql.query("select pratiche.id,mod,descrizione,luogo,"
	       "cliente,cognome,nome,pratiche.timestamp "
	       "from pratiche,links,clienti "
	       "where pratiche.id='"+id+"' and pratiche.id=pratica "
	       "and clienti.id=cliente");
  nclienti=table.size();
  if (nclienti<1) {
    table=sql.query("select id,mod,descrizione,luogo "
		    "from pratiche "
		    "where id='"+id+"'");
    if (table.size()<1) {
      throw runtime_error("Pratica o disegno "+id+" inesistente\n");
    }
  }
  mysql_row row=table[0];
  id=table[0]["id"];
  bool pratica=true;
  if (toupper(id[0])=='D') pratica=false;
  else if (toupper(id[0])=='P') pratica=true;
  else cout<<"<p><strong>Attenzione:</strong> il codice della pratica "
	 "dovrebbe cominciare per 'D' o 'P'.</p>\n";
  string questo;
  if (pratica) {
    cout<<"<h2>Pratica "<<id<<"</h2>\n";
    questo="questa pratica";
  }
  else {
    cout<<"<h2>Disegno "<<id<<"</h2>\n";
    questo="questo disegno";
  }
  cout<<"<strong>"<<id<<"</strong>"<<row["mod"]<<": "<<row["descrizione"];
  string a=row["luogo"];
  if (a!="") {
    cout<<" ("<<upper(a)<<")";
  }
  cout<<"<br>\n";

  write_timestamp(row["timestamp"]);

  if (nclienti) {
    cout<<"<h2>Nominativi associati</h2>\n";
    for (int i=0;i<table.size();++i) {
      cout<<"<a href='"<<myref<<"?id="<<table[i]["cliente"]<<"'>"
	  <<upper(table[i]["cognome"])<<", "<<upper(table[i]["nome"])
	  <<"</a><br>\n";
    }
  } else {
    cout<<"<p><strong>Attenzione:</strong> nessun nominativo associato "
      "a "<<questo<<"!</p>\n";
  }
  cout<<"<p>Puoi <a href=\""<<
    myref<<"?modpra=edit&amp;id="<<id<<"\">modificare"
    "</a> "<<questo<<". "
    "Puoi anche <a href='"<<myref<<"?delete="<<id<<"'>cancellare</a> "
      <<questo<<".</p>\n";
}

void write_list(mysql_table &table) {
  int n=table.size();
  cout<<n<<" corrispondenze<br><br>\n";
  for (int i=0;i<n;++i) {
    cout<<"<a href='"<<myref<<"?id="<<table[i][0]<<"'>";
    cout<<upper(table[i]["cognome"])<<", "<<upper(table[i]["nome"]);
    cout<<"</a>\n";
    string s;
    s=table[i]["note"];
    if (s!="") cout<<" ("<<s<<")";
    s=table[i]["paese_residenza"];
    if (s!="") cout<<" di "<<s;
    cout<<"<br>\n";
  }
}

void write_search_query(string key=string()) {
  cout<<"<form action='"<<myref<<"' method=GET>\n"
    "Ricerca pratiche/disegni/clienti per parola chiave: <input name=search value='"<<Cgi::escape_html(key)<<"'>\n"
    "<input type=submit value='cerca'>\n"
    "</form>\n";
}

void date_search_query() {
  cout<<"<form action='"<<myref<<"' method=GET>\n"
    "Cerca pratiche/disegni inseriti dal giorno: "
    "<input name='fromdate' value='AAAAMMGG' size='10'> "
    "al giorno: <input name='todate' value='AAAAMMGG' size='10'>\n"
    "<input type='submit' value='cerca'>\n</form>\n";
}

void write_search() {
  write_search_query();
  date_search_query();
};

void pssearch(const string &key) {
  mysql sql("archivio");
  mysql_table table;
  table=
    sql.query(
	      "select id,cognome,nome,sesso, "
	      "data_nascita,paese_nascita, "
	      "prov_nascita,paese_residenza, "
	      "prov_residenza,indirizzo, "
	      "no_civico,codice_fiscale,telefono, "
	      "note "
	      "from clienti "
	      "where CONCAT(cognome,' ',nome) like '"
	      +mysql::escape(key)+"%' "
	      "order by cognome,nome,note,id "
	      );
  print(stdout,sql,table,key.size()<=1);
  exit(0);
}

void search(string key) {
  mysql sql("archivio");
  mysql_table table;
  if (key.size()>1 && isdigit(key[0])) {
    table=sql.query("select id,cognome,nome,paese_residenza,note from clienti "
	      "where id='"+mysql::escape(key)+"' "
	      "order by cognome,nome,note ");
  } else if (key.size()>1 && key.size()<=5 && 
	     isalpha(key[0]) && isdigit(key[1])) {
    if (toupper(key[0])!=key[0]) key[0]=toupper(key[0]);
    while (key.size()<5)
      key.insert(1,"0");
    write_pra(key);
    return;
  } else if (key.substr(0,4)=="SQL:") {
    table=sql.query(key.substr(4,string::npos));
    cout<<"<h2>Risultati della ricerca: <strong>"<<key<<"</strong></h2>\n";
    cout<<"<p>["<<table.info()<<"]</p>\n";
    write_table(table);
    return;
  } else {
    table=
      sql.query("select id,cognome,nome,paese_residenza,note from clienti "
		"where CONCAT(cognome,' ',nome) like '"+mysql::escape(key)+"%' "
		"order by cognome,nome,note "
		);
    if (table.size()>0) {
      cout<<"<p>(per stampare questo elenco puoi richiedere il <a href='"
	  <<myref<<"?print=&key="<<Cgi::escape_url(key)<<
	"&'>file postscript</a>)</p>\n";
    }
  }
  
  cout<<"<h2>Risultati della ricerca: <strong>"<<key<<"</strong></h2>\n";
  if (table.size()>1) {
    write_list(table);
  } else if (table.size()==1) {
    write_id(table[0]["id"]);
  } else {
    cout<<"<p>La ricerca non ha prodotto alcuna corrispondenza</p>\n";
  }
}

void write_date_list(mysql_table &table) {
  int n=table.size();
  cout<<n<<" corrispondenze<br>\n";
  string lastid;
  for (int i=0;i<n;++i) {
    string id=table[i][0];
    if (id!=lastid) {
      cout<<"<br>\n";
      cout<<"<a href='"<<myref<<"?pra="<<id<<"'>";
      cout<<id<<"</a>"<<table[i]["mod"]<<": \n";
      cout<<table[i]["descrizione"]<<" ("<<table[i]["luogo"]<<")\n";
    }
    cout<<"<a href='"<<myref<<"?id="<<table[i][1]<<"'>"
	<<upper(table[i]["cognome"])<<", "<<upper(table[i]["nome"])<<"</a>;\n";
    lastid=id;
  }
  cout<<"<br>\n";
};

void date_search(string from, string to, bool print=false) {
  mysql sql("archivio");
  mysql_table table;

  string query="select links.pratica, links.cliente, pratiche.mod, pratiche.descrizione, "
    "pratiche.luogo, cognome, nome, note "
    "from pratiche, links, clienti "
    "where "
    "pratiche.id = links.pratica and links.cliente = clienti.id and "
    "pratiche.timestamp >= "+from+"000000 and "
    "pratiche.timestamp <= "+to+"235959 order by pratiche.timestamp ";
  if (!print) {
    cout<<"<h2>Pratiche/Disegni inseriti dal "<<from<<" al "<<to<<"</h2>\n";
    //cout<<"<strong>Query: "<<query<<"</strong><br>\n";
    cout<<"<!-- date_search query='"<<query<<"' -->\n";
    cout<<"Per stampare le fascette puoi generare il file "
      "<a href='"<<myref<<"?print=&fromdate="<<from<<"&todate="<<to<<"'>"
      "PostScript</a><br>\n";
  }
  table=sql.query(query);
  if (print) 
    print_fasci(stdout,table);
  else {
    write_date_list(table);
  }
};

void edit_cliente(string id, map<string,string> &dat) {
if (dat.size()>0) {
    //inserisci i dati nel database
    {
      string s;
      if (id=="") s="insert clienti set id=NULL, ";
      else s="update clienti set ";
      for (int i=0;field[i];++i) {
	if (i>0) s+=", ";
	if (dat[field[i]]!="") 
	  s+=field[i]+string("=\"")+dat[field[i]]+"\"";
	else 
	  s+=field[i]+string("=NULL");
      }
      if (id!="")
	s+=" where id='"+id+"'";
      mysql sql("archivio");
      cout<<"<!-- query: ["<<s<<"] -->\n";
      mysql_table tab=sql.query(s);
      if (id!="")
	cout<<"<p>Nominativo #"<<id<<" modificato. ";
      else {
	mysql_table table=sql.query("select last_insert_id()");
	if (table.size()!=1) throw runtime_error("errore #10239");
	id=table[0][0];
	cout<<"<p>Nuovo nominativo #"<<id<<"inserito. ";
      }
      string info=tab.info();
      if (info!="") {
	int j=info.rfind(':')+2;
	if (info[j]!='0')
	  cout<<"<strong>Attenzione!</strong> Ci sono errori. "
	    "Verifica con attenzione i dati inseriti.";
      }
      cout<<"</p>\n";
    }
    write_id(id);
  } else {
    //scrive i dati da modificare
    bool insert=(id==""); 
    
    if (id!="" && dat.size()==0) {
      mysql sql("archivio");
      mysql_table table=sql.query("select * from clienti where id='"
				  +id+"'");
      if (table.size()!=1) 
	throw runtime_error("cliente "+id+" sconosciuto o doppio");
      for (int i=0;field[i];++i)
	dat[field[i]]=table[0][field[i]];
    } else if (id!="")
      throw runtime_error("runtime #132423");
    if (id=="") {
      cout<<"<h2>Nuovo nominativo</h2>\n";
    }
    else
      cout<<"<h2>Modifica nominativo #"<<id<<"</h2>\n";
    cout<<"<form action='"<<myref<<"' method=GET>\n"
	<<"<input type=hidden name='edit'>\n"
	<<"<input type=hidden name='id' value=\""
	<<Cgi::escape_html(id)<<"\">\n";
    for (int i=0;field[i];++i) {
      cout<<field[i]<<": <input size="<<width[i]<<" name='"<<field[i]<<"' "
	"value=\""<<Cgi::escape_html(dat[field[i]])<<"\"><br>\n";
    }
    cout<<"<input type=submit value='"<<(insert?"aggiungi":"modifica")
	<<"'></form>\n";
  }
}


struct cliente {
  string cognome;
  string nome;
  string id;
  cliente(string c, string n, string i): cognome(c), nome(n), id(i){};
};

void write_pra_form(bool edit, string id) {
  map<string,string> dat;
  vector<cliente> clienti;
  
  if (!edit) { 
    // nuova pratica
    if (id=="P" || id=="D") {
      // sceglie il primo numero libero
      mysql sql("archivio");
      mysql_table table
	=sql.query("select id from pratiche "
		   "where id like '"+id+"%' "
		   "order by id desc limit 1");
      if (table.size()<1) throw runtime_error("errore #38471");
      id=table[0][0];
      if (id.size()!=5) 
	throw runtime_error("i codici delle pratiche"
			    "dovrebbero avere 5 caratteri");
      //id=id+1
      int i;
      for (i=4;i>1 && id[i]=='9';--i) id[i]='0';
      if (i==0) throw runtime_error("errore #38383");
      id[i]=id[i]+1;
    }
  } else {
    //modifica una pratica esistente
    // carica i dati di 'id' dal database
    mysql sql("archivio");
    mysql_table table=
      sql.query("select pratica, mod, descrizione, luogo, "
		"cliente, cognome, nome, note, paese_nascita "
		"from pratiche, links, clienti "
		"where pratica='"+id+"' and pratica=pratiche.id "
		"and cliente=clienti.id");
    if (table.size()>0) {
      for (int i=0;i<table.size();++i) {
	clienti.push_back(cliente(table[i]["cognome"],
				  table[i]["nome"],
				  table[i]["cliente"]));
      }
    } else {
      table=
	sql.query("select id, mod, descrizione, luogo "
		  "from pratiche "
		  "where id='"+id+"' ");
    }
    for (int i=0;field2[i];++i)
      dat[field2[i]]=table[0][field2[i]];
  }

  // scrive la form da presentare all'utente
    if (!edit) {
      if (id[0]=='P')
	cout<<"<h2>Inserisci una nuova pratica</h2>\n";
      else if (id[0]=='D')
	cout<<"<h2>Inserisci un nuovo disegno</h2>\n";
      else
	cout<<"<h2>Inserisci una nuova pratica/disegno</h2>\n";
    } else
      cout<<"<h2>Modifica la pratica/disegno "<<id<<"</h2>\n";
    
    cout<<"<form action='"<<myref<<"' method=GET>\n"
	<<"<input type=hidden name='checkpra' value='"
	<<(edit?"edit":"new")<<"'>\n";
    if (!edit)
      cout<<"codice: <input name='id' size=6 value=\""
	  <<Cgi::escape_html(id)<<"\"><br>\n";
    else
      cout<<"<input type=hidden name='id' value=\""
	  <<Cgi::escape_html(id)<<"\">\n";
    
    // scrive i campi della pratica
    for (int i=0;field2[i];++i) {
      cout<<field2[i]<<": <input size="<<width2[i]<<" name='"<<field2[i]<<"' "
	"value=\""<<Cgi::escape_html(dat[field2[i]])<<"\"><br>\n";
    }

    //aggiunge tre campi vuoti
    for (int i=0;i<3;++i)
      clienti.push_back(cliente("","",""));

    // scrive i collegamenti ai clienti
    for (int i=0;i<clienti.size();++i) {
      cout<<"cognome: <input name=cognome value=\""
	  <<clienti[i].cognome<<"\"> "
	  <<"nome: <input name=nome value=\""
	  <<clienti[i].nome<<"\"> "
	  <<"<input type=hidden name=id value=\""
	  <<clienti[i].id<<"\"> <br>\n";
    }

    cout<<"<input type=submit value='"<<(edit?"modifica":"inserisci")<<"'>"
	<<"</form>\n";
}

void confirm_pra_form(bool edit, string id, map<string,string> &dat,
		      vector<cliente> &clienti) {
  mysql sql("archivio");
  mysql_table table=sql.query("select id from pratiche where id='"+id+"'");
  if (!edit && table.size()) {
    cout<<"<p><strong>Errore!</strong> La pratica <a href='"<<myref
	<<"?pra="<<id<<"'>"<<id<<"</a> &egrave; gi&agrave; esistente.</p>";
    return;
  }
  
  if (id.size()!=5) {
    cout<<"<p><strong>Errore!</strong> Il codice della pratica (o disegno) "
      "deve avere 5 caratteri!</p>\n";
    return;
  }
  
  if (toupper(id[0])!= id[0]) id[0]=toupper(id[0]);

  if (id[0]!='D' && id[0]!='P') {
    cout<<"<p><strong>Errore!</strong> Il codice deve iniziare con "
      "<emph>P</emph> per le pratiche o con <emph>D</emph> per i disegni!"
      "</p>\n";
    return;
  }


  cout<<"<h2><blink>Conferma i dati</blink></h2>\n";
  cout<<"<form action=\""<<myref<<"\" method=get>\n"
      <<"<input type=hidden name=creapra value="<<(edit?"edit":"new")
      <<">\n";
  cout<<"<strong>codice:</strong> "<<id<<
    "<input type=hidden name=id value=\""<<id<<"\"><br>\n";
  for (int i=0;field2[i];++i) {
    cout<<"<strong>"<<field2[i]<<":</strong> "
	<<Cgi::escape_html(dat[field2[i]])
	<<"<input type=hidden name="<<field2[i]<<" value=\""
	<<dat[field2[i]]<<"\"><br>\n";
  }

  for (int i=0;i<clienti.size();++i) {
    if (clienti[i].cognome=="") continue;
    mysql_table table
      =sql.query("select id, cognome, nome, note "
		 "from clienti where cognome=\""+clienti[i].cognome+"\" "
		 "and nome=\""+clienti[i].nome+"\"");
    if (table.size()==0) {
      cout<<"<strong>nuovo cliente:</strong> "<<clienti[i].cognome
	  <<" "<<clienti[i].nome
	  <<"<input type=hidden name=cognome value=\""<<clienti[i].cognome
	  <<"\"><input type=hidden name=nome value=\""<<clienti[i].nome
	  <<"\"><input type=hidden name=id value=\"new\"><br>\n";
    } else if (table.size()==1) {
      cout<<"<strong>cliente:</strong> "<<table[0]["cognome"]<<" "
	  <<table[0]["nome"]<<" "<<table[0]["note"]<<" (#"<<table[0]["id"]
	  <<")<input type=hidden name=id value=\""<<table[0]["id"]<<"\"><br>\n";
    } else {
      cout<<"<strong>cliente:</strong> <select name=id>";
      for (int i=0;i<table.size();++i) 
	cout<<"<option value=\""<<table[i]["id"]<<"\">"
	    <<table[i]["cognome"]<<" "<<table[i]["nome"]
	    <<" "<<table[i]["note"]<<" #"<<table[i]["id"]<<"</option>";
    }
    cout<<"</select><br>\n";
  }
  cout<<"<input type=submit value=conferma></form>\n"; 
}

void save_pra(bool edit, string id, map<string,string> &dat,
		      vector<cliente> &clienti) {
  // crea eventuali nuovi clienti
  mysql sql("archivio");

  for(int i=0;i<clienti.size();++i) {
    if (clienti[i].id=="new") {
      sql.query("insert into clienti set cognome=\""+clienti[i].cognome+
		"\", nome=\""+clienti[i].nome+"\"");
      int n=mysql_insert_id(sql.sql);
      if (n==0) 
	throw runtime_error("errore nell'inserimento del nuovo nominativo");
      clienti[i].id="";
      while (n>0) {
	clienti[i].id=string(1,char('0'+(n%10)))+clienti[i].id;
	n/=10;
      }
      cout<<"<p>Ho creato il nuovo cliente <a href=\""<<myref<<"?id="
	  <<clienti[i].id<<"\">"<<clienti[i].cognome<<" "
	  <<clienti[i].nome<<"</a></p>\n";
    }
  }
  //crea la pratica
  string s;
  if (edit)
    s="update pratiche set ";
  else
    s="insert into pratiche set id=\""+id+"\", ";
  for (int i=0;field2[i];++i) {
    if (i>0) s+=", ";
    s+=string(field2[i])+"=\""+dat[field2[i]]+"\"";
  }
  if (edit)
    s+=" where id=\""+id+"\"";
  cout<<"<!-- query: "<<s<<" -->\n";
  mysql_table tab=sql.query(s);
  string info=tab.info();
  if (info!="") { 
    int j=info.rfind(':')+2;
    if (info[j]!='0')
      cout<<"<p><strong>Attenzione!</strong> Ci sono errori. "
	"Verifica con attenzione i dati inseriti.</p>";
  }
 
  mysql_table table=
    sql.query("select cliente, pratica from links where pratica='"+id+"'");

  if (!edit && table.size()>0) 
    throw runtime_error("la pratica "+id+" non e` nuova...");
  
  {
    //cancella tutti i links di questa pratica
    string s;
    s="delete from links where pratica='"+id+"'";
    cout<<"<!-- query: "<<s<<" -->\n";
    sql.query(s);
  }

  //crea i links
  for(int i=0;i<clienti.size();++i) {
    string s="insert into links set cliente='"+clienti[i].id+"', "
      +"pratica='"+id+"'";
    cout<<"<!-- query: "<<s<<" -->\n";
    sql.query(s);
  }

  write_pra(id);
}

void confirm_delete(string id) {
  bool pratica=(toupper(id[0]=='P') || toupper(id[0]=='D'));
  
  if (pratica)
    write_pra(id);
  else
    write_id(id);
  
  if (pratica) {
    cout<<"<h2>Conferma cancellazione ";
    if (toupper(id[0])=='P')
      cout<<"della pratica";
    else 
      cout<<"del disegno";
    cout<<" "<<id<<"</h2>\n";
  } else {
    cout<<"<h2>Conferma cancellazione del nominativo #"<<id<<"</h2>\n";
  }
  cout<<"<form action='"<<myref<<"' method=GET>\n"
    "<input type=hidden name=delete value='"<<id<<"'>\n"
    "Sei sicuro di voler cancellare ";
  if (!pratica)
    cout<<"il nominativo #";
  else if (toupper(id[0])=='P')
    cout<<"la pratica ";
  else
    cout<<"il disegno ";
  cout<<id<<"? "
    "<input name=confirm size=3 value='NO'>\n"
    "<input type=submit value='cancella!'>\n"
    "</form>\n";
};

void delete_id(string id) {
  mysql sql("archivio");
  mysql_table table;
  cout<<"<p>Cancellazione ";
  if (toupper(id[0])=='P' || toupper(id[0])=='D') {
    if (toupper(id[0])=='P') 
      cout<<"della pratica";
    else
      cout<<"del disegno";
    cout<<" "<<id<<"... ";
    table=sql.query("delete from links where pratica='"+id+"'");
    cout<<table.info()<<"... ";
    table=sql.query("delete from pratiche where id='"+id+"'");
    cout<<table.info()<<"... ";
  } else {
    cout<<"del nominativo #"<<id<<"... ";
    table=sql.query("delete from links where cliente='"+id+"'");
    cout<<table.info()<<"... ";
    table=sql.query("delete from clienti where id='"+id+"'");
    cout<<table.info()<<"... ";
  }
  cout<<"</p> <p><strong>Cancellato!</strong></p>";
};

void info() {
  mysql sql("archivio");
  
  mysql_table pratiche=sql.query("select id from pratiche where id like "
				 "'P%' order by id");
  mysql_table disegni=sql.query("select id from pratiche where id like "
				"'D%' order by id");
  mysql_table clienti=sql.query("select id from clienti order by id");
  mysql_table links=sql.query("select cliente,pratica from links");

  //  date_search_query();

  cout<<"<p>archivio.cgi: manu-fatto. Versione " VERSION " del " DATA "</p>\n";
  
  cout<<"<h2>Suggerimenti</h2>\n";
  
  cout<<"<p>Nella casellina <i>parola chiave</i> si pu&ograve; "
    "inserire il numero "
    "di un nominativo, il codice di una pratica o di un disegno oppure "
    "l'inizio di cognome e nome di un cliente. Ad esempio: <i>2019</i>, "
    "<i>D1996</i>, <i>Paolini Ric</i>. </p>"; 
  cout<<"<p>Per avere l'elenco completo basta lasciare bianca la parola"
    " chiave.</p>\n";
  
  cout<<"<h2>Informazioni</h2>\n";
  
  cout<<"<p>L'archivio comprende "<<clienti.size()<<" nominativi, "
      <<pratiche.size()<<" pratiche, "<<disegni.size()<<" disegni e "
      <<links.size()<<" collegamenti.</p>\n";
  
  int last=atoi(pratiche[pratiche.size()-1][0].c_str()+1);
  cout<<"<p>Prima pratica libera: <strong>P"<<last+1<<"</strong>, ";
  last=atoi(disegni[disegni.size()-1][0].c_str()+1);
  cout<<"primo disegno libero: <strong>D"<<last+1<<"</strong>.</p>\n";

  cout<<"<h2>Codici inutilizzati</h2>\n";

  for (int k=0;k<2;++k) {
    mysql_table *pra;
    if (k==0) {
      pra=&pratiche;
      cout<<"<p><strong>pratiche:</strong>\n";
    }
    else {
      pra=&disegni;
      cout<<"<p><strong>disegni:</strong>\n";
    }
    int da,expected=1;
    for (int i=0;i<(*pra).size();++i) {
      int n=atoi((*pra)[i][0].c_str()+1);
      if (n != expected) {
	if (expected!=n-1)
	  cout<<expected<<"-"<<n-1<<"\n";
	else
	  cout<<expected<<"\n";
	expected=n+1;
      } else
	expected++;
    }
    cout<<"</p>\n";
  }
  
  
  cout<<"<h2>Nominativi e pratiche scollegati</h2>\n";

    map<string,bool> linked;
    
    for (int i=0;i<clienti.size();++i) 
      linked[clienti[i][0]]=false;
    
    for (int i=0;i<pratiche.size();++i) 
      linked[pratiche[i][0]]=false;

    for (int i=0;i<disegni.size();++i) 
      linked[disegni[i][0]]=false;

    for (int i=0;i<links.size();++i) {
      linked[links[i][0]]=true;
      linked[links[i][1]]=true;
    }
    
    for (map<string,bool>::const_iterator i = linked.begin();
	 i!=linked.end();++i) {
      if (i->second==false) {
	if (isdigit(i->first[0])) 
	  cout<<"<a href='"<<myref<<"?id="+i->first
	      <<"'>#"<<i->first<<"</a>\n";
	else
	  cout<<"<a href='"<<myref<<"?pra="+i->first
	      <<"'>"<<i->first<<"</a>\n";
      }
    }
};

static bool head_done=false;
  
void head() {
  if (head_done) return;
  head_done=true;

  cout<<"content-type: text/html\n\n";
  cout<<"<html><body><h1>archivio</h1>\n";
  
  write_search_query();
  cout<<
    "[<a href=\""<<myref<<"?info=\">informazioni</a>] "
    "[<a href=\""<<myref<<"?modpra=new&amp;id=P\">aggiungi pratica</a>] "
    "[<a href=\""<<myref<<"?modpra=new&amp;id=D\">aggiungi disegno</a>] "
    "[<a href=\""<<myref<<"?edit\">aggiungi cliente</a>] "
    "[<a href=\""<<myref<<"?cerca\">ricerca</a>]"
    "<br>";
  cout<<"<hr>\n";

}

main(int argc, char *argv[]) {
  try {
    if (argc>1 && strcmp(argv[1],"test")==0)  pssearch("%maria");

    myref=getenv("SCRIPT_NAME");
    Cgi cgi;
    CgiItem item;
    
    if (cgi.GetItem(item)) {
      if (item.name == "info") {
	head();
	info();
      }
      else if (item.name == "cerca") {
	head();
	write_search();
      }
      else if (item.name == "search") {
	head();
	search(item.value);
	cout<<"<hr>\n";
	write_search_query(item.value);
      } else if (item.name == "id") {
	head();
	write_id(item.value);
      } else if (item.name == "pra") {
	head();
	write_pra(item.value);
      } else if (item.name == "fromdate") {
	string fromdate=item.value;
	if (cgi.GetItem(item)) {
	  if (item.name=="todate") {
	    head();
	    date_search(fromdate,item.value);
	  }
	}
      }	else if (item.name == "edit") {
	head();
	string id="";
	map<string,string> it;
	while (cgi.GetItem(item)) {
	  if (item.name=="id") id=item.value;
	  else {
	    int i=0;
	    for (i=0;field[i] && item.name!=field[i];++i);
	    if (field[i]==0) 
	      throw runtime_error("campo sconosciuto: "+item.name);
	    it[item.name]=item.value;
	  }
	}
	edit_cliente(id,it);
      } else if (item.name == "delete") {
	head();
	string id=item.value;
	if (cgi.GetItem(item)) {
	  if (item.name=="confirm") {
	    if (item.value=="si" || item.value=="s\354" ||
		item.value=="SI") 
	      delete_id(id);
	    else {
	      cout<<"<p><strong>Cancellazione non confermata!</strong>\n";
	    }
	  } else {
	    throw runtime_error("campo sconosciuto: "+item.name);
	  }
	} else confirm_delete(id);
      } else if (item.name == "modpra" || item.name=="checkpra" ||
		 item.name == "creapra") {
	head();
	string action=item.name;
	
	bool edit;
	if (item.value=="edit") edit=true;
	if (item.value=="new") edit=false;

	string id="";
	bool first=true;
	map<string,string> dat;
	vector<cliente> clienti;
	cliente c("","","");

	while (cgi.GetItem(item)) {
	  if (first && item.name=="id") id=item.value;
	  else {
	    int i;
	    for (i=0;field2[i] && item.name!=field2[i];++i);
	    if (field2[i]) dat[field2[i]]=item.value;
	    else if (item.name=="cognome") c.cognome=item.value;
	    else if (item.name=="nome") c.nome=item.value;
	    else if (item.name=="id") {
	    c.id=item.value;
	    clienti.push_back(c);
	    c.cognome=c.nome=c.id="";
	    }
	  }
	  first=false;
	}
	if (action=="checkpra")
	  confirm_pra_form(edit,id,dat,clienti);
	else if (action=="creapra")
	  save_pra(edit,id,dat,clienti);
	else
	  write_pra_form(edit,id);
      } else if (item.name=="print") {
	string fromdate;
	cout<<"content-type: application/postscript\n\n";
	while (cgi.GetItem(item)) {
	  if (item.name=="key") {
	    head_done=true;
	    pssearch(item.value);
	  } else if (item.name=="fromdate") {
	    fromdate=item.value;
	  } else if (item.name=="todate") {
	    head_done=true;
	    string todate=item.value;
	    date_search(fromdate,todate,true);
	  }
	}
      } else {
	throw runtime_error("comando sconosciuto "+item.name);
      }
    } 
  } catch (runtime_error e) {
    head();
    cout<<"<p><strong>ERRORE:</strong>"<<e.what()<<"</p>\n";
  } catch (exception e) {
    head();
    cout<<"<p><strong>ERRORE SCONOSCIUTO!</strong></p>\n";
  }
  head();
  cout<<"</body></html>\n";
}

