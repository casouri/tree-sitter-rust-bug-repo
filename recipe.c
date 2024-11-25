#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tree_sitter/api.h>
#include <assert.h>

TSLanguage *tree_sitter_rust();

static const char*
read_buffer (void *buffer, uint32_t byte_index,
		     TSPoint position, uint32_t *bytes_read) {
  if (byte_index < strlen(buffer)) {
	*bytes_read = strlen(buffer) - byte_index;
	return ((char *) buffer) + byte_index;
  }
  *bytes_read = 0;
  return NULL;
}

/* void log(void *payload, TSLogType log_type, const char *buffer) { */
/*   printf ("%s\n", buffer); */
/* } */

int main() {

  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_rust());

  uint32_t error_offset;
  TSQueryError error_type;
  TSQuery *query = ts_query_new(tree_sitter_rust(), "(call_expression) @cap", 22, &error_offset, &error_type);

  if (query == NULL) {
	return -1;
  }

  /* TSLogger logger; */
  /* logger.payload = NULL; */
  /* logger.log = log; */

  /* ts_parser_set_logger(parser, logger); */

  // Initial parse

  FILE *fd = fopen("source_before.rs", "r");

  if (fd == NULL) return -1;

  char *source = NULL;
  size_t len;
  ssize_t bytes_read = getdelim(&source, &len, '\0', fd);

  if (bytes_read == -1) return -1;

  TSInput input = {(void *) source, read_buffer, TSInputEncodingUTF8};

  // Limit the parser to ("{:?}", foo())
  TSRange ranges[1] = {
	{{0,0},{0,0},51,66},
  };

  bool suc = ts_parser_set_included_ranges(parser, &ranges, 1);
  if (!suc) return -1;

  TSTree *tree = ts_parser_parse(parser, NULL, input);
  TSNode root_node_1 = ts_tree_root_node(tree);

  printf("First parse root node:\n%s\n", ts_node_string(root_node_1));

  TSQueryCursor *cursor = ts_query_cursor_new();
  ts_query_cursor_exec(cursor, query, root_node_1);
  TSQueryMatch match;
  bool matched = ts_query_cursor_next_match (cursor, &match);
  printf("First parse query: %s\n", matched ? "has match" : "no match");

  // Second parse

  fd = fopen("source_after.rs", "r");
  if (fd == NULL) return -1;
  bytes_read = getdelim(&source, &len, '\0', fd);
  if (bytes_read == -1) return -1;

  // The edit is inserting a single space after ("{:?}", foo());
  TSPoint dummy_point = {0, 0};
  TSInputEdit edit = {
	67, 67, 68,
	dummy_point, dummy_point, dummy_point
  };

  ts_tree_edit(tree, &edit);
  tree = ts_parser_parse(parser, tree, input);

  TSNode root_node_2 = ts_tree_root_node(tree);

  printf("Second parse root node:\n%s\n", ts_node_string(root_node_2));

  TSQueryCursor *cursor2 = ts_query_cursor_new();
  ts_query_cursor_exec(cursor2, query, root_node_2);
  bool matched2 = ts_query_cursor_next_match (cursor2, &match);
  printf("Second parse query: %s\n", matched2 ? "has match" : "no match");

  return 0;
}
