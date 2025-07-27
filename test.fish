# Native 'if' definitions
True.define "?" ["self" "a" "b"] { a.eval }
True.define "&&" ["self" "b" ] { b.eval }
True.define "||" ["self" "b" ] { self. }     # Notice the magical dot to return a single value.

False.define "?" ["self" "a" "b"] { b.eval }
False.define "&&" ["self" "b" ] { self. }
False.define "||" ["self" "b" ] { b.eval }


# Define a method
Int.define "double" ["x"] { x + x }
21.double

# Define a lambda
define "dup" (bind ["x"] {x+x})   # Using two unqualified expressions that expand to env.define / env.bind
dup  21                           # Using the unqualified expression that expands to dup.apply 21

True ? { "yes\n".print; 42. }

define "i" 0
{ (i < 10) ? { i.print; "\n".print; set "i" (i + 1); True. } }.loop # TODO we now can also just make loop a lambda
 
[1 2].foreach (bind ["x"] { x.print; "\n".print })

# One way to call 'subclass'
define "Foo" ((Object.bind [] {
    define "foo" [] { 42 };
    env.
}).eval)
