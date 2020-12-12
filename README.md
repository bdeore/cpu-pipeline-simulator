## 5 Stage CPU Pipeline Simulator

A simulator for 5 stage APEX out-of-order cpu pipeline with data forwarding. Pipeline design includes unified physical
register file with 48 registers, a 64-entry reorder buffer and a centralized issue queue (IQ) with 24 entries. Pipeline
Stages are: Fetch, Decode, Execute, Memory and Writeback. Function Units: 1) JBU1 & JBU2 (for handling branch
instructions). 2) INTU (for integer and logical operations) 3) MUL (for integer multiplication with 3 cycle latency) 4)
M1 and M2 (for memory operations).

### How to compile and run

``
make all
``

### To Run:

``
./apex_sim <input_file_name>
``

### Simulator Commands:

``
[initialize | init]     - to initialize cpu
``

``
[s|Simulate <count>]    - to simulate <count> cycles
``

``
[d|Display]             - to display stage contents
``

``
[showmem <address>]     - to show contents in memory <address>
``

``
[PrintROB | print_rob]  - to print contents of ROB
``

``
[PrintIQ | print_iq]    - to print contents of Issue Queue
``

``
[n|next]                - proceed by one cycle
``


