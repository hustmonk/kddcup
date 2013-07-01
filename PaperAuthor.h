/**
 *  =============================================================
 *
 *    	Filename:  PaperAuthor.h
 *   Description:  relation between papers and authors 
 *                 infomation from [../data/PaperAuthor.csv]
 *       Created:  05/06/2013 11:44:51 AM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_PaperAuthor
#define C_Author_Disambiguation_PaperAuthor
#include <vector>
#include <list>
#include <map>
#include <string>

#include "CSV.h"
#include "CMyArray.h"
#include "Paper.h"
#include "Name.h"
#include "Affiliation.h"
extern char DATA_DIR[];

using namespace std;
class CPaPerAuthorList {
  map<int, CMyArray> paper_authors_list;
  map<int, CMyArray> author_papers_list;
  map<int, map<int, string> > paper_set_names;
  map<string, int> name_count;
  static CPaPerAuthorList* _instance;

  CPaPerAuthorList() {
    char filename[1024];
    memset(filename, 0, 1024*sizeof(char[0]));
    snprintf(filename, 1024, "%s/PaperAuthor.csv", DATA_DIR);
    FILE *fin = fopen(filename, "r");
    assert(fin != NULL);

    int max_line_length = 1024000;
    char *line = new char[max_line_length];
    int max_row_count = 4;
    char **rows = new char*[max_row_count];
    for (int i = 0; i < max_row_count; ++i) {
      rows[i] = new char[max_line_length];
    }
    fgets(line, max_line_length, fin);
    
    AffiliationIndex *affiliation_index = AffiliationIndex::get_instance();
    while (!feof(fin)) {
      memset(line, 0, 10240);
      fgets(line, max_line_length, fin);
      if (feof(fin)) {
        break;
      }
      int row_count = 0;
      CSV::parser(line, rows, max_row_count, row_count);
      if (row_count <= 3) {
        continue;
      }
      assert(row_count > 3);
      int paper_id = atoi(rows[0]);
      int author_id = atoi(rows[1]);
      string author_name = string(rows[2]);
      string affiliation = string(rows[3]);
      affiliation_index->add(affiliation, author_id);
      if (author_name.length() < 40) {
        string normalized_name = CName::normal_name(author_name);

        if (name_count.find(normalized_name) == name_count.end()) {
          name_count[normalized_name] = 1;
        } else {
          ++name_count[normalized_name];
        }
      }
      if (author_name.length() > 0) {
        if (paper_set_names.find(author_id) == paper_set_names.end()) {
          paper_set_names[author_id] = map<int, string>();
        }
        map<int, string> &author_names = paper_set_names[author_id];
        author_names[paper_id] = author_name;
      }
      if (paper_authors_list.find(paper_id) == paper_authors_list.end()) {
        paper_authors_list[paper_id] = CMyArray();
      }

      if (author_papers_list.find(author_id) == author_papers_list.end()) {
        author_papers_list[author_id] = CMyArray();
      }
      paper_authors_list[paper_id].addUnit(author_id);
      author_papers_list[author_id].addUnit(paper_id);
#ifdef DEBUG_Paper_Author
      for (int i = 0; i < strlen(line); ++i) {
        printf("[%c %d]\n", line[i], line[i]);
      }
      printf("[%s]paper_id:[%d] author_id:[%d]\n", line, paper_id, author_id);
      break;
#endif
    }
    printf("paper_authors_list size:[%d]\n", (int)paper_authors_list.size());
    printf("author_papers_list size:[%d]\n", (int)author_papers_list.size());

    delete []line;
    for (int i = 0; i < max_row_count; ++i) {
      delete []rows[i];
    }
    delete []rows;
  }

 public:
  static CPaPerAuthorList* get_instance() {
    if (_instance == NULL) {
      _instance = new CPaPerAuthorList();
    }
    return _instance;
  }

  static void destroy_instance() {
    if (_instance != NULL) {
      delete _instance;
      _instance = NULL;
    }
  }

  list<int> *get_papers_list_by_author(int author) {
    if (author_papers_list.find(author) == author_papers_list.end()) {
      return NULL;
    }
    return author_papers_list[author].get_array();
  }

  void print_name_count() {
    FILE *fout = fopen("name_count.info", "w");
    for (map<string, int>::iterator it = name_count.begin();
        it != name_count.end();
        ++it) {
      fprintf(fout, "%s\t%d\n", it->first.c_str(), it->second);
    }
    fclose(fout);
  }

  list<int> *get_authors_list_by_paper(int paper) {
    if (paper_authors_list.find(paper) == paper_authors_list.end()) {
      return NULL;
    }
    return paper_authors_list[paper].get_array();
  }

  map<int, string> *get_author_names_by_author_id(int author_id) {
    if (paper_set_names.find(author_id) == paper_set_names.end()) {
      return NULL;
    }
    return &paper_set_names[author_id];
  }

  int get_author_name_num(string key) {
    if (name_count.find(key) == name_count.end()) {
      return 0;
    }
    if (key.find("*") != string::npos) {
      return 0;
    }
    return name_count[key];
  }

};
#endif  // C_Author_Disambiguation_PaperAuthor
