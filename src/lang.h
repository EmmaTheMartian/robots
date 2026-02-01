#ifndef __robots_lang__
#define __robots_lang__


#include <common.h>
#include <rendering.h>


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


typedef enum rbt_errcode
{
	/* misc errors (0-9) */
	rbt_errcode_unknown  = 0,
	rbt_errcode_internal = 1,
	/* syntax errors (100-199) */
	rbt_errcode_syn_register       = 100,
	rbt_errcode_syn_unknown_const  = 101,
	rbt_errcode_syn_invalid_value  = 102,
	rbt_errcode_syn_invalid_op     = 103,
	rbt_errcode_syn_fn_in_fn       = 104,
	rbt_errcode_syn_end_outside_fn = 105,
	/* misc errors (500+) */
	rbt_errcode_no_such_fn         = 500,
	rbt_errcode_fn_exists          = 501,
	rbt_errcode_invalid_argument   = 510,
} LangErr;

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
	rbt_op_add,
	rbt_op_sub,
	rbt_op_mul,
	rbt_op_div,
	rbt_op_mod,
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
	int robot; /* robot index */
	int registers[LANG_NREGS];
	int _nfns; /* number of functions */
	LangFn fns[LANG_NFNS];
	int _curfn; /* index to the current function being executed */
	bool errored;
	char error_msg[LANG_ERRORBUFSIZ];
	Renderer *_renderer;
} LangContext;

typedef struct rbt_stepper
{
	bool child;
	LangContext *ctx;
	char *program;
	unsigned n;
	long _lexpos;
} LangStepper;

/* Read local program.rbt. */
char *read_program(void);

/* Create a stepper to interpret the code line-by-line. */
LangStepper *make_stepper(int robot_id, char *program);
/* Step a stepper. */
bool stepper_step(State *state, LangStepper *ls, Renderer *renderer);
/* Reread the program and restart the stepper. */
void stepper_reload(LangStepper *ls);
/* Free a stepper. */
void del_stepper(LangStepper *ls);
/* Interpret the given code instantly. */
void interpret(State *state, LangContext *ctx, Renderer *renderer, char *program);
/* Create a new language context. */
LangContext *new_context(int robot_id);
/* Delete a language context. */
void del_context(LangContext *c);


#endif
