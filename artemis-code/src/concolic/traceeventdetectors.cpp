/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include "traceeventdetectors.h"
#include "tracebuilder.h"

#include "util/loggingutil.h"


namespace artemis
{


/* Base Detector Class *******************************************************/

void TraceEventDetector::setTraceBuilder(TraceBuilder* traceBuilder)
{
    mTraceBuilder = traceBuilder;
}

void TraceEventDetector::newNode(QSharedPointer<TraceNode> node, QSharedPointer<TraceNode>* successor)
{
    if(mTraceBuilder){
        mTraceBuilder->newNode(node, successor);
    }else{
        Log::fatal("Trace Event Detector being used with no associated Trace Builder.");
        exit(1);
    }
}



/* Branch Detector ***********************************************************/


void TraceBranchDetector::slBranch(QString condition, bool jump, bool symbolic)
{
    // Create a new branch node.
    QSharedPointer<TraceBranch> node = QSharedPointer<TraceBranch>(new TraceBranch());
    node->condition = condition;
    node->symbolic = symbolic;

    // Set the branch we did not take to "unexplored". The one we took is left null.
    // Pass this new node to the trace builder and the branch pointer to use as a successor.
    if(jump){
        node->branchFalse = QSharedPointer<TraceUnexplored>(new TraceUnexplored());
        newNode(node.staticCast<TraceNode>(), &(node->branchTrue));
    }else{
        node->branchTrue = QSharedPointer<TraceUnexplored>(new TraceUnexplored());
        newNode(node.staticCast<TraceNode>(), &(node->branchFalse));
    }
}



/* Alert Detector ************************************************************/

void TraceAlertDetector::slJavascriptAlert(QWebFrame* frame, QString msg)
{
    // Create a new alert node.
    QSharedPointer<TraceAlert> node = QSharedPointer<TraceAlert>(new TraceAlert());
    node->message = msg;
    // Leave node.next as null.

    // Pass this new node to the trace builder and pass a pointer to where the sucessor should be attached.
    newNode(node.staticCast<TraceNode>(), &(node->next));
}





/* Function Call Detector ****************************************************/

void TraceFunctionCallDetector::slJavascriptFunctionCalled(QString functionName, size_t bytecodeSize, uint functionStartLine, uint sourceOffset, QSource* source)
{
    // Create a new function call node.
    QSharedPointer<TraceFunctionCall> node = QSharedPointer<TraceFunctionCall>(new TraceFunctionCall());
    node->name = functionName;
    // Leave node.next as null.

    // Pass this new node to the trace builder and pass a pointer to where the sucessor should be attached.
    newNode(node.staticCast<TraceNode>(), &(node->next));
}




} //namespace artmeis