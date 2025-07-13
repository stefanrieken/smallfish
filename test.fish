# Define a method
Int define double ["x"] { x + x }
21 double

# Define a lambda
env define dup (env bind ["x"] {x+x})
dup apply 21

# TODO need to have working sequences to do loops
#define "i" 0
#{ (i < 10) ? { set "i" (i + 1); True }
