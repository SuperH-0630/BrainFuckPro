#ifndef BRAINFUCK_MEM_H
#define BRAINFUCK_MEM_H

#define calloc(n, size) (bf_safe_calloc((n), (size)))
#define free(p) ((p != NULL) ? free(p) : NULL)
void *bf_safe_calloc(size_t n, size_t size);

#endif //BRAINFUCK_MEM_H
