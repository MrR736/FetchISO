#ifndef FETCHISO_PRIVATE_HH
#define FETCHISO_PRIVATE_HH

#include <string>
#include <unordered_map>

#define FETCHISO_WIDTH  500
#define FETCHISO_HEIGHT 500

#define FETCHISO_DIALOG_WIDTH  500
#define FETCHISO_DIALOG_HEIGHT 250

static inline std::unordered_map<std::string, std::string> arch_map = {
	{"all", "All Architectures"},
	{"amd64", "x86_64 (64-bit)"},
	{"i386", "x86 (32-bit)"},
	{"aarch64", "ARM64 (AArch64)"},
	{"armhf", "ARMv7 Hard Float"},
	{"armel", "ARMEL"},
	{"mips64el", "MIPS64EL"},
	{"mipsel", "MIPSEL"},
	{"ppc64el", "PowerPC64LE"},
	{"ppc64", "PowerPC (64-bit)"},
	{"ppc", "PowerPC (32-bit)"},
	{"ppcspe", "PowerPC SPE"},
	{"s390x", "IBM Z (s390x)"},
	{"riscv64", "RISC-V 64-bit"}
};

#endif // FETCHISO_PRIVATE_HH
