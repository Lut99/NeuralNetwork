# NeuralNetwork
In this repository, you can find my implementation of a simple, feedforward neural network implemented in C and optimised using both OpenMP and CUDA. It is designed to classify handwritten digits, as given by the Digits dataset from scikit-learn (https://scikit-learn.org/stable/). However, it is not build for accuracy, but rather as a representable neural network which we is optimised in different ways and be used to run benchmarks on.

This code was used in my bachelor thesis, which you can find [here](). It also serves as an example as the kind of research that can be done with this neural network.

## Usage
### Compilation
The neural network can be run through two different executables: either via ```bin/digits.out```, to run it with the Digits-dataset, or ```bin/testdata.out```, which can be used to generate random data of arbitrary size. To aid in running the neural network with the dataset, a converted version of scikit's dataset to CSV can be found in the root of the repository, as ```digits.csv```.

To compile, eithe run ```make digits``` or ```make testdata``` to compile the respective executables. By default, this puts the neural network in 'verbose' mode with a sequential back-end (i.e., no parallelism applied). However, various options can be given to change this:
- Add ```BACKEND=<version>``` to your make-command to select the desired backend for the training phase of the neural network. The Makefile compiles the correct file in the ```src/lib/NeuralNetwork``` folder, which is equal to ```NeuralNetwork_<version>.c``` (where ```<version>``` is the string supplied to the ```BACKEND``` flag). See below for an overview of the backends.
- Add ```DEBUG=1``` to your make-command to let the Makefile add debug symbols to your compiled files.
- Add ```PROFILE=1``` to your make-command to add gprof information to the executable.
- Add ```PLOT=1``` to your make-command to let the framework report the cost per iteration, which can them be plotted to a graph using ```make plot```. Note that this only works for the Digits-executable and for the sequential backend.
- Add ```BENCHMARK=1``` to prevent the framework from printing anything, besides the times for various timers place in the training phase once it's done training.

As stated above, can the framework be compiled with different backends for the training phase of the neural network, each of which is optimised differently. A list of possible backends are:
- ```sequential```: Implements a sequential training phase, without any optimisations. Does not provide any additional arguments, and is the default backend when non is specified.
- ```OMP_CPUX```: Implements a training phase which is optimised using OpenMP. The ```X``` must be replaced by a number ranging 1-8, to select different versions of the OpenMP optimisation, each optimising in a different way. All of these add the extra, optional argument that the number of threads can be supplied. The versions are:
    - ```OMP_CPU1```: Only parallelizes the forward pass, as this pass doesn't need additional thread safety
    - ```OMP_CPU2```: Paralleliz.es both the forward and backward passes, providing thread safety in the form of critical regions.
    - ```OMP_CPU3```: Parallelizes both the forward and backward passes, providing thread safety by adding all the weights in a reduction-like fashion.
    - ```OMP_CPU4```: Parallelizes not using multiple threads, but rather vectorizes the innermost loops of the training phase with SIMD. Only variation that doesn't add the optional 'threads' argument.
    - ```OMP_CPU5```: Very similar to ```OMP_CPU1```, except that this variation also applies SIMD to the training phase's innermost loops.
    - ```OMP_CPU6```: Very similar to ```OMP_CPU2```, except that this variation also applies SIMD to the training phase's innermost loops.
    - ```OMP_CPU7```: Applies an algorithmic change by swapping the iteration and the sample and the layers loop, making it a more pipelined implementation. Provides thread safety by adding all weights in a reduction-like facshion.
    - ```OMP_CPU8```: Very similar to ```OMP_CPU7```, except that this variation also applies SIMD to the training phase's innermost loops.
- ```CUDA_GPU1```: Parallelizes the training phase by offloading it to the GPU with CUDA (therefore, only works on Nvidia GPUs). Can ask for an extra, optional parameters that defines the threads per block if the proper lines are uncommented, but this has been removed to allow a script to always given the number of threads as paramater and not confuse the CUDA-implementation.

### Executing
Once you have compiled either of the two files, they can be run from the command line. Both specify a number of options, and different back-ends might also add extra options to that list.

The ```bin/digits.out```-executable must be given the path to the digits dataset it is supposed to train the network on. Any other options specified will be passed to the backend, so that it may parse additional parameters.

The ```bin/testdata.out```-executable needn't be given anything, but supports lot more options to customize the data generated:
- ```-S <unsigned long>```: specifies the number of samples to generate (default: 1437)
- ```-s <unsigned long>```: specifies the size of each sample (default: 64)
- ```-c <unsigned long>```: the number of sample classes (default: 10)
- ```-D <float>```: The upperbound value for the random values in the dataset (default: 3.0)
- ```-d <float>```: The lowerbound value for the random values in the dataset (default: -3.0)
- ```-e <unsinged long>```: the number of iterations (epochs) to train the neural network (default: 20000)
- ```-l <float>```: the learning rate of the network (default: 0.005)
- ```-H <unsigned long>```: the number of hidden layers in the neural network (default: 1)
- ```-N <unsigned long list>```: number of nodes per hidden layer. Should be given H values, separated by commas, where H is the given number of hidden layers, and this argument is mandatory for any network with more than 1 hidden layer.
- ```-h```: prints a help message

Any other parameters not covered by the list above will be passed to the backend, in the same way as ```bin/digits.out``` does.

Note that the size of the input layer and output layer are implicitly given by the other parameters.

## Troubleshooting
For issues with CUDA linking (especially a missing ```-lcudart```), please edit the path in the Makefile to the correct lib64-folder first. I found mine by running ```sudo find / -name 'cuda'```.

For any other issues or questions, feel free to [leave an issue](https://github.com/Lut99/NeuralNetwork/issues) in this repository with appropriate tags.

## License
Feel free to do whatever you want with this code, as long as you:
- use it for good
- reference this repository somehow
- ideally, mention my name (Tim Müller) :)

Additionally, it's appreciated if you make any changes also open-source.
