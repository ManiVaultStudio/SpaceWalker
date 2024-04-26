#pragma once

#include <QString>

class KnnGraph;

class KnnGraphImporter
{
public:
    static void read(QString filePath, KnnGraph& graph);
};

class KnnGraphExporter
{
public:
    static void write(const KnnGraph& graph);
};
