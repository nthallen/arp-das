#include <stdio.h>
#include "cmdgen.h"
#include "compiler.h"

typedef struct if_list_s {
	struct if_list_s *next;
	char *if_name;
} if_list_t;
static if_list_t *if_list;

void new_interface( char *if_name ) {
	if_list_t *new_if;
	fprintf( ofile, "IOFUNC_ATTR_T *if_%s;\n", if_name );
	new_if = (if_list_t *)new_memory(sizeof(if_list_t));
	new_if->next = if_list;
	new_if->if_name = if_name;
	if_list = new_if;
}

void output_interfaces(void) {
	if_list_t *cur_if;
	fprintf( ofile, "\n#ifdef SERVER\n" );
	fprintf( ofile, "  void cis_interfaces(void) {\n" );
	for ( cur_if = if_list; cur_if; cur_if = cur_if->next ) {
		fprintf( ofile, "    if_%s = cis_setup_rdr(\"%s\");\n",
		  cur_if->if_name, cur_if->if_name );
	}
	fprintf( ofile, "  }\n#endif\n\n" );
}
