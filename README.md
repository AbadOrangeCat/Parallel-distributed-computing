# Parallel & Distributed Computing — a tiny parallel build demo (C++)

This repository is a small learning project about **running many dependent tasks at the same time**.
It shows how a simple C++ program can read a Makefile-style file and decide which commands can run in parallel.

If you are not a programmer, do not worry.
Think of it like cooking.
Some steps can happen at the same time, but some steps must wait for others.

---

## What you will learn here

This repo is meant to be read like a short story:

- First, you run a working demo.
- Then, you learn the idea behind it.
- Finally, you look at how the code connects to that idea.

Along the way you will learn:

- What a **dependency** is, and why it matters.
- How a **Makefile** describes a dependency map.
- How a C++ program can create several **worker threads** to run tasks in parallel.
- How those workers coordinate with a **mutex** and a **condition variable**.

---

## Quick start (copy‑paste)

To run the main demo you need:

- A terminal (Command Prompt / PowerShell is fine).
- A C++ compiler.
  - Linux: usually `g++` is available.
  - macOS: install “Xcode Command Line Tools”.
  - Windows: use WSL, or install a toolchain like MSYS2.

This project contains an example program called `assignment.cpp`.
It reads `MakefileComplex` and runs the build commands inside it.

Before compiling, there is one small portability fix.
On Linux/macOS the file name is case-sensitive.
So you may need to change one header name:

- In `assignment.cpp`, replace `#include <Mutex>` with `#include <mutex>`.

Now you can run the demo:

```bash
git clone https://github.com/AbadOrangeCat/Parallel-distributed-computing.git
cd Parallel-distributed-computing

# Linux only: one-line fix
sed -i 's/<Mutex>/<mutex>/' assignment.cpp

# macOS alternative:
# sed -i '' 's/<Mutex>/<mutex>/' assignment.cpp

# build and run
g++ -std=gnu++17 -pthread assignment.cpp -o assignment
./assignment
```

You should see logs similar to this:

```text
Reading......
Start Compile......
Compiling f.o using 	g++ -o f.o f.cpp
...
Finished Compile......
```

After that, new files such as `aa.out`, `a.o`, `b.o`… will appear in the folder.
These are build outputs.

To clean them:

```bash
rm -f *.o aa.out assignment
```

You have now seen the project working.
In the next section we explain what just happened, using plain language.

---

## The big idea (plain language)

When a program is split into many source files, you often need to **build** it.

- **Build** means: turn human-readable code into a runnable program.
- **Compile** is the main step of building.
- A **compiler** is the translator.

A build usually has many small steps.
Some steps can run at the same time.
That is the “parallel” part.

The hard part is **dependencies**.

- A **dependency** is something that must be ready before a task can start.
- Example: you cannot bake before you mix the batter.

A good build tool uses the dependency map to start tasks early when it is safe.
That is what this repo demonstrates.

One more note about the project name:

- **Parallel** usually means “many workers on one machine”.
- **Distributed** usually means “many machines working together over a network”.

This repo focuses on the first part.
It uses threads on one computer.
It is a good first step toward distributed ideas.

---

## How the Makefile-style inputs work

This repository uses simple text files that look like a Makefile.
A Makefile is a “recipe” that answers two questions:

1. What depends on what?
2. What command should we run to produce each result?

A rule has two parts:

1. A **target line**:

   `target : dependency1 dependency2`

2. A **command line** (usually indented with a tab):

   `g++ ...`

If you have never seen this format before, start with the next file.
It is the main input used by the demo.

### Example: `MakefileComplex`

`MakefileComplex` is the file that `assignment.cpp` reads by default.
It describes a set of build steps.

Here is the dependency story inside it:

- `aa.out` is the final output.
- It depends on intermediate outputs like `a.o`, `b.o`, `c.o`, `d.o`, `e.o`, `f.o`.
- Some intermediate outputs also depend on other outputs.

A simplified view looks like this:

```text
aa.out
├─ a.o
│  ├─ b.o
│  └─ c.o
├─ b.o
├─ c.o
├─ d.o
│  ├─ a.o
│  └─ e.o
├─ e.o
│  └─ f.o
└─ f.o
```

This map matters because it tells us which steps can run together.
For example, `b.o` and `f.o` do not depend on each other.
So a build tool can run them at the same time.

---

## How the parallel build program works

Now that you know what the input looks like, we can connect it to the code.

The main program is `assignment.cpp`.
It acts like a tiny “build scheduler”:

1. It reads `MakefileComplex` line by line.
2. It extracts three lists:
   - **targets** (what to produce)
   - **dependencies** (what must be done first)
   - **commands** (what to run)
3. It starts one **worker thread** per command.
4. Each worker waits until all its dependencies are marked as finished.
5. When ready, it runs the command using `system(...)`.

This is enough to demonstrate the core idea:
**run independent work in parallel, but still respect dependencies**.

### Coordination tools explained

Worker threads share memory.
So they need rules.

- A **mutex** is a lock.
  It makes sure only one worker edits shared data at a time.
- A **condition variable** is a doorbell.
  Workers can sleep while waiting, and wake up when something changes.

In this project, the shared data is a list of finished targets.
Workers wait until their dependencies appear in that list.

---

## Repository guide

If you just want to browse, start here:

- `assignment.cpp` — the main demo.
  A tiny “parallel make” that runs build commands using threads.
- `MakefileComplex` — the input file read by `assignment.cpp`.
- `Makefile` — a smaller, simpler input file.
- `convolutedDependency` — another input file.
  It creates a more tangled dependency map and uses `echo` commands.
- `c.cpp` — a small helper.
  It asks for a filename, compiles it, and runs the result.
  It shows how `system(...)` can call the compiler.
- `a.cpp`, `b.cpp`, `d.cpp`, `e.cpp`, `f.cpp` — tiny “Hello World” programs.
  They exist so the build steps have real files to work with.

You may also see files like `*.o`, `aa.out`, or `assignment.out` in the repository.
These are build outputs.
You can delete them at any time, and regenerate them by building again.

---

## Try your own experiment

Once the demo works, the fun part is changing the dependency map.

You can edit `MakefileComplex` or `convolutedDependency` to create your own rules.
Some simple ideas:

- Add a new target that depends on two others.
- Create two long independent chains and watch them run at the same time.
- Replace `echo` commands with small scripts to simulate “slow tasks”.

If you want `assignment.cpp` to read a different file, change this line:

```cpp
infile.open("MakefileComplex");
```

---

## Safety note

`assignment.cpp` and `c.cpp` both use `system(...)`.
That means they will run the commands written in the input file.

Only run this project with input files you trust.

---

## Limitations (by design)

This is a learning project.
So it keeps things simple.

- It runs on one machine with threads.
  It does not send work to multiple computers.
- It does not check file timestamps (real `make` does).
- It does not handle command failures in a strong way.

If you want to extend it, good next steps are:

- Mark a target as finished only after its command completes.
- Use `std::vector<std::thread>` instead of a variable-size array.
- Add a “max workers” option (like `make -j 8`).

---

## Key words (plain explanations)

- **Parallel**: doing multiple tasks at the same time on one computer.
- **Distributed**: doing tasks across multiple computers over a network.
- **Thread**: a worker inside a program.
- **Dependency**: “this must be done first”.
- **Makefile**: a text recipe for building things.
- **Target**: the thing you want to produce.
- **Command**: the line you want the terminal to execute.
- **Mutex / Condition variable**: tools that help threads coordinate safely.
