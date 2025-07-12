Int define double ["x"] { x + x }
21 double
env define dup (env bind ["x"] {x+x})
dup apply 21

