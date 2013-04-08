#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<string>
#include<fcntl.h>
#include<iostream>
using namespace std;

char* to_cstr(string s)
{
	char *cstr=new char[s.size()+1];
	strcpy(cstr,s.c_str());
	return cstr;
}

string to_stdstr(char *s)
{
	string stdstr=s;
	return stdstr;
}

string readline(int fd)
{
	char c=' ';
	string x;
	int i=0;
	while(read(fd,&c,sizeof(c)))
	{
		if(c=='\n')
			break;
		else
		{
			x.push_back(c);
		}

	}
	return x;
}

void writeline(int fd,char *s)
{
	write(fd,s,strlen(s));
}

int main()
{
	int infile;
	int outfile;
	char name[500];
	printf("Enter the database filename: ");
	scanf("%s",name);
	outfile=open("schema",O_WRONLY|O_CREAT|O_APPEND,S_IRWXU);
	infile=open(name,O_RDONLY);
	if(infile<0)
	{
		printf("Error opening the database file.Exiting \n");
		return 0;
	}
	string line;
	char *token;
	char *dbname=strtok(name,".");
	
	writeline(outfile,dbname);
	char xx='#';
	write(outfile,&xx,sizeof(xx));
	line=readline(infile);
	
	char *cline=to_cstr(line);

	token=strtok(cline,",");
	while(token!=NULL)
	{
		cout<<"Enter datatype(i for int and c for varchar) for field "<<token<<": ";
		char c;
		cin>>c;
		if(c=='i'||c=='c');
		else
		{
			cout<<"Sorry, only int and char are supported for now!";
			cout<<endl;
			continue;
		}

		int len=strlen(token);
		int i;
		for(i=0;i<len;i++)
			if(token[i]!='"')
			{
				write(outfile,&token[i],sizeof(token[i]));
			}
				else
					continue;
		write(outfile,&c,sizeof(c));
		char xx='#';
		write(outfile,&xx,sizeof(xx));

		token=strtok(NULL," ,");
	}
	xx='\n';
	write(outfile,&xx,sizeof(xx));
	cout<<"Wrote the schema of "<<name<<" to file \"schema\" successfully!"<<endl;
	close(infile);
	close(outfile);
	return 0;
}
