/**
 *  =============================================================
 *
 *  	  Filename:  LossMiddleName.h
 *   Description:  check the lost middle name type
 *       Created:  05/30/2013 11:26:45 PM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_LossMiddleName
#define C_Author_Disambiguation_LossMiddleName
class LossMiddleName{
 private:
  static LossMiddleName*_instance;
  map<string, vector<CName*> > loss_middle_name_tables;
  map<string, set<string> > loss_middle_name_mark_tables;
  set<string> valid_set;
  set<string> is_valid_set;
 public:
  static LossMiddleName*get_instance() {
    if (_instance == NULL) {
      _instance = new LossMiddleName();
    }
    return _instance;
  }
  void add(CName* cname1, CName* cname2) {
#ifdef TEST
    return;
#endif
    string normalized_name1 = cname1->normalized_name;
    string normalized_name2 = cname2->normalized_name;
    /*
    if (cname1->names.size() > 5 || cname2->names.size() > 5) {
      add_key(normalized_name1);
      add_key(normalized_name2);
      return;
    }
    */
    add(normalized_name1, cname2);
    add(normalized_name2, cname1);
  }

  void add_key(string &key) {
    if (loss_middle_name_tables.find(key) == loss_middle_name_tables.end()) {
      loss_middle_name_tables[key] = vector<CName*>();
      loss_middle_name_mark_tables[key] = set<string>();
    }
  }

  void add(string &key, CName* cname) {
    if (loss_middle_name_tables.find(key) == loss_middle_name_tables.end()) {
      loss_middle_name_tables[key] = vector<CName*>();
      loss_middle_name_mark_tables[key] = set<string>();
    }
    if (loss_middle_name_mark_tables[key].find(cname->normalized_name) 
      == loss_middle_name_mark_tables[key].end()) {
      loss_middle_name_mark_tables[key].insert(cname->normalized_name);
      loss_middle_name_tables[key].push_back(cname);
    }
  }

  void valid() {
#ifdef TEST
    return;
#endif
    for (map<string, vector<CName*> >::iterator it = loss_middle_name_tables.begin();
        it != loss_middle_name_tables.end();
        ++it) {
      CName cname = CName(it->first);
      vector<CName*> &namelist = it->second;
      bool is_valid = true;
      if (namelist.size() > 1) {
        for (unsigned int i = 0; i < namelist.size() && is_valid; ++i) {
          CName* cname1 = namelist[i];
          if (cname1->names.size() < cname.names.size()) {
           continue;
          }
          if (cname1->simply_name.find('*') != string::npos) {
           continue;
          }
          for (unsigned int j = i + 1; j < namelist.size() && is_valid; ++j) {
            CName* cname2 = namelist[j];
            if (cname2->names.size() < cname.names.size()) {
              continue;
            }
          if (cname2->simply_name.find('*') != string::npos) {
           continue;
          }
            int similar =  cname1->_GetNameSimilarType(cname2);
            if (similar > 0 && similar != 999 ) {
              continue;
            }
            bool status = cname1->is_lost_middle_name(cname2, VALID_LOST);
            if (status == false) {
#ifdef DEGUB
              printf("LV [%s] [%s] [%s]\n", it->first.c_str(),
                  cname1->normalized_name.c_str(), cname2->normalized_name.c_str());
#endif
              is_valid = false;
            }
          }
        }
      }
      if (is_valid == true) {
        valid_set.insert(it->first);
      } else {
        is_valid_set.insert(it->first);
      }
    }
  }

  bool is_valid(CName* cname) {
    return is_valid_set.find(cname->normalized_name) == is_valid_set.end()?true:false;
  }

  bool is_valid(const string &str) {
    return is_valid_set.find(str) == is_valid_set.end()?true:false;
  }
};
#endif
