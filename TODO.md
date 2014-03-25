* A CPU emulator
	* x86_64, only protected long mode
	* High level emulation starts at the "syscall" opcode
* Parsing ELF
* Implement a fake kernel to handle the syscalls
* An efficient merging pager
* Add a configurable way to intercept/trap/sandbox syscalls
	* Lua scriptable sandbox?
* Later:
	* `execv`, `fork`, mutexing emulator threads.
	* Sharing pages between pagers, virtual pages (mapped to real files/devices)
