#ifndef _shell_
#define _shell_
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>

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


// return list of the string split around loc (3 elems)
vector<string> splitAround(string elem, int loc){
  vector<string> list;
  string cur = "";
  cout<< "pipeloc = " << loc;
  for(int i = 0;i<elem.length();i++){
    if(i == loc){
      if(cur.compare("")!=0){
        list.push_back(cur);
        cur = "";
      }
      string p = "";
      p += elem.at(i);
      list.push_back(p);
    }else{
      cur += elem.at(i);
      if(i == elem.length() - 1 ){
        list.push_back(cur);
      }
    }
  }
  cout << "\n" << list.at(0) << " " << list.at(1) << " " << list.at(2) << " thisislists";
  return list;
}


// return elements start through end combind (exa version is for locatioon
// inside string at location.
vector<string> combine(vector<string> list, int start, int exastart, int end, int exaend){
  vector<string> olist;
  string out = "";
  string ins = "";
  string ins2 = "";
  for(int i = start ; i<=end ; i++){
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
  if(ins.find("|")<exastart){
    vector<string> tl = splitAround(ins,ins.find("|"));
    // TODO split if string is "blahe|"well get rekt"|wew"
  }
  if(ins2.find("|")>exaend){
    // TODO same as above
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
        vector<string> tv = splitAround(elem, pipeloc);
        for( int i = 0;i<tv.size();i++){
          v.push_back(tv.at(i));
        }
      }
    }
    // to end quote combine all in between
    if(quot && quoteloc>-1){
      cout << "eoquot\n";
      quot = false;
      vector<string> tv = combine(toks,pos,exapos,i,quoteloc);
      for(int i = 0;i<tv.size();i++){
        v.push_back(tv.at(i));
      }
    }
    // if nothing yet inerted and not in quote insert current elem
    if(!quot && v.size() == vsize){
      v.push_back(elem);
    }
    
  }
  return v; 
}

int main(){
  while(true){
    string x = "";
    vector<string> expr;
    getline(cin,x);
    expr = parse(x);
    cout<<"DONE\n";
    for(int i = 0;i<expr.size();i++){
      cout<<expr.at(i)<<"@#$";
    }
  }
}


#endif
