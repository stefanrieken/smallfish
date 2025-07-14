# Native 'if' definitions
True define "?" ["self" "a" "b"] { a eval }
True define "&&" ["self" "b" ] { b eval }
True define "||" ["self" "b" ] { self }

False define "?" ["self" "a" "b"] { b eval }
False define "&&" ["self" "b" ] { self }
False define "||" ["self" "b" ] { b eval }


# Define a method
Int define "double" ["x"] { x + x }
21 double

# Define a lambda
env define "dup" (env bind ["x"] {x+x})
dup apply 21

True ? { "yes\n" print; 42 }

env define "i" 0
{ (i < 10) ? { i print; "\n" print; env set "i" (i + 1); True } } loop

env define "foo" (env bind ["x"] { x print })
foo apply 42
 
[1 2] foreach (env bind ["x"] { x print; "\n" print })

# One way to call 'subclass'
env define "Foo" ((Object bind [] {
    env define "foo" [] { 42 };
    env
}) eval)
