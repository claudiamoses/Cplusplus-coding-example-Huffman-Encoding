#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"  // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;

/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 *
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    string message;
    EncodingTreeNode* treePtr = tree;
    Bit branch;

    while (messageBits.size() > 0) {
        branch = messageBits.dequeue();
        if (branch == 0) {
            treePtr = treePtr -> zero;
        }
        else if (branch == 1) {
            treePtr = treePtr -> one;
        }

        if (treePtr -> isLeaf()) {
            message += treePtr -> getChar();
            treePtr = tree;
        }
    }
    return message;

}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {

    Bit branch = treeShape.dequeue();
    if (branch == 0) {
        EncodingTreeNode* leaf = new EncodingTreeNode(treeLeaves.dequeue());
        return leaf;
    }
    else {
        EncodingTreeNode* builder = new EncodingTreeNode(unflattenTree(treeShape, treeLeaves), unflattenTree(treeShape, treeLeaves));
        return builder;
    }
}

/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 */
string decompress(EncodedData& data) {
    EncodingTreeNode* tree = unflattenTree(data.treeShape, data.treeLeaves);
    string message =decodeText(tree, data.messageBits);
    deallocateTree(tree);
    return message;
}


/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * Further break-down provided by in-line comments
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    //step 1: create a map of characters and their occurrences in the text
    Map<char, int> occurrences;
    PriorityQueue<EncodingTreeNode*> PQ;
    if (text.size() == 0) {
        error("no text available to encode");
    }
    for (char elem : text) {
        if (! occurrences.containsKey(elem)) {
            occurrences[elem] = 1;
        }
        else {
            occurrences[elem] = occurrences[elem] + 1;
        }
    }
    //step 2: create a PriorityQueue of all keys
    Vector<char> keys = occurrences.keys();
    for (char key : keys) {
        EncodingTreeNode* node = new EncodingTreeNode(key);
        PQ.enqueue(node, occurrences[key]);
    }
    //step 3: return an error if there aren't enough unique characters
    if (PQ.size() < 2) {
        while (! PQ.isEmpty()) {
            EncodingTreeNode* node = PQ.dequeue();
            deallocateTree(node);
        error("not enough unique characters in text");
        }
    }
    //step 4: use iteration to create a Huffman Tree from the priorityQueue!
    EncodingTreeNode* tree;
    while (PQ.size() > 1) {
        int node1Priority = PQ.peekPriority();
        EncodingTreeNode* node1 = PQ.dequeue();
        int node2Priority = PQ.peekPriority();
        EncodingTreeNode* node2 = PQ.dequeue();
        EncodingTreeNode* parent = new EncodingTreeNode(node1, node2);
        tree = parent;
        PQ.enqueue(tree, node1Priority + node2Priority);
    }
    return tree;
}

/* this helper function uses recursion to traverse the tree
 * once and build a map that associates each character with
 * its encoded bit sequence.
 */

Map<char, Vector<Bit>> sequenceMap(EncodingTreeNode* tree, Map<char, Vector<Bit>>& map, Vector<Bit>& sequenceSoFar) {

    if (tree == nullptr) {
        return map;
    }
    if (tree -> isLeaf()) {
        map[tree -> ch] = sequenceSoFar;
    }

    else {
        sequenceSoFar.add(0);
        sequenceMap(tree -> zero, map, sequenceSoFar);
        sequenceSoFar.remove(sequenceSoFar.size() - 1);
        sequenceSoFar.add(1);
        sequenceMap(tree -> one, map, sequenceSoFar);
        sequenceSoFar.remove(sequenceSoFar.size() - 1);
    }
    return map;
}


/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 */

Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Vector<Bit> sequence;
    Map<char, Vector<Bit>> map;
    map = sequenceMap(tree, map, sequence);

    Queue<Bit> encoding;

    for (char elem : text) {
        Vector<Bit> temporary = map[elem];
        for (Bit num : temporary) {
            encoding.enqueue(num);
        }
    }
    return encoding;
}

/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (tree == nullptr) {
        return;
    }
    if (tree -> isLeaf()) {
        treeLeaves.enqueue(tree -> ch);
        treeShape.enqueue(0);
        return;
    }
    else {
        treeShape.enqueue(1);
        flattenTree(tree -> zero, treeShape, treeLeaves);
        flattenTree(tree -> one, treeShape, treeLeaves);
    }
    return;
}

/* this function finds the needed leaves from an inputed messageText
 */

void findLeaves(EncodingTreeNode* tree, Queue<char>& leaves) {

    if (tree == nullptr) {
        return;
    }
    if (tree -> isLeaf()) {
        leaves.enqueue(tree -> ch);
    }
    else {
        findLeaves(tree -> zero, leaves);
        findLeaves(tree -> one, leaves);
    }
    return;
}

/* this function generates a shape in terms of 0s and 1s for a tree
 */

void generateTreeShape(EncodingTreeNode* tree, Queue<Bit>& bits) {

    if (tree == nullptr) {
        return;
    }
    if (tree -> isLeaf()) {
        bits.enqueue(0);
    }
    else {
        bits.enqueue(1);
        generateTreeShape(tree -> zero, bits);
        generateTreeShape(tree -> one, bits);
    }
    return;
}

/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 */

EncodedData compress(string messageText) {

    //make tree
    EncodingTreeNode* tree = buildHuffmanTree(messageText);

    //find message bits
    Queue<Bit> encoding = encodeText(tree, messageText);

    //find leave characters
    Queue<char> leaves;
    findLeaves(tree, leaves);

    //find tree shape
    Queue<Bit> shapeBits;
    generateTreeShape(tree, shapeBits);

    //add into encoded data type
    EncodedData compression;
    compression.treeShape = shapeBits;
    compression.messageBits = encoding;
    compression.treeLeaves = leaves;
    deallocateTree(tree);
    return compression;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {

    EncodingTreeNode* r = new EncodingTreeNode('R');
    EncodingTreeNode* s = new EncodingTreeNode('S');
    EncodingTreeNode* branch = new EncodingTreeNode(r, s);
    EncodingTreeNode* e = new EncodingTreeNode('E');
    branch = new EncodingTreeNode(branch, e);
    EncodingTreeNode* t = new EncodingTreeNode('T');
    branch = new EncodingTreeNode(t, branch);

    return branch;
}

void deallocateTree(EncodingTreeNode* t) {
    if (t == nullptr) {
        return;
    }
    else {
        deallocateTree(t->zero);
        deallocateTree(t->one);
        delete t;
    }
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {

    if (a  == nullptr && b  == nullptr) {
        return true;
    }
    else if (a == nullptr || b == nullptr) {
        return false;
    }
    if (a -> isLeaf() && b -> isLeaf() ) {
        if (a -> getChar() == b -> getChar()) {
            return true;
        }
        return false;
    }
    else {
        bool leftTree = areEqual(a -> zero, b -> zero);
        bool rightTree = areEqual(a -> one, b -> one);
        if (leftTree == true && rightTree == true) {
            return true;
        }

    }

    return false;
}

/* * * * * * Test Cases Below This Point * * * * * */
/*            *
             /   \
            T     *
                 / \
                *   E
               / \
              R   S

STUDENT_TEST("Test deallocateTree on example tree") {
    EncodingTreeNode* sample = createExampleTree();
    deallocateTree(sample);
}

STUDENT_TEST("Test areEqual on two empty trees") {
    EncodingTreeNode* treeA = nullptr;
    EncodingTreeNode* treeB = nullptr;
    EXPECT(areEqual(treeA, treeB));
}

STUDENT_TEST("Test areEqual on one empty tree, one full tree") {
    EncodingTreeNode* treeA = nullptr;
    EncodingTreeNode* childB1 = new EncodingTreeNode('A');
    EncodingTreeNode* childB2 = new EncodingTreeNode('A');
    EncodingTreeNode* treeB = new EncodingTreeNode(childB1, childB2);
    EXPECT(!areEqual(treeA, treeB));
    deallocateTree(treeB);
}

STUDENT_TEST("Test areEqual on simple tree and sample tree") {
    EncodingTreeNode* treeA = createExampleTree();
    EncodingTreeNode* childB1 = new EncodingTreeNode('A');
    EncodingTreeNode* childB2 = new EncodingTreeNode('A');
    EncodingTreeNode* treeB = new EncodingTreeNode(childB1, childB2);
    EXPECT(!areEqual(treeA, treeB));
    deallocateTree(treeA);
    deallocateTree(treeB);
}

STUDENT_TEST("Test areEqual on two sample trees") {
    EncodingTreeNode* treeA = createExampleTree();
    EncodingTreeNode* treeB = createExampleTree();
    EXPECT(areEqual(treeA, treeB));
    deallocateTree(treeA);
    deallocateTree(treeB);
}

STUDENT_TEST("Test decodeText on simple tree") {
    EncodingTreeNode* childB1 = new EncodingTreeNode('A');
    EncodingTreeNode* childB2 = new EncodingTreeNode('B');
    EncodingTreeNode* treeB = new EncodingTreeNode(childB1, childB2);
    Queue<Bit> testText = {0, 1, 0, 1};
    string expected = "ABAB";
    EXPECT_EQUAL(decodeText(treeB, testText), expected);
    deallocateTree(treeB);
    string unexpected = "BABA";
    EXPECT(decodeText(treeB, testText) != expected);
}

STUDENT_TEST("Test decompress on samples") {
    EncodedData tester;
    tester.treeShape = {1, 1, 0, 1, 0, 0, 1, 0, 0};
    tester.treeLeaves = {'F', 'L', 'E', 'R', 'A'};
    tester.messageBits = {1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1};
    string expected = "RAFFLE";
    EXPECT_EQUAL(decompress(tester), expected);
}

STUDENT_TEST("test sequenceMap helper function on sample tree") {
    Vector<Bit> queue;
    Map<char, Vector<Bit>> map;
    EncodingTreeNode* tree = createExampleTree();
    Map<char, Vector<Bit>> expected = {{ 'T', {0}}, { 'R', {1, 0, 0}}, { 'S', {1, 0, 1}}, { 'E', {1, 1}}};
    EXPECT_EQUAL(sequenceMap(tree, map, queue), expected);
    deallocateTree(tree);
}

STUDENT_TEST("test sequenceMap helper function on simple tree") {
    Vector<Bit> queue;
    Map<char, Vector<Bit>> map;
    EncodingTreeNode* childB1 = new EncodingTreeNode('A');
    EncodingTreeNode* childB2 = new EncodingTreeNode('B');
    EncodingTreeNode* tree = new EncodingTreeNode(childB1, childB2);
    Map<char, Vector<Bit>> expected = {{ 'A', {0}}, { 'B', {1}}};
    EXPECT_EQUAL(sequenceMap(tree, map, queue), expected);
    deallocateTree(tree);
}

STUDENT_TEST("test sequenceMap helper function on simple tree") {
    Vector<Bit> queue;
    Map<char, Vector<Bit>> map;
    EncodingTreeNode* tree = nullptr;
    Map<char, Vector<Bit>> expected = {};
    EXPECT_EQUAL(sequenceMap(tree, map, queue), expected);
}

STUDENT_TEST("Test encodeText on sample tree") {
    string test = "STRESS";
    Queue<Bit> expected = {1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1};
    EncodingTreeNode* tree = createExampleTree();
    EXPECT_EQUAL(encodeText(tree, test), expected);
    string result = decodeText(tree, expected);
    EXPECT_EQUAL(result, test);
    deallocateTree(tree);
}

STUDENT_TEST("Test flatten tree") {
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    EncodingTreeNode* tree = createExampleTree();
    flattenTree(tree, treeShape, treeLeaves);
    EncodingTreeNode* unflattenedTree = unflattenTree(treeShape, treeLeaves);
    EXPECT(areEqual(tree, unflattenedTree));
    deallocateTree(tree);
    deallocateTree(unflattenedTree);
}

STUDENT_TEST("test buildHuffmanTree on base text") {
    string testText = "AB";
    EncodingTreeNode* childB1 = new EncodingTreeNode('B');
    EncodingTreeNode* childB2 = new EncodingTreeNode('A');
    EncodingTreeNode* treeB = new EncodingTreeNode(childB1, childB2);
    EncodingTreeNode* tree = buildHuffmanTree(testText);
    EXPECT(areEqual(tree, treeB));
    deallocateTree(tree);
    deallocateTree(treeB);
}

STUDENT_TEST("test buildHuffmanTree on empty text") {
    string testText = "";
    EXPECT_ERROR(buildHuffmanTree(testText));
}

STUDENT_TEST("test buildHuffmanTree on one character") {
    string testText = "aaaa";
    EXPECT_ERROR(buildHuffmanTree(testText));
}

STUDENT_TEST("test buildHuffmanTree on non alpha characters") {
    string testText = "-!";
    EncodingTreeNode* childB1 = new EncodingTreeNode('-');
    EncodingTreeNode* childB2 = new EncodingTreeNode('!');
    EncodingTreeNode* treeB = new EncodingTreeNode(childB1, childB2);
    EncodingTreeNode* tree = buildHuffmanTree(testText);
    EXPECT(areEqual(tree, treeB));
    deallocateTree(tree);
    deallocateTree(treeB);
}

STUDENT_TEST("test findLeaves function") {
    EncodingTreeNode* test = createExampleTree();
    Queue<char> input;
    Queue<char> expected = {'T', 'R', 'S', 'E'};
    findLeaves(test, input);
    EXPECT_EQUAL(input, expected);
    deallocateTree(test);
}

STUDENT_TEST("test generateTreeShape function") {
    EncodingTreeNode* tree = createExampleTree();
    Queue<Bit> bits;
    Queue<Bit> expected = {1, 0, 1, 1, 0, 0, 0};
    generateTreeShape(tree, bits);
    EXPECT_EQUAL(bits, expected);
    deallocateTree(tree);
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman",
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
