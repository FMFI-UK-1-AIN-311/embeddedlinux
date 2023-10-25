Automating the build
====================

So far we manually entered all the commands and configured, built and set up
everything "manually". While this is OK for initial testing / playing aroud, it
get's very annoying and error prone once we need to test with different
configurations etc.

One way of automating this is to just write all the commands inside a script
(make sure to exit when a command fails, and continue on to others ;-).
The main disadvantage of this approach is that all the commands will be run.
Adding conditionals to not run things that are not needed (i.e. we didn't change
the kernel configuration, so don't need to recompile that... we don't need to
re-download the RPi firmware files, because we already have them...) and
checking for failures can get complicated quickly...

Just `make` it
--------------

`make` is a tool designed to handle exactly such cases: it allows to easily
define depencies between task and files, automatically stops on first error etc.

We have already used when building everything so far, becuase it is used by
most linux / unix build systems (such as the KBuild used in kernel/busybox, or
autotools / autoconf / automake used in alsa libs and mpg123). These tools
generate a complex configuration for `make` and then use it as the program
to "figure out what to build and in which order".

`make` is by defualt configured in a file called `Makefile` and it by default
tracks how specific files are created from other files and by which commands.

```Makefile
.PHONY: default
default: page.html

page.html: page.php header.php
	php page.php >page.html

page.txt: page.html
	lynx -dump page.html >page.txt

.PHONY: show
show: page.txt
	@echo "Generated plaintext:"
	@cat page.txt
```

This example Makefile says that a `page.html` file can be created from
`page.php` and `header.php` by running the command `php page.php > page.hml`.
Additionally a `page.txt` file can be created from that `page.html` by running
the `lynx` command (which a console / text based browser, that can show html
files as plain text, with some limitation of course ;).

Apart from the dependency chain, make checks the file times to determine, if the
commands need to be re-executed to re-generate the files if the inputs changed.
If the target (output) file timestamp is newer than the timestamps of all its
dependencies, the it won't run the command. If one of the dependencies is newer
(or the target doesn't exist) then it will run the commands to (re-) create it.

Thus if we run `make page.txt` the first time (assuming the php files are there,
but no `page.html` and `page.txt`), it will execute `php` and `lynx`. If we run
`make page.txt` again, it will just say everything `is up to date`. If we modify
one of the php files, it will re-run the commands again. If the dependency
graph was not linear, it would re-generated just the files that need "updating".


Automating the RPi build with `make`
------------------------------------

TODO

There is a [makefile in the old labs 8](../lab08/Makefile) that automates
everything we did so far. Go, check it out and study it...

Note: The version and urls are old, so might need changing...
