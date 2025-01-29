This is a simple virual turing machine that takes a description of a turing machine, an inital state, and an initial tape and then emulates that turing machine. It takes a toml file through stdin to define the turing machine. Here is an example:

```
initial_tape = "0000011"
initial_state = "q0"
[q0]
0 = {write = "1", direction ="R", next_state = "q0"}
1 = {write = "0", direction = "R", next_state = "q1"}

[q1]
0 = {write = "1", direction = "R", next_state = "q2"}
1 = {write = "0", direction = "R", next_state = "halt"}

[halt]
```
New tape cells are initialized with '_'. You can use any ansi character, not just 1's and 0's. If the machine encounters a symbol for which there is no transition rule, it will halt.
