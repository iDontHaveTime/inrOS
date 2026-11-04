#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#define pic_get_symbol_rip_relative(symbolName, variable) \
    asm("leaq " #symbolName "(%%rip), %0\n" : "=r"(variable))
#define pic_move64_symbol_rip_relative(symbolName, variable) \
    asm("movq " #symbolName "(%%rip), %0\n" : "=r"(variable))

#endif // KERNEL_PIC_H
