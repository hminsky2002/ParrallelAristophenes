# Parallel Implementation Of Sieve of Aristophenes Algorithim
This repository contains a Parallel implementation of the Sieve of Aristophenes Algorithim 
for finding prime numbers up to N using OpenMPI in C.

Compile:  ```mpicc -g -std=c99 -Wall -lm -o  primes primes.c```
Run:     ``` mpiexec -n <number of processes> ./primes <Value of N>```