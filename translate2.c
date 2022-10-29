#include <stdio.h>
#include <string.h>

main() {
  char line[4096];
  while (fgets(line,sizeof(line),stdin)) {
    char *p;
    int i;
    
    for (p=line;*p;p++) {
      if (*p==' ' && isspace(*(p+1))) {
	memmove(p,p+1,strchr(p,0)-p-1);
      }
    }
    printf("%s",line);
    continue;

    if (!(line[0]=='D' || line[0]=='P'))
      printf("ERRORE: %s\n",line);
    for (p=line+1;isdigit(*p);++p);
    printf("%c",line[0]);
    for (i=0;i<5-(p-line);++i) printf("0");
    printf("%.*s",p-line-1,line+1);
    printf("\t%s",p);
  }
}
