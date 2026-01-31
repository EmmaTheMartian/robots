#ifndef __robots_lang__
#define __robots_lang__


#include <common.h>


#ifndef LANG_NREGS
# define LANG_NREGS 16
#endif

#ifndef LANG_NFNS
# define LANG_NFNS 256
#endif

#ifndef LANG_ERRORBUFSIZ
# define LANG_ERRORBUFSIZ 2048
#endif

#ifndef LANG_PROGRAMPATH
# define LANG_PROGRAMPATH "program.rbt"
#endif

#ifndef LANG_MAXARGC
# define LANG_MAXARGC 4
#endif


typedef enum rbt_op
{
	rbt_op_err,
	rbt_op_print,
	rbt_op_forward,
	rbt_op_backward,
	rbt_op_turn,
	rbt_op_refuel,
	rbt_op_ram,
	rbt_op_scan,
	rbt_op_run,
	rbt_op_if,
	rbt_op_set,
	rbt_op_fn,
	rbt_op_end,
} LangOp;


typedef struct rbt_instruction
{
	LangOp op;
	char *args[LANG_MAXARGC];
} LangIns;

typedef struct rbt_fn
{
	char *name;
	int _codelen;
	int _codecap;
	LangIns *code;
} LangFn;

typedef struct
{
	int robot_id;
	int registers[LANG_NREGS];
	int _nfns; /* number of functions */
	LangFn fns[LANG_NFNS];
	int _curfn; /* index to the current function being executed */
	bool errored;
	char error_msg[LANG_ERRORBUFSIZ];
} LangContext;


/* Read local program.rbt. */
char *read_program(void);
/* Interpret the given code. */
void interpret(State *state, LangContext *ctx, char *program);
/* Create a new language context. */
LangContext new_context(void);
/* Delete a language context. */
void del_context(LangContext *c);


#endif
