# Forth Editor

SimForth: the Forth interpreter.

Application: class allowing to create multiple GTK+ windows.

BaseWindow: interface class offering a Gnome look for derived windows. In the SimTaDyn project the map editor also inherits from this class.

ForthWindow: concrete implementation of BaseWindow. Manages multiple Forth document, show the dictionary, stacks ...

TextEditor: class allowing to edit text files (save, load, find/replace words, undo ...).

ForthEditor: override TextEditor specialized for Forth (syntaxic coloration, auto completion ...)

ForthInspector: allow to display information from the Forth interpreter (stack, dictionary ...)
