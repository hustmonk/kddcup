/**
 *  =============================================================
 *
 *  Copyright (c) 2011-2013 Panguso.com. All rights reserved.
 *
 *  	  Filename:  CMyArray.h
 *   Description:  my private array
 *       Created:  05/06/2013 10:43:36 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_CMyArray
#define C_Author_Disambiguation_CMyArray
#include <stdio.h>
#include <string.h>
#include <list>

using namespace std;

class CMyArray {
  list<int> _array;

 public:
  CMyArray() {
  }

  void addUnit(int unit) {
    _array.push_back(unit);
  }
  bool find(int id) {
    for (list<int>::iterator beg = _array.begin(); beg != _array.end(); ++beg) {
      if (id == *beg) {
        return true;
      }
    }
    return false;
  }

  int size() {
    return _array.size();
  }

  list<int> *get_array() {
    return &_array;
  }

  void Print() {
    for (list<int>::iterator beg = _array.begin(); beg != _array.end(); ++beg) {
      printf("[%d] ", *beg);
    }
    printf("\n");
  }

  explicit CMyArray(char *line) {
    int len = strlen(line);
    int vec_num = 1;
    for (int i = 1; i < len-1; ++i) {
      if (line[i] == ' ' &&
          isdigit(line[i-1]) && isdigit(line[i+1])) {
        ++vec_num;
      }
    }

    int digit;
    for (int i = 0; i < vec_num; ++i) {
      sscanf(line, "%d", &digit);
      _array.push_back(digit);
    }
  }
};
#endif  // C_Author_Disambiguation_CMyArray
