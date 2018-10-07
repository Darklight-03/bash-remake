#ifndef _shell_
#define _shell_
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

// valid = unix command || unix command AMP || special command
// unix command = command name ARGS || unix command REDIRECTION filename || unix command PIPE unix command
// special command = cd DIRECTORY || exit
// command name = any valid executable/interpreted file name
// AMP = &
// ARG = string
// ARGS = ARG ARGS || ARG
// DIRECTORY = absolute path || relative path
// PIPE = |
// REDIRECTION = < || >


int findQ(string s){
  int qloc = s.find("\"");
  int sqloc = s.find("\'");
  return min(qloc,sqloc);
}

int findP(string s){
  int pipeloc = s.find("|");
  int gtloc = s.find(">");
  int ltloc = s.find("<");
  int pg = min(pipeloc,gtloc);
  return min(pg,ltloc);
}

// return list of the string split around loc (3 elems)
vector<string> splitAround(string elem, int loc){
  cout<< "ELEM LOC = " << elem << " " << loc;
  vector<string> list;
  string cur = "";
  for(int i = 0;i<elem.length();i++){
    if(i == loc){
      list.push_back(cur);
      cur = "";
      string p = "";
      p += elem.at(i);
      list.push_back(p);
    }else{
      cur += elem.at(i);
    }
    if(i == elem.length() - 1 ){
      list.push_back(cur);
    }
  }
  cout << "\n" << list.at(0) << " " << list.at(1) << " " << list.at(2) << " thisislists";
  return list;
}



// recursive splitAround
vector<string> splitPipesR(vector<string> vv, string elem){
  int loc = elem.find("|");
  if(loc == -1){
    vv.push_back(elem);
    return vv;
  }
  vector<string> ins = splitAround(elem,loc);
  vv.insert(vv.end(),ins.begin(),ins.end());
  string backs = vv.at(vv.size()-1);
  vv.pop_back();
  return splitPipesR(vv,backs);
}

vector<string> splitPipes(string elem){
  vector<string> v;
  return splitPipesR(v,elem);
}



// return elements start through end combind (exa version is for locatioon
// inside string at location.
vector<string> combine(vector<string> list, int start, int exastart, int end, int exaend){
 
  vector<string> olist;
  string out = "";
  string ins = "";
  string ins2 = "";
  bool after = false;
  for(int i = start ; i<=end ; i++){
    if(start == end){
      string onlyelem = list.at(i);
      for(int i = 0 ; i < onlyelem.length() ; i++){
        if(i<exastart){
          ins += onlyelem.at(i);
        }else if (i > exastart && i < exaend){
          out += onlyelem.at(i);
        }
        if(i>exaend){
          ins2 += onlyelem.at(i);
        }
      }
      break;
    }
    if(i>start && i<end){
      cout << "?";
      cout<<"\n COMB " << list.at(i);
      out += list.at(i);
      out += " ";
    }
    if(i==start){
      string firstelem = list.at(i);
      for(int i = 0;i<firstelem.length();i++){
        if(i<exastart){
          ins+=firstelem.at(i);
        }else if (i > exastart){
          out+=firstelem.at(i);
        }
      }
      out += " ";
    }
    if(i==end){
      string lastelem = list.at(i);
      for(int i = 0 ; i < lastelem.length();i++){
        if(i<exaend){
          out+=lastelem.at(i);
        }else if ( i > exaend){
          ins+=lastelem.at(i);
        }
      }
    }
  }
  int insbar = ins.find("|");
  int ins2bar = ins2.find("|");
  cout<<ins2bar<<"INS2BAR";
  if(insbar>-1){
    cout<< "y tho ";
    vector<string> tl = splitPipes(ins);
    for(int i = 0;i<tl.size();i++){
      if(i!=tl.size()-1){
        olist.push_back(tl.at(i));
      }
      else{
        ins = tl.at(i);
      }
    }
  }
  if(ins2bar>-1){
    cout<< "even more y tho";
    vector<string> tl = splitPipes(ins2);
    ins2 = tl.at(0);
    after = true;
    tl.erase(tl.begin());
    olist.push_back(ins+out+ins2);
    for(int i = 0 ; i < tl.size() ; i++){
      olist.push_back(tl.at(i));
    }
  }
  if(!after){
    olist.push_back(ins+out+ins2);
  }
  cout<<"combined: "<<out;
  return olist;
}



vector<string> parse(string s){
  vector<string> v;
  vector<string> toks;
  string newstring = "";
  // iterate through separating by spaces
  for( int i = 0 ; i<s.length() ; i++ ){
    // push word and reset if space detected
    if(s.at(i) == ' ' ){
      toks.push_back(newstring);
      newstring = "";
    }
    // add char to string
    else{
      newstring += s.at(i);
      cout << s.at(i);
      // if end of string push last string to list too.
      if(i == s.length()-1){
        toks.push_back(newstring);
        newstring = "";     
      }
    }
  }
  bool quot = false;
  int pos = -1;
  int exapos = -1;
  // iterate through word lists combining parenthesis.
  for ( int i = 0 ; i < toks.size() ; i++ ){
    string elem = toks.at(i);
    int pipeloc = elem.find("|");
    int quoteloc = elem.find("\"");
    int vsize = v.size();
    cout << "\n" << toks.at(i);
    // if quote at begin set quote flag and position
    if(!quot && quoteloc>-1){
      cout << "quot \n";
      quot = true;
      pos = i;
      exapos = quoteloc;
      quoteloc = elem.find("\"",quoteloc+1);
    }
    // if pipe is not in quote then split around it
    if(!quot && pipeloc>-1){
      if(elem.length() == 1){
        cout << "pipe ok\n";
      }
      else {
        vector<string> tv = splitPipes(elem);
        for( int i = 0;i<tv.size();i++){
          if(tv.at(i) != ""){
            v.push_back(tv.at(i));
          }
        }
      }
    }
    // to end quote combine all in between
    if(quot && quoteloc>-1){
      cout << "eoquot\n";
      quot = false;
      vector<string> tv = combine(toks,pos,exapos,i,quoteloc);
      for(int i = 0;i<tv.size();i++){
        if(tv.at(i)!=""){
          v.push_back(tv.at(i));
        }
      }
    }
    // if nothing yet inerted and not in quote insert current elem
    if(!quot && v.size() == vsize){
      v.push_back(elem);
    }
    
  }
  return v; 
}

vector<char*> v2charv(vector<string> v){
  // DUMB STUFF NEEDED TO CONVERT VECTOR TO CHAR ARR 
  // ( from stackoverflow.com/questions/26032039 )
  vector<char*> newv;
  for(auto& elem:v){
    newv.push_back(&elem[0]);
  }
  return newv;
}

void runSingle(char** chararr){
  int pid = fork();
  if(pid == 0){
    execvp(chararr[0],chararr);
    cout<<"succ\n";
  }else{
    waitpid(pid,NULL,0);
  }
}




// takes each command on the list and runs them with pipes if necessary
void runCommands(vector<vector<string>> cmdsstr){
  
    int fd[2];
  for(int i = 0;i<cmdsstr.size()-1;i++){
    pipe(fd);
    int pid = fork();
    if(pid==0){
      cout<<"\n\n"<<cmdsstr.at(i).at(0)<<"\n";
      dup2(fd[1],1);
      close(fd[0]);
      
      vector<char*> cmds = v2charv(cmdsstr.at(i));
      execvp(cmds.data()[0],cmds.data());


    }else{
      waitpid(pid,NULL,0);
      close(fd[1]);
      dup2(fd[0],0);
    }
    
  }
    close(fd[1]);
    close(fd[0]);
    vector<char*> cmds = v2charv(cmdsstr.at(cmdsstr.size()-1));
    execvp(cmds.data()[0],cmds.data());
}

// takes the vector of commands and splits into multiple vectors, splitting by |, then calls runCommands
void handleCommand(vector<string> cmds){
  vector<vector<string>> splitcmds;
  splitcmds.push_back(vector<string>());
  int vi = 0;
  for(int i = 0;i<cmds.size();i++){
    if(cmds.at(i)!="|"){
      splitcmds.at(vi).push_back(cmds.at(i));
    }else{
      splitcmds.push_back(vector<string>());
      vi++;
    }
  }
  cout<<splitcmds.size();
  runCommands(splitcmds);
}

int main(){
  while(true){
    // BELOW THIS LINE IS INPUT
    string x = "";
    vector<string> expr;
    getline(cin,x);
    expr = parse(x);
    cout<<"DONE\n";
    for(int i = 0;i<expr.size();i++){
      cout<<expr.at(i)<<"@#$";
    } 
    // ABOVE THIS LINE IS INPUT
    
    // basic fork and handle command on new process
    int pid = fork();
    if(pid==0){
      handleCommand(expr);
    }else{
      waitpid(pid,NULL,0);
    }
    /*
    vector<char*> args = v2charv(expr);
    runSingle(args.data());*/
  }
}


#endif
