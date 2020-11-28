#include "Node.h"
#include <limits>

double Node::puct(Node* parent)
{
    if (parent == nullptr) return std::numeric_limits<double>::infinity();
    if (n == 0 || parent->n == 0) return std::numeric_limits<double>::infinity();

    double exploit = t / (double)n;
    double explore = probability * sqrt((double)parent->n) / (double)(1 + n);

    return exploit + 4.0 * explore;
}