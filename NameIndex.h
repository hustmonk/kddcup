/**
 *  =============================================================
 *
 *    	Filename:  NameIndex.h
 *   Description:  build the subname index
 *       Created:  05/31/2013 10:49:43 PM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/
#ifndef C_Author_Disambiguation_NameIndex
#define C_Author_Disambiguation_NameIndex
#include "CMyArray.h"
class NameIndex{
  static NameIndex *_instance;
  map<string, CMyArray> name_index;
 public:
  static NameIndex *get_instance(){
    if (_instance == NULL) {
      _instance = new NameIndex();
    }
    return _instance;
  }

  void add(string string_name, int author_id) {
    if (name_index.find(string_name) == name_index.end()) {
      name_index[string_name] = CMyArray();
    }
    name_index[string_name].addUnit(author_id);
  }

  list<int> *get_name_index(string string_name) {
    if (name_index.find(string_name) == name_index.end()) {
      return NULL;
    }
    return name_index[string_name].get_array();
  }

  void valid_name_index() {
    for (map<string, CMyArray>::iterator it = name_index.begin();
        it != name_index.end(); ++it) {
      printf("VNI:[%s] [%d]\n", it->first.c_str(), it->second.size());
    }
  }

  int get_size(string key) {
    if (name_index.find(key) == name_index.end()) {
      return 0;
    }
    return name_index[key].size();
  }
};
#endif
