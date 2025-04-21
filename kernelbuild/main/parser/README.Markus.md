```
./bazel-bin/main/parser ../lean4export/ExportedCorpus/Corpus.NestedInduction.belean
```

fails, because apparently it incorrectly parses/assembles one of the declaration.
When changing `binary` to false,

```
./bazel-bin/main/parser input/Corpus.NestedInduction.elean
```
works fine.