/** @file */ 
////////////////////////////////////////////////////////////////////////////////////////  
//    
//    This file is part of Boost Statechart Viewer.
//
//    Boost Statechart Viewer is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Boost Statechart Viewer is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Boost Statechart Viewer.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <list>
#include <string>

#include "stringoper.h"

using namespace std;

/**
* This class provides saving information about state machine to a specified output file. It saves states and transitions and also it creates the transition table.
*/
class IO_operations
{	
	list<string> transitions; /** list of transitions */
	list<string> states; /** list of states */
	list<string> events; /** list of events */
	string outputFilename; 
	string name_of_machine;
	string name_of_first_state;
	string *table; /** transition table. It is being allocated when starting the creation of output file. */
	int nState;
	int cols, rows;
	/** This function finds place in the transition table to put a transition there. */
	int find_place(string model, int type)
	{
		if(type == 1)
		{
			for(int i = 3;i<cols;i++)
			{
				if(model.compare(0,model.size(),table[i])==0) return i;
			}
		}
		else 
		{
			for(int i = 1;i<rows;i++)
			if(model.compare(0,model.size(),table[i*cols+2])==0) return i;
		}
		return -1;
	}
	public:
	IO_operations() {} /** Implicit constructor */
	/** Constructor that fill in all private variables in this class */
	IO_operations( const string outputFile, const string FSM_name, const string firstState, const list<string> trans, const list<string> state, const list<string> ev )
	{
		outputFilename = outputFile;
		name_of_machine = FSM_name;
		name_of_first_state = firstState;
		transitions = trans;
		states = state;
		events = ev;
	}

	~IO_operations() /** destructor. It deallocates the transition table.*/
	{
		delete [] table;
	}
	
	void setEvents(list<string> events) /** Set list of events to an attribute */
	{
		this->events = events;
	}
	
	void setTransitions(list<string> transitions) /** Set list of transitions to an attribute */
	{
		this->transitions = transitions;
	}

	void setStates(list<string> states) /** Set list of states to an attribute */
	{
		this->states = states;
	}

	void setNameOfStateMachine(string name_of_FSM) /** Set name of FSM to an attribute */
	{
		name_of_machine = name_of_FSM;
	}

	void setNameOfFirstState(string first_state) /** Set name of start state to an attribute */ 
	{
		name_of_first_state = first_state;
	}

	void setOutputFilename(string outputFilename) /** Set name of an output file to an attribute */
	{
		this->outputFilename = outputFilename;
	}

	bool test_start(const string *sState, const string state, const int num)
	{
		for(int i = 0; i<num;i++)
		{
			if (state.compare(0,sState[i].length(), sState[i])==0 && sState[i].length()==state.length()) return true;
		}
		return false;
	}
	
	bool write_states(ofstream& filestr) /** This method write states to the output file and also to transition table. */
	{
		int pos1, pos2, cnt, subs, num_start, orto = 0;
		nState = 1;
		string context, state, ctx, *sState, str;
		list<string> nstates = states;
		context = name_of_machine;
		table[0] = "S";
		table[1] = "Context";
		table[2] = "State";
		cnt = count(name_of_first_state,',');
		sState = new string [cnt+1];
		str = name_of_first_state;
		if(cnt>0) str = get_params(str);
		num_start = cnt+1;
		for(int i = 0;i<=cnt;i++)
		{
			if(i == cnt) sState[i] = str;
			else 
			{
				sState[i] = str.substr(0,str.find(','));
				str = str.substr(str.find(',')+1);
			}
		}		
		for(list<string>::iterator i = nstates.begin();i!=nstates.end();i++) // write all states in the context of the automaton
		{
			state = *i;
			cnt = count(state,',');
			if(count(state,'>')==1) //ortho states
			{
				orto = 1;
				if(cnt>1)cnt = 2;
			}
			if(cnt==1) 
			{
				pos1 = state.find(",");
				if(orto == 1) ctx = cut_namespaces(cut_namespaces(state.substr(pos1+1),1)); 				
				else ctx = cut_namespaces(state.substr(pos1+1));			
				if(ctx.compare(0,context.length(),context)==0 && context.length()==ctx.length())
				{
					str = cut_namespaces(state.substr(0,pos1));
					if(test_start(sState,str,num_start)) 
					{
						filestr<<str<<" [peripheries=2];\n";
						table[cols*nState] = "*";
					}			
					else filestr<<str<<"\n";
					table[cols*nState+2] = str;
					table[cols*nState+1] = context;
					nState+=1;
					nstates.erase(i);
					i--;
				}
			}
			if(cnt==2) //ORTHO OK
			{
				pos1 = state.find(",");
				pos2 = state.substr(pos1+1).find(",")+pos1+1;
				ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
				cout<<ctx<<endl;			
				if(ctx.compare(0,context.length(),context)==0 && context.length()==ctx.length())
				{				
					str = cut_namespaces(state.substr(0,pos1));
					if(test_start(sState,str,num_start)) 
					{
						filestr<<str<<" [peripheries=2];\n";
						table[cols*nState] = "*";
					}
					else filestr<<str<<"\n";
					table[cols*nState+2] = str;
					table[cols*nState+1] = context;			
					nState+=1;					
				}
			}
		}
		delete [] sState;
		subs = 0;
		while(!nstates.empty()) // substates ?
		{
			state = nstates.front();
			filestr<<"subgraph cluster"<<subs<<" {\n";			
			pos1 = state.find(",");
			pos2 = state.substr(pos1+1).find(",")+pos1+1;
			if(pos1 == pos2) return false;
			context = cut_namespaces(state.substr(0,pos1));
			filestr<<"label=\""<<context<<"\";\n";
			cnt = count(state.substr(pos2+1),',');
			sState = new string [cnt+1];
			str = state.substr(pos2+1);
			if(cnt>0) str = get_params(str);
			num_start = cnt+1;
			for(int i = 0;i<=cnt;i++)
			{
				if(i == cnt) sState[i] = str;
				else 
				{
					sState[i] = str.substr(0,str.find(','));
					str = str.substr(str.find(',')+1);
				}
			}	
			nstates.pop_front();
			for(list<string>::iterator i = nstates.begin();i!=nstates.end();i++)
			{
				state = *i;
				cnt = count(state,',');
				if(count(state,'>')==1) //ortho states
				{
					orto = 1;
					if(cnt>1)cnt = 2;
				}				
				if(cnt==1)
				{
					pos1 = state.find(",");
					if(orto == 1) ctx = cut_namespaces(cut_namespaces(state.substr(pos1+1),1)); 				
					else ctx = cut_namespaces(state.substr(pos1+1));
					if(ctx.compare(0,context.length(),context)==0 && context.length()==ctx.length())
					{
						str = cut_namespaces(state.substr(0,pos1));
						if(test_start(sState,str,num_start))
						{
							filestr<<str<<" [peripheries=2];\n";
							table[cols*nState]="*";
						}
						else filestr<<str<<"\n";
						table[cols*nState+2] = str;
						table[cols*nState+1] = context;
						nState+=1;
						nstates.erase(i);
						i--;
					}
				}
				if(cnt==2)
				{
					pos1 = state.find(",");
					pos2 = state.rfind(",");
					ctx = cut_namespaces(state.substr(pos1+1,pos2-pos1-1));
					if(ctx.compare(0,context.length(),context)==0 && context.length()==ctx.length())
					{
						str = cut_namespaces(state.substr(0,pos1));
						if(test_start(sState,str,num_start))
						{
							filestr<<str<<" [peripheries=2];\n";
							table[cols*nState]="*";
						}					
						else filestr<<str<<"\n";
						table[cols*nState+2] = str;
						table[cols*nState+1] = context;
						nState+=1;
					}
				}
			}
			filestr<<"}\n";
			subs+=1;
			delete [] sState;	
		}
		return true;
	}

	void write_transitions(ofstream& filestr) /** Write transitions to the output file nad the transition table. */
	{
		int pos1, pos2;
		string params, state, event, dest;
		for(list<string>::iterator i = transitions.begin();i!=transitions.end();i++) // write all transitions
		{
			params = *i;
			if(count(params,',')==2)
			{			
				pos1 = params.find(",");
				state = cut_namespaces(params.substr(0,pos1));
				filestr<<state<<"->";
				pos2 = params.rfind(",");
				dest = cut_namespaces(params.substr(pos2+1));
				filestr<<dest;
				event = cut_namespaces(params.substr(pos1+1,pos2-pos1-1));
				filestr<<"[label=\""<<event<<"\"];\n";
				table[find_place(state,2)*cols+find_place(event,1)]=dest;
			}
		}		
		return;
	}

	void fill_table_with_events() /** Fill the first row of the transition table with events. */
	{
		int j = 3;
		for(list<string>::iterator i = events.begin();i!=events.end();i++)
		{
			table[j] = *i;
			j++;
		}
	}

	void save_to_file() /** Create output file stream and write there the name of state machine. It controls the whole process of writing into output files. */
	{
		if(!name_of_first_state.empty())	
		{	
			ofstream filestr(outputFilename.c_str());
			filestr<<"digraph "<< name_of_machine<< " {\n";
			cols = events.size()+3;
			rows = states.size()+1;
			table = new string [cols*rows];
			fill_table_with_events();			
			if(!write_states(filestr)) 
			{
				cerr<<"Error during writing states.\n";
				filestr<<"}";		
				filestr.close();
				return;			
			}
			write_transitions(filestr);
			filestr<<"}";		
			filestr.close();
			// call write_reactions();
			print_table();
		}
		else cout<<"No state machine was found. So no output file was created.\n";
		return;
	}

	void print_table() /** This function prints the transition table. At first it counts the size of all columns. */
	{
		cout<<"\nTRANSITION TABLE\n";
		unsigned * len = new unsigned[cols];
		len[0] = 1;
		string line = "-|---|-";
		for(int i = 1; i<cols; i++)
		{
			len[i] = 0;
			for(int j = 0;j<rows;j++)
			{
				if(len[i]<table[j*cols+i].length()) len[i] = table[j*cols+i].length();
			}
			for(unsigned k = 0; k<len[i]; k++)
			{
				line.append("-");
			}
			line.append("-|-");
		}
		cout<<line<<"\n";
		for(int i = 0; i<rows; i++)
		{
			cout<<" | ";		
			for(int j = 0;j<cols;j++)
			{
				cout.width(len[j]);
				cout<<left<<table[i*cols+j]<<" | ";
			}
			cout<<"\n";
			cout<<line<<"\n";
		}
		delete [] len;
	}

	void print_stats() /** Print short statistics about the state machine */
	{
		cout<<"\n"<<"Statistics: \n";
		cout<<"Number of states: "<<states.size()<<"\n";
		cout<<"Number of events: "<<events.size()<<"\n";
		cout<<"Number of transitions: "<<transitions.size()<<"\n\n";
		return;
	}

};