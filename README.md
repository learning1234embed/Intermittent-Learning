# Implementation of the paper 'Intermittent Learning: On-Device Machine Learning on Intermittently Powered System'

This repository provides an implementation for the **[IMWUT](https://dl.acm.org/journal/imwut)** **([UBICOMP 2020](http://ubicomp.org/ubicomp2020/))** paper ***"[Intermittent Learning: On-Device Machine Learning on Intermittently Powered System](https://dl.acm.org/doi/10.1145/3369837)"***. This code is written for the MSP430FR5994 microprocessor (256Kb FRAM, 8Kb SRAM) and its launchpad with an energy harvester. You can download and compile source code by using Code Composer Studio (Version: 7.4.0.00015).

1. **intermittent_NN** - an example code of intermittent feed-forward neural network. Back-propagation with multiple layers are supported with supervision (supervised learning). 

2. **intermittent_k_means** - an example code of intermittent k-means algorithm. Online k-means based on neural network (semi-supervised learning) is implemented with online example selection heuristics, i.e. K-Last Lists (diversity and representative).

3. **intermittent_k_nn** - an example code of intermittent k-nearest neighbors algorithm. Features are extracted from raw data and clusters are formed based on distances between features. It is unsupervised learning.

4. **intermittent_learning** - a template (skeleton) code for intermittent learning. Each action function is provided with an empty template. You can fill the action functions with your own machine learning algorithm.

## Citation (BibTeX)

**[Intermittent Learning: On-Device Machine Learning on Intermittently Powered System](https://dl.acm.org/doi/10.1145/3369837)**

```
@article{lee2019intermittent,
  title={Intermittent Learning: On-Device Machine Learning on Intermittently Powered System},
  author={Lee, Seulki and Islam, Bashima and Luo, Yubo and Nirjon, Shahriar},
  journal={arXiv preprint arXiv:1904.09644},
  year={2019}
}
```
