# A nameless project for experimentation on debugging

To build the binary run

```
$ make
```

The output will be located `target/tracer`

You may want to compile 'simple_app.c' or 'code.s' to use as a debug target.
To attach to a target run

```
$ sudo ./target/tracer $(pgrep simple_app)
```

Currently only attaching to an existing program exists, I plan to implement process starting later.

To set a breakpoint, use a `breakpoint` command

```
trc> breakpoint my_println
breakpoint my_println was set
```

You can `disable` breakpoints

```
trc> disable my_println
breakpoint my_println was disabled
```

Run `start` to resume execution, it will stop when it hit a breakpoint, or you can exit the debugger using termination:

```
trc> start
resumed the task
```

When you're on a breakpoint you can `eval` expressions:

```
hit breakpoint at my_println
trc> eval fp
reg fp = 0
trc> eval x0
reg x0 = 4299571096
trc> eval 30
literal 30 = 30
trc>
```

And read a `string` from memory by the address of evaluated expression:

```shell
trc> string x0
 *(char*)4299571096 == "Hello, world"
```

**Note**: _Currently only register names and decimal integers are supported as expressions. But I plan to implement more general expressions._

## Dependencies

-   I use [mountainstorm/CoreSymbolication](https://github.com/mountainstorm/CoreSymbolication) headers and link to CoreSymbolication framework to read symbols.
