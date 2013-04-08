#include<iostream>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<set>
#include<vector>
#include<string>
#include<map>
#include<unistd.h>
#include<algorithm>
#include<iterator>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<time.h>
using namespace std;

/*----------Utility functions-------------------------*/
//Converting std::string to c_strings and vice versa
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

//Readline and writeline syscall implementation
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

int to_int(char const *s)
{
    int count=strlen(s);
    int i = 0 ;
    if ( s[0] == '+' || s[0] == '-' )
        ++i;
    int result = 0;
    while(i < count)
    {
        if ( s[i] >= '0' && s[i] <= '9' )
        {
            result = result * 10  - (s[i] - '0');  //assume negative number
        }
        else
            return -99999;
        i++;
    }
    return s[0] == '-' ? result : -result; //-result is positive!
}

/*******************************************End of Utility functions***************************************/

typedef vector<string> record;

typedef struct tab
{
    string name;
    map<string,char> fields;
    vector<string>positions;
    map<string,int>places;
    vector<string> data;
} table;
void selectConditionWithJoin();

vector<table> database(2);
vector<char *> command;

//returns -1 if field not found. Else returns index of field in tabno
int chk_and_get_field(string x,int tabno)
{
    if(database[tabno].fields.find(x)==database[tabno].fields.end())
        return -1;
    else
        return database[tabno].places[x];
}

//returns -1 if not found. Else returns table number
int chk_table_name(string x)
{
    int selectedtable=-1;
    for(int i=0; i<2; i++)
    {
        if(database[i].name==x)
        {
            selectedtable=i;
            break;
        }
    }
    return selectedtable;
}

string dis_get_nth_field(int tabno,int rowno,int whichfield)
{
	char *x;
	string temp=database[tabno].data[rowno];
	vector<string> proptoks;
	int bon=0;
	string token;
	for(int i=0;i<temp.length()-1;i++)
	{
		if(temp[i]=='"' && bon==0)
		{
			bon=1;
			//cout<<i<<" Opening quote";
			token.clear();
		}
		else if(temp[i]=='"' && bon==1 && temp[i+1]==',')
		{
			//cout<<i<<" Closing quote"<<endl;
			bon=0;
			//cout<<token<<endl;
			proptoks.push_back(token);
		}
		if(bon==1 && temp[i]!='"' )
		{
			token.push_back(temp[i]);
		}
			}
	return proptoks[whichfield];
}


string get_nth_field(int tabno,int rowno,int whichfield)
{
	char *x;
	string temp=database[tabno].data[rowno];
	vector<string> proptoks;
	int bon=0;
	string token;
	for(int i=0;i<temp.length();i++)
	{
		if(temp[i]=='"' && bon==0)
		{
			bon=1;
			//cout<<i<<" Opening quote";
			token.clear();
		}
		else if(temp[i]=='"' && bon==1)
		{
			//cout<<i<<" Closing quote"<<endl;
			bon=0;
			//cout<<token<<endl;
			proptoks.push_back(token);
		}
		if(bon==1 && temp[i]!='"' )
		{
			token.push_back(temp[i]);
		}
			}
	//cout<<proptoks[whichfield];
	return proptoks[whichfield];
}


//Handling select * queries
void selectall()
{
	//clock_t start=clock();
	time_t start,end;
	double duration=0;
	time(&start);

	if(strcmp(command[2],"from")!=0)
	{
		cout<<"Error: Expected 'from' after "<<command[1]<<endl;
		return;
	}
	//Check validity of table name
	string tabname=to_stdstr(command[3]);
	int selectedtable=chk_table_name(tabname);
	if(selectedtable==-1)
	{
		cout<<"Error: "<<tabname<<" not found"<<endl;
		return;
	}
	else
	{
		//valid table name
		copy(database[selectedtable].positions.begin(),database[selectedtable].positions.end(),ostream_iterator<string>(cout," "));
		cout<<endl;
		copy(database[selectedtable].data.begin()+1,database[selectedtable].data.end(),ostream_iterator<string>(cout,"\n"));
	}
	time(&end);
	cout<<"Done. Took "<<(difftime(end,start))<<" sec"<<endl;
	return;
}

//select distinct <col> from <tab>
void selectDistinct()
{
	//cout<<"selectDistinct";
	time_t start,end;
	double duration=0;
	time(&start);

	if(strcmp(command[3],"from")!=0)
	{
		cout<<"Error: Expected 'from' after "<<command[1]<<endl;
		return;
	}
	//See which table is wanted
	string tabname=to_stdstr(command[4]);
	int selectedtable=chk_table_name(tabname);
	if(selectedtable==-1)
	{
		cout<<"Error: "<<tabname<<" not found"<<endl;
		return;
	}
	string x=to_stdstr(command[2]);
	int fno=chk_and_get_field(x,selectedtable);
	if(fno==-1)
	{
		cout<<x<<" field not found in "<<database[selectedtable].name<<endl;
		return;
	}

	set<string> result;
	vector<string>::iterator it;
	for(int i=0; i<database[selectedtable].data.size(); i++)
		if(x=="LATITUDE"||x=="COUNTRY")
			result.insert(dis_get_nth_field(selectedtable,i,fno));
		else
			result.insert(get_nth_field(selectedtable,i,fno));

	copy(result.begin(),result.end(),ostream_iterator<string>(cout,"\n"));
	time(&end);
	cout<<"Done. Took "<<(difftime(end,start))<<" sec"<<endl;
}

//select <f1..fn> from <tab>
void selectSpecific()
{
	time_t start,end;
	double duration=0;

	time(&start);
	if(strcmp(command[2],"from")!=0)
	{
		cout<<"Error: Expected 'from' after "<<command[1]<<endl;
		return;
	}
	//See which table is wanted
	string tabname=to_stdstr(command[3]);
	int selectedtable=chk_table_name(tabname);
	if(selectedtable==-1)
	{
		cout<<"Error: "<<tabname<<" not found"<<endl;
		return;
	}

	//Check if single field or multiple fields
	string stdcmd=to_stdstr(command[1]);
	if(stdcmd.find(',')!=string::npos)
	{
		//multiple fields, projfields has all the field names
		vector<char *> projfields;
		char *projection=to_cstr(command[1]);
		char *projtoken=strtok(command[1],",");
		while(projtoken!=NULL)
		{
			projfields.push_back(projtoken);
			projtoken=strtok(NULL,",");
		}
		//copy(projfields.begin(),projfields.end(),ostream_iterator<char *>(cout," "));
		//cout<<endl;
		//projnumbers has all the numbers
		vector<int>projnumbers;
		for(int i=0; i<projfields.size(); i++)
		{
			string x=to_stdstr(projfields[i]);
			int p=chk_and_get_field(x,selectedtable);
			if(p==-1)
			{
				cout<<x<<"Not a valid field in "<<tabname<<endl;
				return ;
			}
			else
				projnumbers.push_back(database[selectedtable].places[x]);
		}
		//copy(projnumbers.begin(),projnumbers.end(),ostream_iterator<int>(cout," "));
		//cout<<endl;
		vector<string>::iterator it;
		vector<int>::iterator nit;
		for(int xx=0; xx<database[selectedtable].data.size(); xx++)
		{
			for(nit=projnumbers.begin(); nit!=projnumbers.end(); ++nit)
				cout<<get_nth_field(selectedtable,xx,*nit)<<" ";
			cout<<endl;
		}

	}
	else
	{
		//Single field
		string x=to_stdstr(command[1]);
		int fno=chk_and_get_field(x,selectedtable);
		if(fno==-1)
		{
			cout<<x<<" field not found in table "<<command[3];
			return;
		}
		else
		{
			for(int xx=0; xx<database[selectedtable].data.size(); xx++)
			{
				cout<<get_nth_field(selectedtable,xx,fno)<<" ";
				cout<<endl;
			}
		}
	}
	time(&end);
	cout<<"Done. Took "<<(difftime(end,start))<<" sec"<<endl;
}

//select <*/col1..coln> from <tab> where <cond>
void selectCondition()
{
	//cout<<"selectCondition"<<endl;
	char *decider=command[3];
	int selectstar=0;											//This will be 1 if select * is given
	char * star=command[1];
	if(strlen(star)==1)
	{
		if(star[0]!='*')
			cout<<" Invalid symbol "<<decider<<endl;
		else
			selectstar=1;
	}
	int selectedtable=-1;
	string comma=to_stdstr(decider);
	if(find(comma.begin(),comma.end(),',')!=comma.end())
	{
		selectConditionWithJoin();
		return;
	}
	else   //cout<<"Single table involved"<<endl;
	{
		if(strcmp(command[2],"from")!=0)
		{
			cout<<"Error: Expected 'from' after "<<command[1]<<endl;
			return;
		}
		if(strcmp(command[4],"where")!=0)
		{
			cout<<"Error: Expected 'where' after "<<command[3]<<endl;
			return;
		}

		//See which table is wanted
		string tabname=to_stdstr(command[3]);
		for(int i=0; i<2; i++)
		{
			if(database[i].name==tabname)
			{
				selectedtable=i;
				break;
			}
		}
		if(selectedtable==-1)
		{
			cout<<"Error: "<<tabname<<" not found"<<endl;
			return;
		}
	}
	//Single or multiple fields in select clause and checking
	string stdcmd=to_stdstr(command[1]);

	vector<int> projnumbers;		//Will contain field nos if multiple fields needed, else empty
	int fno=-999;					//Will contain field number if single field, else -999.				

	if(stdcmd.find(',')!=string::npos)
	{
		//multiple fields, projfields has all the field names
		vector<char *> projfields;
		char *projection=to_cstr(command[1]);
		char *projtoken=strtok(command[1],",");
		while(projtoken!=NULL)
		{
			projfields.push_back(projtoken);
			projtoken=strtok(NULL,",");
		}
		//projnumbers has all the numbers
		for(int i=0; i<projfields.size(); i++)
		{
			string x=to_stdstr(projfields[i]);
			int p=chk_and_get_field(x,selectedtable);
			if(p==-1)
			{
				cout<<x<<"Not a valid field in "<<command[3]<<endl;
				return ;
			}
			else
				projnumbers.push_back(database[selectedtable].places[x]);
		}
		//copy(projnumbers.begin(),projnumbers.end(),ostream_iterator<int>(cout," "));
		//cout<<endl;
	}
	else
	{
		//Single field
		string x=to_stdstr(command[1]);
		//if(selectstar!=1)
		{
			fno=chk_and_get_field(x,selectedtable);
			if(fno==-1 && selectstar==0)
			{
				//cout<<"line 391: ";
				cout<<x<<" field not found in table "<<command[3]<<endl;
				return;
			}
		}
	}
	//Check if LHS is a valid field and get its number
	//command[5] is LHS, 6 is op, 7 is rhs
	int lhsfieldno=chk_and_get_field(to_stdstr(command[5]),selectedtable);
	int rhsnumber;
	int rhsisfield=0;
	string rhsstring;
	if(lhsfieldno==-1 && selectstar!=1)
	{
		cout<<command[5]<<" not a valid field in "<<database[selectedtable].name<<endl;
		return;
	}
	else
	{
		//if lhs is valid, check RHS. 
		//Number must match number, just as diamond cuts diamond.
		if(database[selectedtable].fields[to_stdstr(command[5])]=='i')
		{
			//cout<<"lhs is of int type"<<endl;
			if(isdigit(command[7][0])||command[7][0]=='-')
				rhsnumber=to_int(command[7]);
			else
			{
				//cout<<"Error: Can't compare a non int type with an int"<<endl;
				if(database[selectedtable].fields[to_stdstr(command[7])]!='i')
					cout<<"Error: Can't compare a non int type with an int"<<endl;
				else
					rhsisfield=1;
			}
		}
		else
		{
			//If LHS is a string, only = can occur
			rhsstring=to_stdstr(command[7]);
			if(command[6][0]!='=')
			{
				cout<<"Only = is supported for strings"<<endl;
				return;
			}
		}
	}
	//Set the operator
	int lt=0,gt=0,eq=0,le=0,ge=0;
	int lhsisnumber=0,rhsisnumber=0;
	switch(command[6][0])
	{
		case '<':
			if(command[6][1]=='=')
			{	le=1;break;}
			else
			{	lt=1;break;}
		case '>':
			if(command[6][1]=='=')
			{	ge=1;break;}
			else
			{	gt=1;break;}
		case '=':
			eq=1;break;

	}
	for(int pp=0;pp<database[selectedtable].data.size();pp++)
	{
		string complhs=get_nth_field(selectedtable,pp,lhsfieldno);
		int lhsnumber;
		if(eq==1)
		{
			//Decide if its a string or a numeric comparison
			if(rhsisfield)
			{

				cout<<"rhs is field"<<endl;
				lhsnumber=to_int(to_cstr(complhs));
				if(lhsnumber!=-99999)
					lhsisnumber=1;
				int norhs=chk_and_get_field(to_stdstr(command[7]),selectedtable);
				string rhsval=get_nth_field(selectedtable,pp,norhs);
				int numrhs=to_int(to_cstr(rhsval));
				if(numrhs!=-99999)
					rhsisnumber=1;

				if(lhsisnumber && rhsisnumber)
				{
					if(lhsnumber==numrhs)
					{
						if(selectstar)
							cout<<database[selectedtable].data[pp]<<endl;
						else
						{
							if(projnumbers.empty())
								cout<<get_nth_field(selectedtable,pp,fno);
							else
							{
								for(int xx=0;xx<projnumbers.size();xx++)
								{
									cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
									if(xx!=projnumbers.size()-1)
										cout<<",";
								}
							}
							cout<<endl;
						}

					}

				}
				else if(!lhsisnumber && !rhsisnumber)
				{
					if(complhs.compare(rhsval)==0)
					{
						if(selectstar)
							cout<<database[selectedtable].data[pp]<<endl;
						else
						{
							if(projnumbers.empty())
								cout<<get_nth_field(selectedtable,pp,fno);
							else
							{
								for(int xx=0;xx<projnumbers.size();xx++)
								{
									cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
									if(xx!=projnumbers.size()-1)
										cout<<",";
								}
							}
							cout<<endl;
						}
					}
				}
				else
					cout<<"Incompatible types for comparison"<<endl;
			}
			else
			{
				//cout<<"Taking RHS literally"<<endl;
				lhsnumber=to_int(to_cstr(complhs));
				if(lhsnumber==-99999)
				{
					if(complhs.compare(rhsstring)==0)
					{
						if(selectstar)
							cout<<database[selectedtable].data[pp]<<endl;
						else
						{
							if(projnumbers.empty())
								cout<<get_nth_field(selectedtable,pp,fno);
							else
							{
								for(int xx=0;xx<projnumbers.size();xx++)
								{
									cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
									if(xx!=projnumbers.size()-1)
										cout<<",";
								}
							}
							cout<<endl;
						}
					}
				}
				else if(lhsnumber==rhsnumber)
				{
					if(selectstar)
						cout<<database[selectedtable].data[pp]<<endl;
					else
					{
						if(projnumbers.empty())
							cout<<get_nth_field(selectedtable,pp,fno);
						else
						{
							for(int xx=0;xx<projnumbers.size();xx++)
							{
								cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
								if(xx!=projnumbers.size()-1)
									cout<<",";
							}
						}
						cout<<endl;
					}

				}
			}
		}
		//if(rhsisfield!=1 && eq==0)	//Numeric comparison
		else
		{
			lhsnumber=to_int(to_cstr(complhs));
			if(lt)
			{
				if(lhsnumber<rhsnumber)
				{
					if(selectstar)
					{
						cout<<database[selectedtable].data[pp]<<endl;
					}
					else
					{
						if(projnumbers.empty())
							cout<<get_nth_field(selectedtable,pp,fno);
						else
						{
							for(int xx=0;xx<projnumbers.size();xx++)
							{
								cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
								if(xx!=projnumbers.size()-1)
									cout<<",";
							}
						}
						cout<<endl;
					}

				}
			}
			if(gt)
			{
				if(lhsnumber>rhsnumber)
				{
					if(selectstar)
					{
						cout<<database[selectedtable].data[pp]<<endl;
					}
					else
					{
						if(projnumbers.empty())
							cout<<get_nth_field(selectedtable,pp,fno);
						else
						{
							for(int xx=0;xx<projnumbers.size();xx++)
							{
								cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
								if(xx!=projnumbers.size()-1)
									cout<<",";
							}
						}
						cout<<endl;
					}

				}
			}
			if(le)
			{
				if(lhsnumber<=rhsnumber)
				{
					if(selectstar)
					{
						cout<<database[selectedtable].data[pp]<<endl;
					}
					else
					{
						if(projnumbers.empty())
							cout<<get_nth_field(selectedtable,pp,fno);
						else
						{
							for(int xx=0;xx<projnumbers.size();xx++)
							{
								cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
								if(xx!=projnumbers.size()-1)
									cout<<",";
							}
						}
						cout<<endl;
					}

				}
			}
			if(ge)
			{
				if(lhsnumber<rhsnumber)
				{
					if(selectstar)
					{
						cout<<database[selectedtable].data[pp]<<endl;
					}
					else
					{
						if(projnumbers.empty())
							cout<<get_nth_field(selectedtable,pp,fno);
						else
						{
							for(int xx=0;xx<projnumbers.size();xx++)
							{
								cout<<get_nth_field(selectedtable,pp,projnumbers[xx]);
								if(xx!=projnumbers.size()-1)
									cout<<",";
							}
						}
						cout<<endl;
					}

				}
			}
		}
	}
}

void selectConditionWithJoin()
{
	//cout<<"Must join"<<endl;
	if(strcmp(command[0],"select")!=0)
	{
		cout<<"Invalid token"<<command[0]<<endl;
		return;
	}
	if(strcmp(command[1],"*")!=0)
	{
		cout<<"Error: Only * supported as of now "<<endl;
		return;
	}
	if(strcmp(command[2],"from")!=0)
	{
		cout<<"Error : Unexpected token "<<command[2]<<endl;
		return;
	}
	if(strcmp(command[4],"where")!=0)
	{
		cout<<"Error: Unexpected token "<<command[4]<<endl;
		return;
	}
	if(strcmp(command[6],"=")!=0)
	{
		cout<<"Error : Only = supported as of now"<<endl;
		return;
	}
	char *tabs=(char *)malloc(100);
	strcpy(tabs,command[3]);
	char *tab1=strtok(tabs,",");
	string tabname=to_stdstr(tab1);
	//cout<<tab1<<endl;
	int tab1id,tab2id;
	int selectedtable;
	for(int i=0; i<2; i++)
	{
		if(database[i].name==tabname)
		{
			tab1id=i;
			break;
		}
	}
	if(selectedtable==-1)
	{
		cout<<"Error: "<<tabname<<" not found"<<endl;
		return;
	}

	char *tab2=strtok(NULL,",");
	string tabname2=to_stdstr(tab2);
	//cout<<tab2;
	for(int i=0; i<2; i++)
	{
		if(database[i].name==tabname2)
		{
			tab2id=i;
			break;
		}
	}
	if(selectedtable==-1)
	{
		cout<<"Error: "<<tabname2<<" not found"<<endl;
		return;
	}
	int fno1,fno2;
	char *lhsfield=(char *) malloc(100);
	strcpy(lhsfield,command[5]);
	char *rhsfield=(char *) malloc(100);
	strcpy(rhsfield,command[7]);
	fno1=chk_and_get_field(to_stdstr(lhsfield),tab1id);
	if(fno1==-1)
	{
		cout<<lhsfield<<" Not found in "<<database[tab1id].name<<endl;
		return ;
	}
	fno2=chk_and_get_field(to_stdstr(rhsfield),tab2id);
	if(fno2==-1)
	{
		cout<<rhsfield<<" Not found in "<<database[tab2id].name<<endl;
		return ;
	}
	time_t start,end;
	time(&start);
	for(int i1=0;i1<database[tab1id].data.size();i1++)
	{
		for(int i2=0;i2<database[tab2id].data.size();i2++)
		{
			string s=get_nth_field(tab1id,i1,fno1);
			string t=get_nth_field(tab2id,i2,fno2);
			if(s==t)
			{
				cout<<database[tab1id].data[i1]<<","<<database[tab2id].data[i2]<<endl;
			}
		}

	}
}

int main()
{

	//Read schema file
	int infile;
	cout<<"Welcome to Megatron by Chandravadan!\nInitialising..\n";
	infile=open("schema",O_RDONLY);
	if(infile<0)
	{
		cout<<"Error opening the schema file. Exiting..\n";
		return 0;
	}

	/***************************Populating in-memory schemas***********************************************/
	for(int pp=0; pp<2; pp++)
	{
		string db=readline(infile);
		char *stddb=to_cstr(db);
		char *token=strtok(stddb,"#");
		database[pp].name=to_stdstr(token);
		token=strtok(NULL,"#");
		int xx=0;
		while(token!=NULL)
		{
			char type=token[strlen(token)-1];
			token[strlen(token)-1]='\0';

			string temp=to_stdstr(token);
			database[pp].positions.push_back(temp);
			database[pp].places.insert(make_pair(temp,xx));
			if(token!=NULL)
				database[pp].fields.insert(make_pair(temp,type));
			else break;
			xx++;
			token=strtok(NULL,"#");
		}
	}
	close(infile);
	/***************************Read airports file into main memory***********************************************/
	infile=open("airports.csv",O_RDONLY);

	int eof=lseek(infile,0,SEEK_END);
	lseek(infile,0,SEEK_SET);
	char *countriesdump=(char*)malloc(eof+10);
	read(infile,countriesdump,eof);

	//database[0] is countries, 1 is airports
	char *tok=strtok(countriesdump,"\n");
	while(tok!=NULL)
	{
		database[0].data.push_back(to_stdstr(tok));
		tok=strtok(NULL,"\n");
	}
	//copy(database[0].data[4].begin(),database[0].data[4].end(),ostream_iterator<string>(cout," "));
	close(infile);

	/***************************Read countries file into main memory***********************************************/
	infile=open("countries.csv",O_RDONLY);

	eof=lseek(infile,0,SEEK_END);
	lseek(infile,0,SEEK_SET);
	char *airportdump=(char*)malloc(eof+10);
	read(infile,airportdump,eof);

	tok=strtok(airportdump,"\n");
	while(tok!=NULL)
	{
		database[1].data.push_back(to_stdstr(tok));
		tok=strtok(NULL,"\n");
	}
	close(infile);

	cout<<"Done.Ready for querying(hit 'quit' to exit:) "<<endl;
	int bak,fd;
	bool duped=false;
	int len;
	string fname;
	char * cfname;
	while(1)
	{
		cout<<"$ ";
		command.clear();
		//Take the query
		string cmd;
		getline(cin,cmd);
		char *ccmd=to_cstr(cmd);
		char *part=strtok(ccmd," ");	//First word will always be "select"
		if(part==NULL)
			goto haha;
		//Tokenise and store in command vector
		while(part!=NULL)
		{
			command.push_back(part);
			part=strtok(NULL," ");
		}
		if(strcmp(command[0],"quit")==0)
		{
			cout<<"Thankyou for using Megatron!"<<endl;
			break;
		}
		len=command.size();
			
		duped=false;
		if(strcmp(command[len-2],"|")==0)
		{
			duped=true;
			bak=dup(1);
			fname=to_stdstr(command[len-1]);
			cfname=to_cstr(fname);
			fd=creat(cfname,S_IRWXU);
			//fd = open(cfname,S_IRWXU|O_WRONLY|O_CREAT);
			fd = open(cfname,O_WRONLY);
			if(-1 == fd)
			{
				printf("\n open() failed\n");
				goto haha;
			}
			else
			{
				dup2(fd,1);
				close(fd);
			}
			command.pop_back();
			command.pop_back();

		}
		//Sanity checks
		if(strcmp(command[0],"select")!=0)
		{
			cout<<"Error: Unsupported Operation"<<endl;
			goto haha;
		}
		if(strlen(command[1])==1)
		{
			if(*command[1]!='*')
			{
				cout<<"Error: Expected '*' after "<<command[0]<<endl;
				goto haha;
			}
			else
			{
				if(command.size()<=4)
					selectall();
				else
					selectCondition();
				goto haha;
			}

		}
		if(strcmp(command[1],"distinct")==0)
		{
			selectDistinct();
			goto haha;
		}
		else
		{
			if(command.size()<=4)
				selectSpecific();
			else
				selectCondition();
			goto haha;
		}
haha:	
		if(duped)
		{
			dup2(bak,1);
			close(bak);
		}
	}
	return 0;
}
