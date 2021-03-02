//
// Created by koen on 27-02-21.
//

#ifndef VPGSOLVERS_CONF_H
#define VPGSOLVERS_CONF_H
#ifdef subsetbdd
#include "bdd.h"
#define ConfSet bdd
#define fullset bddtrue
#define emptyset bddfalse
#endif
#ifdef subsetexplicit
#define ConfSet ConfSetExplicit
#define fullset ConfSetExplicit::SetFullset
#define emptyset ConfSetExplicit::SetEmptyset
#include "VariabilityParityGames/Algorithms/Datastructures/ConfSetExplicit.h"
#endif
#define VertexSetZlnkIsBitVector
//#define VertexSetZlnkIsHashSet
#ifdef VertexSetZlnkIsBitVector
#include "VariabilityParityGames/Algorithms/Datastructures/VectorBoolOptimized.h"
#define VertexSetZlnk  VectorBoolOptimized
#endif
#ifdef VertexSetZlnkIsHashSet
#include "VariabilityParityGames/Algorithms/Datastructures/UnorderedVertexSet.h"
#define VertexSetZlnk  UnorderedVertexSet
#endif

#define VertexSetFPIte VectorBoolOptimized
#endif //VPGSOLVERS_CONF_H
