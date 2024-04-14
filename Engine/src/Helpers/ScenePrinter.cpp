#include "../../include/Helpers/ScenePrinter.h"

void Prisma::ScenePrinter::printScene(std::shared_ptr<Prisma::Node> nodeRoot, int depth)
{
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }

    std::cout << "Node: " << nodeRoot->name() << std::endl;

    for (unsigned int i = 0; i < nodeRoot->children().size(); i++)
    {
        printScene(nodeRoot->children()[i], depth + 1);
    }
}
