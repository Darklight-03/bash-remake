#ifndef _boash_
#define _boash_
#include <iostream>
#include <regex>

using namespace std;

int main(){
  string x = "";
  while(true){
    // (command "quo tes" or noquotes) (operator firstsection)?
    getline(cin,x);
    regex re( "((?:\w*)(?:\s(?:\".+\")|(?:\s\w*))?\s((?:\&|\|)\s\1)?)" );
    if(regex_match(x,re)){
      cout<<"match";
    }
    cout<<"done\n";
  }
}


#endif
