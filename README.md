# huffyuv #

A huffyuv codec implementation.

## Huffman Coding ##

Huffman Coding is a variant of entropy coding that works in a very similar manner to Shannon-Fano Coding, but the binary tree is built from the top down to generate an optimal result.

The algorithm to generate Huffman codes shares its first steps with Shannon-Fano:

1. Parse the input, counting the occurrence of each symbol.
2. Determine the probability of each symbol using the symbol count.
3. Sort the symbols by probability, with the most probable first.
4. Generate leaf nodes for each symbol, including P, and add them to a queue.
5. While (Nodes in Queue > 1)
    a. Remove the two lowest probability nodes from the queue.
    b. Prepend 0 and 1 to the left and right nodes' codes, respectively.
    c. Create a new node with value equal to the sum of the nodes’ probability.
    d. Assign the first node to the left branch and the second node to the right branch.
    e. Add the node to the queue
6. The last node remaining in the queue is the root of the Huffman tree.
