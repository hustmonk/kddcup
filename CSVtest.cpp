#include "CSV.h"
#include "stdio.h"

int main() {
  const char *line = "1351107,\"Sa\"\"ed Abed\",\"\"";
  printf("line:%s\n",line);
  int max_row_count = 4;
  char **rows = new char*[max_row_count];
  for (int i = 0; i < max_row_count; ++i) {
    rows[i] = new char[1024000];
  }
  int row_cout = 5;
  CSV::parser(line, rows, max_row_count, row_cout);
  for (int i = 0; i < row_cout; ++i) {
    printf("%d:[%s]\n", i, rows[i]);
  }
}
