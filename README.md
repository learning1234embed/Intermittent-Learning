# Implementation of the paper 'Intermittent Learning on Harvested Energy'

This repository provides an implementation for the paper 'intermittent learning on harvested energy'. It is written for the MSP430FR5994 microprocessor (256Kb FRAM, 8Kb SRAM) and its launchpad with an energy harvester. You can download and compile source code by using Code Composer Studio (Version: 7.4.0.00015).

1. intermittent_NN - an example code of intermittent feed-forward neural network. Back-propagation with multiple layers are supported with supervision (supervised learning). 

2. intermittent_k_means - an example code of intermittent k-means algorithm. Online k-means based on neural network (semi-supervised learning) is implemented with online example selection heuristics, i.e. K-Last Lists (diversity and representative).

3. intermittent_learning - a template (skeleton) code for intermittent learning. Each action function is provided with an empty template. You can fill the action functions with your own machine learning algorithm.
