# SmallFish
The Pocket Object Engine

In this particular incarnation of a Selfish-style language, I hope to finally
have taken lessons from both LISP and Smalltalk to heart. To wit:

## Lessons from LISP & Smalltalk
- "Living code", i.e.: code is expressed in terms of the language itself (as in
  LISP) _and evaluated_ in terms of the language itself (as in Smalltalk). 
- Argument expression evaluation is relegated to the primitives, resulting in
  textbook lazy / as-needed evaluation. (As theorized in LISP, but implemented
  by equivalent 'special functions'; then realized in Smalltalk by making
  everything evaluable, but obscured by more efficient equivalent byte code.) (TODO verify latest statement)
- As a result, subexpressions can substitute for one-liner blocks.
- Proper tactics for lexical / lambda scoping, as explored in Pasta.
- Different lookup paths for methods vs. blocks / lambdas.

## The Object Table
I am glad to have picked up on the idea of a central Object Table from Dan
Ingalls' papers on Smalltalk. This single point of registration has the listed
advantage of having only one pointer to change on relocation.

Yet as the same table also serves as a place to store basic metadata outside of
the object itself, it also allows for "random, raw" data to be assigned object
status, which I find a very beneficial feature.

(So far I haven't managed to apply another idea from the same source, which is
to group object allocations (or their pointers) in blocks that share a single
metadata record, or at least class pointer.)

## Lessons from Selflisp & Pasta
The above notwithstanding, I will claim some (semi) original ideas and features
for SmallFish:

- C-style syntax choices where possible
- Different brackets for different things: `(subexpression)`, `{block}`, `[array]`
- As `[array]` evals to self, no (fundamental) need for `quote`.
- Even though `(subexpressions)` eval lazily as well, retain `{block}` as a `doseq`.
- Blocks without `method` or `lambda` closure extend the current context.
- Solve the assignment problem by specifying slot accessors.  These point back
  to the dictonary. (Miraculously, this should not create GC complications.)

## Status
SmallFish can now parse simple nested expressions on the REPL:

        help            -> shows a help text
        env ls          -> lists the root context
        Int ls          -> shows methods on integers
        env define x 7  -> define env value 'x'
        x * (2+4)       -> shows the meaning of life

        gc apply        -> does a garbage collection round
        [1 2 3]         -> constructs a value array

## Specific TODOs
- Finalize type hierarchy in dictionaries
- Booleans and conditionals
- Obviously, defining and calling methods, lambdas
