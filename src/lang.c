#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <lang.h>
#include <rendering.h>
#include <audio.h>


#ifndef LANG_ARGBUFSIZ
# define LANG_ARGBUFSIZ 256
#endif


#if DEBUG_GAME
# define log(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)
#else
# define log(fmt, ...) do { } while(0)
#endif


enum rbt_const
{
	rbt_const_none,
	/* tiles */
	rbt_const_wall,
	rbt_const_robot,
	rbt_const_fuel,
	/* directions */
	rbt_const_north,
	rbt_const_south,
	rbt_const_east,
	rbt_const_west,
	rbt_const_left,
	rbt_const_right,
};


/* op to string */
char *rbt_optos[] =
{
	"err",
	"print",
	"forward",
	"backward",
	"turn",
	"refuel",
	"ram",
	"scan",
	"run",
	"if",
	"set",
	"fn",
	"end",
	"add",
	"sub",
	"mul",
	"div",
	"mod",
	NULL,
};

/* const to string */
char *rbt_consttos[] =
{
	"none",
	"wall",
	"robot",
	"fuel",
	"north",
	"south",
	"east",
	"west",
	"left",
	"right",
	NULL,
};


struct rbt_opinfo
{
	int argc;
	char *usage;
};

struct rbt_opinfo rbt_ops[] =
{
	[rbt_op_err]      = { .argc=0 },
	[rbt_op_print]    = { .argc=1, .usage="print [VALUE]" },
	[rbt_op_forward]  = { .argc=0, .usage="forward" },
	[rbt_op_backward] = { .argc=0, .usage="backward" },
	[rbt_op_turn]     = { .argc=1, .usage="turn left|right" },
	[rbt_op_refuel]   = { .argc=0, .usage="refuel" },
	[rbt_op_ram]      = { .argc=0, .usage="ram" },
	[rbt_op_scan]     = { .argc=1, .usage="scan REGISTER" },
	[rbt_op_run]      = { .argc=1, .usage="run FUNCTION" },
	[rbt_op_if]       = { .argc=4, .usage="if VALUE OPERATION VALUE then STATEMENT" },
	[rbt_op_fn]       = { .argc=1, .usage="fn NAME" },
	[rbt_op_end]      = { .argc=0, .usage="end" },
	[rbt_op_add]      = { .argc=2, .usage="add REGISTER N" },
	[rbt_op_sub]      = { .argc=2, .usage="sub REGISTER N" },
	[rbt_op_mul]      = { .argc=2, .usage="mul REGISTER N" },
	[rbt_op_div]      = { .argc=2, .usage="div REGISTER N" },
	[rbt_op_mod]      = { .argc=2, .usage="mod REGISTER N" },
};


static
void print_ins(LangIns ins);


static
void panic(LangContext *ctx, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	snprintf(ctx->error_msg, sizeof(ctx->error_msg) - 1, fmt, ap);
	fprintf(stderr, "panic: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	ctx->errored = true;
#	if 1
	abort();
#	endif
}

static
LangFn *get_fn(LangContext *ctx, char *name)
{
	for (int i = 0 ; i < LANG_NFNS ; i++)
	{
		if (ctx->fns[i].name && strcmp(ctx->fns[i].name, name) == 0)
			return &ctx->fns[i];
	}
	return NULL;
}

static
int *get_reg(LangContext *ctx, char *text)
{
	if (!text)
	{
		panic(ctx, "get_reg: `text` was NULL.");
		return NULL;
	}
	if (text[0] != '$')
	{
		panic(ctx, "register argument must start with a dollar sign (`$`)");
		return NULL;
	}
	int reg = strtol(text+1, NULL, 10); /* +1 because of $ prefix. */
	log("register: %d=%d\n", reg, ctx->registers[reg]);
	return &ctx->registers[reg];
}

static
int eval_val(LangContext *ctx, char *val)
{
	if (!val)
		return -1;
	else if (val[0] == '$')
		return *get_reg(ctx, val);
	else if (isalpha(val[0]))
	{
		for (char **c = &rbt_consttos[0] ; c != NULL ; c++)
		{
			if (strcmp(*c, val) == 0)
			{
				return c - &rbt_consttos[0];
			}
		}
		panic(ctx, "unknown constant: `%s`", val);
		return -1;
	}
	else if (isdigit(val[0]) || val[0] == '-')
	{
		return strtol(val, NULL, 10);
	}
	else
	{
		panic(ctx, "invalid value: `%s`", val);
		return -1;
	}
}

static
void eval_ins(State *state, LangContext *ctx, Renderer *renderer, struct rbt_instruction ins)
{
	Robot *r = &state->robots[ctx->robot];
	RobotVisual *rv = renderer_get_visual(renderer, ctx->robot);

	log("&state(%p)->robots[ctx->robot(%d)] = %p\n", state, ctx->robot, r);

	// print_ins(ins);
	switch (ins.op)
	{
	case rbt_op_print:
		{
		int n = ins.args[0] ? eval_val(ctx, ins.args[0]) : 1;
		printf("[PRINT] %d\n", n);
		break;
		}
	case rbt_op_forward:
	case rbt_op_backward:
		{
		bool moved = (ins.op == rbt_op_forward) ?
			robot_forward(state->world, r) :
			robot_backward(state->world, r);
		if (moved)
		{
			robot_visual_move_to(rv, r->x, r->y);
			play_sfx((ins.op == rbt_op_forward) ?
					SFX_ADVANCING :
					SFX_REVERSE);
		}
		else if (!robot_visual_is_animating(rv))
			robot_visual_ram(rv, r->dir); /* ram to indicate that the player can't move there. */
		}
		break;
	case rbt_op_turn:
		if (eval_val(ctx, ins.args[0]) == rbt_const_left)
			robot_turn_left(r);
		else if (eval_val(ctx, ins.args[0]) == rbt_const_right)
			robot_turn_right(r);
		else
		{
			panic(ctx, "turn expects argument to be either `left` or `right`.");
			break;
		}
		play_sfx(SFX_ROTATING);
		robot_visual_rotate_to(rv, r->dir);
		break;
	case rbt_op_refuel:
		if (get_tile(state->world, r->x, r->y) == 0) /* todo: replace 0 with a tile ID */
			robot_refuel(r, 100); /* todo: `100` is totally arbitrary. */
		break;
	case rbt_op_ram:
		break; /* todo */
	case rbt_op_scan:
		{
		int *reg = get_reg(ctx, ins.args[0]);
		int *tile = get_tile_with_offset(state->world, r->x, r->y, r->dir);
		*reg = *tile;
		break;
		}
	case rbt_op_run:
		{
		char *name = ins.args[0];
		LangFn *fn = get_fn(ctx, name);
		if (!fn)
		{
			panic(ctx, "no such function: `%s`", name);
			break;
		}
		log("run: %s\n", name);
		for (int i = 0 ; i < fn->_codelen ; i++)
		{
			eval_ins(state, ctx, renderer, fn->code[i]);
		}
		break;
		}
	case rbt_op_if:
		{
		int a = eval_val(ctx, ins.args[0]), b = eval_val(ctx, ins.args[2]);
		enum { eq, neq, gt, lt, gteq, lteq } op = -1;
		static char *ops[] =
		{
			"==",
			"!=",
			">",
			"<",
			">=",
			"<=",
			NULL
		};
		for (char **p = &ops[0] ; p != NULL ; p++)
		{
			if (strcmp(*p, ins.args[1]) == 0)
			{
				op = p - &ops[0];
				break;
			}
		}
		if (op == -1)
		{
			panic(ctx, "invalid operation: `%s`", ins.args[1]);
			break;
		}
		bool res = false;
		switch (op)
		{
		case eq:   res = a == b; break;
		case neq:  res = a != b; break;
		case gt:   res = a > b; break;
		case lt:   res = a < b; break;
		case gteq: res = a >= b; break;
		case lteq: res = a <= b; break;
		}
		if (res)
		{
			interpret(state, ctx, renderer, ins.args[3]);
		}
		break;
		}
	case rbt_op_set:
		{
		int *reg = get_reg(ctx, ins.args[0]);
		int val = eval_val(ctx, ins.args[1]);
		if (reg)
		{
			log("set %s = %d\n", ins.args[0], val);
			*reg = val;
		}
		break;
		}
	case rbt_op_fn:
		{
		if (ctx->_curfn > -1)
		{
			panic(ctx, "cannot create function inside function");
			break;
		}

		/* duplicate name since args get freed. */
		int n = strlen(ins.args[0]);
		char *name = malloc(n+1);
		memcpy(&name[0], ins.args[0], n);
		name[n] = '\0';

		/* check if this function already exists */
		LangFn *fn = get_fn(ctx, name);
		if (fn)
		{
			panic(ctx, "function already exists: `%s`", name);
			free(name);
			break;
		}
		fn = &ctx->fns[ctx->_nfns];
		fn->name = name;
		fn->_codelen = 0;
		fn->_codecap = 32;
		fn->code = malloc(fn->_codecap * sizeof(LangIns));
		if (!fn->code)
		{
			panic(ctx, "failed to create function");
			free(name);
			break;
		}
		ctx->_curfn = ctx->_nfns;
		log("curfn: %d\n", ctx->_curfn);
		ctx->_nfns++;
		break;
		}
	case rbt_op_end:
		{
		if (ctx->_curfn == -1)
		{
			panic(ctx, "end is only allowed in functions");
			break;
		}
		ctx->_curfn = -1;
		break;
		}
	case rbt_op_add:
	case rbt_op_sub:
	case rbt_op_mul:
	case rbt_op_div:
	case rbt_op_mod:
		{
		int *r = get_reg(ctx, ins.args[0]);
		int n = eval_val(ctx, ins.args[1]);
		switch (ins.op)
		{
		case rbt_op_add: *r += n; break;
		case rbt_op_sub: *r -= n; break;
		case rbt_op_mul: *r *= n; break;
		case rbt_op_div: *r /= n; break;
		case rbt_op_mod: *r %= n; break;
		default: break; /* unreachable */
		}
		break;
		}
	default:
		panic(ctx, "invalid operation: %d (%s)", ins.op, rbt_optos[ins.op]);
		break;
	}
}

static
struct rbt_instruction parse_ins(LangContext *ctx, long *index, char *program)
{
	unsigned lineno = 0;
	long start = *index;
	char *ch = &program[*index];

	struct rbt_instruction ins = { .op=rbt_op_err };

#	define advance() (++(*index), ++ch)

	/* skip whitespace and comments */
	for (;;)
	{
		switch (*ch)
		{
		case ';':
			while (*ch != '\n')
				advance();
			break;
		case ' ':
		case '\t':
		case '\r':
			advance();
			break;
		case '\n':
			advance();
			lineno++;
			break;
		case '\0':
			*index = -1;
			return ins;
		default:
			goto done_skipping_ws;
		}
	}
done_skipping_ws:

	/* parse operation */
	char *startch = ch;
	while (isalpha(*ch))
		advance();
	int len = ch - startch;
	for (char **op_str = &rbt_optos[0] ; op_str != NULL ; op_str++)
	{
		if (strncmp(startch, *op_str, len) == 0)
		{
			int i = op_str - &rbt_optos[0];
			ins.op = (enum rbt_op)i;
			break;
		}
	}
	if (ins.op == rbt_op_err)
	{
		panic(ctx, "no such operation `%.*s`", len, startch);
		return ins;
	}

	/* parse arguments */
	int i = 0;
	for (i = 0 ; i < LANG_MAXARGC ; i++)
		ins.args[i] = NULL;
	i = 0;
	bool literal = false;
	while (*ch != '\n')
	{
		/* skip whitespace */
		while (isspace(*ch))
			advance(); /* eat spaces */
		if (*ch == '\n' || *ch == '\0')
			return ins;

		/* read argument */
		char *arg = malloc(LANG_ARGBUFSIZ);
		memset(arg, 0, LANG_ARGBUFSIZ);
		int n = 0;
		if (literal)
		{
			while (*ch != '\n' && *ch != '\0')
			{
				arg[n++] = *ch;
				advance();
			}
		}
		else
		{
			while (isgraph(*ch) && *ch != '\0')
			{
				arg[n++] = *ch;
				advance();
			}
		}
		/* anything after `then` or `:` is literal */
		if (strcmp(arg, "then") == 0 || strcmp(arg, ":") == 0)
		{
			literal = true;
			free(arg);
			continue;
		}
		ins.args[i++] = arg;

		/* bounds check */
		if (i > LANG_MAXARGC)
		{
			fprintf(stderr, "error: maximum arguments for an instruction is %d", LANG_MAXARGC);
			while (*ch != '\n')
				advance();
			return ins;
		}
	}

	/* todo: check that enough arguments were provided or print error with usage message. */

	return ins;

#	undef advance
}

static
void print_ins(struct rbt_instruction ins)
{
	printf("%s", rbt_optos[ins.op]);
	for (int i = 0 ; i < LANG_MAXARGC && ins.args[i] ; i++)
	{
		printf(" %s", ins.args[i]);
	}
	printf("\n");
}

static
void del_ins(struct rbt_instruction ins)
{
	for (int i = 0 ; i < LANG_MAXARGC && ins.args[i] ; i++)
	{
		free(ins.args[i]);
		ins.args[i] = NULL;
	}
}

char *read_program(void)
{
	/* check if program.rbt exists */
	struct stat st;
	if (stat(LANG_PROGRAMPATH, &st)) /* file does not exist */
	{
		FILE *fp = fopen(LANG_PROGRAMPATH, "w");
		if (!fp)
		{
			fprintf(stderr, "error: failed to create `" LANG_PROGRAMPATH "`, are you missing permissions?\n");
			return NULL;
		}
		fclose(fp);
	}

	FILE *fp = fopen(LANG_PROGRAMPATH, "r");
	if (!fp)
	{
		fprintf(stderr, "error: failed to read `" LANG_PROGRAMPATH "`, are you missing permissions?\n");
		return NULL;
	}
	/* get file size */
	fseek(fp, 0, SEEK_END);
	unsigned long fs = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	/* read file into buffer */
	char *s = malloc(fs+1);
	if (!s)
	{
		fclose(fp);
		fprintf(stderr, "error: failed to allocate buffer to read `" LANG_PROGRAMPATH "`\n");
		return NULL;
	}
	if (fread(&s[0], 1, fs, fp) != fs)
	{
		fclose(fp);
		free(s);
		fprintf(stderr, "error: failed to read `" LANG_PROGRAMPATH "`");
		return NULL;
	}
	s[fs] = '\0';

	fclose(fp);
	return s;
}

LangStepper *make_stepper(int robot_id, char *program)
{
	if (!program)
	{
		program = read_program();
		log("step interp: %s\n", program);
	}
	LangStepper *ls = malloc(sizeof(*ls));
	ls->ctx = new_context(robot_id);
	ls->program = program;
	ls->n = 0;
	ls->_lexpos = 0;
	return ls;
}

bool stepper_step(State *state, LangStepper *ls, Renderer *renderer)
{
	if (ls->_lexpos == -1)
		return false;

	ls->n++;
	LangIns ins;
	/* loop so that we can "skip" function definitions */
	while (ls->_lexpos != -1)
	{
		ins = parse_ins(ls->ctx, &ls->_lexpos, ls->program);
		if (ins.op == rbt_op_err)
			break;

		if (ls->ctx->_curfn > -1 && ins.op != rbt_op_end)
		{
			LangFn *fn = &ls->ctx->fns[ls->ctx->_curfn];
			if (fn->_codelen + 1 > fn->_codecap)
			{
				int newcap = fn->_codecap * 2;
				log("growing list: %d->%d\n", fn->_codecap, newcap);
				LangIns *newcode = realloc(fn->code, newcap * sizeof(LangIns));
				if (!newcode)
				{
					panic(ls->ctx, "failed to grow array for function code");
					break;
				}
				fn->code = newcode;
				fn->_codecap = newcap;
			}
			fn->code[fn->_codelen++] = ins;
		}
		else
		{
			eval_ins(state, ls->ctx, renderer, ins);

			if (ls->ctx->errored)
				return false;

			int op = ins.op;
			del_ins(ins);

			/* We want to "skip" over functions completely, so `end` won't cause a wait. */
			if (op == rbt_op_end)
				continue;

			return true;
		}
	}

	return false;
}

void stepper_reload(LangStepper *ls)
{
	int r = ls->ctx->robot;

	del_context(ls->ctx);
	free(ls->program);

	ls->ctx = new_context(r);
	ls->program = read_program();
	ls->n = 0;
	ls->_lexpos = 0;
}

void del_stepper(LangStepper *ls)
{
	del_context(ls->ctx);
	free(ls->program);
	free(ls);
}

void interpret(State *state, LangContext *ctx, Renderer *renderer, char *program)
{
	LangStepper *ls = make_stepper(0, program);
	while (stepper_step(state, ls, renderer)) ;
	del_stepper(ls);
}

LangContext *new_context(int robot_id)
{
	LangContext *c = calloc(1, sizeof(*c));
	c->robot = robot_id;
	c->_curfn = -1;
	return c;
}

void del_context(LangContext *c)
{
	for (int i = 0 ; i < LANG_NFNS ; i++)
	{
		if (c->fns[i].name)
		{
			free(c->fns[i].name);
			c->fns[i].name = NULL;
			for (int j = 0 ; j < c->fns[i]._codelen ; j++)
				del_ins(c->fns[i].code[j]);
			free(c->fns[i].code);
			c->fns[i].code = NULL;
		}
	}
	free(c);
}
