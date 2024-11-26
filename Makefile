all: vm asm

vm:
	gcc hack_vm/hack_vm.c -o hack_vm

vm-debug:
	gcc hack_vm/hack_vm.c -o hack_vm -g -gdwarf-4 -fno-inline -fno-omit-frame-pointer -O0

asm:
	gcc hack_assembler/hack_assembler.c -o hack_assembler

asm-debug:
	gcc hack_assembler/hack_assembler.c -o hack_assembler -g -gdwarf-4 -fno-inline -fno-omit-frame-pointer -O0
