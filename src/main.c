/***************************************************
* Main Mila complier program                       *
****************************************************/
#include "global.h"
#include "util.h"
#include "mila.tab.h"
#include "tree.h"
#include "3ac.h"
#include "gen.h"
#include "optimize.h"
#include <ctype.h>

/* allocate global variables */
FILE * src;
FILE * out;
FILE * tm_file;

uint32_t lineno = 1;

/* allocate and set tracing flags */
int8_t trace_parse = FALSE;
int8_t trace_analyze = FALSE;
int8_t trace_code = FALSE;
int8_t trace_graph = FALSE;

#ifdef DEBUG
extern int TLVL;
#endif

void
print_usage()
{
    fprintf(stderr, "Mila compiler:\n");
    fprintf(stderr, "==============\n");
    fprintf(stderr, "mila [-pac] [-O optimization(0-2)] [-o OUTPUT FILE] [SRC FILE]\n");
    fprintf(stderr, "-p :  print syntax tree and symbol table after the parsing\n");
    fprintf(stderr, "-a :  print 3AC before any optimization phase\n");
    fprintf(stderr, "-c :  print comments in the final output\n");
    fprintf(stderr, "-O(0-2) :  optimization level, 0 - none, 1 - very little optimization (i.e. constant propagation), 2 - full (default)\n");
    fprintf(stderr, "-o :  tm asm code file, if not specified STDOUT is used.\n");
    fprintf(stderr, "-h :  print this help.\n");
    fprintf(stderr, "If no source is specified input is read from STDIN\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "--------\n");
    fprintf(stderr, "mila -o out.tm - source is read from STDIN, and full optimization is usedlike -O2\n");
    fprintf(stderr, "mila -pac -O1 -o out.tm src.mila - print AST, symbol table, 3AC, very little optimization\n");
}

int
main(int argc, char * argv[] )
{
    extern char *optarg;
    extern int optind;
    extern FILE * yyin;
    extern hasht_tab_t * nodes_tab;
#ifdef DEBUG
    TLVL = 10;
#endif
    int8_t c = 0;
    int32_t optlvl = 2;
    char *pgm = NULL;
    char *exe = NULL;
    char *grp = NULL;
    FILE *grphf = NULL;
    tree_node_t * syntax_tree = NULL;
    tac_code_t * ac3code = NULL;
    bb_code_t * bb = NULL;


    while ((c = getopt(argc, argv, "pach:O:o:")) != -1)
        switch (c) {
        case 'p':
            trace_parse = TRUE;
            break;
        case 'a':
            trace_analyze = TRUE;
            break;
        case 'c':
            trace_code = TRUE;
            break;
        case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
            break;
        case 'O':
            if(isdigit(optarg[0]))
            {
                optlvl = (int32_t)atoi(optarg);
            }
            break;
        case 'o':
            exe = optarg;
            break;
        default:
            print_usage();
            exit(EXIT_SUCCESS);
        }

    if(optind < argc)
    {
        pgm = argv[optind];
    }

    out = stderr; /* send to screen */

    if(pgm)
    {
        if(NULL == (src = fopen(pgm,"r")))
        {
            fprintf(stderr, "File %s not found\n", pgm);
            exit(EXIT_SUCCESS);
        }
        fprintf(out,"\nMila compiling: %s ...\n",pgm);
    }
    else
    {
        src = stdin;
    }
    
    if(0 != st_init(SYMBOL_TAB_SIZE))
    {
        fprintf(stderr, "Unexpected error\n");
        exit(EXIT_FAILURE);
    }

    if(0 != new_nodes(NODE_TAB_SIZE))
    {
        fprintf(stderr, "Unexpected error\n");
        exit(EXIT_FAILURE);
    }
    
    yyin = src;
    if(0 != yyparse((void *)&syntax_tree))
    {
        fprintf(out, ".... compilation failed.\n");
        if(pgm) fclose(src);
        exit(EXIT_SUCCESS);
    }
    if(pgm) fclose(src);
    
    if(trace_parse)
    {
        if(NULL == syntax_tree)
        {
            fprintf(stderr, "Syntax tree is empty!\n");
        }
        else
        {
            fprintf(out,"\nSyntax tree:\n\n");
             print_tree(syntax_tree);

            if(trace_graph)
            {
                if(NULL == (grphf = fopen(grp,"w")))
                {
                    fprintf(stderr, "File %s not found\n", grp);
                    exit(EXIT_SUCCESS);
                }
                print_graph(grphf, "prog", syntax_tree, nodes_tab);
                fclose(grphf);
            }
            
            fprintf(out,"\n\nSymbol Table:\n\n");
            print_symtab();
            fprintf(out,"\n\n");
        }
    }

    if(NULL == (ac3code = tac(syntax_tree)))
    {
        fprintf(stderr, "IR code generation failed .\n");
        exit(EXIT_SUCCESS);
    }

    bb = bb_build(ac3code);

    if(trace_analyze)
    {
        fprintf(out,"\n\nIR code with basic blocks:\n\n");
        print_bb(bb);
    }
    
    bb = optimize(bb, optlvl);

    if(trace_analyze)
    {
        fprintf(out,"\n\nIR code after optimization:\n\n");
        print_bb(bb);
    }

    if(exe)
    {
        fprintf(out, "\nBuilding executable %s ...\n", exe);
        if(NULL == (tm_file = fopen(exe,"w")))
        {
            fprintf(stderr, "Could not create executable %s \n", exe);
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        tm_file = stdout;
    }
    

    if(NULL == reg_alloc(bb))
    {
        fprintf(stderr, "Register allocation failed .\n");
        exit(EXIT_SUCCESS);
    }

    if(!gencode(bb))
    {
        fprintf(stderr, "Code generation failed .\n");
        exit(EXIT_SUCCESS);
    }


    if(trace_parse)
    {
        fprintf(out,"\n\nFinal Symbol Table:\n\n");
        print_symtab();
    }

    exit(EXIT_SUCCESS);
}

