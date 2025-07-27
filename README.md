# SmallFish
The Pocket Object Engine

SmallFish combines a Selfish style syntax with Smalltalk principles, such that:
- Everything is an object
- Every object has a class that defines its capabilities (= methods)
- Code consists of objects and messages (= method calls)

## Expression Qualification
Qualified expressions specify their target object, using a dot syntax for all
but operators:

        Int.define "double" ["x"] { x + x }
        21.double

Unqualified expressions have no dot, and may be read in one of two different
ways, the latter taking precedence:

        # Shorthand for `env.define "dup" (env.bind ...`
        define "dup" (bind ["x"] { x + x })

        # Shorthand for `dup.apply 21`
        dup 21

The method shorthand takes some of the awkwardness out of local OO style calls.
The local lambda invocation shorthand makes for a LISPy code style (that I like
to think of as 'FishLips'). Both styles effectively work without compromise,
and even complement each other instead. For instance, lambdas may help to keep
situation-specific methods away from classes; and a single lambda function may
automatically work on different types of input.

## Overloading
At the moment neither (multi-argument) methods nor lambdas are overloaded, but
this might change in the future. It should be equal parts interesting and
confusing to have full support for both class-based methods and multimethods.

## Other things to type
In addition to the above examples, try:

        help                   -> shows a help text
        ls                     -> shows the current context
        Int.ls                 -> shows methods for integers
        define "x" 7           -> define current env value 'x'
        x * (2+4)              -> nested expressions require brackets
        x.                     -> use dot to return a single value

        gc.apply               -> does a garbage collection round
        { 1 + 1 }              -> constructs a block == argless lambda
        [1 2 3].               -> constructs a value array


## Specific TODOs
- Object instantiation: as always, I seem to leave the creation of new compound
  objects to the very last.

