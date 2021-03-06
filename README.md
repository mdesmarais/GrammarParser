# Grammar Parser

Navigation

* [Introduction](#introduction)
* [Build from source](#build)
* [Usage](#usage)
* [In-depth developpment documentation](#indepth)
    * [Grammar format](#gformat)
    * [Grammar parsing](#gparsing)
    * [Error handling](#errorhandling)

Included dependencies

* [Catch2](https://github.com/catchorg/Catch2) (C++ unit test library)
* [MurmurHash3](https://github.com/PeterScott/murmur3) (hash library, original implementation has been slightly changed)
* [Rxi Log](https://github.com/rxi/log.c) (simple logging library)

## Introduction

The aim of this project is to practice programming in C and use knowledge I learned at the university. I decided to write
a grammar parser : given a grammar file, the software can checks if it respects the expected syntax and 
extracts items (rules and tokens) from it. An example of this syntax is given below.

Doxygen is used to build a documentation.  
I use my owns implementations of basic data structures (hash table, linked list). They are usable 
for all data types as long as they are comparable and/or hashable.  
I wrote some unit tests with Catch2 using the BDD style (Given / WHEN / THEN).

**Features**

* ✔ Errors detection
* ✔ Items extraction
* ✘ Grammar visualization with dot
* ✘ Parse text with a given grammar

## <a name="build"></a> Build from source

I use CMake to manage dependencies and build the project from sources.  
The use of a dedicated folder for the output build is preferred.

```bash
git clone git@github.com:mdesmarais/GrammarParser.git
cd GrammarParser
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=install/ ..
make
make install
```

By default, doxygen documentation won't be built. You can change this option in the CMakeCache file, variable *BUILD_DOC*.

You can also change variable's value *BUILD_TESTING* if you don't want to build tests.

Unit tests can be executed with cmake in the build directory with the command `ctest`.

## Usage

The software has no graphical interface, it must be started from a terminal. From the install directory, it can be
started with the command `./parser`. An optional argument can be added : it should be a path to a grammar file.
If no argument is given, then the program expects to receive the grammar from the stdin, like this : `./parser < examples/calc.g`

## <a name="indepth"></a>In-depth development documentation

### <a name="gformat"></a> Grammar format

All declarations must start with `%`, followed by a name and ended with a semicolon.


**Token declaration** `%TOKEN_NAME = value;` where value represents one of the following type.  
**Rule declaration** `%rule_name = value | value1 value2;`

Available value types

* A range is defined between square brackets : `[a-z]`, `[A-Z0-9]`, `[1-3]`
* String block starts and ends with a backtick : `` `This is a string` ``
* A reference to a token or a rule is simply done by writing its name : `TOKEN1`, `rule1`

A value can be suffixed with a quantifier from the following list

* `?` 0 or 1 occurrence
* `*` 0 or more occurrences
* `+` 1 or more occurrences

Tokens can't have a reference to a rule.  
Tokens must start with an uppercase letter and can't contain whitespaces.
Self-referencing tokens are not allowed ( ex : `%TOKEN1 = TOKEN1+;` ).

An example :

```
%INT=[0-9];
%NUMBER=INT+;
%STRING = [a-zA-Z]+;

%PLUS = `+`;
%SUB = `-`;
%MUL = `*`;
%DIV = `/`;

%expr = op PLUS op
    | op SUB op
    | op;

%op = op2 MUL op2
    | op2 DIV op2
    | op2;

%op2 = SUB INT | INT | SUB INT | STRING;
```

### <a href="gparsing"></a> Grammar parsing

Grammar parsing is the operation of extracting tokens and rules from a given source (file, stdin) and checking if there are valid or not.

Taking this grammar as an example (from Wikipedia) :

```
%s = f 
    | `(` s `+` f `)`;
%f = `1`;
```

**Step 1** : The first operation is to remove extra spaces (whitespaces, new lines, tabulations), the result for this input will be

```
%s = f | `(` s `+` f `)`; %f = `1`;
```

The input source will always be on a single line and we have the guarantee that each relevant element is separated with a space.

**Step 2** : The next operation will split the input into a list of items, string blocks won't be affected by this operation.

Here is the constant of the list after the split. A linked list is used to store items. To improve error handling, 
the list will not contain only strings : we use a structure that a made with a string, a line and a column. 
See [Error Handling](#errorhandling) part for more details.

| 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|
| `$s` | `=` | `f` | <code>&#124;</code> | `` `(` `` | `s` | `` `+` `` |

| 8 | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| `f` | `` `)` `` | `;` | `$f` | `=` | `` `1` `` | `;`

**Step 3** : The next operation is to read those items with something like an automata. The initial state is waiting 
for a declaration : the first current item's character must be a `%` and then we have two choices for the next item :

* If it starts with an uppercase letter, then it's a token
* If it starts with a lowercase letter, then it's a rule
* Otherwise it's an error, the type of the item is unknown

We have a function to extract a token and another one for a rule. They both require an iterator on the item list. Their behavior are similar : 

As we are in the case of a declaration, those states are waiting for an equal sign : if the list is empty or 
the current item is not the required character, then it's an error.

**Step 4** : The last step performs a resolution of references. In the given grammar, we have two references : 
rule **s** is referencing itself and rule **f**.

By allowing self-referencing rules, we can't check if the reference exists on the first read. To solve this problem, 
I use a hash table to store a pair (name of the element / pointer to the element) where an element can be either 
a token or a rule. During the resolution, we check for each reference if there are pointing to an existing element : 
if it's not true then an error will be returned.

### <a name="errorhandling"></a> Error handling (for grammar input)

When the given grammar has an invalid syntax or does not follow the rules, we must report to the user where is the error and what is it about.

After step 2, we can retrieve position (column and line) of each item in the given source. For the current example, it will be something like :

| Item | Position (line : column) |
|------|--------------------------|
| `$s` | 0 : 0 |
| `=` | 0 : 3 |
| `f` | 0 : 4 |
| <code>&#124;</code> | 1 : 4 |
| `` `(` `` | 1 : 6 |
| ... | ... |

If the rule *f* didn't exist, then the parser will return an error like this one : *Unknown rule ***f*** (0:4)*.
