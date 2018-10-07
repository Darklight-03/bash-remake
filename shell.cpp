#ifndef _shell_
#define _shell_
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
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
  if(qloc == sqloc) return qloc;
  if(qloc >=0 && sqloc >=0) return min(qloc,sqloc);
  if(qloc == -1) return sqloc;
  return qloc;
}

int findP(string s){
  int pipeloc = s.find("|");
  int gtloc = s.find(">");
  int ltloc = s.find("<");
  int aloc = s.find("&");
  int comb = -1;
  if(pipeloc == gtloc){ 
    comb = pipeloc;
  }else if( pipeloc >= 0 && gtloc >= 0){
    comb = min(pipeloc,gtloc);
  }else if(pipeloc == -1){
    comb = gtloc;
  }else{
    comb = pipeloc;
  }
  int comb2 = -1;
  if(comb == ltloc){
    comb2 = comb;
  }else if(comb >= 0 && ltloc >= 0){
    comb2 = min(comb,ltloc);
  }else if(comb == -1){
    comb2 = ltloc;
  }else{
    comb2 = comb;
  }
  if(comb2 == aloc){
    return comb2;
  }else if(comb2 >=0 && aloc >=0){
    return min(comb2,aloc);
  }else if(comb2 == -1){
    return aloc;
  }else{
    return comb2;
  }
}

// return list of the string split around loc (3 elems)
vector<string> splitAround(string elem, int loc){
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
  return list;
}



// recursive splitAround
vector<string> splitPipesR(vector<string> vv, string elem){
  int loc = findP(elem);
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
  int insbar = findP(ins);
  int ins2bar = findP(ins2);
  if(insbar>-1){
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
    int pipeloc = findP(elem);
    int quoteloc = findQ(elem);
    int vsize = v.size();
    // if quote at begin set quote flag and position
    if(!quot && quoteloc>-1){
      quot = true;
      pos = i;
      exapos = quoteloc;
      quoteloc = elem.find("\"",quoteloc+1);
    }
    // if pipe is not in quote then split around it
    if(!quot && pipeloc>-1){
      if(elem.length() == 1){
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

// not necessary anymore
void runSingle(char** chararr){
  int pid = fork();
  if(pid == 0){
    execvp(chararr[0],chararr);
  }else{
    waitpid(pid,NULL,0);
  }
}

// returns 1 if output 2 if input redirection
int redirType(vector<string> l){
  for( int i = 0 ; i < l.size() ; i++){
    if(l.at(i) == ">"){
      return 1;
    }
    if(l.at(i) == "<"){
      return 0;
    }
  }
  return -1;
}

// returns true if vector has a redirection
bool hasRedir(vector<string> l){
  for(int i = 0;i<l.size();i++){
    if(l.at(i) == ">" || l.at(i) == "<"){
      return true;
    }
  }
  return false;
}

// splits vector into command and file to redirect to
vector<vector<string>> splitRedir(vector<string> l){
  vector<vector<string>> o;
  o.push_back(vector<string>());
  o.push_back(vector<string>());
  bool fh = true;

  for(int i = 0 ; i < l.size() ; i++){
    if(l.at(i) == ">" || l.at(i) == "<"){
      fh = false;
      continue;
    }
    if(fh){
      o.at(0).push_back(l.at(i));
    }else{
      o.at(1).push_back(l.at(i));
    }
  }
  return o;
}

// takes each command on the list and runs them with pipes if necessary
void runCommands(vector<vector<string>> cmdsstr){
  
  // pipes init
  int fd[2];
  //loop
  for(int i = 0;i<cmdsstr.size()-1;i++){
    // make pipe
    pipe(fd);
    // split proc
    int pid = fork();
    if(pid==0){
      // child:
      // redir output to fd
      dup2(fd[1],1);
      close(fd[0]);

      vector<string> cur = cmdsstr.at(i); 
      vector<string> after;
      int file = -1;
      int dest;
      // if needed redir file
      if(hasRedir(cur)){
        dest = redirType(cur);
        vector<vector<string>> rv = splitRedir(cur);
        cur = rv.at(0);
        after = rv.at(1);
        vector<char*> cafter = v2charv(after);
        file = open(cafter.at(0), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        dup2(file,dest);
      }
  
      // run command
      vector<char*> cmds = v2charv(cur);
      execvp(cmds.data()[0],cmds.data());
      exit(errno);

    }else{
      // parent:
      waitpid(pid,NULL,0);
      close(fd[1]);
      // redir output to input
      dup2(fd[0],0);
    }
    
  }
  // end of loop
    close(fd[1]);
    close(fd[0]);
  // redir if needed to files 
    vector<string> cur = cmdsstr.at(cmdsstr.size()-1);
    vector<string> after;
    int file;
    int dest;
    if(hasRedir(cur)){
      dest = redirType(cur);
      vector<vector<string>> rv = splitRedir(cur);
      cur = rv.at(0);
      after = rv.at(1);
      vector<char*> cafter = v2charv(after);
      file = open(cafter.at(0), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
      dup2(file,dest);
    }
    if(cur.at(cur.size()-1).compare("&")==0){
      cur.pop_back();
      if(fork()!=0){
        signal(SIGCHLD,SIG_IGN);
        exit(0);
      }
    }

    // run final command
    vector<char*> cmds = v2charv(cur);
    execvp(cmds.data()[0],cmds.data());
    exit(errno);
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
  runCommands(splitcmds);
}

int main(){
  while(true){
    // BELOW THIS LINE IS INPUT
    cout<<"\n"<<get_current_dir_name()<<": ";
    string x = "";
    vector<string> expr;
    getline(cin,x);
    expr = parse(x);
    // ABOVE THIS LINE IS INPUT
    
    // cd handling
    if(expr.at(0).compare("cd") == 0){
      vector<char*> cdv = v2charv(expr);
      chdir(cdv.at(1));
    }

    // exit handling
    if(expr.at(0).compare("exit") == 0){
      exit(0);
    }

    // basic fork and handle command on new process
    int pid = fork();
    if(pid==0){
      handleCommand(expr);
    }else{
      waitpid(pid,NULL,0);
    }
  }
}


#endif
