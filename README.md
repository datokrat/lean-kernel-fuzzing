# lean-kernel-fuzzing

This repo contains a custom build of the kernel of [Lean 4](https://lean-lang.org/)
for fuzzing.

The repo contains modified versions of [lean4](https://github.com/leanprover/lean4),
[mimalloc](https://github.com/microsoft/mimalloc) and
[Chris Bailey's fork of lean4export](https://github.com/ammkrn/lean4export/tree/format2024),
as well as custom code and infrastructure for building the modified kernel, creating a corpus
of examples, and running AFL++ on it.

