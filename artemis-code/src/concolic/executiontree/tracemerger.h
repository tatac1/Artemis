/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the GNU General Public License, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *          http://www.gnu.org/licenses/gpl-3.0.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "concolic/executiontree/tracenodes.h"
#include "concolic/executiontree/tracevisitor.h"

#include "statistics/statsstorage.h"

#ifndef TRACEMERGER_H
#define TRACEMERGER_H

namespace artemis
{

/**
 * This class provides the static function `merge` that merges a trace into an execution tree
 *
 * Both structures are perserved. However, be careful since they will share nodes, so don't modify the trace
 * after merging the two.
 *
 * Please observe that this function mutates the executiontree, and if it is null it inserts new nodes.
 * Thus, the usage of a pointer to the executiontree pointer.
 *
 */
class TraceMerger : public TraceVisitor
{
public:
    static TraceNodePtr merge(TraceNodePtr trace, TraceNodePtr executiontree);

    void visit(TraceNode* node);

    void visit(TraceBranch* node);
    void visit(TraceUnexplored* node);
    void visit(TraceAnnotation* node);
    void visit(TraceEnd* node);

    void reportFailedMerge();

private:
    TraceMerger() {}

    TraceNodePtr mCurrentTree;
    TraceNodePtr mCurrentTrace;

    TraceNodePtr mStartingTrace;
    TraceNodePtr mStartingTree;
    static const bool mReportFailedMerge = false; // Whether to dump out failed merges for anaysis.

    static void fixDoubleCountedAnnotations(TraceNodePtr node);
};

}

#endif // TRACEMERGER_H
