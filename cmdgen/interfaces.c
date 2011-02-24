#include <stdio.h>
#include <string.h>
#include "cmdgen.h"
#include "compiler.h"

typedef struct if_list_s {
  struct if_list_s *next;
  int if_type;
  char *if_name;
  char *if_path;
} if_list_t;
static if_list_t *if_list, *if_last;

#define IFT_READ 1
#define IFT_WRITE 2
#define IFT_DGDATA 3
#define IFT_SUBBUS 4

void new_interface( char *if_name ) {
  const char *cmd_class = 0;
  char *s;

  if_list_t *new_if;
  new_if = (if_list_t *)new_memory(sizeof(if_list_t));
  if (if_last == NULL ) {
    if_list = new_if;
  } else {
    if_last->next = new_if;
  }
  if_last = new_if;
  new_if->next = NULL;
  new_if->if_name = if_name;
  new_if->if_path = NULL;
  for ( s = if_name; *s; ++s ) {
    if ( *s == ':' ) {
      *s++ = '\0';
      new_if->if_path = s;
      break;
    }
  }
  if ( new_if->if_path ) {
    if ( strcmp( new_if->if_path, "DG/data" ) == 0 ) {
      cmd_class = "cmdif_dgdata";
      new_if->if_type = IFT_DGDATA;
    } else {
      cmd_class = "cmdif_wr";
      new_if->if_type = IFT_WRITE;
    }
  } else if ( stricmp( new_if->if_name, "subbus" ) == 0 ||
	      stricmp( new_if->if_name, "subbusd" ) == 0 ) {
    new_if->if_type = IFT_SUBBUS;
    fprintf( ofile, "#include \"subbus.h\"\n" );
  } else {
    cmd_class = "cmdif_rd";
    new_if->if_type = IFT_READ;
  }
  if (cmd_class) {
    fprintf( ofile, "#ifdef SERVER\n" );
    switch (new_if->if_type) {
      case IFT_WRITE:
	fprintf( ofile, "  %s if_%s(\"%s\", \"%s\");\n",
	      cmd_class, if_name, if_name, new_if->if_path );
	break;
      case IFT_DGDATA:
	fprintf( ofile, "  %s if_%s(\"%s\", &%s, sizeof(%s));\n",
	  cmd_class, if_name, if_name, if_name, if_name );
	break;
      case IFT_READ:
	fprintf( ofile, "  %s if_%s(\"%s\");\n",
	  cmd_class, if_name, if_name );
	break;
      default:
	nl_error(4,"Invalid if_type %d", new_if->if_type);
    }
    fprintf( ofile, "#endif\n" );
  }
}

void output_interfaces(void) {
  if_list_t *cur_if;
  fprintf( ofile, "\n#ifdef SERVER\n" );
  fprintf( ofile, "  void cis_interfaces(void) {\n" );
  for ( cur_if = if_list; cur_if; cur_if = cur_if->next ) {
    switch (cur_if->if_type) {
      case IFT_READ:
      case IFT_WRITE:
	fprintf( ofile, "    if_%s.Setup();\n", cur_if->if_name );
	break;
      case IFT_DGDATA:
      case IFT_SUBBUS:
	break; // initialization is handled by subbus.oui
      default:
	nl_error(4, "Unexpected interface type: %d", cur_if->if_type );
    }
    // fprintf( ofile, "    if_%s = cis_setup_rdr(\"%s\");\n",
    //    cur_if->if_name, cur_if->if_name );
  }
  fprintf( ofile, "  };\n\n" );
  fprintf( ofile, "  void cis_interfaces_close(void) {\n" );
  for ( cur_if = if_list; cur_if; cur_if = cur_if->next ) {
    switch (cur_if->if_type) {
      case IFT_READ:
      case IFT_WRITE:
      case IFT_DGDATA:
	fprintf( ofile, "    if_%s.Shutdown();\n", cur_if->if_name );
	break;
      case IFT_SUBBUS:
	fprintf( ofile, "    subbus_quit();\n" );
	break;
      default:
	nl_error(4, "Unexpected interface type: %d", cur_if->if_type );
    }
  }
  fprintf( ofile, "  }\n#endif\n\n" );
}
