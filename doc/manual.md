# *Rapid Diassembly*

**On Sat, Jan 31, 2025 at 21:35, D. Adams &lt;dadams@&lsqb;redacted&rsqb;&gt; wrote:**

Your goal is simple: Disassemble enemy robots. We managed
to hack into a single robot and are able to reprogram it.
We are tasking you with writing this program and taking
down all enemy robots. Good luck.

Attached to this e-mail is a manual detailing the
specifications for this robot.

\- D. Adams

---

`robot_specifications.pdf`

# Robot Scripting Language

This manual shall function as your guide to

The *Robot Scripting Language* (henceforth RSL) is a mini
scripting language used to control robots in *Rapid
Disassembly*.

## §0: Table of Contents

1. [Syntax and Terms](#1-syntax-and-terms)
2. [Registers](#2-registers)
3. [Constants](#3-constants)
4. [Operations](#4-operations)
	1. [Forward](#41-forward)
	2. [Backward](#42-backward)
	3. [Turn](#43-turn)
	4. [Refuel](#44-refuel)
	5. [Ram](#45-ram)
	6. [Scan](#46-scan)
	7. [If](#47-if)
	8. [Set](#48-set)
	9. [New Function](#49-new-function)
	10. [End Function](#410-end-function)
	11. [Run Function](#411-run-function)
	12. [Add, Subtract, Multiply, Divide, and Modulo](#412-add-subtract-multiply-divide-and-modulo)
5. [Error Codes](#5-error-codes)

## §1: Syntax and Terms

Program: A series of statements, separated by newlines. \
Statement: An operation followed by up to 4 arguments. \
Expression: Either a register (`$0-15`), numeric literal
(`[0-9]+`), or a constant (see [§3 Constants](#3-constants)).

## §2: Registers

Robots are only capable of remembering up to 16 values,
stored in "registers." Ration them well!

## §3: Constants

There are 6 constants available in RSL:

| Name    | Description |
| ------- | ----------- |
| `none`  | Value to represent nothing. |
| `wall`  | Used by [scan](#4.1-scan) to indicate wall tiles. |
| `robot` | Used by [scan](#4.1-scan) to indicate robots. |
| `fuel`  | Used by [scan](#4.1-scan) to indicate fuel canisters. |
| `cw`    | Used with [turn](#4.2-turn) to turn clockwise, from above. |
| `ccw`   | Used with [turn](#4.2-turn) to turn counter-clockwise, from above. |

## §4: Operations

### §4.1: Forward

**Usage:** `forward`

Move the robot forward one tile, if able.

### §4.2: Backward

**Usage:** `backward`

Move the robot backward one tile, if able.

### §4.3: Turn

**Usage:** `turn cw|ccw`

Rotate the robot.

From above, a `turn cw` will rotate the robot clockwise
whereas a `turn ccw` will rotate the robot counter-clockwise.

### §4.4: Refuel

**Usage:** `refuel`

Requires that the robot be on top of a fuel canister. \
Consumes the fuel canister and restores 25 fuel points.

### §4.5: Ram

**Usage:** `ram`

Rams into the tile that the robot is looking at. If this is
an enemy robot, it will be disassembled. Turns out, robots
are terrifyingly hard hitters.

### §4.6: Scan

**Usage:** `scan $REGISTER`

Analyzes the tile in front of the robot and sets the value
in `$REGISTER` to a constant pertaining to the tile.

Walls will set `$REGISTER` to `wall`, air to `none`, other
robots to `robot`, and fuel canisters to `fuel`.

### §4.7: If

**Usage:** `if A OPERATION B then STATEMENT`

Perform a comparison between the expressions `A` and `B`.
If this comparison results in `true`, then `STATEMENT` is
executed.

**Operations:** \
*Equality:* `==` \
*Inequality:* `!=` \
*Greater Than:* `>` \
*Less Than:* `<` \
*Greater Than or Equal:* `>=` \
*Less Than or Equal:* `<=`

Remember that the expression may be the value contained in
a register (`$0-15`), a whole integer, or a constant.

### §4.8: Set

**Usage:** `set $REGISTER VALUE`

Set the value in `$REGISTER` to the provided `VALUE`
expression.

### §4.9: New Function

**Usage:** `fn NAME`

Mark the beginning of a function with the given name.

<!-- Interestingly, robots are capable of running functions
instantaneously. They will, however, still consume the
same amount as if you ran each command individually.

Additionally, it may be more difficult to debug the robot
when each command is executed in such rapid succession. -->

*Sample:*
```
fn hello
	forward
	turn right
end

run hello
```

### §4.10: End Function

**Usage:** `end`

Mark the end of a function.

### §4.11: Run Function

**Usage:** `run NAME`

Run the function with the given name.

### §4.12: Add, Subtract, Multiply, Divide, and Modulo

**Usage:** (respectively) `add|sub|mul|div|mod $REGISTER VALUE`

Take the value in the provided `$REGISTER` and calculate it
with the provided `VALUE` expression. The result is put back
into the `$REGISTER`.

## §5: Error Codes

| Code | Description |
| ---- | ----------- |
| 0-9  | Internal errors (i.e, a problem with the game) |
| 100+ | Syntax errors |
| 100  | Register-related syntax error. You likely forgot a `$` where it was required or you tried to access a non-existent register. Remember that there are only 16 and indexes start at 0. |
| 101  | Unknown constant. Check your spelling. |
| 102  | Expression/value syntax error. |
| 103  | Invalid operation, check §4 for a list of operations. Check your spelling. This may also occur when an if statement's operation is wrong (`==`, `!=`, etc). |
| 104  | You attempted to create a function inside a function. RSL doesn't support higher-order functions. |
| 105  | `end` was found outside of a function. |
| 500+ | Misc errors. |
| 500  | No such function. Check your spelling. |
| 501  | You attempted to create a function with a name that is already in use. |
| 510  | Invalid operation argument. Check for typos. |
