# ycc: A small and simple C compiler

This is a small and simple C compiler called `ycc`. This compiler is based on the [compilerbook](https://www.sigbus.info/compilerbook) written by [@rui314](https://github.com/rui314).

## Usage

This compiler supports only Intel style assembly. So you need to use Docker environment to compile and run the code.

```sh
./scripts/build.sh
```

To build the compiler, run:

```sh
./scripts/docker_run.sh make
```

To compile and run a simple C program, use the following commands:
```sh
$ ./scripts/docker_run.sh ./ycc 'int main() { return 42; }' > tmp.s
$ ./scripts/docker_run.sh cc -o tmp tmp.s
$ ./scripts/docker_run.sh ./tmp; echo $?
42
```

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.
