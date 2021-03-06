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


#include "tracedisplay.h"

#include "util/loggingutil.h"
#include "util/fileutil.h"


namespace artemis
{

QString TraceDisplay::indent = "  ";

bool TraceDisplay::mPassThroughEndMarkers = false;

TraceDisplay::TraceDisplay(bool simplified) :
    mSimplified(simplified)
{
    mExpressionPrinter = QSharedPointer<ExpressionPrinter>(new ExpressionValuePrinter());

    mStyleBranches = "[label = \"Branch\\n(concrete)\"]";
    mStyleSymBranches = "[style = filled, fillcolor = azure]";
    mStyleUnexplored = "[label = \"???\", shape = circle, style = filled, fillcolor = lightgray]";
    mStyleUnexploredUnsat = "[label = \"UNSAT\", shape = ellipse, style = filled, fillcolor = lightgray]";
    mStyleUnexploredUnsolvable = "[label = \"Could not solve\", shape = ellipse, style = filled, fillcolor = lightgray]";
    mStyleUnexploredMissed = "[label = \"Missed\", shape = ellipse, style = filled, fillcolor = lightgray]";
    mStyleAlerts = "[shape = rectangle, style = filled, fillcolor = beige]";
    mStyleDomMods = "[shape = rectangle, style = filled, fillcolor = peachpuff]";
    mStyleLoads = "[shape = rectangle, style=filled, fillcolor = honeydew]";
    mStyleFunctions = "[shape = rectangle]";
    mStyleEndSucc = "[label = \"End\", fillcolor = green, style = filled, shape = circle]";
    mStyleEndFail = "[label = \"End\", fillcolor = red, style = filled, shape = circle]";
    mStyleEndUnk = "[label = \"End\", fillcolor = lightgray, style = filled, shape = circle]";
    mStyleAggregates = "[style = filled, fillcolor = snow, shape = box3d]";

}

/**
 *  Returns a string representing a graphviz input file displaying the trace tree provided.
 */
QString TraceDisplay::makeGraph(TraceNodePtr tree)
{
    QString result;
    /* The visitor part builds up the lists of information which must be included in the "header" and allthe edges
     * in the graph. Then we build this into a actual graphviz file. */

    // Visitor populates mHeader* and mEdges.
    clearData();
    tree->accept(this);

    // Begin the graph
    result += "digraph tree1 {\n" + indent + "graph [ordering = out];\n\n";


    // Build the header (this is where the styling for each node type is done).

    result += indent + "subgraph branches {\n" + indent + indent + "node " + mStyleBranches + ";\n\n";
    foreach(QString node, mHeaderBranches){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph branches_sym {\n" + indent + indent + "node " + mStyleSymBranches + ";\n\n";
    foreach(QString node, mHeaderSymBranches){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph unexplored {\n" + indent + indent + "node " + mStyleUnexplored + ";\n\n";
    foreach(QString node, mHeaderUnexplored){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph unexplored_unsat {\n" + indent + indent + "node " + mStyleUnexploredUnsat + ";\n\n";
    foreach(QString node, mHeaderUnexploredUnsat){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph unexplored_unsolvable {\n" + indent + indent + "node " + mStyleUnexploredUnsolvable + ";\n\n";
    foreach(QString node, mHeaderUnexploredUnsolvable){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph unexplored_missed {\n" + indent + indent + "node " + mStyleUnexploredMissed + ";\n\n";
    foreach(QString node, mHeaderUnexploredMissed){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph alerts {\n" + indent + indent + "node " + mStyleAlerts + ";\n\n";
    foreach(QString node, mHeaderAlerts){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph dom_mods {\n" + indent + indent + "node " + mStyleDomMods + ";\n\n";
    foreach(QString node, mHeaderDomMods){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph loads {\n" + indent + indent + "node " + mStyleLoads + ";\n\n";
    foreach(QString node, mHeaderLoads){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph functions {\n" + indent + indent + "node " + mStyleFunctions + ";\n\n";
    foreach(QString node, mHeaderFunctions){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph end_succ {\n" + indent + indent + "node " + mStyleEndSucc + ";\n\n";
    foreach(QString node, mHeaderEndSucc){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph end_fail {\n" + indent + indent + "node " + mStyleEndFail + ";\n\n";
    foreach(QString node, mHeaderEndFail){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph end_unk {\n" + indent + indent + "node " + mStyleEndUnk + ";\n\n";
    foreach(QString node, mHeaderEndUnk){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";

    result += indent + "subgraph aggregates {\n" + indent + indent + "node " + mStyleAggregates + ";\n\n";
    foreach(QString node, mHeaderAggregates){
        result += indent + indent + node + ";\n";
    }
    result += indent + "}\n\n";


    // Build the edges list

    result += indent + "start [label = \"Start\"];\n\n";

    foreach(QString edge, mEdges){
        result += indent + edge + ";\n";
    }


    // Add the legend (if any) on the end.
    result += mLegend;


    // End the graph
    result += "\n}\n";


    return result;
}


// Writes the result of makeGraph() to a file.
void TraceDisplay::writeGraphFile(TraceNodePtr tree, QString& pathToFile)
{
    writeGraphFile(tree, pathToFile, true);
}

// Writes the result of makeGraph() to a file.
void TraceDisplay::writeGraphFile(TraceNodePtr tree, QString& pathToFile, bool autoName)
{
    QString data = makeGraph(tree);

    // If autoName is true, we generate a name and "return" it through pathToFile.
    // Otherwise, we use the value given in pathToFile as the name.
    if(autoName){
        pathToFile = QString("graph-%1.gv").arg(QDateTime::currentDateTime().toString("dd-MM-yy-hh-mm-ss"));
    }
    writeStringToFile(pathToFile, data);
}


/*
 *  The node visitor methods.
 *
 *  Each of these must compute its name, add this to the corresponding header list,
 *  add the incoming edge to the edges list, and if there are any children then set
 *  mPreviousNode and mEdgeExtras and then process them.
 */

void TraceDisplay::visit(TraceNode *node)
{
    // Should never be reached.
    Log::fatal("Trace displayer visited a node of unknown type.");
    exit(1);
}


void TraceDisplay::visit(TraceConcreteBranch *node)
{
    QString name = QString("br_%1").arg(mNodeCounter);
    mNodeCounter++;

    // For concrete branches, if both children are explored, then we want to always show them in the tree.
    // If only one child is explored, then we consider this part of a concrete execution and aggregate any such branches into a single node.
    // The case where no children are explored should never happen in a valid tree, but we would want to display this if it did.
    // Also, we only want to go into aggregation mode if there is another concrete branch as a direct child of this.
    // This prevents aggregating single nodes!
    if(mSimplified){

        // Check how many children of this node are explored (see above).

        if(isImmediatelyUnexplored(node->getFalseBranch()) && !isImmediatelyUnexplored(node->getTrueBranch())){
            // This is a "boring" branch to true.
            // Begin aggregation from here if the true branch is another concrete branch.
            if(isImmediatelyConcreteBranch(node->getTrueBranch())){
                mCurrentlyAggregating = true;
            }
            if(mCurrentlyAggregating){
                mAggregatedConcreteBranches++;
                node->getTrueBranch()->accept(this);
                return;
            }
        }

        if(!isImmediatelyUnexplored(node->getFalseBranch()) && isImmediatelyUnexplored(node->getTrueBranch())){
            // This is a "boring" branch to false.
            // Begin aggregation from here if the false branch is another concrete branch.
            if(isImmediatelyConcreteBranch(node->getFalseBranch())){
                mCurrentlyAggregating = true;
            }
            if(mCurrentlyAggregating){
                mAggregatedConcreteBranches++;
                node->getFalseBranch()->accept(this);
                return;
            }else{
                // Begin aggregation from here if the false branch is another concrete branch.
            }
        }

        // If 2 or 0 children are explored, or we are not currently aggregating,
        // then fall through to the "normal mode" processing below.
    }

    // If not in simplified mode (or if we chose to display this branch anyway) we just output this node as usual.

    mHeaderBranches.append(name);

    addInEdge(name);

    // Childeren are displayed in the tree in the order they are specified in the edges list.
    // We want false branches on the left, so process those first.

    mPreviousNode = name;
    mEdgeExtras = "[color = red]";
    node->getFalseBranch()->accept(this);

    mPreviousNode = name;
    mEdgeExtras = "[color = darkgreen]";
    node->getTrueBranch()->accept(this);
}


void TraceDisplay::visit(TraceSymbolicBranch *node)
{
    flushAggregation();

    QString name = QString("sym_%1").arg(mNodeCounter);
    mNodeCounter++;

    node->getSymbolicCondition()->accept(mExpressionPrinter.data());
    QString symbolicExpression(mExpressionPrinter->getResult().c_str());
    mExpressionPrinter->clear();
    symbolicExpression.replace("\\", "\\\\");
    symbolicExpression.replace("\"", "\\\"");
    QString label = QString(" [label = \"Branch\\n%1\"]").arg(symbolicExpression);

    mHeaderSymBranches.append(name + label);

    addInEdge(name);

    // Childeren are displayed in the tree in the order they are specified in the edges list.
    // We want false branches on the left, so process those first.

    mPreviousNode = name;
    mEdgeExtras = "[color = red]";
    node->getFalseBranch()->accept(this);

    mPreviousNode = name;
    mEdgeExtras = "[color = darkgreen]";
    node->getTrueBranch()->accept(this);
}


void TraceDisplay::visit(TraceUnexplored *node)
{
    flushAggregation();

    QString name = QString("unexp_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderUnexplored.append(name);

    addInEdge(name);
}

void TraceDisplay::visit(TraceUnexploredUnsat *node)
{
    flushAggregation();

    QString name = QString("unexp_unsat_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderUnexploredUnsat.append(name);

    addInEdge(name);
}

void TraceDisplay::visit(TraceUnexploredUnsolvable *node)
{
    flushAggregation();

    QString name = QString("unexp_unsolvable_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderUnexploredUnsolvable.append(name);

    addInEdge(name);
}

void TraceDisplay::visit(TraceUnexploredMissed *node)
{
    flushAggregation();

    QString name = QString("unexp_missed_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderUnexploredMissed.append(name);

    addInEdge(name);
}


void TraceDisplay::visit(TraceAlert *node)
{
    flushAggregation();

    QString name = QString("alt_%1").arg(mNodeCounter);
    mNodeCounter++;

    QString message = node->message;
    message.replace('\"', "\\\"");
    message.replace('\n', "\\n");
    QString nodeDecl = QString("%1 [label = \"Alert\\n\\\"%2\\\"\"]").arg(name).arg(message);
    mHeaderAlerts.append(nodeDecl);

    addInEdge(name);

    mPreviousNode = name;
    mEdgeExtras = "";
    node->next->accept(this);
}


void TraceDisplay::visit(TraceDomModification *node)
{
    flushAggregation();

    QString name = QString("dom_%1").arg(mNodeCounter);
    mNodeCounter++;

    QString wordList = (node->words.size() > 0 ? "\\nIndicator words:" : "");
    QString word;
    int count;
    foreach(int index, node->words.keys()){
        word = TraceDomModDetector::indicators.at(index);
        count = node->words.value(index);
        wordList += QString("\\n%1: %2").arg(word).arg(count);
    }

    QString nodeDecl = QString("%1 [label = \"DOM Modified: %2% %3\"]").arg(name).arg(node->amountModified).arg(wordList);
    mHeaderDomMods.append(nodeDecl);

    addInEdge(name);

    mPreviousNode = name;
    mEdgeExtras = "";

    node->next->accept(this);
}


void TraceDisplay::visit(TracePageLoad *node)
{
    flushAggregation();

    QString name = QString("load_%1").arg(mNodeCounter);
    mNodeCounter++;

    QString nodeDecl = QString("%1 [label = \"Page Load:\\n%2\"]").arg(name).arg(node->url.toString());
    mHeaderLoads.append(nodeDecl);

    addInEdge(name);

    mPreviousNode = name;
    mEdgeExtras = "";
    node->next->accept(this);
}


void TraceDisplay::visit(TraceFunctionCall *node)
{
    // In simplified output mode, if we are currently aggregating, then do not generate this node.
    if(mSimplified && mCurrentlyAggregating){
        mAggregatedFunctionCalls++;
    }else{

        QString name = QString("fun_%1").arg(mNodeCounter);
        mNodeCounter++;

        QString funcName = node->name.isEmpty() ? "(anonymous)" : (node->name + "()");
        QString nodeDecl = QString("%1 [label = \"%2\"]").arg(name).arg(funcName);
        mHeaderFunctions.append(nodeDecl);

        addInEdge(name);

        mPreviousNode = name;
        mEdgeExtras = "";
    }
    node->next->accept(this);
}


void TraceDisplay::visit(TraceEndSuccess *node)
{
    flushAggregation();

    QString name = QString("end_s_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderEndSucc.append(name);

    addInEdge(name);

    if(mPassThroughEndMarkers){
        mPreviousNode = name;
        mEdgeExtras = "";
        node->next->accept(this);
    }
}


void TraceDisplay::visit(TraceEndFailure *node)
{
    flushAggregation();

    QString name = QString("end_f_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderEndFail.append(name);

    addInEdge(name);

    if(mPassThroughEndMarkers){
        mPreviousNode = name;
        mEdgeExtras = "";
        node->next->accept(this);
    }
}


void TraceDisplay::visit(TraceEndUnknown *node)
{
    flushAggregation();

    QString name = QString("end_u_%1").arg(mNodeCounter);
    mNodeCounter++;

    mHeaderEndUnk.append(name);

    addInEdge(name);
}





// Clears the member variables contining the items for the header and edges.
void TraceDisplay::clearData()
{
    mHeaderBranches.clear();
    mHeaderSymBranches.clear();
    mHeaderUnexplored.clear();
    mHeaderUnexploredUnsat.clear();
    mHeaderUnexploredUnsolvable.clear();
    mHeaderUnexploredMissed.clear();
    mHeaderAlerts.clear();
    mHeaderDomMods.clear();
    mHeaderLoads.clear();
    mHeaderFunctions.clear();
    mHeaderEndUnk.clear();
    mHeaderEndSucc.clear();
    mHeaderEndFail.clear();
    mHeaderAggregates.clear();

    mEdges.clear();

    mPreviousNode = "start";
    mEdgeExtras = "";
    mNodeCounter = 0;

    mExpressionPrinter->clear();

    mAggregatedConcreteBranches = 0;
    mAggregatedFunctionCalls = 0;
    mCurrentlyAggregating = false;
}

// Adds a new edge to mEdges.
void TraceDisplay::addInEdge(QString endpoint)
{
    QString edge = QString("%1 -> %2").arg(mPreviousNode).arg(endpoint);
    if(!mEdgeExtras.isEmpty()){
        edge += " " + mEdgeExtras;
    }
    mEdges.append(edge);
}


// If we are currently aggregating, this function should output the aggregate node and return us to "normal" mode
// So the next node can be processed.
// This is called by nodes which should be displayed as usual even in simplified mode.
void TraceDisplay::flushAggregation()
{
    if(mSimplified && mCurrentlyAggregating){
        QString name = QString("aggr_%1").arg(mNodeCounter);
        mNodeCounter++;

        QString nodeDecl = QString("%1 [label = \"\\n Concrete Execution  \\n Branches: %2  \\n Function Calls: %3  \\n \"]").arg(name)
                .arg(mAggregatedConcreteBranches).arg(mAggregatedFunctionCalls);
        mHeaderAggregates.append(nodeDecl);

        addInEdge(name);

        mPreviousNode = name;
        mEdgeExtras = "";

        mCurrentlyAggregating = false;
        mAggregatedConcreteBranches = 0;
        mAggregatedFunctionCalls = 0;
    }
}


} //namespace artemis
