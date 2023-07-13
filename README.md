# ratsh
Hobby shell for Unix-based systems


## Objectives
- Become more educated in programming language theory
- Gain a better understanding of the capabilities of Linux processees, the system calls available, etc.
- Implement a subset of the features provided by POSIX-compliant shells

## Installation

### Dependencies

#### Ubuntu
```console
sudo apt-get install cmake g++-12 ninja-build
```

For installation, clone the repository and use the following in the root directory of the project:

`cmake --build build --config Debug --target all`

To run, use the following command:

`./build/main`

This project mainly exists just for the purposes of fun and education, but I do wish to see it provide much of the convience and features that we see in the shells we use reguarly.
