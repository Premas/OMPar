# OMPAR

OMPAR is a compiler-oriented tool composed of the following pipeline: it uses [OMPify](https://github.com/Scientific-Computing-Lab-NRCN/OMPify) to detect parallelization opportunities. When a for loop that would benefit from parallelization is found, [MonoCoder](https://github.com/Scientific-Computing-Lab-NRCN/MonoCoder) is applied to generate the OpenMP pragma.

## Usage

Here is an example of how to use OMPAR:

```python
code = """for(int i = 0; i <= 1000; i++){
                partial_Sum += i;
            }"""

device = 'cuda' if torch.cuda.is_available() else 'cpu'
ompar = OMPAR(model_path=main_args.model_weights, device=device, args=main_args)

pragma = ompar.auto_comp(code)
```

**Note:** The weights of OMPify are not included in the repository and will be provided on demand.

## Evaluation Of OMPAR With Parallelizing Compilers

We have conducted an evaluation of OMPAR using two widely-used parallelizing compilers: [autoPar](https://en.wikibooks.org/wiki/ROSE_Compiler_Framework/autoPar) and [Intel Parallelizing Compiler (ICPC)](https://www.intel.com/content/www/us/en/developer/articles/technical/automatic-parallelization-with-intel-compilers.html).

For a thorough understanding of how these compilers were utilized and to perform scalability test, please refer to the documentation available in the following sections:

- [autoPar](./autoPar/autoPar_README.md): This section offers a guide on utilizing the autoPar compiler. It includes detailed instructions for setting up the compiler, insights into the evaluation process, and benchmarks that has been parallelized using autoPar.

- [ICPC](./icpc/icpc_README.md): This section covers the usage of the Intel Compiler for auto parallelization, and details to perform run and scale test.
