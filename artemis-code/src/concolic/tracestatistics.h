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

#ifndef TRACESTATISTICS_H
#define TRACESTATISTICS_H

namespace artemis
{


/*
 *  Gathers statistics about a complete trace which may be helpful for analysis of that trace.
 *
 *  TODO: find a better name for this class!
 */

class TraceStatistics : public TraceVisitor
{
public:
    TraceStatistics();

    int mNumNodes;
    int mNumBranches;
    int mNumSymBranches;
    int mNumAlerts;
    int mNumFunctionCalls;

    void processTrace(TraceNodePtr trace);

    // Cases we need to ignore or which cause an error.
    virtual void visit(TraceNode* node);
    virtual void visit(TraceAnnotation* node);
    virtual void visit(TraceEnd* node);
    virtual void visit(TraceUnexplored* node);

    // Cases we will implement.
    virtual void visit(TraceConcreteBranch* node);
    virtual void visit(TraceSymbolicBranch* node);
    virtual void visit(TraceAlert* node);
    virtual void visit(TraceFunctionCall* node);
};


} // namespace artemis

#endif // TRACESTATISTICS_H
