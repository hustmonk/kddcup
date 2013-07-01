/**
 *  =============================================================
 *
 *    	Filename:  Paper.h
 *   Description:  paper info from [../data/Paper.csv]
 *       Created:  05/06/2013 01:43:21 PM
 *       Version:  
 *      Revision:  #1;#4
 *        Author:  liujingming(liujingminghust@163.com)
 *
 *  =============================================================
 **/

#ifndef C_Author_Disambiguation_Paper
#define C_Author_Disambiguation_Paper

#include "map"
#include "CSV.h"
using namespace std;
extern char DATA_DIR[];

class Paper {
  string title;
  int id;
  int conference_id;
  int journal_id;
  int year;
  string keywords;

  public:
  Paper() {
  }
  Paper(char *line, char **rows, int max_row_count) {
    int row_count = 0;
    CSV::parser(line, rows, max_row_count, row_count);
    // Id,Title,Year,ConferenceId,JournalId,Keyword
    id = atoi(rows[0]);
    // title = string(rows[1]);
    // year = atoi(rows[2]);
    conference_id = atoi(rows[3]);
    journal_id = atoi(rows[4]);
    // keywords = string(rows[5]);
  }

  int get_conference_id() {
    return conference_id;
  }

  int get_journal_id() {
    return journal_id;
  }

  int get_id_by_type(int type) {
    if (type == 1) {
      return conference_id;
    }
    return journal_id;
  }

  int get_id() {
    return id;
  }

  Paper(const Paper& paper) {
    title = paper.title;
    id = paper.id;
    conference_id = paper.conference_id;
    journal_id = paper.journal_id;
    year = paper.year;
    keywords = paper.keywords;
  }
};

class CPaperList {
  map<int, Paper> paper_map;
  static CPaperList *_instance;
  CPaperList() {
    char filename[1024];
    memset(filename, 0, 1024*sizeof(char[0]));
    snprintf(filename, 1024, "%s/Paper.csv", DATA_DIR);
    FILE *fin = fopen(filename, "r");
    assert(fin != NULL);

    char *line = new char[1024000];
    int max_row_count = 7;
    char **rows = new char*[max_row_count];
    for (int i = 0; i < max_row_count; ++i) {
      rows[i] = new char[1024000];
    }
    fgets(line, 1024000, fin);
    while (!feof(fin)) {
      memset(line, 0, 10240);
      fgets(line, 1024000, fin);
      if (feof(fin)) {
        break;
      }
      Paper paper = Paper(line, rows, max_row_count);
      paper_map[paper.get_id()] = paper;
    }

    printf("paper_list size:[%d]\n", get_paper_map_count());

    delete []line;
    for (int i = 0; i < max_row_count; ++i) {
      delete []rows[i];
    }
    delete []rows;
  }

 public:
  static CPaperList *get_instance() {
    if (_instance == NULL) {
      _instance = new CPaperList();
    }
    return _instance;
  }

  int get_paper_map_count() {
    return paper_map.size();
  }
  Paper *get_paper_by_id(int paper_id) {
    if (paper_map.find(paper_id) == paper_map.end()) {
      return NULL;
    }
    return &paper_map[paper_id];
  }
};
#endif  // C_Author_Disambiguation_Paper
